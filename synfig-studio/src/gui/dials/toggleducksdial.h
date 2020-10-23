/* === S Y N F I G ========================================================= */
/*!	\file toggleducksdial.h
**	\brief Template Header
**
**	$Id$
**
**	\legal
**	Copyright (c) 2002-2005 Robert B. Quattlebaum Jr., Adrian Bentley
**	Copyright (c) 2008 Chris Moore
**	Copyright (c) 2009 Gerco Ballintijn
**	Copyright (c) 2009 Carlos Lopez
**
**	This file is part of Synfig.
**
**	Synfig is free software: you can redistribute it and/or modify
**	it under the terms of the GNU General Public License as published by
**	the Free Software Foundation, either version 2 of the License, or
**	(at your option) any later version.
**
**	Synfig is distributed in the hope that it will be useful,
**	but WITHOUT ANY WARRANTY; without even the implied warranty of
**	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
**	GNU General Public License for more details.
**
**	You should have received a copy of the GNU General Public License
**	along with Synfig.  If not, see <https://www.gnu.org/licenses/>.
**	\endlegal
*/
/* ========================================================================= */

/* === S T A R T =========================================================== */

#ifndef __SYNFIG_STUDIO_TOOGLEDUCKSDIAL_H
#define __SYNFIG_STUDIO_TOOGLEDUCKSDIAL_H

/* === H E A D E R S ======================================================= */

#include <gtkmm/toolbar.h>
#include <gtkmm/toggletoolbutton.h>

#include <gui/duckmatic.h>

/* === M A C R O S ========================================================= */

/* === T Y P E D E F S ===================================================== */

/* === C L A S S E S & S T R U C T S ======================================= */

namespace studio
{

class ToggleDucksDial
{
	Gtk::ToggleToolButton ducks_position;
	Gtk::ToggleToolButton ducks_vertex;
	Gtk::ToggleToolButton ducks_tangent;
	Gtk::ToggleToolButton ducks_radius;
	Gtk::ToggleToolButton ducks_width;
	Gtk::ToggleToolButton ducks_angle;

	void init_label_button(Gtk::ToggleToolButton &button, Gtk::IconSize iconsize, const char *stockid, const char *label, const char *tooltip);

public:

	ToggleDucksDial(const Gtk::IconSize &size);
	void update_toggles(Duck::Type mask);

	void insert_to_toolbar(Gtk::Toolbar &toolbar, int index = -1);
	void remove_from_toolbar(Gtk::Toolbar &toolbar);

	Glib::SignalProxy0<void> signal_ducks_position()  { return ducks_position.signal_toggled(); }
	Glib::SignalProxy0<void> signal_ducks_vertex()    { return ducks_vertex.  signal_toggled(); }
	Glib::SignalProxy0<void> signal_ducks_tangent()   { return ducks_tangent. signal_toggled(); }
	Glib::SignalProxy0<void> signal_ducks_radius()    { return ducks_radius.  signal_toggled(); }
	Glib::SignalProxy0<void> signal_ducks_width()     { return ducks_width.   signal_toggled(); }
	Glib::SignalProxy0<void> signal_ducks_angle()     { return ducks_angle.   signal_toggled(); }

}; // END of class ToggleDucksDial

}; // END of namespace studio


/* === E N D =============================================================== */

#endif
