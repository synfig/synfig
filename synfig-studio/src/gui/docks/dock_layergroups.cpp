/* === S Y N F I G ========================================================= */
/*!	\file dock_layergroups.cpp
**	\brief Template File
**
**	$Id$
**
**	\legal
**	Copyright (c) 2002-2005 Robert B. Quattlebaum Jr., Adrian Bentley
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

#include "docks/dock_layergroups.h"
#include "app.h"

#include <gtkmm/scrolledwindow.h>
#include <cassert>
#include "instance.h"
#include "canvasview.h"

#include "trees/layergrouptreestore.h"
#include "trees/layergrouptree.h"
#include "actionmanagers/groupactionmanager.h"

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

Dock_LayerGroups::Dock_LayerGroups():
	Dock_CanvasSpecific("groups",_("Sets"),Gtk::StockID("synfig-group")),
	action_group_group_ops(Gtk::ActionGroup::create("action_group_dock_layergroups")),
	group_action_manager(new GroupActionManager)
{
	group_action_manager->set_ui_manager(App::ui_manager());

	action_group_group_ops->add( Gtk::Action::create("toolbar-groups", _("Set Ops")) );

	action_group_add=Gtk::Action::create("action-group_add", Gtk::Stock::ADD,_("Add a New Set"),_("Add a New Set"));
	action_group_group_ops->add(action_group_add);
	action_group_add->set_sensitive(false);

	App::ui_manager()->insert_action_group(action_group_group_ops);

    Glib::ustring ui_info =
	"<ui>"
	"	<toolbar action='toolbar-groups'>"
	"	<toolitem action='action-GroupRemove' />"
	"	<toolitem action='action-group_add' />"
	"	</toolbar>"
	"</ui>"
	;

	App::ui_manager()->add_ui_from_string(ui_info);

	set_toolbar(*dynamic_cast<Gtk::Toolbar*>(App::ui_manager()->get_widget("/toolbar-groups")));
}

Dock_LayerGroups::~Dock_LayerGroups()
{
	delete group_action_manager;
}

void
Dock_LayerGroups::init_canvas_view_vfunc(etl::loose_handle<CanvasView> canvas_view)
{
	Glib::RefPtr<LayerGroupTreeStore> layer_group_tree_store;
	layer_group_tree_store=LayerGroupTreeStore::create(canvas_view->canvas_interface());

	LayerGroupTree* layer_group_tree(new LayerGroupTree());
	layer_group_tree->set_model(layer_group_tree_store);
	layer_group_tree->signal_popup_layer_menu().connect(sigc::mem_fun(*canvas_view,&CanvasView::popup_layer_menu));

	canvas_view->set_tree_model(get_name(),layer_group_tree_store);
	canvas_view->set_ext_widget(get_name(),layer_group_tree);
}

void
Dock_LayerGroups::changed_canvas_view_vfunc(etl::loose_handle<CanvasView> canvas_view)
{
	if(canvas_view)
	{
		Gtk::Widget* tree_view(canvas_view->get_ext_widget(get_name()));

		add(*tree_view);
		tree_view->show();

		group_action_manager->set_group_tree(dynamic_cast<LayerGroupTree*>(tree_view));
		group_action_manager->set_canvas_interface(canvas_view->canvas_interface());
		group_action_manager->refresh();
	}
	else
	{
		group_action_manager->clear();
		group_action_manager->set_canvas_interface(0);
		group_action_manager->set_group_tree(0);
	}
}
