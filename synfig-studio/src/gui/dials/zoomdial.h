/* === S Y N F I G ========================================================= */
/*!	\file zoomdial.h
**	\brief Zoom widget
**
**	\legal
**	Copyright (c) 2002-2005 Robert B. Quattlebaum Jr., Adrian Bentley
**	Copyright (c) 2008 Chris Moore
**	Copyright (c) 2016 caryoscelus
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

#ifndef __SYNFIG_STUDIO_ZOOMDIAL_H
#define __SYNFIG_STUDIO_ZOOMDIAL_H

/* === H E A D E R S ======================================================= */

#include <gtkmm/button.h>
#include <gtkmm/entry.h>
#include <gtkmm/grid.h>

#include <synfig/real.h>

/* === M A C R O S ========================================================= */

/* === T Y P E D E F S ===================================================== */

/* === C L A S S E S & S T R U C T S ======================================= */

namespace studio
{

class ZoomDial : public Gtk::Grid
{
	Gtk::Button *zoom_in;
	Gtk::Button *zoom_out;
	Gtk::Button *zoom_fit;
	Gtk::Button *zoom_norm;
	Gtk::Entry *current_zoom;

	Gtk::Button *create_icon(Gtk::IconSize size, const Gtk::BuiltinStockID & stockid,
			const char * tooltip);
	void after_event(GdkEvent *event);
	bool current_zoom_event(GdkEvent* event);

public:
	ZoomDial(Gtk::IconSize &size);

	Glib::SignalProxy0<void> signal_zoom_in()   { return zoom_in->signal_clicked(); }
	Glib::SignalProxy0<void> signal_zoom_out()  { return zoom_out->signal_clicked(); }
	Glib::SignalProxy0<void> signal_zoom_fit()  { return zoom_fit->signal_clicked(); }
	Glib::SignalProxy0<void> signal_zoom_norm() { return zoom_norm->signal_clicked(); }
	Glib::SignalProxy0<void> signal_zoom_edit() { return current_zoom->signal_activate(); }

	void set_zoom(synfig::Real value);
	synfig::Real get_zoom(synfig::Real default_value = 0.0);
}; // END of class ZoomDial

}; // END of namespace studio

/* === E N D =============================================================== */

#endif
