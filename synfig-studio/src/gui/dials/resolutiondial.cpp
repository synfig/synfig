/* === S Y N F I G ========================================================= */
/*!	\file resolutiondial.cpp
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

#include "resolutiondial.h"

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

ResolutionDial::ResolutionDial(const std::string& action_prefix)
{
	const std::string action_increase_resolution = action_prefix.empty() ? "" : action_prefix + "." + "decrease-low-res-pixel-size";
	const std::string action_decrease_resolution = action_prefix.empty() ? "" : action_prefix + "." + "increase-low-res-pixel-size";
	const std::string action_toggle_low_res      = action_prefix.empty() ? "" : action_prefix + "." + "toggle-low-res";
	if (App::get_action_database()->has(action_increase_resolution)) {
		ActionWidgetHelper::init_icon_only_toolbutton(increase_resolution, action_increase_resolution);
		ActionWidgetHelper::init_icon_only_toolbutton(decrease_resolution, action_decrease_resolution);
		ActionWidgetHelper::init_toolbutton(use_low_resolution, action_toggle_low_res);
	} else {
		ActionWidgetHelper::init_toolbutton(increase_resolution, action_increase_resolution, "incr_resolution_icon", _("Increase Resolution"), _("Increase Display Resolution"));
		ActionWidgetHelper::init_toolbutton(decrease_resolution, action_decrease_resolution, "decr_resolution_icon", _("Decrease Resolution"), _("Decrease Display Resolution"));
		ActionWidgetHelper::init_toolbutton(use_low_resolution, action_toggle_low_res, "", _("Low Res"), _("Use Low Resolution when enabled"));
	}
}

void
ResolutionDial::insert_to_toolbar(Gtk::Toolbar &toolbar, int index)
{
	if (index < 0) index = toolbar.get_n_items();

	// reverse order
	toolbar.insert(increase_resolution, index);
	toolbar.insert(use_low_resolution,  index);
	toolbar.insert(decrease_resolution, index);
}

void
ResolutionDial::remove_from_toolbar(Gtk::Toolbar &toolbar)
{
	toolbar.remove(decrease_resolution);
	toolbar.remove(use_low_resolution);
	toolbar.remove(increase_resolution);
}

void
ResolutionDial::update_lowres(bool flag)
{
	use_low_resolution.set_active(flag);
}
