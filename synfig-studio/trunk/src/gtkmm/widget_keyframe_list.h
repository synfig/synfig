/* === S Y N F I G ========================================================= */
/*!	\file widget_keyframe_list.h
**	\brief A custom widget to manage keyframes in the timeline.
**
**	$Id$
**
**	\legal
**	Copyright (c) 2002-2005 Robert B. Quattlebaum Jr., Adrian Bentley
**	Copyright (c) 2009 Carlos López
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

#ifndef __SYNFIG_STUDIO_WIDGET_KEYFRAME_LIST_H
#define __SYNFIG_STUDIO_WIDGET_KEYFRAME_LIST_H

/* === H E A D E R S ======================================================= */

#include <gtkmm/drawingarea.h>
#include <gtkmm/adjustment.h>
#include <synfig/keyframe.h>
#include <sigc++/connection.h>


/* === M A C R O S ========================================================= */

/* === T Y P E D E F S ===================================================== */

/* === C L A S S E S & S T R U C T S ======================================= */

namespace studio {

class Widget_Keyframe_List : public Gtk::DrawingArea
{
	//! Time adjustment window
	Gtk::Adjustment adj_default;
	Gtk::Adjustment *adj_timescale;

	//!The list of keyframes to be drawn on the widget and moved with mouse
	synfig::KeyframeList default_kf_list_;
	mutable synfig::KeyframeList* kf_list_;

	//! The frames per second of the canvas
	float fps;

	//!True if it is editable. Keyframes can be moved.
	bool editable_;

	//!True if a keyframe is being dragged.
	bool dragging_;

	//!True if a keyframe has been moved
	bool changed_;

	//!Holds the selected keyframe of the keyframe list
	synfig::Keyframe selected_kf;

	//!The time of the selected keyframe
	synfig::Time selected_kf_time;

	//!The time of the selected keyframe during draging
	synfig::Time dragging_kf_time;

	//!Connectors for handling the signals of the time adjustment
	sigc::connection time_value_change;
	sigc::connection time_other_change;

public:

	//!Default constructor
	Widget_Keyframe_List();

	//!Destructror
	~Widget_Keyframe_List();

	//!Loads a new keyframe list on the widget.
	void set_kf_list(synfig::KeyframeList* x);

	//!Member for private data.
	synfig::KeyframeList* get_kf_list()const { return kf_list_; }

	//!Member for private data
	void set_editable(bool x=true) { editable_=x; }

	//!Member for private data
	bool get_editable()const { return editable_; }


	//!Store the selected keyframe value
	void set_selected_keyframe(const synfig::Keyframe &x);

	//!Returns the selected keyframe
	const synfig::Keyframe& get_selected_keyframe() { return selected_kf; }

	//! Set the time adjustment and proper connects its change signals
	void set_time_adjustment(Gtk::Adjustment *x);

	//! Set the fps
	void set_fps(float x);

	//! Performs the keyframe movement. Returns true if it was sucessful
	bool perform_move_kf();



/* ======================= EVENTS HANDLERS ===========================*/
	//!Redraw event. Should draw all the keyframes +  the selected + the dragged
	bool redraw(GdkEventExpose*bleh=NULL);

	//!Mouse event handler.
	bool on_event(GdkEvent *event);
}; // END of class Keyframe_List

}; // END of namespace studio

/* === E N D =============================================================== */

#endif
