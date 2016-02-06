/* === S Y N F I G ========================================================= */
/*!	\file zoomdial.h
**	\brief Template Header
**
**	$Id$
**
**	\legal
**	Copyright (c) 2002-2005 Robert B. Quattlebaum Jr., Adrian Bentley
**	Copyright (c) 2008 Chris Moore
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

#ifndef __SYNFIG_STUDIO_ZOOMDIAL_H
#define __SYNFIG_STUDIO_ZOOMDIAL_H

/* === H E A D E R S ======================================================= */

#include <gtkmm/tooltip.h>
#include <gtkmm/table.h>
#include <gtkmm/button.h>

/* === M A C R O S ========================================================= */

/* === T Y P E D E F S ===================================================== */

/* === C L A S S E S & S T R U C T S ======================================= */

namespace studio
{

class ZoomDial : public Gtk::Table
{
	Gtk::Button *zoom_in;
	Gtk::Button *zoom_out;
	Gtk::Button *zoom_fit;
	Gtk::Button *zoom_norm;

	Gtk::Button *create_icon(Gtk::IconSize size, const Gtk::BuiltinStockID & stockid,
			const char * tooltip);

public:
	ZoomDial(Gtk::IconSize &size);

	Glib::SignalProxy0<void> signal_zoom_in()   { return zoom_in->signal_clicked(); }
	Glib::SignalProxy0<void> signal_zoom_out()  { return zoom_out->signal_clicked(); }
	Glib::SignalProxy0<void> signal_zoom_fit()  { return zoom_fit->signal_clicked(); }
	Glib::SignalProxy0<void> signal_zoom_norm() { return zoom_norm->signal_clicked(); }

}; // END of class ZoomDial

}; // END of namespace studio

/* === E N D =============================================================== */

#endif
