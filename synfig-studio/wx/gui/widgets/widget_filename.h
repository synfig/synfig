/* === S Y N F I G ========================================================= */
/*!	\file widgets/widget_filename.h
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

#ifndef __SYNFIG_STUDIO_WIDGET_FILENAME_H
#define __SYNFIG_STUDIO_WIDGET_FILENAME_H

/* === H E A D E R S ======================================================= */

#include <sigc++/signal.h>
#include <sigc++/slot.h>
#include <gtkmm/grid.h>
#include <gtkmm/entry.h>
#include <gtkmm/button.h>

/* === M A C R O S ========================================================= */

/* === T Y P E D E F S ===================================================== */

/* === C L A S S E S & S T R U C T S ======================================= */

namespace Gtk { class Entry; class Button; };

namespace studio {

class Widget_Filename : public Gtk::Grid
{
	Gtk::Entry *entry_filename;
	Gtk::Button *button_choose;
	Gtk::Label *label_find;

	void on_button_choose_pressed();

	sigc::signal<void> signal_value_changed_;

public:
	sigc::signal<void> &signal_value_changed() { return signal_value_changed_; }
	Glib::SignalProxy0<void> signal_activate() { return entry_filename->signal_activate(); }

	void on_value_changed();

	void set_value(const  std::string &data);
	std::string get_value() const;
	void set_has_frame(bool x);
	Widget_Filename();
	~Widget_Filename();
}; // END of class Widget_Filename

}; // END of namespace studio

/* === E N D =============================================================== */

#endif
