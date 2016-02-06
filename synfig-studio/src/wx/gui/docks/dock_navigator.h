/* === S Y N F I G ========================================================= */
/*!	\file docks/dock_navigator.h
**	\brief Navigator Dock Header
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

/* === S T A R T =========================================================== */

#ifndef __SYNFIG_DOCK_NAVIGATOR_H
#define __SYNFIG_DOCK_NAVIGATOR_H

/* === H E A D E R S ======================================================= */
#include "sigc++/signal.h"

#include <gtkmm/drawingarea.h>
#include <gdkmm/pixbuf.h>
#include <gtkmm/adjustment.h>
#include <gtkmm/label.h>

#include <synfig/renddesc.h>

#include "canvasview.h"
#include "docks/dock_canvasspecific.h"
#include "widgets/widget_distance.h"

#include <ETL/smart_ptr>

/* === M A C R O S ========================================================= */

/* === T Y P E D E F S ===================================================== */

/* === C L A S S E S & S T R U C T S ======================================= */

namespace studio {
class AsyncRenderer;

class Widget_NavView : public Gtk::Table
{
	//handle to out parent canvas
	CanvasView::LooseHandle canvview;

	Glib::RefPtr<Gdk::Pixbuf> prev;
	bool dirty;

	//The drawing stuff
	Gtk::DrawingArea drawto;

	//The input stuff
	Glib::RefPtr<Gtk::Adjustment> adj_zoom;
	Gtk::Label zoom_print;

	//zoom window stuff
	bool	scrolling;

	//asynchronous rendering stuff
	etl::handle<AsyncRenderer>	renderer;
	etl::smart_ptr<synfig::Surface> surface;
	cairo_surface_t* cairo_surface;
	bool rendering;

	//drawing functionality
	void on_start_render(); //breaks out into asynchronous rendering
	void on_finish_render();
	void on_draw(); //renders the small thing we have
	void on_dirty_preview(); //dirties the preview for rerender

	//for the zoom buttons
	void on_zoom_in();
	void on_zoom_out();

	//handles the zoom scroller
	using Gtk::Widget::on_scroll_event;
	bool on_scroll_event(GdkEvent *event);
	void on_number_modify();

	//
	bool on_mouse_event(GdkEvent * e);

	//draws the gotten bitmap on the draw area
	bool on_drawto_draw(const Cairo::RefPtr<Cairo::Context> &cr);

	//for when the canvasview view changes (boolean value scrolling solves cyclic problems)
	void on_workarea_view_change();

public:
	Widget_NavView(CanvasView::LooseHandle cv = CanvasView::LooseHandle());
	~Widget_NavView();

	etl::loose_handle<studio::CanvasView> get_canvas_view() {return canvview;}
};

class Dock_Navigator : public Dock_CanvasSpecific
{
	Widget_NavView	dummy;

public:
	Dock_Navigator();
	~Dock_Navigator();

	virtual void changed_canvas_view_vfunc(etl::loose_handle<CanvasView> canvas_view);
};

}; // END of namespace studio

/* === E N D =============================================================== */

#endif
