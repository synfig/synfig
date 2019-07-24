/* === S Y N F I G ========================================================= */
/*!	\file dock_params.cpp
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

#include "docks/dock_params.h"
#include "app.h"

#include <gtkmm/scrolledwindow.h>
#include <cassert>
#include "instance.h"
#include "canvasview.h"
#include "trees/layerparamtreestore.h"
#include "workarea.h"

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

Dock_Params::Dock_Params():
	Dock_CanvasSpecific( "params", _("Parameters"),Gtk::StockID("synfig-params") ),
	action_group( Gtk::ActionGroup::create("action_group_dock_params") ),
	vadjustment( Gtk::Adjustment::create(0, 0, 1, 1, 1) )
{
}

Dock_Params::~Dock_Params()
{
	refresh_selected_param_connection.disconnect();
}


void
Dock_Params::init_canvas_view_vfunc(etl::loose_handle<CanvasView> canvas_view)
{
	canvas_view->get_adjustment_group(get_name())->add(vadjustment);
}

void
Dock_Params::refresh_selected_param()
{
	etl::loose_handle<CanvasView> canvas_view(get_canvas_view());
	if (!canvas_view) return;

	Gtk::TreeView* tree_view = dynamic_cast<Gtk::TreeView*>(canvas_view->get_ext_widget(get_name()));
	assert(tree_view);

	if (Gtk::TreeModel::iterator iter = tree_view->get_selection()->get_selected()) {
		LayerParamTreeStore::Model model;
		canvas_view->get_work_area()->set_selected_value_node(
			(synfig::ValueNode::Handle)(*iter)[model.value_node] );
	} else {
		canvas_view->get_work_area()->set_selected_value_node(0);
	}
}

void
Dock_Params::changed_canvas_view_vfunc(etl::loose_handle<CanvasView> canvas_view)
{
	reset_container();
	refresh_selected_param_connection.disconnect();
	
	if(canvas_view)
	{
		Gtk::TreeView* tree_view = dynamic_cast<Gtk::TreeView*>(canvas_view->get_ext_widget(get_name()));
		assert(tree_view);

		refresh_selected_param_connection = tree_view->get_selection()->signal_changed().connect(
			sigc::mem_fun(
				*this,
				&Dock_Params::refresh_selected_param ));
		tree_view->show();

		add(*tree_view);
		get_container()->set_vadjustment(vadjustment);
	}
}
