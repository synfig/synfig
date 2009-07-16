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
#include <synfig/keyframe.h>

/* === M A C R O S ========================================================= */

/* === T Y P E D E F S ===================================================== */

/* === C L A S S E S & S T R U C T S ======================================= */

namespace studio {

class Widget_Keyframe_List : public Gtk::DrawingArea
{
	sigc::signal<void> signal_value_changed_;
	sigc::signal<void> signal_clicked_;

	sigc::signal<void,synfig::Keyframe> signal_keyframe_selected_;

	synfig::KeyframeList kf_list_;

	bool editable_;

	bool changed_;

	synfig::Keyframe selected_kf;

	void popup_menu(float x);

	//void insert_cpoint(float x);

	//void remove_cpoint(float x);

public:

	Widget_Keyframe_List();

	~Widget_Keyframe_List();

	sigc::signal<void>& signal_value_changed() { return signal_value_changed_; }
	sigc::signal<void>& signal_clicked() { return signal_clicked_; }

	sigc::signal<void,synfig::Keyframe>& signal_keyframe_selected() { return signal_keyframe_selected_; }

	void set_value(const synfig::KeyframeList& x);

	const synfig::KayframeList& get_value()const { return kf_list_; }

	void set_editable(bool x=true) { editable_=x; }

	bool get_editable()const { return editable_; }



	void set_selected_keyframe(const synfig::Keyframe &x);

	const synfig::Kayframe& get_selected_keyframe() { return selected_kf; }

	void update_keyframe(const synfig::Keyframe &x);



	bool redraw(GdkEventExpose*bleh=NULL);

	bool on_event(GdkEvent *event);
}; // END of class Keyframe_List

}; // END of namespace studio

/* === E N D =============================================================== */

#endif
