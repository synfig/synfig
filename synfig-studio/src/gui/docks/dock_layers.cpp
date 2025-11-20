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
#include <glibmm/markup.h>
#include <gtkmm/stylecontext.h>

#include <gui/actionmanagers/layeractionmanager.h>
#include <gui/app.h>
#include <gui/canvasview.h>
#include <gui/exception_guard.h>
#include <gui/localization.h>
#include <gui/trees/layertreestore.h>
#include <gui/trees/layertree.h>

#endif

/* === U S I N G =========================================================== */

using namespace synfig;
using namespace studio;

/* === M A C R O S ========================================================= */

/* === G L O B A L S ======================================================= */

/* === P R O C E D U R E S ================================================= */

static Gtk::ToolButton*
create_action_toolbutton(const std::string& action_name, const std::string& icon_name, const std::string& tooltip)
{
	Gtk::ToolButton* button = Gtk::manage(new Gtk::ToolButton());
	gtk_actionable_set_action_name(GTK_ACTIONABLE(button->gobj()), action_name.c_str());
	button->set_icon_name(icon_name);
	button->set_tooltip_text(tooltip);
	button->show();
	return button;
}

static Gtk::ToolButton*
create_synfigapp_action_toolbutton(const std::string& action_name)
{
	auto action_it = synfigapp::Action::book().find(action_name);
	if (action_it == synfigapp::Action::book().end()) {
		synfig::error(_("Internal error: can't find synfigapp action to create its button: '%s'"), action_name.c_str());
		return nullptr; // FIXME: SHOULD RETURN NULL OR an empty ToolButton?
	}
	return create_action_toolbutton("layer.action-" + action_name, get_action_icon_name(action_it->second), action_it->second.local_name);
}

static Gtk::SeparatorToolItem*
create_separator_toolitem()
{
	Gtk::SeparatorToolItem* separator = Gtk::manage(new Gtk::SeparatorToolItem());
	separator->show();
	return separator;
}

/* === M E T H O D S ======================================================= */

