/* === S Y N F I G ========================================================= */
/*!	\file jackdial.cpp
**	\brief Template File
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

/* === H E A D E R S ======================================================= */

#ifdef USING_PCH
#	include "pch.h"
#else
#ifdef HAVE_CONFIG_H
#	include <config.h>
#endif

#include "jackdial.h"

#include <gtkmm/image.h>
#include <gtkmm/stock.h>
#include <gtkmm/alignment.h>

#include <gui/localization.h>
#endif

/* === U S I N G =========================================================== */

using namespace std;
using namespace studio;

/* === M A C R O S ========================================================= */

/* === G L O B A L S ======================================================= */

/* === P R O C E D U R E S ================================================= */

/* === M E T H O D S ======================================================= */

JackDial::JackDial(): Gtk::Grid()
{
	Gtk::IconSize iconsize=Gtk::IconSize::from_name("synfig-small_icon_16x16");
	toggle_jack = create_icon(iconsize, "synfig-jack",_("Disable JACK"));
	offset = manage(new Widget_Time());
	offset->set_value(synfig::Time(0.0));
	offset->set_size_request(0,-1); // request horizontal shrink
	offset->set_width_chars(6);
	offset->set_tooltip_text(_("JACK Offset"));

	attach(*toggle_jack, 0, 0, 1, 1);
	attach(*offset, 1, 0, 1, 1);

	offset->hide();
#ifndef WITH_JACK
	offset->set_sensitive(false);
#endif
}

Gtk::ToggleButton *
JackDial::create_icon(Gtk::IconSize iconsize, const char *stockid, const char *tooltip)
{
	iconsize = Gtk::IconSize::from_name("synfig-small_icon_16x16");
	Gtk::Image *icon = manage(new Gtk::Image(Gtk::StockID(stockid), iconsize));
	Gtk::ToggleButton *button = manage(new class Gtk::ToggleButton());
	button->add(*icon);
	button->set_tooltip_text(tooltip);
	icon->set_padding(0, 0);
	icon->show();
	button->set_relief(Gtk::RELIEF_NONE);
	button->show();

	return button;
}
