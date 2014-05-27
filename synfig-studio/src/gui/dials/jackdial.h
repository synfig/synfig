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

#include "general.h"

/* === M A C R O S ========================================================= */

/* === T Y P E D E F S ===================================================== */

/* === C L A S S E S & S T R U C T S ======================================= */

namespace studio
{

class JackDial : public Gtk::Table
{
	Gtk::Button *enable_jack;
	Gtk::Button *disable_jack;

	Gtk::Button *create_icon(Gtk::IconSize iconsize, const char * stockid, const char * tooltip);

public:
	JackDial();
#ifdef WITH_JACK
	Glib::SignalProxy0<void> signal_enable_jack()         { return enable_jack->signal_clicked(); }
	Glib::SignalProxy0<void> signal_disable_jack()        { return disable_jack->signal_clicked(); }

	void toggle_enable_jack(bool jack_is_enabled);
#endif

}; // END of class FrameDial

}; // END of namespace studio


/* === E N D =============================================================== */

#endif