Dock_Layers::Dock_Layers():
	Dock_CanvasSpecific("layers",_("Layers"),"layer_icon"),
	layer_action_manager(new LayerActionManager)
{
	set_name("layers_panel");

	// Make Layers button small for space efficiency
	get_style_context()->add_class("synfigstudio-efficient-workspace");

	if (layer_action_manager)
		layer_action_manager->set_action_widget_and_menu(App::main_window, App::menu_selected_layers, App::menu_special_layers);

	action_group_new_layers2 = Gio::SimpleActionGroup::create();

	// map: category local name -> (layer name, layer local name)
	std::map<std::string, std::vector<std::pair<std::string, std::string>>> layer_category_map;

	auto new_layer_slot = sigc::track_obj([=](const Glib::VariantBase& v) {
		std::string layer_name = Glib::VariantBase::cast_dynamic<Glib::Variant<Glib::ustring>>(v).get();
		add_layer(layer_name);
	}, *this);
	action_group_new_layers2->add_action_with_parameter("layer-new", Glib::VARIANT_TYPE_STRING, new_layer_slot);

	// Build layer creation actions
	for (const auto& lyr : synfig::Layer::book()) {
		if(lyr.second.category==CATEGORY_DO_NOT_USE)
			continue;
		
		layer_category_map[dgettext("synfig", lyr.second.category.c_str())].push_back({lyr.first, lyr.second.local_name});
	}

	{
		const std::string symbolic_suffix = ""; // App::use-symbolic-icons ? "-symbolic" : "";

		for (const auto& item : layer_category_map) {
			const std::string& category = item.first;
			auto submenu = Gio::Menu::create();
			for (const auto& lyr : item.second) {
				const std::string& layer_name = lyr.first;
				const std::string& layer_local_name = lyr.second;

				auto item = Gio::MenuItem::create(layer_local_name, strprintf("new_layers.layer-new(\"%s\")", layer_name.c_str()));
				item->set_icon(Gio::ThemedIcon::create(layer_icon_name(layer_name) + symbolic_suffix));
				submenu->append_item(item);
			}
			App::menu_layers->append_submenu(category, submenu);
		}
	}




	action_popup_new_layer = Gio::SimpleAction::create("popup-layer-new");
	action_popup_new_layer->signal_activate().connect(sigc::hide(sigc::mem_fun(*this, &Dock_Layers::popup_add_layer_menu)));

	Glib::ustring ui_info =
	"<ui>"
	"	<toolbar action='toolbar-layer'>"
	"	<toolitem action='popup-layer-new' />"
	"	<separator />"
	"	<toolitem action='action-LayerRaise' />"
	"	<toolitem action='action-LayerLower' />"
	"	<separator />"
	"	<toolitem action='action-LayerDuplicate' />"
	"	<toolitem action='action-LayerEncapsulate' />"
	"	<toolitem action='select-all-child-layers' />"
	"	<toolitem action='action-LayerRemove' />"
	"	<separator />"
	"	<toolitem action='cut' />"
	"	<toolitem action='copy' />"
	"	<toolitem action='paste' />"
	"	</toolbar>"
	"</ui>"
	;



	auto toolbar = Gtk::manage(new Gtk::Toolbar());
	toolbar->show_all();
	toolbar->append(*create_action_toolbutton("doc.popup-layer-new", "list-add", _("New Layer")));

	toolbar->append(*create_separator_toolitem());

	toolbar->append(*create_synfigapp_action_toolbutton("LayerRaise"));
	toolbar->append(*create_synfigapp_action_toolbutton("LayerLower"));

	toolbar->append(*create_separator_toolitem());

	toolbar->append(*create_synfigapp_action_toolbutton("LayerDuplicate"));
	toolbar->append(*create_synfigapp_action_toolbutton("LayerEncapsulate"));
	toolbar->append(*create_action_toolbutton("layer.select-all-child-layers", "select_all_child_layers_icon", _("Select All Child Layers")));
	toolbar->append(*create_synfigapp_action_toolbutton("LayerRemove"));


	toolbar->append(*create_separator_toolitem());

	toolbar->append(*create_action_toolbutton("layer.cut", "edit-cut", _("Cut")));
	toolbar->append(*create_action_toolbutton("layer.copy", "edit-copy", _("Copy")));
	toolbar->append(*create_action_toolbutton("layer.paste", "edit-paste", _("Paste")));

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

	//! layer_tree is registered thru CanvasView::set_ext_widget
	//! and will be deleted during CanvasView::~CanvasView()
	//! \see CanvasView::set_ext_widget
	//! \see CanvasView::~CanvasView
	LayerTree* layer_tree(new LayerTree());
	layer_tree->set_time_model(canvas_view->time_model());

	layer_tree->signal_edited_value().connect(
		sigc::hide_return(
			sigc::bind(sigc::mem_fun(*canvas_view->canvas_interface(), &synfigapp::CanvasInterface::change_value), false)
		)
	);
	layer_tree->signal_no_layer_user_click().connect(sigc::mem_fun(*this, &Dock_Layers::on_layertree_no_layer_clicked));

	// (a) should be before (b), (b) should be before (c)
	canvas_view->set_ext_widget(get_name()+"_cmp", layer_tree); // (a)
	canvas_view->set_ext_widget(get_name(), &layer_tree->layer_tree_view(), false);
	canvas_view->set_ext_widget("params", &layer_tree->param_tree_view(), false);
	
	canvas_view->set_adjustment_group("params", new AdjustmentGroup());

	layer_tree->set_model(layer_tree_store); // (b)
	canvas_view->set_tree_model("params", layer_tree->param_tree_view().get_model()); // (c)

	if (auto doc_action_group = Glib::RefPtr<Gio::SimpleActionGroup>::cast_dynamic(canvas_view->get_action_group("doc"))) {
		doc_action_group->add_action(action_popup_new_layer);
	}
	// Hide the time bar
	//if(canvas_view->get_canvas()->rend_desc().get_time_start()==canvas_view->get_canvas()->rend_desc().get_time_end())
	//	canvas_view->hide_timebar();
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
		App::main_window->insert_action_group("new_layers", action_group_new_layers2);
		if(layer_action_manager)
		{
			layer_action_manager->set_layer_tree(dynamic_cast<LayerTree*>(canvas_view->get_ext_widget(get_name()+"_cmp")));
			layer_action_manager->set_canvas_interface(canvas_view->canvas_interface());
			layer_action_manager->refresh();
		}
	}
	else
	{
		App::main_window->remove_action_group("new_layers");
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
	Gtk::Menu* menu = Gtk::manage(new Gtk::Menu(App::menu_layers));
	if (!menu) {
		synfig::error(_("Internal error: couldn't instantiate menu Add Layer to pop it up."));
		return;
	}
	menu->attach_to_widget(*this);
	menu->popup(0, gtk_get_current_event_time());
}

bool
studio::Dock_Layers::on_layertree_no_layer_clicked(GdkEventButton* ev)
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
