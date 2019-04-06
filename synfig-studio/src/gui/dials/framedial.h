/* === S Y N F I G ========================================================= */
/*!	\file zoomdial.h
**	\brief Template Header
**
**	$Id$
**
**	\legal
**	Copyright (c) 2002-2005 Robert B. Quattlebaum Jr., Adrian Bentley
**  Copyright (c) 2008 Chris Moore
**  Copyright (c) 2009 Gerco Ballintijn
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

#ifndef __SYNFIG_STUDIO_FRAMEDIAL_H
#define __SYNFIG_STUDIO_FRAMEDIAL_H

/* === H E A D E R S ======================================================= */

#include <gtkmm/box.h>
#include <gtkmm/button.h>
#include <gtkmm/togglebutton.h>
#include "widgets/widget_time.h"

/* === M A C R O S ========================================================= */

/* === T Y P E D E F S ===================================================== */

/* === C L A S S E S & S T R U C T S ======================================= */

namespace studio
{

class FrameDial : public Gtk::HBox
{
private:
	Gtk::Button *seek_begin;
	Gtk::Button *seek_prev_keyframe;
	Gtk::Button *seek_prev_frame;
	Gtk::Button *play;
	Gtk::Button *pause;
	Gtk::Button *seek_next_frame;
	Gtk::Button *seek_next_keyframe;
	Gtk::Button *seek_end;
	Widget_Time *end_time;
	Gtk::ToggleButton *repeat;
	Gtk::Button *bound_lower;
	Gtk::ToggleButton *bounds_enable;
	Gtk::Button *bound_upper;

	sigc::signal<void, bool> signal_repeat_;
	sigc::signal<void, bool> signal_bounds_enable_;

	void init_button(Gtk::Button &button, const char *stockid, const char *tooltip);
	void create_separator();
	Gtk::Button *create_button(const char *stockid, const char *tooltip, bool separator = false);
	Gtk::ToggleButton *create_toggle(const char *stockid, const char *tooltip, bool separator = false);

	void on_repeat_toggled();
	void on_bounds_toggled();
	Widget_Time* create_end_time_entry(const char *tooltip);

public:
	FrameDial();

	Glib::SignalProxy0<void> signal_seek_begin()         { return seek_begin->signal_clicked(); }
	Glib::SignalProxy0<void> signal_seek_prev_keyframe() { return seek_prev_keyframe->signal_clicked(); }
	Glib::SignalProxy0<void> signal_seek_prev_frame()    { return seek_prev_frame->signal_clicked(); }
	Glib::SignalProxy0<void> signal_play()               { return play->signal_clicked(); }
	Glib::SignalProxy0<void> signal_pause()              { return pause->signal_clicked();}
	Glib::SignalProxy0<void> signal_seek_next_frame()    { return seek_next_frame->signal_clicked(); }
	Glib::SignalProxy0<void> signal_seek_next_keyframe() { return seek_next_keyframe->signal_clicked(); }
	Glib::SignalProxy0<void> signal_seek_end()           { return seek_end->signal_clicked(); }
	Glib::SignalProxy0<void> signal_end_time_changed()   { return end_time->signal_activate(); }
	sigc::signal<void, bool> signal_repeat()             { return signal_repeat_; }
	sigc::signal<void, bool> signal_bounds_enable()      { return signal_bounds_enable_; }
	Glib::SignalProxy0<void> signal_bound_lower()        { return bound_lower->signal_clicked(); }
	Glib::SignalProxy0<void> signal_bound_upper()        { return bound_upper->signal_clicked(); }

	void toggle_play_pause_button(bool is_playing);
	void toggle_repeat(bool enable);
	void toggle_bounds_enable(bool enable);
	void set_end_time(float fps, float value);
	void on_end_time_widget_changed();
	float get_end_time();
}; // END of class FrameDial

}; // END of namespace studio


/* === E N D =============================================================== */

#endif
