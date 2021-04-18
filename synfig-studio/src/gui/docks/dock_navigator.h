/* === S Y N F I G ========================================================= */
/*!	\file docks/dock_navigator.h
**	\brief Navigator Dock Header
**
**	$Id$
**
**	\legal
**	Copyright (c) 2002-2005 Robert B. Quattlebaum Jr., Adrian Bentley
**  ......... ... 2018 Ivan Mahonin
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

#include <cairomm/context.h>

#include <gtkmm/drawingarea.h>
#include <gtkmm/adjustment.h>
#include <gtkmm/label.h>
#include <gtkmm/grid.h>

#include <gui/docks/dock_canvasspecific.h>

/* === M A C R O S ========================================================= */

/* === T Y P E D E F S ===================================================== */

/* === C L A S S E S & S T R U C T S ======================================= */

namespace studio {

class CanvasView;

class Widget_NavView : public Gtk::Grid
{
	etl::loose_handle<CanvasView> canvas_view;
	Cairo::RefPtr<Cairo::ImageSurface> surface;

	Gtk::DrawingArea drawto;
	Glib::RefPtr<Gtk::Adjustment> adj_zoom;
	Gtk::Label zoom_print;
	int scrolling;

	sigc::connection view_window_changed;
	sigc::connection rendering_tile_finished;
	sigc::connection time_changed;

	void on_number_modify();
	bool on_mouse_event(GdkEvent *event);
	bool on_drawto_draw(const Cairo::RefPtr<Cairo::Context> &cr);

	void on_view_window_changed();
	void on_rendering_tile_finished(synfig::Time time);

public:
	Widget_NavView();
	~Widget_NavView();

	const etl::loose_handle<CanvasView>& get_canvas_view() const { return canvas_view; }
	void set_canvas_view(const etl::loose_handle<CanvasView> &x);
};


class Dock_Navigator : public Dock_CanvasSpecific
{
private:
	Widget_NavView navview;
public:
	Dock_Navigator();
	~Dock_Navigator();
	virtual void changed_canvas_view_vfunc(etl::loose_handle<CanvasView> canvas_view);
};

}; // END of namespace studio

/* === E N D =============================================================== */

#endif
