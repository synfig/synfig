/* === S Y N F I G ========================================================= */
/*!	\file state_null.cpp
**	\brief Null State File
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

#include "state_null.h"
#include "workarea.h"
#include "event_mouse.h"
#include "event_layerclick.h"
#include "toolbox.h"
#include "dialog_tooloptions.h"
#include <gtkmm/dialog.h>
#include "widget_waypointmodel.h"
#include <synfig/valuenode_animated.h>
#include <synfig/valuenode_composite.h>
#include <synfig/valuenode_const.h>
#include "canvasview.h"
#include "general.h"

#endif

/* === U S I N G =========================================================== */

using namespace std;
using namespace etl;
using namespace synfig;
using namespace studio;

/* === M A C R O S ========================================================= */

/* === G L O B A L S ======================================================= */

StateNull studio::state_null;

/* === C L A S S E S & S T R U C T S ======================================= */

class studio::StateNull_Context : public sigc::trackable
{
	CanvasView *canvas_view;

	CanvasView* get_canvas_view() { return canvas_view; }
	Canvas::Handle get_canvas() { return canvas_view->get_canvas(); }
	WorkArea* get_work_area() { return canvas_view->get_work_area(); }
	etl::handle<synfigapp::CanvasInterface> get_canvas_interface() { return canvas_view->canvas_interface(); }

	Gtk::Table options_table;

public:
	StateNull_Context(CanvasView *canvas_view);
	~StateNull_Context();

	Smach::event_result event_refresh_tool_options(const Smach::event& x);

	void refresh_tool_options();
}; // END of class StateNull_Context

/* === P R O C E D U R E S ================================================= */

/* === M E T H O D S ======================================================= */

StateNull::StateNull():
	Smach::state<StateNull_Context>("null")
{
	insert(event_def(EVENT_REFRESH_TOOL_OPTIONS,&StateNull_Context::event_refresh_tool_options));
}

StateNull::~StateNull()
{
}

StateNull_Context::StateNull_Context(CanvasView *canvas_view):
	canvas_view(canvas_view)
{
	// Synfig Studio's default state is initialized in the canvas view constructor
	// As a result, it cannot reference canvas view or workarea when created
	// Other states need to reference the workarea,
	//    so a null state was created to be the default

	options_table.attach(*manage(new Gtk::Label(_("Welcome to Synfig Studio"))),	0, 2,  0,  1, Gtk::EXPAND|Gtk::FILL, Gtk::EXPAND|Gtk::FILL, 0, 0);
	options_table.show_all();
	refresh_tool_options();
}

StateNull_Context::~StateNull_Context()
{
}

void
StateNull_Context::refresh_tool_options()
{
	App::dialog_tool_options->clear();
	App::dialog_tool_options->set_widget(options_table);
	App::dialog_tool_options->set_local_name(_("Welcome to Synfig Studio"));
	App::dialog_tool_options->set_name("null");
}

Smach::event_result
StateNull_Context::event_refresh_tool_options(const Smach::event& /*x*/)
{
	refresh_tool_options();
	return Smach::RESULT_ACCEPT;
}
