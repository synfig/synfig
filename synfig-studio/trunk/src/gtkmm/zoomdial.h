/* === S Y N F I G ========================================================= */
/*!	\file zoomdial.h
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

#ifndef __SYNFIG_STUDIO_ZOOMDIAL_H
#define __SYNFIG_STUDIO_ZOOMDIAL_H

/* === H E A D E R S ======================================================= */

#include <gtkmm/tooltips.h>
#include <gtkmm/button.h>

/* === M A C R O S ========================================================= */

#define SMALL_BUTTON(button,stockid,tooltip)	\
	button = manage(new class Gtk::Button());	\
	icon=manage(new Gtk::Image(Gtk::StockID(stockid),iconsize));	\
	button->add(*icon);	\
	tooltips.set_tip(*button,tooltip);	\
	icon->set_padding(0,0);\
	icon->show();	\
	button->set_relief(Gtk::RELIEF_NONE); \
	button->show()

/* === T Y P E D E F S ===================================================== */

/* === C L A S S E S & S T R U C T S ======================================= */

namespace studio
{

class ZoomDial : public Gtk::Table
{

	Gtk::Tooltips tooltips;
	Gtk::IconSize iconsize;


public:
	Gtk::Button *zoom_in;
	Gtk::Button *zoom_out;
	Gtk::Button *zoom_fit;
	Gtk::Button *zoom_norm;

	ZoomDial(Gtk::IconSize &size):Table(3, 1, false),iconsize(size)
	{
		Gtk::Image *icon;

		SMALL_BUTTON(zoom_in,"gtk-add",_("Zoom In"));
		SMALL_BUTTON(zoom_out,"gtk-remove",_("Zoom Out"));
		SMALL_BUTTON(zoom_fit,"gtk-zoom-fit",_("Zoom to Fit"));
		SMALL_BUTTON(zoom_norm,"gtk-zoom-100",_("Zoom to 100%"));

		attach(*zoom_out, 0, 1, 0, 1, Gtk::SHRINK, Gtk::SHRINK, 0, 0);
		attach(*zoom_norm, 1, 2, 0, 1, Gtk::SHRINK, Gtk::SHRINK, 0, 0);
		attach(*zoom_fit, 2, 3, 0, 1, Gtk::SHRINK, Gtk::SHRINK, 0, 0);
		attach(*zoom_in, 3, 4, 0, 1, Gtk::SHRINK, Gtk::SHRINK, 0, 0);
	}

	Glib::SignalProxy0<void> signal_zoom_in()
	{ return zoom_in->signal_clicked(); }
	Glib::SignalProxy0<void> signal_zoom_out()
	{ return zoom_out->signal_clicked(); }
	Glib::SignalProxy0<void> signal_zoom_fit()
	{ return zoom_fit->signal_clicked(); }
	Glib::SignalProxy0<void> signal_zoom_norm()
	{ return zoom_norm->signal_clicked(); }

}; // END of class ZoomDial

}; // END of namespace studio

/* === E N D =============================================================== */

#endif
