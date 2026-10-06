/* === S Y N F I G ========================================================= */
/*!	\file dock_params.cpp
**	\brief Template File
**
**	\legal
**	Copyright (c) 2002-2005 Robert B. Quattlebaum Jr., Adrian Bentley
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

#include "docks/dock_params.h"

#include <cassert>

#include <gui/canvasview.h>
#include <gui/localization.h>
#include <gui/trees/layerparamtreestore.h>
#include <gui/workarea.h>

#endif

/* === U S I N G =========================================================== */

using namespace synfig;
using namespace studio;

/* === M A C R O S ========================================================= */

/* === G L O B A L S ======================================================= */

/* === P R O C E D U R E S ================================================= */

/* === M E T H O D S ======================================================= */

Dock_Params::Dock_Params():
	Dock_CanvasSpecific("params", _("Parameters"), "parameters_icon"),
	vadjustment( Gtk::Adjustment::create(0, 0, 1, 1, 1) )
{
	set_name("parameters_panel");
}

Dock_Params::~Dock_Params()
{
	refresh_selected_param_connection.disconnect();
}


void
Dock_Params::init_canvas_view_vfunc(CanvasView::LooseHandle canvas_view)
{
	canvas_view->get_adjustment_group(get_name())->add(vadjustment);
}

void
Dock_Params::refresh_selected_param()
{
	CanvasView::LooseHandle canvas_view(get_canvas_view());
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
Dock_Params::changed_canvas_view_vfunc(CanvasView::LooseHandle canvas_view)
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

		// Connect to the canvas's signal_changed() to detect state changes
		canvas_changed_connection = canvas_view->get_canvas()->signal_changed().connect(
			sigc::mem_fun(
				*this, &Dock_Params::refresh_tree_view));

		tree_view->show();

		add(*tree_view);
		get_container()->set_vadjustment(vadjustment);
	}
}

void
Dock_Params::refresh_tree_view()
{
	Gtk::TreeView* tree_view = dynamic_cast<Gtk::TreeView*>(get_canvas_view()->get_ext_widget(get_name()));
	if (tree_view)
	{
		tree_view->queue_draw();
	}
}
