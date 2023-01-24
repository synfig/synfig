/* === S Y N F I G ========================================================= */
/*!	\file dials/framedial.cpp
**	\brief Template File
**
**	\legal
**	Copyright (c) 2002-2005 Robert B. Quattlebaum Jr., Adrian Bentley
**  Copyright (c) 2008 Chris Moore
**  Copyright (c) 2009 Gerco Ballintijn
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

#include "framedial.h"

#include <gtkmm/separator.h>

#include <gui/localization.h>

#endif

/* === U S I N G =========================================================== */

using namespace studio;

/* === M A C R O S ========================================================= */

/* === G L O B A L S ======================================================= */

/* === P R O C E D U R E S ================================================= */

/* === M E T H O D S ======================================================= */

FrameDial::FrameDial():
	seek_begin         (create_button("animate_seek_begin_icon"         , _("Seek to begin")            )),
	seek_prev_keyframe (create_button("animate_seek_prev_keyframe_icon" , _("Seek to previous keyframe"))),
	seek_prev_frame    (create_button("animate_seek_prev_frame_icon"    , _("Seek to previous frame")   )),
	play               (create_button("animate_play_icon"               , _("Play")                     )),
	pause              (create_button("animate_pause_icon"              , _("Pause")                    )),
	seek_next_frame    (create_button("animate_seek_next_frame_icon"    , _("Seek to next frame")       )),
	seek_next_keyframe (create_button("animate_seek_next_keyframe_icon" , _("Seek to next keyframe")    )),
	seek_end           (create_button("animate_seek_end_icon"           , _("Seek to end")              )),
	end_time           (create_end_time_entry(                              _("End Time")                 )),
	repeat             (create_toggle("animate_loop_icon"               , _("Loop")                     , true)),
	bound_lower        (create_button("animate_bound_lower_icon"        , _("Set lower playback bound") , true)),
	bounds_enable      (create_toggle("animate_bounds_icon"             , _("Enable playback bounds")   )),
	bound_upper        (create_button("animate_bound_upper_icon"        , _("Set upper playback bound") ))
{
	repeat->signal_toggled().connect(
		sigc::mem_fun(*this, &FrameDial::on_repeat_toggled) );
	bounds_enable->signal_toggled().connect(
		sigc::mem_fun(*this, &FrameDial::on_bounds_toggled) );
	toggle_play_pause_button(false);
}

void
FrameDial::on_repeat_toggled()
	{ signal_repeat()(repeat->get_active()); }

void
FrameDial::on_bounds_toggled()
	{ signal_bounds_enable()(bounds_enable->get_active()); }

void
FrameDial::create_separator()
{
	auto separator = manage(new Gtk::Separator());
	separator->show();
	add(*separator);
}

void
FrameDial::init_button(Gtk::Button& button, const std::string& icon_name, const std::string& tooltip)
{
	button.set_image_from_icon_name(icon_name);
	button.set_tooltip_text(tooltip);
	button.set_relief(Gtk::RELIEF_NONE);
	button.show();
	add(button);
}

Gtk::Button*
FrameDial::create_button(const std::string& icon_name, const std::string& tooltip, bool separator)
{
	if (separator) create_separator();
	Gtk::Button *button = manage(new class Gtk::Button());
	init_button(*button, icon_name, tooltip);
	return button;
}

Gtk::ToggleButton*
FrameDial::create_toggle(const std::string& icon_name, const std::string& tooltip, bool separator)
{
	if (separator) create_separator();
	Gtk::ToggleButton *toggle = manage(new class Gtk::ToggleButton());
	init_button(*toggle, icon_name, tooltip);
	return toggle;
}

Widget_Time*
FrameDial::create_end_time_entry(const char *tooltip)
{
	end_time = manage(new Widget_Time());
	end_time->set_width_chars(6);
	end_time->set_tooltip_text(tooltip);
	end_time->show();
	add(*end_time);
	return end_time;
}

void
FrameDial::toggle_play_pause_button(bool is_playing)
{
	if (is_playing) {
		play->hide();
		pause->show();
	} else {
		pause->hide();
		play->show();
	}
}

void
FrameDial::toggle_repeat(bool enable)
	{ repeat->set_active(enable); }

void
FrameDial::toggle_bounds_enable(bool enable)
	{ bounds_enable->set_active(enable); }

void
FrameDial::set_end_time(float fps, float value)
{
	end_time->set_fps(fps);
	end_time->set_value(value);
}

float
FrameDial::get_end_time()
{
	return end_time->get_value();
}

void
FrameDial::on_end_time_widget_changed()
{
	end_time->set_position(-1);
}
