/* === S Y N F I G ========================================================= */
/*!	\file dock_canvasspecific.cpp
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

#include "app.h"
#include "docks/dock_canvasspecific.h"

#include "instance.h"
#include "canvasview.h"

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

Dock_CanvasSpecific::Dock_CanvasSpecific(const synfig::String& name,const synfig::String& local_name,Gtk::StockID stock_id_):
	Dockable(name,local_name,stock_id_)
{
	App::signal_instance_created().connect(sigc::mem_fun(*this,&Dock_CanvasSpecific::init_instance));
}

Dock_CanvasSpecific::~Dock_CanvasSpecific()
{
}

etl::loose_handle<studio::CanvasView>
Dock_CanvasSpecific::get_canvas_view()
{
	return App::get_selected_canvas_view();
}

etl::loose_handle<synfigapp::CanvasInterface>
Dock_CanvasSpecific::get_canvas_interface()
{
	if(get_canvas_view())
		return get_canvas_view()->canvas_interface();
	return 0;
}

void
Dock_CanvasSpecific::init_canvas_view_vfunc(etl::loose_handle<CanvasView> /*canvas_view*/)
{
}

void
Dock_CanvasSpecific::init_instance_vfunc(etl::loose_handle<Instance> /*instance*/)
{
}

void
Dock_CanvasSpecific::changed_canvas_view_vfunc(etl::loose_handle<CanvasView> /*canvas_view*/)
{
}

void
Dock_CanvasSpecific::init_instance(etl::handle<Instance> instance)
{
	instance->signal_canvas_view_created().connect(sigc::mem_fun(*this,&Dock_CanvasSpecific::init_canvas_view));
	init_instance_vfunc(instance);
}

void
Dock_CanvasSpecific::delete_instance(etl::handle<Instance> /*instance*/)
{
	changed_canvas_view_vfunc(0);
}

void
Dock_CanvasSpecific::init_canvas_view(CanvasView* canvas_view)
{
	/*
	canvas_view->signal_focus_in_event().connect(
		sigc::hide(
			sigc::bind_return(
				sigc::mem_fun(
					*this,
					&Dock_CanvasSpecific::canvas_view_changed
				),
				false
			)
		)
	);
	*/
	App::signal_canvas_view_focus().connect(
		sigc::hide(
			sigc::mem_fun(
				*this,
				&Dock_CanvasSpecific::canvas_view_changed
			)
		)
	);
	init_canvas_view_vfunc(canvas_view);
}

void
Dock_CanvasSpecific::canvas_view_changed()
{
	etl::loose_handle<CanvasView> canvas_view(App::get_selected_canvas_view());
/*	if(canvas_view)
	{
		canvas_delete_connection.disconnect();
		canvas_delete_connection=canvas_view->signal_deleted().connect(
			sigc::bind(
				sigc::mem_fun(
					*this,
					&Dock_CanvasSpecific::changed_canvas_view
				),
				etl::loose_handle<CanvasView>(0)
			)
		);
	}
*/

	changed_canvas_view_vfunc(canvas_view);
}
