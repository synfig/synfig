/* === S Y N F I G ========================================================= */
/*!	\file toggleducksdial.cpp
**	\brief Template File
**
**	\legal
**	Copyright (c) 2002-2005 Robert B. Quattlebaum Jr., Adrian Bentley
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

/* === H E A D E R S ======================================================= */

#ifdef USING_PCH
#	include "pch.h"
#else
#ifdef HAVE_CONFIG_H
#	include <config.h>
#endif

#include "toggleducksdial.h"
#include <gui/localization.h>

#endif

/* === U S I N G =========================================================== */

using namespace studio;

/* === M A C R O S ========================================================= */

/* === G L O B A L S ======================================================= */

/* === P R O C E D U R E S ================================================= */

/* === M E T H O D S ======================================================= */
static void
init_label_button(Gtk::ToggleToolButton &button, const std::string& icon_name, const std::string& label, const std::string& tooltip, const std::string& action)
{
	button.set_label(label);
	button.set_tooltip_text(tooltip);
	button.set_icon_name(icon_name);
	gtk_actionable_set_action_name(GTK_ACTIONABLE(button.gobj()), action.c_str());
	button.show();
}

ToggleDucksDial::ToggleDucksDial(const Gtk::IconSize &size)
{
	init_label_button(ducks_position, "duck_position_icon", _("Position handles"), _("Toggle position handles"), "doc.mask-position-ducks");
	init_label_button(ducks_vertex,   "duck_vertex_icon",   _("Vertex handles"),   _("Toggle vertex handles"), "doc.mask-vertex-ducks");
	init_label_button(ducks_tangent,  "duck_tangent_icon",  _("Tangent handles"),  _("Toggle tangent handles"), "doc.mask-tangent-ducks");
	init_label_button(ducks_radius,   "duck_radius_icon",   _("Radius handles"),   _("Toggle radius handles"), "doc.mask-radius-ducks");
	init_label_button(ducks_width,    "duck_width_icon",    _("Width handles"),    _("Toggle width handles"), "doc.mask-width-ducks");
	init_label_button(ducks_angle,    "duck_angle_icon",    _("Angle handles"),    _("Toggle angle handles"), "doc.mask-angle-ducks");
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
ToggleDucksDial::update_toggles(Duck::Type mask)
{
	ducks_position. set_active((mask & Duck::TYPE_POSITION));
	ducks_vertex  . set_active((mask & Duck::TYPE_VERTEX));
	ducks_tangent . set_active((mask & Duck::TYPE_TANGENT));
	ducks_radius  . set_active((mask & Duck::TYPE_RADIUS));
	ducks_width   . set_active((mask & Duck::TYPE_WIDTH));
	ducks_angle   . set_active((mask & Duck::TYPE_ANGLE));
}
