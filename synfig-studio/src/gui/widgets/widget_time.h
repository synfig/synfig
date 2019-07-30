/* === S Y N F I G ========================================================= */
/*!	\file widgets/widget_time.h
**	\brief Template Header
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

#ifndef __SYNFIG_STUDIO_WIDGET_TIME_H
#define __SYNFIG_STUDIO_WIDGET_TIME_H

/* === H E A D E R S ======================================================= */

#include <sigc++/sigc++.h>
#include <gtkmm/entry.h>
#include <synfig/time.h>

/* === M A C R O S ========================================================= */

/* === T Y P E D E F S ===================================================== */

/* === C L A S S E S & S T R U C T S ======================================= */

namespace studio {

class Widget_Time : public Gtk::Entry
{

	sigc::signal<void> signal_value_changed_;

	float fps_;

	synfig::Time time_;

	void init();

protected:
	bool on_focus_out_event(GdkEventFocus* event);

	bool on_focus_in_event(GdkEventFocus* event);

	//void on_activate();

	void refresh_text();

	void refresh_value();

	bool on_event(GdkEvent* event);

public:
	sigc::signal<void> &signal_value_changed() { return signal_value_changed_; }

	void set_value(const synfig::Time &data);
	synfig::Time get_value()const;
	void set_fps(float x);
	Widget_Time();
	~Widget_Time();

// Glade & GtkBuilder related
public:
	Widget_Time(BaseObjectType* cobject);
	static Glib::ObjectBase* wrap_new(GObject* o);
	static void register_type();
private:
	static GType gtype;
}; // END of class Widget_Time

}; // END of namespace studio

/* === E N D =============================================================== */

#endif
