/* === S I N F G =========================================================== */
/*!	\file dock_navigator.h
**	\brief Navigator Dock Header
**
**	$Id: dock_navigator.h,v 1.3 2005/01/12 00:31:11 darco Exp $
**
**	\legal
**	Copyright (c) 2002 Robert B. Quattlebaum Jr.
**
**	This software and associated documentation
**	are CONFIDENTIAL and PROPRIETARY property of
**	the above-mentioned copyright holder.
**
**	You may not copy, print, publish, or in any
**	other way distribute this software without
**	a prior written agreement with
**	the copyright holder.
**	\endlegal
*/
/* ========================================================================= */

/* === S T A R T =========================================================== */

#ifndef __SINFG_DOCK_NAVIGATOR_H
#define __SINFG_DOCK_NAVIGATOR_H

/* === H E A D E R S ======================================================= */
#include "sigc++/signal.h"

#include <gtkmm/drawingarea.h>
#include <gdkmm/pixbuf.h>
#include <gtkmm/adjustment.h>
#include <gtkmm/label.h>

#include <sinfg/renddesc.h>

#include "canvasview.h"
#include "dock_canvasspecific.h"
#include "widget_distance.h"

#include <ETL/smart_ptr>

/* === M A C R O S ========================================================= */

/* === T Y P E D E F S ===================================================== */

/* === C L A S S E S & S T R U C T S ======================================= */

namespace studio {
class AsyncRenderer;
	
class Widget_NavView : public Gtk::Table
{
	//handle to out parent canvas
	CanvasView::LooseHandle		canvview;
	
	Glib::RefPtr<Gdk::Pixbuf>	prev;
	bool dirty;
	
	//The drawing stuff	
	Gtk::DrawingArea	drawto;
	
	//The input stuff
	Gtk::Adjustment		adj_zoom;
	Gtk::Label			zoom_print;
	
	//zoom window stuff
	bool				scrolling;
	
	//asyncronous rendering stuff
	etl::handle<AsyncRenderer>	renderer;
	etl::smart_ptr<sinfg::Surface> surface;
	bool						rendering;
	
	//drawing functionality
	void on_start_render(); //breaks out into asynchronous rendering
	void on_finish_render();
	void on_draw(); //renders the small thing we have
	void on_dirty_preview(); //dirties the preview for rerender
	
	//for the zoom buttons
	void on_zoom_in();
	void on_zoom_out();
	
	//handles the zoom scroller
	bool on_scroll_event(GdkEvent *event);
	void on_number_modify();
	
	//
	bool on_mouse_event(GdkEvent * e);
	
	//draws the gotten bitmap on the draw area
	bool on_expose_draw(GdkEventExpose *exp=0);
	
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
