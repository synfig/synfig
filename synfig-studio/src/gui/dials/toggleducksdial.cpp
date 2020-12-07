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

#include <gui/localization.h>

#endif

/* === U S I N G =========================================================== */

using namespace std;
using namespace studio;

/* === M A C R O S ========================================================= */

/* === G L O B A L S ======================================================= */

/* === P R O C E D U R E S ================================================= */

/* === M E T H O D S ======================================================= */

ToggleDucksDial::ToggleDucksDial(const Gtk::IconSize &size)
{
	init_label_button(ducks_position, size, "synfig-toggle_duck_position", _("Position handles"), _("Toggle position handles"));
	init_label_button(ducks_vertex,   size, "synfig-toggle_duck_vertex",   _("Vertex handles"),   _("Toggle vertex handles"));
	init_label_button(ducks_tangent,  size, "synfig-toggle_duck_tangent",  _("Tangent handles"),  _("Toggle tangent handles"));
	init_label_button(ducks_radius,   size, "synfig-toggle_duck_radius",   _("Radius handles"),   _("Toggle radius handles"));
	init_label_button(ducks_width,    size, "synfig-toggle_duck_width",    _("Width handles"),    _("Toggle width handles"));
	init_label_button(ducks_angle,    size, "synfig-toggle_duck_angle",    _("Angle handles"),    _("Toggle angle handles"));
}

void
ToggleDucksDial::insert_to_toolbar(Gtk::Toolbar &toolbar, int index)
{
	if (index < 0) index = toolbar.get_n_items();

	// reverse order
	toolbar.insert(ducks_angle,    index);
	toolbar.insert(ducks_width,    index);
	toolbar.insert(ducks_radius,   index);
	toolbar.insert(ducks_tangent,  index);
	toolbar.insert(ducks_vertex,   index);
	toolbar.insert(ducks_position, index);
}

void
ToggleDucksDial::remove_from_toolbar(Gtk::Toolbar &toolbar)
{
	toolbar.remove(ducks_position);
	toolbar.remove(ducks_vertex);
	toolbar.remove(ducks_tangent);
	toolbar.remove(ducks_radius);
	toolbar.remove(ducks_width);
	toolbar.remove(ducks_angle);
}

void
ToggleDucksDial::init_label_button(Gtk::ToggleToolButton &button, Gtk::IconSize iconsize, const char *stockid, const char *label, const char *tooltip)
{
	Gtk::Image *icon = manage(new Gtk::Image(Gtk::StockID(stockid), iconsize));
	icon->set_padding(0, 0);
	icon->show();

	button.set_label(label);
	button.set_tooltip_text(tooltip);
	button.set_icon_widget(*icon);
	button.show();
}

void
ToggleDucksDial::update_toggles(Duck::Type mask)
{
	ducks_position. set_active((mask & Duck::TYPE_POSITION));
	ducks_vertex  . set_active((mask & Duck::TYPE_VERTEX));
	ducks_tangent . set_active((mask & Duck::TYPE_TANGENT));
	ducks_radius  . set_active((mask & Duck::TYPE_RADIUS));
	ducks_width   . set_active((mask & Duck::TYPE_WIDTH));
	ducks_angle   . set_active((mask & Duck::TYPE_ANGLE));
}
