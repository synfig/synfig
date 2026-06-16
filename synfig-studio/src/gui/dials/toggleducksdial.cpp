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

#include <gui/actiondatabase.h>
#include <gui/actionwidgethelper.h>
#include <gui/app.h>
#include <gui/localization.h>

#endif

/* === U S I N G =========================================================== */

using namespace studio;

/* === M A C R O S ========================================================= */

/* === G L O B A L S ======================================================= */

/* === P R O C E D U R E S ================================================= */

/* === M E T H O D S ======================================================= */

ToggleDucksDial::ToggleDucksDial(const std::string& action_prefix)
{
	const std::string action_position_duck = action_prefix.empty() ? "" : action_prefix + "." + "mask-position-ducks";
	const std::string action_vertex_duck   = action_prefix.empty() ? "" : action_prefix + "." + "mask-vertex-ducks";
	const std::string action_tangent_duck  = action_prefix.empty() ? "" : action_prefix + "." + "mask-tangent-ducks";
	const std::string action_radius_duck   = action_prefix.empty() ? "" : action_prefix + "." + "mask-radius-ducks";
	const std::string action_width_duck    = action_prefix.empty() ? "" : action_prefix + "." + "mask-width-ducks";
	const std::string action_angle_duck    = action_prefix.empty() ? "" : action_prefix + "." + "mask-angle-ducks";
	if (App::get_action_database()->has(action_position_duck)) {
		ActionWidgetHelper::init_icon_only_toolbutton(ducks_position, action_position_duck);
		ActionWidgetHelper::init_icon_only_toolbutton(ducks_vertex,   action_vertex_duck);
		ActionWidgetHelper::init_icon_only_toolbutton(ducks_tangent,  action_tangent_duck);
		ActionWidgetHelper::init_icon_only_toolbutton(ducks_radius,   action_radius_duck);
		ActionWidgetHelper::init_icon_only_toolbutton(ducks_width,    action_width_duck);
		ActionWidgetHelper::init_icon_only_toolbutton(ducks_angle,    action_angle_duck);
	} else {
		ActionWidgetHelper::init_toolbutton(ducks_position, action_position_duck, "duck_position_icon", _("Position handles"), _("Toggle position handles"));
		ActionWidgetHelper::init_toolbutton(ducks_vertex,   action_vertex_duck,   "duck_vertex_icon",   _("Vertex handles"),   _("Toggle vertex handles"));
		ActionWidgetHelper::init_toolbutton(ducks_tangent,  action_tangent_duck,  "duck_tangent_icon",  _("Tangent handles"),  _("Toggle tangent handles"));
		ActionWidgetHelper::init_toolbutton(ducks_radius,   action_radius_duck,   "duck_radius_icon",   _("Radius handles"),   _("Toggle radius handles"));
		ActionWidgetHelper::init_toolbutton(ducks_width,    action_width_duck,    "duck_width_icon",    _("Width handles"),    _("Toggle width handles"));
		ActionWidgetHelper::init_toolbutton(ducks_angle,    action_angle_duck,    "duck_angle_icon",    _("Angle handles"),    _("Toggle angle handles"));
	}
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
