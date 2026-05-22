/* === S Y N F I G ========================================================= */
/*!	\file dock_layers.cpp
**	\brief Template File
**
**	\legal
**	Copyright (c) 2002-2005 Robert B. Quattlebaum Jr., Adrian Bentley
**	Copyright (c) 2007, 2008 Chris Moore
**
**	This file is part of Synfig.
**
**	Synfig is free software: you can redistribute it and/or modify
**	it under the terms of the GNU General Public License as published by
**	the Free Software Foundation, either version 2 of the License, or
**	(at your option) any later version.
**
**	Synfig is distributed in the hope that it will be useful,
**	but WITHOUT ANY WARRANTY; without even the implied warranty of
**	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
**	GNU General Public License for more details.
**
**	You should have received a copy of the GNU General Public License
**	along with Synfig.  If not, see <https://www.gnu.org/licenses/>.
**	\endlegal
*/
/* ========================================================================= */

/* === H E A D E R S ======================================================= */

#ifdef USING_PCH
#	include "pch.h"
#else
#ifdef HAVE_CONFIG_H
#	include <config.h>
#endif

#include "docks/dock_layers.h"

#include <giomm/themedicon.h>
#include <gtkmm/stylecontext.h>

#include <gui/actiondatabase.h>
#include <gui/actionmanagers/layeractionmanager.h>
#include <gui/actionwidgethelper.h>
#include <gui/app.h>
#include <gui/canvasview.h>
#include <gui/exception_guard.h>
#include <gui/localization.h>
#include <gui/trees/layertreestore.h>

#endif

/* === U S I N G =========================================================== */

using namespace synfig;
using namespace studio;

/* === M A C R O S ========================================================= */

/* === G L O B A L S ======================================================= */

/* === P R O C E D U R E S ================================================= */

static Gtk::SeparatorToolItem*
create_separator_toolitem()
{
	Gtk::SeparatorToolItem* separator = Gtk::manage(new Gtk::SeparatorToolItem());
	separator->show();
	return separator;
}

/* === M E T H O D S ======================================================= */

