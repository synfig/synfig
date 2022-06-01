/* === S Y N F I G ========================================================= */
/*!	\file keyframedial.h
**	\brief Template Header
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

#ifndef __SYNFIG_STUDIO_KEYFRAMEDIAL_H
#define __SYNFIG_STUDIO_KEYFRAMEDIAL_H

/* === H E A D E R S ======================================================= */

#include <gtkmm/box.h>
#include <gtkmm/togglebutton.h>
#include <synfigapp/editmode.h>

/* === M A C R O S ========================================================= */

/* === T Y P E D E F S ===================================================== */

/* === C L A S S E S & S T R U C T S ======================================= */

namespace studio
{

class KeyFrameDial : public Gtk::Box
{
	Gtk::ToggleButton *toggle_keyframe_past;
	Gtk::ToggleButton *toggle_keyframe_future;

public:

	KeyFrameDial();
	void on_mode_changed(synfigapp::EditMode mode); // Updates button icons/state
	Glib::SignalProxy<void> signal_toggle_keyframe_past() { return toggle_keyframe_past->signal_toggled(); }
	Glib::SignalProxy<void> signal_toggle_keyframe_future() { return toggle_keyframe_future->signal_toggled(); }

}; // END of class KeyFrameDial

} // END of namespace studio


/* === E N D =============================================================== */

#endif
