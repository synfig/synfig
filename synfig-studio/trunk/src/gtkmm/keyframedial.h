/* === S Y N F I G ========================================================= */
/*!	\file keyframedial.h
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

#ifndef __SYNFIG_STUDIO_KEYFRAMEDIAL_H
#define __SYNFIG_STUDIO_KEYFRAMEDIAL_H

/* === H E A D E R S ======================================================= */

#include <gtkmm/tooltips.h>
#include <gtkmm/table.h>
#include <gtkmm/button.h>

#include "general.h"

/* === M A C R O S ========================================================= */

/* === T Y P E D E F S ===================================================== */

/* === C L A S S E S & S T R U C T S ======================================= */

namespace studio
{

class KeyFrameDial : public Gtk::Table
{
	Gtk::Tooltips tooltips;

	Gtk::Button *seek_prev_keyframe;
	Gtk::Button *seek_next_keyframe;
	Gtk::Button *lock_keyframe;

	Gtk::Button *create_icon(Gtk::IconSize iconsize, const char * stockid, const char * tooltip);

public:

	KeyFrameDial();
	Glib::SignalProxy0<void> signal_seek_prev_keyframe()  { return seek_prev_keyframe->signal_clicked(); }
	Glib::SignalProxy0<void> signal_seek_next_keyframe()  { return seek_next_keyframe->signal_clicked(); }
	Glib::SignalProxy0<void> signal_lock_keyframe()  { return lock_keyframe->signal_clicked(); }
	Gtk::Button *get_lock_button() { return lock_keyframe; }

}; // END of class KeyFrameDial

}; // END of namespace studio


/* === E N D =============================================================== */

#endif