Dock_Layers::Dock_Layers()
	: Dock_CanvasSpecific("layers", _("Layers"), "layer_icon"),
	  layer_action_manager(new LayerActionManager())
{
	// Set widget name for style purposes
	set_name("layers_panel");

	// Make Layers button small for space efficiency
	get_style_context()->add_class("synfigstudio-efficient-workspace");

	if (layer_action_manager)
		layer_action_manager->set_action_widget(App::main_window);

	layer_action_manager->signal_add_layer_selected().connect([=](const std::string& layer_id) {add_layer(layer_id); return true;});

	action_popup_new_layer = Gio::SimpleAction::create("popup-layer-new");
	action_popup_new_layer->signal_activate().connect(sigc::hide(sigc::mem_fun(*this, &Dock_Layers::popup_add_layer_menu)));

	if (App::get_action_database()) {
		// Register all synfigapp actions of Layer category adding 'layer' prefix: 'layer.action-SYNFIGAPP_ACTION_NAME'
		const std::map<std::string, std::string> synfigapp_layer_default_accels = {
			{"LayerRaise", "<Shift>Page_Up"},
			{"LayerLower", "<Shift>Page_Down"},
			{"LayerDuplicate", "<Primary>u"},
			{"LayerEncapsulate", "<Primary>g"},
			{"LayerRemove", "Delete"},
		};
		for (const auto& item : synfigapp::Action::book()) {
			const auto& entry = item.second;
			if (entry.category & synfigapp::Action::CATEGORY_LAYER && !(entry.category & synfigapp::Action::CATEGORY_HIDDEN)) {
				const std::string full_action_name = synfig::strprintf("%s.action-%s", "layer", entry.name.c_str());
				const auto it = synfigapp_layer_default_accels.find(entry.name);
				std::string default_accel = it != synfigapp_layer_default_accels.cend() ? it->second : "";
				App::get_action_database()->add({full_action_name, entry.local_name, default_accel, studio::get_action_icon_name(entry)});
			}
		}

		// Register custom actions of this dock with 'layer' prefix: 'layer.CUSTOM_ACTION_NAME'
		struct ActionMetadata {
			std::string name;
			std::string icon;
			std::string accel;
			std::string label;
			std::string tooltip;
			// std::function<void()> slot;
		};
		const std::vector<ActionMetadata> action_list = {
			{"layer.select-all-child-layers", "select_all_child_layers_icon", {}, _("Select All Child Layers"), _("Select all child layers of the selected layer group")},
			{"layer.cut", "edit-cut", "<Primary>x", _("Cut"), _("Cut layer(s) to clipboard")},
			{"layer.copy", "edit-copy", "<Primary>c", _("Copy"), _("Copy layer(s) to clipboard")},
			{"layer.paste", "edit-paste", "<Primary>v", _("Paste"), _("Paste layer(s) from clipboard")},
			{"doc.popup-layer-new", "list-add", {}, _("New Layer"), _("Pops up a menu to select a new layer")},
		};
		for (const auto& entry : action_list)
			App::get_action_database()->add({entry.name, entry.label, entry.accel, entry.icon, entry.tooltip});

		// Register layer creation actions
		for (const auto& lyr : synfig::Layer::book()) {
			if(lyr.second.category==CATEGORY_DO_NOT_USE)
				continue;

			const std::string& layer_name = lyr.first;
			const std::string label_create_layer = strprintf(_("Create layer \"%s\""), lyr.second.local_name.c_str());
			const std::string layer_action_name = strprintf("layer.new(\"%s\")", layer_name.c_str());
			App::get_action_database()->add({layer_action_name, label_create_layer, {}, studio::layer_icon_name(layer_name)});
		}
	}

	auto toolbar = Gtk::manage(new Gtk::Toolbar());
	toolbar->show_all();
	toolbar->append(*ActionWidgetHelper::create_action_toolbutton("doc.popup-layer-new"));

	toolbar->append(*create_separator_toolitem());

	toolbar->append(*ActionWidgetHelper::create_synfigapp_action_toolbutton("layer", "LayerRaise"));
	toolbar->append(*ActionWidgetHelper::create_synfigapp_action_toolbutton("layer", "LayerLower"));

	toolbar->append(*create_separator_toolitem());

	toolbar->append(*ActionWidgetHelper::create_synfigapp_action_toolbutton("layer", "LayerDuplicate"));
	toolbar->append(*ActionWidgetHelper::create_synfigapp_action_toolbutton("layer", "LayerEncapsulate"));
	toolbar->append(*ActionWidgetHelper::create_action_toolbutton("layer.select-all-child-layers"));
	toolbar->append(*ActionWidgetHelper::create_synfigapp_action_toolbutton("layer", "LayerRemove"));

	toolbar->append(*create_separator_toolitem());

	toolbar->append(*ActionWidgetHelper::create_action_toolbutton("layer.cut"));
	toolbar->append(*ActionWidgetHelper::create_action_toolbutton("layer.copy"));
	toolbar->append(*ActionWidgetHelper::create_action_toolbutton("layer.paste"));

	set_toolbar(*toolbar);
}


Dock_Layers::~Dock_Layers()
{
	delete layer_action_manager;
}


void
Dock_Layers::init_canvas_view_vfunc(CanvasView::LooseHandle canvas_view)
{
	Glib::RefPtr<LayerTreeStore> layer_tree_store;
	layer_tree_store=LayerTreeStore::create(canvas_view->canvas_interface());

	canvas_view->set_tree_model(get_name(),layer_tree_store);

	if (auto doc_action_group = Glib::RefPtr<Gio::SimpleActionGroup>::cast_dynamic(canvas_view->get_action_group("doc"))) {
		doc_action_group->add_action(action_popup_new_layer);
	}

	//! layer_tree is registered thru CanvasView::set_ext_widget
	//! and will be deleted during CanvasView::~CanvasView()
	//! \see CanvasView::set_ext_widget
	//! \see CanvasView::~CanvasView
	LayerTree* layer_tree(new LayerTree());
	layer_tree->set_time_model(canvas_view->time_model());

	layer_tree->signal_layer_user_click().connect(sigc::mem_fun(*this, &Dock_Layers::on_layertree_layer_clicked));
	layer_tree->signal_no_layer_user_click().connect(sigc::mem_fun(*this, &Dock_Layers::on_layertree_no_layer_clicked));

	// (a) should be before (b), (b) should be before (c)
	canvas_view->set_ext_widget(get_name()+"_cmp", layer_tree); // (a)
	canvas_view->set_ext_widget(get_name(), &layer_tree->layer_tree_view(), false);
	canvas_view->set_ext_widget("params", &layer_tree->param_tree_view(), false);
	
	canvas_view->set_adjustment_group("params", new AdjustmentGroup());

	layer_tree->set_model(layer_tree_store); // (b)
	canvas_view->set_tree_model("params", layer_tree->param_tree_view().get_model()); // (c)

	layer_tree_store->rebuild();
	present();
}

