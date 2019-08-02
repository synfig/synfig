/* === S Y N F I G ========================================================= */
/*!	\file widgets/widget_timeslider.h
**	\brief Time Slider Widget Header
**
**	$Id$
**
**	\legal
**	Copyright (c) 2004 Adrian Bentley
**	......... ... 2018 Ivan Mahonin
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

#ifndef __SYNFIG_WIDGET_TIMESLIDER_H
#define __SYNFIG_WIDGET_TIMESLIDER_H

/* === H E A D E R S ======================================================= */

#include <gtkmm/drawingarea.h>

#include <synfig/time.h>

#include <gui/timemodel.h>

/* === M A C R O S ========================================================= */

/* === T Y P E D E F S ===================================================== */

/* === C L A S S E S & S T R U C T S ======================================= */

namespace studio {

class TimePlotData;

//! Design for the timeslider...
//! Concept: Scalable ruler
//!     Ticks are done every so often (30 s, 10 frames, 5 frames, etc.)
//!     Print out frame numbers next to the big ticks
//!     Show blue pills in separate area (above or below)
class Widget_Timeslider: public Gtk::DrawingArea
{
protected: // implementation that other interfaces can see
	Glib::RefPtr<Pango::Layout> layout; // implementation awesomeness for text drawing

	Cairo::RefPtr<Cairo::SurfacePattern> play_bounds_pattern;

	// last mouse position for dragging
	double lastx;

	// distance between two small marks, also uses for left/right scroll
	synfig::Time step;

	sigc::connection time_change;
	sigc::connection time_bounds_change;

	TimePlotData * time_plot_data;

	virtual bool on_button_press_event(GdkEventButton *event); //for clicking
	virtual bool on_button_release_event(GdkEventButton *event); //for clicking
	virtual bool on_motion_notify_event(GdkEventMotion* event); //for dragging
	virtual bool on_scroll_event(GdkEventScroll* event); //for zooming
	virtual bool on_draw(const Cairo::RefPtr<Cairo::Context> &cr);

	virtual void draw_background(const Cairo::RefPtr<Cairo::Context> &cr);

	virtual bool on_configure_event(GdkEventConfigure * configure);

public:
	Widget_Timeslider();
	~Widget_Timeslider();

	const etl::handle<TimeModel>& get_time_model() const;
	void set_time_model(const etl::handle<TimeModel> &x);
};

}; // END of namespace studio

/* === E N D =============================================================== */

#endif
