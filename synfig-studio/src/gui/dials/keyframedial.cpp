/* === S Y N F I G ========================================================= */
/*!	\file keyframedial.cpp
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

#include "keyframedial.h"

#include <gui/localization.h>

#endif

/* === U S I N G =========================================================== */

using namespace studio;

/* === M A C R O S ========================================================= */

/* === G L O B A L S ======================================================= */

/* === P R O C E D U R E S ================================================= */

/* === M E T H O D S ======================================================= */

static Gtk::ToggleButton*
create_toggle_button(const std::string& icon_name, const std::string& tooltip)
{
	Gtk::ToggleButton *button = manage(new class Gtk::ToggleButton());
	button->set_tooltip_text(tooltip);
	button->set_image_from_icon_name(icon_name);
	button->set_relief(Gtk::RELIEF_NONE);
	button->set_active();
	button->show();

	return button;
}

void KeyFrameDial::on_mode_changed(synfigapp::EditMode mode)
{
	if (mode & synfigapp::EditMode::MODE_ANIMATE_FUTURE)
	{
		toggle_keyframe_future->set_image_from_icon_name("keyframe_lock_future_on_icon");
		toggle_keyframe_future->set_tooltip_text(_("Unlock future keyframes"));
		toggle_keyframe_future->set_active(true);
	}
	else
	{
		toggle_keyframe_future->set_image_from_icon_name("keyframe_lock_future_off_icon");
		toggle_keyframe_future->set_tooltip_text(_("Lock future keyframes"));
		toggle_keyframe_future->set_active(false);
	}

	if (mode & synfigapp::EditMode::MODE_ANIMATE_PAST)
	{
		toggle_keyframe_past->set_image_from_icon_name("keyframe_lock_past_on_icon");
		toggle_keyframe_past->set_tooltip_text(_("Unlock past keyframes"));
		toggle_keyframe_past->set_active(true);
	}
	else
	{
		toggle_keyframe_past->set_image_from_icon_name("keyframe_lock_past_off_icon");
		toggle_keyframe_past->set_tooltip_text(_("Lock past keyframes"));
		toggle_keyframe_past->set_active(false);
	}
}
KeyFrameDial::KeyFrameDial(): Gtk::Box(Gtk::Orientation::ORIENTATION_HORIZONTAL, 1)
{
	toggle_keyframe_past = create_toggle_button("keyframe_lock_past_on_icon",_("Unlock past keyframe"));
	toggle_keyframe_future = create_toggle_button("keyframe_lock_future_on_icon",_("Unlock future keyframe"));
	add(*toggle_keyframe_past);
	add(*toggle_keyframe_future);
}
