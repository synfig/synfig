/* === S Y N F I G ========================================================= */
/*!	\file widget_keyframe_list.h
**	\brief A custom widget to manage keyframes in the timeline.
**
**	$Id$
**
**	\legal
**	Copyright (c) 2002-2005 Robert B. Quattlebaum Jr., Adrian Bentley
**	Copyright (c) 2009 Carlos López
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

#ifndef __SYNFIG_STUDIO_WIDGET_KEYFRAME_LIST_H
#define __SYNFIG_STUDIO_WIDGET_KEYFRAME_LIST_H

/* === H E A D E R S ======================================================= */

#include <sigc++/sigc++.h>

#include <gtkmm/drawingarea.h>
#include <gtkmm/tooltip.h>
#include <gtkmm/window.h>

#include <synfig/keyframe.h>

#include <gui/timemodel.h>

#include <synfigapp/canvasinterface.h>

/* === M A C R O S ========================================================= */

/* === T Y P E D E F S ===================================================== */

/* === C L A S S E S & S T R U C T S ======================================= */

namespace studio {

class Widget_Keyframe_List : public Gtk::DrawingArea
{
	//! The list of keyframes to be drawn on the widget and moved with mouse
	synfig::KeyframeList* kf_list;

	//! The canvas interface being watched
	etl::loose_handle<synfigapp::CanvasInterface> canvas_interface;

	//! Time model
	etl::handle<TimeModel> time_model;

	//! True if it is editable. Keyframes can be moved.
	bool editable;

	//! True if a keyframe is being dragged.
	bool dragging;

	//! True if a keyframe has been moved
	bool changed;

	synfig::Time time_ratio;

	//! Holds the selected keyframe of the keyframe list
	synfig::Keyframe selected_kf;
	bool selected;

	//! The time of the selected keyframe
	synfig::Time selected_kf_time;

	//! The time of the selected keyframe during dragging
	synfig::Time dragging_kf_time;

	//! The Moving handmade tooltip window
	Gtk::Window moving_tooltip;
	//! The Moving handmade tooltip label
	Gtk::Label moving_tooltip_label;
	//! The Moving handmade tooltip y fixed coordinate
	int moving_tooltip_y;

	//! Connectors for handling the signal of the time model
	sigc::connection time_model_change;

	//! Connectors for handling the signal of the time model
	sigc::connection keyframe_added;
	sigc::connection keyframe_changed;
	sigc::connection keyframe_removed;
	sigc::connection keyframe_selected;

public:
	Widget_Keyframe_List();
	~Widget_Keyframe_List();

	//!Loads a new keyframe list on the widget.
	void set_kf_list(synfig::KeyframeList *x);
	synfig::KeyframeList* get_kf_list() const { return kf_list; }

	//! Set the canvas interface, it's the place where signals are connected
	//! This function also replaces the keyframe list (see: set_fk_list())
	void set_canvas_interface(const etl::loose_handle<synfigapp::CanvasInterface> &x);
	const etl::loose_handle<synfigapp::CanvasInterface>& get_canvas_interface() const { return canvas_interface; }

	//! Set the time model and proper connects its change signals
	void set_time_model(const etl::handle<TimeModel> &x);
	const etl::handle<TimeModel>& get_time_model() const { return time_model; }

	void set_editable(bool x = true) { editable = x; }
	bool get_editable() const { return editable; }

	//!Store the selected keyframe value and fired keyframe selected signal
	void reset_selected_keyframe();
	void set_selected_keyframe(const synfig::Keyframe &x);
	bool is_keyframe_selected() const { return selected; }
	const synfig::Keyframe& get_selected_keyframe() const { return selected_kf; }

	//! Performs the keyframe movement. Returns true if it was successful
	//! \param[in] delta If false permorm normal move. If true perform delta movement.
	//! \return true: if success otherwise false
	bool perform_move_kf(bool delta);

	static void draw_arrow(
		const Cairo::RefPtr<Cairo::Context> &cr,
		double x, double y,
		double width, double height,
		bool fill,
		const synfig::Color &color );

	// Events handlers
	bool on_draw(const Cairo::RefPtr<Cairo::Context> &cr);
	bool on_event(GdkEvent *event);
	void on_keyframe_selected(synfig::Keyframe keyframe);
}; // END of class Keyframe_List

}; // END of namespace studio


/* === E N D =============================================================== */

#endif
