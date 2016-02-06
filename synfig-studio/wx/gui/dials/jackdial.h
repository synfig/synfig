/* === S Y N F I G ========================================================= */
/*!	\file jackdial.h
**	\brief Template Header
**
**	$Id$
**
**	\legal
**	Copyright (c) 2002-2005 Robert B. Quattlebaum Jr., Adrian Bentley
**	Copyright (c) 2008 Chris Moore
**	Copyright (c) 2009 Gerco Ballintijn
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

#ifndef __SYNFIG_STUDIO_JACKDIAL_H
#define __SYNFIG_STUDIO_JACKDIAL_H

/* === H E A D E R S ======================================================= */

#include <gtkmm/tooltip.h>
#include <gtkmm/table.h>
#include <gtkmm/button.h>
#include <gtkmm/togglebutton.h>

#include "general.h"

#include <gui/widgets/widget_time.h>

/* === M A C R O S ========================================================= */

/* === T Y P E D E F S ===================================================== */

/* === C L A S S E S & S T R U C T S ======================================= */

namespace studio
{

class JackDial : public Gtk::Table
{
	Gtk::ToggleButton *toggle_jack;
	Widget_Time *offset;

	Gtk::ToggleButton *create_icon(Gtk::IconSize iconsize, const char * stockid, const char * tooltip);

public:
	JackDial();
	Glib::SignalProxy0<void> signal_toggle_jack()	{ return toggle_jack->signal_toggled(); }
	Gtk::ToggleButton *get_toggle_jackbutton()	{ return toggle_jack; }
	Widget_Time *get_offsetwidget()	{ return offset; }

	sigc::signal<void>& signal_offset_changed()      { return offset->signal_value_changed(); }

	void set_offset(const synfig::Time &value)       { offset->set_value(value); }
	synfig::Time get_offset() const                  { return offset->get_value(); }
	void set_fps(float value)                        { offset->set_fps(value); }
}; // END of class FrameDial

}; // END of namespace studio


/* === E N D =============================================================== */

#endif
