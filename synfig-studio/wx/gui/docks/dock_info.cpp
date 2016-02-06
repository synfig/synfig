/* === S Y N F I G ========================================================= */
/*!	\file dock_info.cpp
**	\brief Dock Info File
**
**	$Id$
**
**	\legal
**	Copyright (c) 2002-2005 Robert B. Quattlebaum Jr., Adrian Bentley
**	Copyright (c) 2007 Chris Moore
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

#include "docks/dock_info.h"
#include "canvasview.h"
#include "workarea.h"

#include <synfig/canvas.h>
#include <synfig/color.h>		// for gamma_in()
#include <synfig/context.h>

#include <gtkmm/separator.h>
#include <gtkmm/invisible.h>

#include "general.h"

#endif

/* === U S I N G =========================================================== */

using namespace std;
using namespace etl;
using namespace synfig;

/* === M A C R O S ========================================================= */

/* === G L O B A L S ======================================================= */

/* === P R O C E D U R E S ================================================= */

/* === M E T H O D S ======================================================= */

/* === E N T R Y P O I N T ================================================= */

void studio::Dock_Info::on_mouse_move()
{
	Point pos = get_canvas_view()->work_area->get_cursor_pos();

	Distance xv(pos[0],Distance::SYSTEM_UNITS);
	xv.convert(App::distance_system, get_canvas_view()->get_canvas()->rend_desc());

	Distance yv(pos[1],Distance::SYSTEM_UNITS);
	yv.convert(App::distance_system, get_canvas_view()->get_canvas()->rend_desc());

	//get the color and set the labels

	x.set_text(xv.get_string(3));
	y.set_text(yv.get_string(3));

	float cr=0.f,cg=0.f,cb=0.f,ca=0.f;
	if (App::workarea_uses_cairo)
	{
		// TODO: This is disabled for now, because it crashes Synfig when *.lst files are imported
		
		//CairoColor c = get_canvas_view()->get_canvas()->get_context( get_canvas_view()->get_context_params() ).get_cairocolor(pos);
		//cr = c.get_r(); cg = c.get_g(); cb = c.get_b(); ca = c.get_a();
	}
	else
	{
		Color c = get_canvas_view()->get_canvas()->get_context( get_canvas_view()->get_context_params() ).get_color(pos);
		cr = c.get_r(); cg = c.get_g(); cb = c.get_b(); ca = c.get_a();
	}

	if(use_colorspace_gamma())
	{
		cr = gamma_in(cr);
		cg = gamma_in(cg);
		cb = gamma_in(cb);
	}

	r.set_text(strprintf("%.1f%%",cr*100));
	g.set_text(strprintf("%.1f%%",cg*100));
	b.set_text(strprintf("%.1f%%",cb*100));
	a.set_text(strprintf("%.1f%%",ca*100));
}

studio::Dock_Info::Dock_Info()
:Dock_CanvasSpecific("info",_("Info"),Gtk::StockID("synfig-info"))
{
	set_use_scrolled(false);

	Gtk::Table *table = manage(new Gtk::Table);

	//pos labels
	table->attach(*manage(new Gtk::Label(_("X: "))),0,1,0,2,Gtk::EXPAND|Gtk::FILL,Gtk::SHRINK|Gtk::FILL);
	table->attach(*manage(new Gtk::Label(_("Y: "))),0,1,2,4,Gtk::EXPAND|Gtk::FILL,Gtk::SHRINK|Gtk::FILL);

	//pos
	table->attach(x,1,2,0,2,Gtk::EXPAND|Gtk::FILL,Gtk::SHRINK|Gtk::FILL);
	table->attach(y,1,2,2,4,Gtk::EXPAND|Gtk::FILL,Gtk::SHRINK|Gtk::FILL);

	//separator
	table->attach(*manage(new Gtk::VSeparator),2,3,0,4,Gtk::EXPAND|Gtk::FILL,Gtk::SHRINK|Gtk::FILL);

	//color label
	table->attach(*manage(new Gtk::Label(_("R: "))),3,4,0,1,Gtk::EXPAND|Gtk::FILL,Gtk::SHRINK|Gtk::FILL);
	table->attach(*manage(new Gtk::Label(_("G: "))),3,4,1,2,Gtk::EXPAND|Gtk::FILL,Gtk::SHRINK|Gtk::FILL);
	table->attach(*manage(new Gtk::Label(_("B: "))),3,4,2,3,Gtk::EXPAND|Gtk::FILL,Gtk::SHRINK|Gtk::FILL);
	table->attach(*manage(new Gtk::Label(_("A: "))),3,4,3,4,Gtk::EXPAND|Gtk::FILL,Gtk::SHRINK|Gtk::FILL);

	//color
	table->attach(r,4,5,0,1,Gtk::EXPAND|Gtk::FILL,Gtk::SHRINK|Gtk::FILL);
	table->attach(g,4,5,1,2,Gtk::EXPAND|Gtk::FILL,Gtk::SHRINK|Gtk::FILL);
	table->attach(b,4,5,2,3,Gtk::EXPAND|Gtk::FILL,Gtk::SHRINK|Gtk::FILL);
	table->attach(a,4,5,3,4,Gtk::EXPAND|Gtk::FILL,Gtk::SHRINK|Gtk::FILL);

	table->attach(*manage(new Gtk::Label),0,5,4,5);

	table->show_all();

	add(*table);
}

studio::Dock_Info::~Dock_Info()
{
}

void studio::Dock_Info::changed_canvas_view_vfunc(etl::loose_handle<CanvasView> canvas_view)
{
	mousecon.disconnect();

	if(canvas_view && canvas_view->get_work_area())
	{
		mousecon = get_canvas_view()->work_area->signal_cursor_moved().connect(sigc::mem_fun(*this,&Dock_Info::on_mouse_move));
	}
}
