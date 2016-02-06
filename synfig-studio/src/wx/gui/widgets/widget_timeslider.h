/* === S Y N F I G ========================================================= */
/*!	\file widgets/widget_timeslider.h
**	\brief Time Slider Widget Header
**
**	$Id$
**
**	\legal
**	Copyright (c) 2004 Adrian Bentley
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
#include <gtkmm/adjustment.h>

#include <synfig/time.h>
#include "canvasview.h"

/* === M A C R O S ========================================================= */

/* === T Y P E D E F S ===================================================== */

/* === C L A S S E S & S T R U C T S ======================================= */

namespace studio {

void render_time_point_to_window(const Cairo::RefPtr<Cairo::Context> &cr,const Gdk::Rectangle& ca,const synfig::TimePoint &tp,bool selected=false);


/* Design for the timeslider...

	Concept: Scalable ruler
		Ticks are done every so often (30 s, 10 frames, 5 frames, etc.)
		Print out frame numbers next to the big ticks
		Show blue pills in separate area (above or below)
*/

class Widget_Timeslider : public Gtk::DrawingArea
{
protected: //implementation that other interfaces can see
	Glib::RefPtr<Pango::Layout> layout; //implementation awesomeness for text drawing

	Glib::RefPtr<Gtk::Adjustment> adj_default;
	Glib::RefPtr<Gtk::Adjustment> adj_timescale;

	//HACK - I should not have to see this...
	Glib::RefPtr<Gtk::Adjustment> adj_bounds;
	double time_per_tickmark;

	//Statistics used for drawing stuff (and making sure we don't if we don't need to)
	/*double start,end;
	double current;

	bool invalidated;*/

	guint32 last_event_time;

	float fps;

	sigc::connection time_value_change;
	sigc::connection time_other_change;

	//TODO: fill out blue pill stuff

	//input functions

	virtual bool on_motion_notify_event(GdkEventMotion* event); //for dragging
	virtual bool on_scroll_event(GdkEventScroll* event); //for zooming
	virtual bool on_button_press_event(GdkEventButton *event); //for clicking
	virtual bool on_button_release_event(GdkEventButton *event); //for clicking

	virtual bool on_draw(const Cairo::RefPtr<Cairo::Context> &cr);

	//void update_times();

	void zoom_in(bool centerontime = false);
	void zoom_out(bool centerontime = false);

	//Drag the Frame
	bool dragscroll;

	/*NOTE: if we can set the mouse position to the original position
			this would only have to be set once (and it would be good otherwise too)
	*/
	double lastx; //last mouse position for dragging

public: //structors
	Widget_Timeslider();
	~Widget_Timeslider();

public: //Normal Interface

	void draw() {queue_draw();}
	virtual void refresh(); //reget bluepills, time values and queue_draw if need be

public: //Time Interface

	//Run FPS stuff through it to the MAX
	double get_global_fps() const {return fps;}
	void set_global_fps(float d);

	//accessors for the time adjustment
	Glib::RefPtr<Gtk::Adjustment> get_time_adjustment() const { return adj_timescale; }
	void set_time_adjustment(const Glib::RefPtr<Gtk::Adjustment> &x);

	//HACK - I should not have to see these bounds (should be boundless)
	Glib::RefPtr<Gtk::Adjustment> get_bounds_adjustment() const { return adj_bounds; }
	void set_bounds_adjustment(const Glib::RefPtr<Gtk::Adjustment> &x) { adj_bounds = x; }
};

}; // END of namespace studio

/* === E N D =============================================================== */

#endif
