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

/* === M E T H O D S ======================================================= */

Dock_Layers::Dock_Layers():
	Dock_CanvasSpecific("layers",_("Layers"),"layer_icon"),
	layer_action_manager(new LayerActionManager)
{
	set_name("layers_panel");

	// Make Layers button small for space efficiency
	get_style_context()->add_class("synfigstudio-efficient-workspace");

	if(layer_action_manager)layer_action_manager->set_ui_manager(App::ui_manager());

	action_group_new_layers=Gtk::ActionGroup::create("action_group_new_layers");
	action_group_layer_ops=Gtk::ActionGroup::create("action_group_layer_ops");

	action_group_new_layers2 = Gio::SimpleActionGroup::create();

	std::map<synfig::String,synfig::String> category_map;
	// map: category local name -> (layer name, layer local name)
	std::map<std::string, std::vector<std::pair<std::string, std::string>>> layer_category_map;

	auto new_layer_slot = sigc::track_obj([=](const Glib::VariantBase& v) {
		std::string layer_name = Glib::VariantBase::cast_dynamic<Glib::Variant<Glib::ustring>>(v).get();
		add_layer(layer_name);
	}, *this);
	action_group_new_layers2->add_action_with_parameter("layer-new", Glib::VARIANT_TYPE_STRING, new_layer_slot);

	// Build layer creation actions
	synfig::Layer::Book::iterator iter;
	for(iter=synfig::Layer::book().begin();iter!=synfig::Layer::book().end();++iter)
	{
		synfig::Layer::Book::value_type lyr(*iter);

		if(lyr.second.category==CATEGORY_DO_NOT_USE)
			continue;
		
		action_group_new_layers->add(Gtk::Action::create_with_icon_name(
			strprintf("layer-new-%s",lyr.first.c_str()),
			layer_icon_name(lyr.first),
			lyr.second.local_name,lyr.second.local_name
		),
			sigc::hide_return(
				sigc::bind(
					sigc::mem_fun(*this,&studio::Dock_Layers::add_layer),
					lyr.first
				)
			)
		);

		layer_category_map[dgettext("synfig", lyr.second.category.c_str())].push_back({lyr.first, lyr.second.local_name});

		category_map[lyr.second.category]+=strprintf("<menuitem action='layer-new-%s' />",lyr.first.c_str());

		//(*category_map)[lyr.second.category]->items().push_back(Gtk::Menu_Helpers::MenuElem(lyr.second.local_name,
		//));
	}

	{
		Glib::RefPtr<Gtk::ActionGroup> action_group_categories(Gtk::ActionGroup::create("layer-category"));
		synfig::String layer_ui_info;

		std::map<synfig::String,synfig::String>::iterator iter;
		for(iter=category_map.begin();iter!=category_map.end();++iter)
		{
			layer_ui_info+=strprintf("<menu action='%s'>%s</menu>",iter->first.c_str(),iter->second.c_str());
			#ifdef ENABLE_NLS
				action_group_categories->add(Gtk::Action::create(iter->first.c_str(), dgettext("synfig", iter->first.c_str())));
			#else
				action_group_categories->add(Gtk::Action::create(iter->first.c_str(), iter->first.c_str()));
			#endif
		}

		App::ui_manager()->insert_action_group(action_group_categories);
		App::ui_manager()->insert_action_group(action_group_new_layers);

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

		try
		{
			synfig::String ui_info;
			ui_info = "<ui><popup action='menu-main'><menu action='menu-layer'><menu action='menu-layer-new'>"
			        + layer_ui_info
			        + "</menu></menu></popup></ui>";
			App::ui_manager()->add_ui_from_string(ui_info);
			ui_info = "<ui><menubar action='menubar-main'><menu action='menu-layer'><menu action='menu-layer-new'>"
			        + layer_ui_info
			        + "</menu></menu></menubar></ui>";
			App::ui_manager()->add_ui_from_string(ui_info);
			ui_info = "<ui><popup action='popup-layer-new'>"
					+ layer_ui_info
			        + "</popup></ui>";
			App::ui_manager()->add_ui_from_string(ui_info);
		}
		catch(Glib::MarkupError& x)
		{
			error("%s:%d caught MarkupError code %d: %s", __FILE__, __LINE__, x.code(), x.what().c_str());
			error("%s:%d with markup: \"%s\"", __FILE__, __LINE__, layer_ui_info.c_str());
			exit(1);
		}
	}

	if(layer_action_manager)
		action_group_layer_ops->add(layer_action_manager->get_action_select_all_child_layers());

	action_group_layer_ops->add( Gtk::Action::create("toolbar-layer", _("Layer Ops")) );

	action_new_layer = Gtk::Action::create_with_icon_name("popup-layer-new", "list-add", _("New Layer"), _("New Layer"));
	action_new_layer->signal_activate().connect(sigc::mem_fun(*this, &Dock_Layers::popup_add_layer_menu));

	action_group_layer_ops->add( action_new_layer );
	App::ui_manager()->insert_action_group(action_group_layer_ops);

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

	App::ui_manager()->add_ui_from_string(ui_info);

	action_group_new_layers->set_sensitive(false);

	if (Gtk::Toolbar* toolbar = dynamic_cast<Gtk::Toolbar*>(App::ui_manager()->get_widget("/toolbar-layer"))) {
		set_toolbar(*toolbar);
	}
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
	layer_tree->signal_no_layer_user_click().connect([=](GdkEventButton *ev){
		SYNFIG_EXCEPTION_GUARD_BEGIN()
		if (ev->button == 3 && action_new_layer->is_sensitive()) {
			popup_add_layer_menu();
			return true;
		}
		return false;
		SYNFIG_EXCEPTION_GUARD_END_BOOL(true)
	});

	// (a) should be before (b), (b) should be before (c)
	canvas_view->set_ext_widget(get_name()+"_cmp", layer_tree); // (a)
	canvas_view->set_ext_widget(get_name(), &layer_tree->layer_tree_view(), false);
	canvas_view->set_ext_widget("params", &layer_tree->param_tree_view(), false);
	
	canvas_view->set_adjustment_group("params", new AdjustmentGroup());

	layer_tree->set_model(layer_tree_store); // (b)
	canvas_view->set_tree_model("params", layer_tree->param_tree_view().get_model()); // (c)

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
		action_group_new_layers->set_sensitive(true);
		action_new_layer->set_sensitive(true);
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
		action_group_new_layers->set_sensitive(false);
		action_new_layer->set_sensitive(false);
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
	if (!action_new_layer->is_sensitive())
		return;
	Gtk::Menu* menu = dynamic_cast<Gtk::Menu*>(App::ui_manager()->get_widget("/popup-layer-new"));
	if (menu)
		menu->popup(0, gtk_get_current_event_time());
}
