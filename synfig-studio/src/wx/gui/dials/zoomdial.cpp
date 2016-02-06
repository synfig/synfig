/* === S Y N F I G ========================================================= */
/*!	\file zoomdial.cpp
**	\brief Template File
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

/* === H E A D E R S ======================================================= */

#ifdef USING_PCH
#	include "pch.h"
#else
#ifdef HAVE_CONFIG_H
#	include <config.h>
#endif

#include "zoomdial.h"
#include <gtkmm/image.h>
#include <gtkmm/stock.h>

#include "general.h"

#endif

/* === U S I N G =========================================================== */

using namespace std;
using namespace studio;

/* === M A C R O S ========================================================= */

/* === G L O B A L S ======================================================= */

/* === P R O C E D U R E S ================================================= */

/* === M E T H O D S ======================================================= */

ZoomDial::ZoomDial(Gtk::IconSize & size): Table(3, 1, false)
{
	zoom_in = create_icon(size, Gtk::Stock::ZOOM_IN, _("Zoom In"));
	zoom_out = create_icon(size, Gtk::Stock::ZOOM_OUT, _("Zoom Out"));
	zoom_fit = create_icon(size, Gtk::Stock::ZOOM_FIT, _("Zoom to Fit"));
	zoom_norm = create_icon(size, Gtk::Stock::ZOOM_100, _("Zoom to 100%"));

	attach(*zoom_out, 0, 1, 0, 1, Gtk::SHRINK, Gtk::SHRINK, 0, 0);
	attach(*zoom_norm, 1, 2, 0, 1, Gtk::SHRINK, Gtk::SHRINK, 0, 0);
	attach(*zoom_fit, 2, 3, 0, 1, Gtk::SHRINK, Gtk::SHRINK, 0, 0);
	attach(*zoom_in, 3, 4, 0, 1, Gtk::SHRINK, Gtk::SHRINK, 0, 0);
}

Gtk::Button *
ZoomDial::create_icon(Gtk::IconSize size, const Gtk::BuiltinStockID & stockid,
		const char * tooltip)
{
	Gtk::Button *button = manage(new class Gtk::Button());
	Gtk::Image *icon = manage(new Gtk::Image(stockid, size));
	button->add(*icon);
	button->set_tooltip_text(tooltip);
	icon->set_padding(0, 0);
	icon->show();
	button->set_relief(Gtk::RELIEF_NONE);
	button->show();

	return button;
}

