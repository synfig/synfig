/* === S Y N F I G ========================================================= */
/*!	\file jackdial.cpp
**	\brief Template File
**
**	\legal
**	Copyright (c) 2002-2005 Robert B. Quattlebaum Jr., Adrian Bentley
**	Copyright (c) 2008 Chris Moore
**	Copyright (c) 2009 Gerco Ballintijn
**	Copyright (c) 2009 Carlos López
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

#include "jackdial.h"
#include <gui/localization.h>
#endif

/* === U S I N G =========================================================== */

using namespace studio;

/* === M A C R O S ========================================================= */

/* === G L O B A L S ======================================================= */

/* === P R O C E D U R E S ================================================= */

/* === M E T H O D S ======================================================= */

// TODO(ice0): duplicated code
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

void
JackDial::set_state(bool enabled)
{
	// start
	if (enabled) {
		toggle_jack_button->set_tooltip_text(_("Disable JACK"));
	} else {
		toggle_jack_button->set_tooltip_text(_("Enable JACK"));
	}
	toggle_jack_button->set_active(enabled);
	offset_widget->set_visible(enabled);
}

JackDial::JackDial(): Gtk::Box()
{
	offset_widget = manage(new Widget_Time());
	offset_widget->set_value(synfig::Time(0.0));
	//offset_widget->set_size_request(0,-1); // request horizontal shrink
	offset_widget->set_width_chars(6);
	offset_widget->set_tooltip_text(_("JACK Offset"));

	toggle_jack_button = create_toggle_button("jack_icon", "");
	set_state(false);

	add(*toggle_jack_button);
	add(*offset_widget);
	toggle_jack_button->set_margin_start(8);
	toggle_jack_button->set_margin_end(4);

	offset_widget->hide();
#ifndef WITH_JACK
	offset_widget->set_sensitive(false);
#endif
}
