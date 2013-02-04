/* === S Y N F I G ========================================================= */
/*!	\file toggleducksdial.cpp
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

#include "toggleducksdial.h"
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

ToggleDucksDial::ToggleDucksDial(Gtk::IconSize & size): Gtk::Table(1, 6, false)
{
	ducks_position = create_label_button(size, "synfig-toggle_duck_position", _("Toggle position handles"));
	ducks_vertex = create_label_button(size, "synfig-toggle_duck_vertex", _("Toggle vertex handles"));
	ducks_tangent = create_label_button(size, "synfig-toggle_duck_tangent", _("Toggle tangent handles"));
	ducks_radius = create_label_button(size, "synfig-toggle_duck_radius", _("Toggle radius handles"));
	ducks_width = create_label_button(size, "synfig-toggle_duck_width", _("Toggle width handles"));
	ducks_angle = create_label_button(size, "synfig-toggle_duck_angle", _("Toggle angle handles"));

	attach(*ducks_position, 0, 1, 0, 1, Gtk::SHRINK, Gtk::SHRINK, 0, 0);
	attach(*ducks_vertex, 1, 2, 0, 1, Gtk::SHRINK, Gtk::SHRINK, 0, 0);
	attach(*ducks_tangent, 2, 3, 0, 1, Gtk::SHRINK, Gtk::SHRINK, 0, 0);
	attach(*ducks_radius, 3, 4, 0, 1, Gtk::SHRINK, Gtk::SHRINK, 0, 0);
	attach(*ducks_width, 4, 5, 0, 1, Gtk::SHRINK, Gtk::SHRINK, 0, 0);
	attach(*ducks_angle, 5, 6, 0, 1, Gtk::SHRINK, Gtk::SHRINK, 0, 0);
}

Gtk::ToggleButton *
ToggleDucksDial::create_label_button(Gtk::IconSize iconsize, const char *stockid,
		const char * tooltip)
{
	Gtk::ToggleButton *tbutton = manage(new class Gtk::ToggleButton());
	Gtk::Image *icon = manage(new Gtk::Image(Gtk::StockID(stockid), iconsize));
	tbutton->set_tooltip_text(tooltip);
	tbutton->add(*icon);
	icon->set_padding(0, 0);
	icon->show();
	tbutton->set_relief(Gtk::RELIEF_NONE);
	tbutton->show();

	return tbutton;
}

void
ToggleDucksDial::update_toggles(Duck::Type mask)
{
	ducks_position-> set_active((mask & Duck::TYPE_POSITION));
	ducks_vertex  -> set_active((mask & Duck::TYPE_VERTEX));
	ducks_tangent -> set_active((mask & Duck::TYPE_TANGENT));
	ducks_radius  -> set_active((mask & Duck::TYPE_RADIUS));
	ducks_width   -> set_active((mask & Duck::TYPE_WIDTH));
	ducks_angle   -> set_active((mask & Duck::TYPE_ANGLE));
}
