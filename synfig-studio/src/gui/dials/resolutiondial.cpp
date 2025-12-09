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

#include <gui/localization.h>

#endif

/* === U S I N G =========================================================== */

using namespace studio;

/* === M A C R O S ========================================================= */

/* === G L O B A L S ======================================================= */

/* === P R O C E D U R E S ================================================= */

static void
init_button(Gtk::ToolButton& button, const std::string& icon_name, const std::string& label, const std::string& tooltip, const std::string& action)
{
	button.set_icon_name(icon_name);
	button.set_label(label);
	button.set_tooltip_text(tooltip);
	button.show();

	gtk_actionable_set_action_name(GTK_ACTIONABLE(button.gobj()), action.c_str());
}

static void
init_toggle_button(Gtk::ToggleToolButton& button, const std::string& label, const std::string& tooltip, const std::string& action)
{
	// For label left/right padding
	button.set_name("low-resolution");

	button.set_label(label);
	button.set_tooltip_text(tooltip);
	button.set_is_important(true);
	button.show();

	gtk_actionable_set_action_name(GTK_ACTIONABLE(button.gobj()), action.c_str());
}

/* === M E T H O D S ======================================================= */

ResolutionDial::ResolutionDial()
{
	init_button(increase_resolution, "incr_resolution_icon", _("Increase Resolution"), _("Increase Display Resolution"), "doc.decrease-lowres-pixel-size");
	init_button(decrease_resolution, "decr_resolution_icon", _("Decrease Resolution"), _("Decrease Display Resolution"), "doc.increase-lowres-pixel-size");
	init_toggle_button(use_low_resolution, _("Low Res"), _("Use Low Resolution when enabled"), "doc.toggle-low-resolution");
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
