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

#ifndef __SYNFIG_STUDIO_TOOGLEDUCKSDIAL_H
#define __SYNFIG_STUDIO_TOOGLEDUCKSDIAL_H

/* === H E A D E R S ======================================================= */

#include <gtkmm/tooltip.h>
#include <gtkmm/table.h>
#include <gtkmm/togglebutton.h>
#include "duckmatic.h"

#include "general.h"


/* === M A C R O S ========================================================= */

/* === T Y P E D E F S ===================================================== */

/* === C L A S S E S & S T R U C T S ======================================= */

namespace studio
{

class ToggleDucksDial : public Gtk::Table
{
	Gtk::ToggleButton *ducks_position;
	Gtk::ToggleButton *ducks_vertex;
	Gtk::ToggleButton *ducks_tangent;
	Gtk::ToggleButton *ducks_radius;
	Gtk::ToggleButton *ducks_width;
	Gtk::ToggleButton *ducks_angle;

	Gtk::ToggleButton *create_label_button(Gtk::IconSize iconsize, const char * stockid, const char * tooltip);

public:

	ToggleDucksDial(Gtk::IconSize & size);
	void update_toggles(Duck::Type mask);
	Glib::SignalProxy0<void> signal_ducks_position()  { return ducks_position->signal_toggled(); }
	Glib::SignalProxy0<void> signal_ducks_vertex()    { return ducks_vertex->  signal_toggled(); }
	Glib::SignalProxy0<void> signal_ducks_tangent()   { return ducks_tangent-> signal_toggled(); }
	Glib::SignalProxy0<void> signal_ducks_radius()    { return ducks_radius->  signal_toggled(); }
	Glib::SignalProxy0<void> signal_ducks_width()     { return ducks_width->   signal_toggled(); }
	Glib::SignalProxy0<void> signal_ducks_angle()     { return ducks_angle->   signal_toggled(); }

}; // END of class ToggleDucksDial

}; // END of namespace studio


/* === E N D =============================================================== */

#endif
