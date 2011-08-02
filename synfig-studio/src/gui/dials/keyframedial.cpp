/* === S Y N F I G ========================================================= */
/*!	\file keyframedial.cpp
**	\brief Template File
**
**	$Id$
**
**	\legal
**	Copyright (c) 2002-2005 Robert B. Quattlebaum Jr., Adrian Bentley
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

/* === H E A D E R S ======================================================= */

#ifdef USING_PCH
#	include "pch.h"
#else
#ifdef HAVE_CONFIG_H
#	include <config.h>
#endif

#include "keyframedial.h"
#include <gtkmm/image.h>
#include <gtkmm/stock.h>

#endif

/* === U S I N G =========================================================== */

using namespace std;
using namespace studio;

/* === M A C R O S ========================================================= */

/* === G L O B A L S ======================================================= */

/* === P R O C E D U R E S ================================================= */

/* === M E T H O D S ======================================================= */

KeyFrameDial::KeyFrameDial(): Gtk::Table(1, 3, false)
{
	Gtk::IconSize iconsize = Gtk::IconSize::from_name("synfig-small_icon_16x16");

	seek_prev_keyframe = create_icon(iconsize, Gtk::Stock::GO_BACK, _("Previous keyframe"));
	seek_next_keyframe = create_icon(iconsize, Gtk::Stock::GO_FORWARD, _("Next keyframe"));
	lock_keyframe = create_icon(Gtk::ICON_SIZE_BUTTON, "synfig-keyframe_lock_all",_("All keyframes locked"));

	attach(*seek_prev_keyframe, 0, 1, 0, 1, Gtk::SHRINK, Gtk::SHRINK, 0, 0);
	attach(*seek_next_keyframe, 1, 2, 0, 1, Gtk::SHRINK, Gtk::SHRINK, 0, 0);
	attach(*lock_keyframe, 2, 3, 0, 1, Gtk::SHRINK, Gtk::SHRINK, 0, 0);
}

Gtk::Button *
KeyFrameDial::create_icon(Gtk::IconSize iconsize, const char * stockid,
		const char * tooltip)
{
	iconsize = Gtk::IconSize::from_name("synfig-small_icon_16x16");
	Gtk::Image *icon = manage(new Gtk::Image(Gtk::StockID(stockid), iconsize));
	Gtk::Button *button = manage(new class Gtk::Button());
	button->add(*icon);
	tooltips.set_tip(*button, tooltip);
	icon->set_padding(0, 0);
	icon->show();
	button->set_relief(Gtk::RELIEF_NONE);
	button->show();

	return button;
}

Gtk::Button *
KeyFrameDial::create_icon(Gtk::IconSize iconsize, const Gtk::BuiltinStockID & stockid,
		const char * tooltip)
{
	iconsize = Gtk::IconSize::from_name("synfig-small_icon_16x16");
	Gtk::Image *icon = manage(new Gtk::Image(Gtk::StockID(stockid), iconsize));
	Gtk::Button *button = manage(new class Gtk::Button());
	button->add(*icon);
	tooltips.set_tip(*button, tooltip);
	icon->set_padding(0, 0);
	icon->show();
	button->set_relief(Gtk::RELIEF_NONE);
	button->show();

	return button;
}
