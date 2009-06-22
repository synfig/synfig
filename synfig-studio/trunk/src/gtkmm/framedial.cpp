/* === S Y N F I G ========================================================= */
/*!	\file framedial.cpp
**	\brief Template File
**
**	$Id$
**
**	\legal
**	Copyright (c) 2002-2005 Robert B. Quattlebaum Jr., Adrian Bentley
**  Copyright (c) 2009 Gerco Ballintijn
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

#include "framedial.h"
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

FrameDial::FrameDial(): Gtk::Table(3, 1, false)
{
	Gtk::IconSize iconsize = Gtk::IconSize::from_name("synfig-small_icon");

	seek_begin = create_icon(iconsize, Gtk::Stock::MEDIA_PREVIOUS,
					_("Seek to Begin"));
	seek_prev_frame = create_icon(iconsize, Gtk::Stock::MEDIA_REWIND,
					_("Previous Frame"));
	seek_next_frame = create_icon(iconsize, Gtk::Stock::MEDIA_FORWARD,
					_("Next Frame"));
	seek_end = create_icon(iconsize, Gtk::Stock::MEDIA_NEXT,
					_("Seek to End"));

	attach(*seek_begin, 0, 1, 0, 1, Gtk::SHRINK, Gtk::SHRINK, 0, 0);
	attach(*seek_prev_frame, 1, 2, 0, 1, Gtk::SHRINK, Gtk::SHRINK, 0, 0);
	attach(*seek_next_frame, 2, 3, 0, 1, Gtk::SHRINK, Gtk::SHRINK, 0, 0);
	attach(*seek_end, 3, 4, 0, 1, Gtk::SHRINK, Gtk::SHRINK, 0, 0);
}

Gtk::Button *
FrameDial::create_icon(Gtk::IconSize size, const Gtk::BuiltinStockID & stockid,
		const char * tooltip)
{
	Gtk::Button *button = manage(new class Gtk::Button());
	Gtk::Image *icon = manage(new Gtk::Image(stockid, size));
	button->add(*icon);
	tooltips.set_tip(*button, tooltip);
	icon->set_padding(0, 0);
	icon->show();
	button->set_relief(Gtk::RELIEF_NONE);
	button->show();

	return button;
}

