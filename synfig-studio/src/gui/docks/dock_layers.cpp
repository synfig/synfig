/* === S Y N F I G ========================================================= */
/*!	\file dock_layers.cpp
**	\brief Template File
**
**	$Id$
**
**	\legal
**	Copyright (c) 2002-2005 Robert B. Quattlebaum Jr., Adrian Bentley
**	Copyright (c) 2007, 2008 Chris Moore
**
**	This package is free software; you can redistribute it and/or
**	modify it under the terms of the GNU General Public License as
**	published by the Free Software Foundation; either version 2 of
**	the License, or (at your option) any later version.
**
**	This package is distributed in the hope that it will be useful,
**	but WITHOUT ANY WARRANTY; without even the implied warranty of
**	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
**	General Public License for more details.
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

#include <synfig/general.h>

#include <glibmm/markup.h>

#include "docks/dock_layers.h"
#include "app.h"

#include "instance.h"
#include <sigc++/sigc++.h>
#include "trees/layertreestore.h"
#include "trees/layertree.h"
#include "canvasview.h"
#include "actionmanagers/layeractionmanager.h"

#include <gui/localization.h>

#endif

/* === U S I N G =========================================================== */

using namespace std;
using namespace etl;
using namespace synfig;
using namespace studio;

/* === M A C R O S ========================================================= */

/* === G L O B A L S ======================================================= */

/* === P R O C E D U R E S ================================================= */

/* === M E T H O D S ======================================================= */

Dock_Layers::Dock_Layers():
	Dock_CanvasSpecific("layers",_("Layers"),Gtk::StockID("synfig-layer")),
	layer_action_manager(new LayerActionManager)
{
	if(layer_action_manager)layer_action_manager->set_ui_manager(App::ui_manager());

	action_group_new_layers=Gtk::ActionGroup::create("action_group_new_layers");
	action_group_layer_ops=Gtk::ActionGroup::create("action_group_layer_ops");

	std::map<synfig::String,synfig::String> category_map;

	// Build layer creation actions
	synfig::Layer::Book::iterator iter;
	for(iter=synfig::Layer::book().begin();iter!=synfig::Layer::book().end();++iter)
	{
		synfig::Layer::Book::value_type lyr(*iter);

		if(lyr.second.category==CATEGORY_DO_NOT_USE)
			continue;
		
		action_group_new_layers->add(Gtk::Action::create(
			strprintf("layer-new-%s",lyr.first.c_str()),
			layer_icon(lyr.first.c_str()),
			lyr.second.local_name,lyr.second.local_name
		),
			sigc::hide_return(
				sigc::bind(
					sigc::mem_fun(*this,&studio::Dock_Layers::add_layer),
					lyr.first
				)
			)
		);

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

	action_new_layer = Gtk::Action::create("popup-layer-new", Gtk::StockID("gtk-add"), _("New Layer"), _("New Layer"));
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

	set_toolbar(*dynamic_cast<Gtk::Toolbar*>(App::ui_manager()->get_widget("/toolbar-layer")));
}


Dock_Layers::~Dock_Layers()
{
	delete layer_action_manager;
}


void
Dock_Layers::init_canvas_view_vfunc(etl::loose_handle<CanvasView> canvas_view)
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
		if (ev->button == 3 && action_new_layer->is_sensitive()) {
			popup_add_layer_menu();
			return true;
		}
		return false;
	});

	// (a) should be before (b), (b) should be before (c)
	canvas_view->set_ext_widget(get_name()+"_cmp",layer_tree); // (a)
	canvas_view->set_ext_widget(get_name(),&layer_tree->get_layer_tree_view());
	canvas_view->set_ext_widget("params",&layer_tree->get_param_tree_view());
	
	canvas_view->set_adjustment_group("params", new AdjustmentGroup());

	layer_tree->set_model(layer_tree_store); // (b)
	canvas_view->set_tree_model("params",layer_tree->get_param_tree_view().get_model()); // (c)

	/*
	canvas_view->layermenu.items().push_back(Gtk::Menu_Helpers::StockMenuElem(Gtk::StockID("gtk-delete"),Gtk::AccelKey("Delete"),
		sigc::mem_fun(*layer_tree, &LayerTree::on_delete_pressed))
	);
	*/

	// Hide the time bar
	//if(canvas_view->get_canvas()->rend_desc().get_time_start()==canvas_view->get_canvas()->rend_desc().get_time_end())
	//	canvas_view->hide_timebar();
	layer_tree_store->rebuild();
	present();
}

void
Dock_Layers::changed_canvas_view_vfunc(etl::loose_handle<CanvasView> canvas_view)
{
	if(canvas_view)
	{
		Gtk::Widget* tree_view(canvas_view->get_ext_widget(get_name()));

		add(*tree_view);
		tree_view->show();
		action_group_new_layers->set_sensitive(true);
		action_new_layer->set_sensitive(true);
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
	etl::loose_handle<CanvasView> canvas_view(get_canvas_view());
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
