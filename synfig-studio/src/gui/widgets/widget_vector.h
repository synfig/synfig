/* === S Y N F I G ========================================================= */
/*!	\file widgets/widget_vector.h
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

#ifndef __SYNFIG_STUDIO_WIDGET_VECTOR_H
#define __SYNFIG_STUDIO_WIDGET_VECTOR_H

/* === H E A D E R S ======================================================= */

#include <gtkmm/box.h>
#include <gtkmm/adjustment.h>
#include <synfig/vector.h>
#include <synfig/canvas.h>

/* === M A C R O S ========================================================= */

/* === T Y P E D E F S ===================================================== */

/* === C L A S S E S & S T R U C T S ======================================= */

namespace Gtk {
	class SpinButton;
	class Entry;
};

namespace studio {

class Widget_Distance;

class Widget_Vector : public Gtk::Box
{
	Gtk::Entry* entry_x;
	Gtk::Entry* entry_y;
    
	Gtk::SpinButton* spinbutton_x;
	Gtk::SpinButton* spinbutton_y;

	Widget_Distance* distance_x;
	Widget_Distance* distance_y;

	Glib::RefPtr<Gtk::Adjustment> x_adjustment;
	Glib::RefPtr<Gtk::Adjustment> y_adjustment;

	synfig::Vector vector;

	sigc::signal<void> signal_value_changed_;

	sigc::signal<void> signal_activate_;

	synfig::Canvas::LooseHandle canvas_;

	void init();

public:

	void activate() { signal_activate_(); }

	void set_canvas(synfig::Canvas::LooseHandle);
	const synfig::Canvas::LooseHandle& get_canvas()const { return canvas_; }

	sigc::signal<void>& signal_value_changed() { return signal_value_changed_; }

	sigc::signal<void>& signal_activate() { return signal_activate_; }

	void on_value_changed();
	void on_entry_x_changed();
	void on_entry_y_changed();
	void on_grab_focus();

	void set_value(const synfig::Vector &data);
	const synfig::Vector &get_value();
	void set_has_frame(bool x);
	void set_digits(int x);

	Widget_Vector();
	~Widget_Vector();

protected:
	void show_all_vfunc();

// Glade & GtkBuilder related
public:
	Widget_Vector(BaseObjectType* cobject);
	static Glib::ObjectBase* wrap_new(GObject* o);
	static void register_type();
private:
	static GType gtype;
}; // END of class Widget_Vector

}; // END of namespace studio

/* === E N D =============================================================== */

#endif