void
Dock_Layers::changed_canvas_view_vfunc(CanvasView::LooseHandle canvas_view)
{
	if(canvas_view)
	{
		Gtk::Widget* tree_view(canvas_view->get_ext_widget(get_name()));

		add(*tree_view);
		tree_view->show();
		if(layer_action_manager)
		{
			layer_action_manager->set_layer_tree(dynamic_cast<LayerTree*>(canvas_view->get_ext_widget(get_name()+"_cmp")));
			layer_action_manager->set_canvas_interface(canvas_view->canvas_interface());
			layer_action_manager->refresh();
		}
	}
	else
	{
		if(layer_action_manager)
		{
			layer_action_manager->clear();
			layer_action_manager->set_canvas_interface(0);
			layer_action_manager->set_layer_tree(0);
		}
	}
}

void
Dock_Layers::add_layer(synfig::String id)
{
	CanvasView::LooseHandle canvas_view(get_canvas_view());
	if(canvas_view)
	{
		canvas_view->add_layer(id);
	}
}

void Dock_Layers::popup_add_layer_menu()
{
	Gtk::Menu* menu = Gtk::manage(new Gtk::Menu(layer_action_manager->create_add_layer_menu()));
	if (!menu) {
		synfig::error(_("Internal error: couldn't instantiate menu Add Layer to pop it up."));
		return;
	}
	menu->attach_to_widget(*this);
	menu->signal_hide().connect(sigc::bind(sigc::ptr_fun(&delete_widget), menu));
	menu->popup_at_pointer(nullptr);
}

bool
Dock_Layers::on_layertree_layer_clicked(int button, Gtk::TreeRow row, LayerTree::ColumnID /*column_id*/)
{
	if (button == 3) {
		LayerTree* layer_tree = dynamic_cast<LayerTree*>(get_canvas_view()->get_ext_widget(get_name() + "_cmp"));
		if (layer_tree && layer_action_manager) {
			auto selected_layers = layer_tree->get_selected_layers();
			if (selected_layers.empty()) {
				return true;
			}
			auto context_menu_model = layer_action_manager->create_context_menu(selected_layers);
			auto add_layer_menu_model = layer_action_manager->create_add_layer_menu();
			if (context_menu_model && add_layer_menu_model) {
				context_menu_model->prepend_submenu(_("New Layer"), add_layer_menu_model);
				Gtk::Menu* menu = Gtk::manage(new Gtk::Menu(context_menu_model));
				if (menu) {
					menu->attach_to_widget(layer_tree->layer_tree_view());
					menu->signal_hide().connect(sigc::bind(sigc::ptr_fun(&delete_widget), menu));
#if GTK_CHECK_VERSION(3, 22, 0)
					menu->popup_at_pointer(nullptr);
#else
					menu->popup(button, gtk_get_current_event_time());
#endif
				}
			}
		}
		return true;
	}
	return false;
}

bool
Dock_Layers::on_layertree_no_layer_clicked(GdkEventButton* ev)
{
	SYNFIG_EXCEPTION_GUARD_BEGIN()
	if (ev->button == 3) {
		// No need to check if there is an open document, because LayerTree only exists for a document/CanvasView
		popup_add_layer_menu();
		return true;
	}
	return false;
	SYNFIG_EXCEPTION_GUARD_END_BOOL(true)
}
