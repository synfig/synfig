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

static void
init_button(Gtk::Button& button, const std::string& action)
{
	ActionWidgetHelper::init_icon_only_button(button, action);
	button.set_relief(Gtk::RELIEF_NONE);
}

static void
init_button(Gtk::Button& button, const std::string& action, const std::string& icon, const std::string& tooltip)
{
	ActionWidgetHelper::init_button(button, action, icon, "", tooltip);
	button.set_relief(Gtk::RELIEF_NONE);
}

/* === M E T H O D S ======================================================= */

FrameDial::FrameDial(const std::string& action_prefix)
{
	const std::string action_seek_begin          = action_prefix.empty() ? "" : action_prefix + "." + "seek-begin";
	const std::string action_seek_prev_keyframe  = action_prefix.empty() ? "" : action_prefix + "." + "jump-prev-keyframe";
	const std::string action_seek_prev_frame     = action_prefix.empty() ? "" : action_prefix + "." + "seek-prev-frame";
	const std::string action_play                = action_prefix.empty() ? "" : action_prefix + "." + "play";
	const std::string action_pause               = action_prefix.empty() ? "" : action_prefix + "." + "pause";
	const std::string action_seek_next_frame     = action_prefix.empty() ? "" : action_prefix + "." + "seek-next-frame";
	const std::string action_seek_next_keyframe  = action_prefix.empty() ? "" : action_prefix + "." + "jump-next-keyframe";
	const std::string action_seek_end            = action_prefix.empty() ? "" : action_prefix + "." + "seek-end";
	const std::string action_repeat              = action_prefix.empty() ? "" : action_prefix + "." + "toggle-animation-loop";
	const std::string action_bound_lower         = action_prefix.empty() ? "" : action_prefix + "." + "set-lower-bound";
	const std::string action_toggle_bound        = action_prefix.empty() ? "" : action_prefix + "." + "toggle-animation-bounds";
	const std::string action_bound_upper         = action_prefix.empty() ? "" : action_prefix + "." + "set-upper-bound";

	if (App::get_action_database()->has(action_seek_begin)) {
		init_button(seek_begin, action_seek_begin);
		init_button(seek_prev_keyframe, action_seek_prev_keyframe);
		init_button(seek_prev_frame, action_seek_prev_frame);
		init_button(play, action_play);
		init_button(pause, action_pause);
		init_button(seek_next_frame, action_seek_next_frame);
		init_button(seek_next_keyframe, action_seek_next_keyframe);
		init_button(seek_end, action_seek_end);
		// init_end_time_entry(end_time);
		// create_separator();
		init_button(repeat, action_repeat);
		// create_separator();
		init_button(bound_lower, action_bound_lower);
		init_button(bounds_enable, action_toggle_bound);
		init_button(bound_upper, action_bound_upper);
	} else {
		init_button(seek_begin, action_seek_begin, "animate_seek_begin_icon", _("Seek to begin"));
		init_button(seek_prev_keyframe, action_seek_prev_keyframe, "animate_seek_prev_keyframe_icon" , _("Seek to previous keyframe"));
		init_button(seek_prev_frame, action_seek_prev_frame, "animate_seek_prev_frame_icon", _("Seek to previous frame"));
		init_button(play, action_play, "animate_play_icon", _("Play"));
		init_button(pause, action_pause, "animate_pause_icon", _("Pause"));
		init_button(seek_next_frame, action_seek_next_frame, "animate_seek_next_frame_icon", _("Seek to next frame"));
		init_button(seek_next_keyframe, action_seek_next_keyframe, "animate_seek_next_keyframe_icon", _("Seek to next keyframe"));
		init_button(seek_end, action_seek_end, "animate_seek_end_icon", _("Seek to end"));
		// init_end_time_entry(end_time);
		// create_separator();
		init_button(repeat, action_repeat, "animate_loop_icon", _("Loop"));
		// create_separator();
		init_button(bound_lower, action_bound_lower, "animate_bound_lower_icon", _("Set lower playback bound"));
		init_button(bounds_enable, action_toggle_bound, "animate_bounds_icon", _("Enable playback bounds"));
		init_button(bound_upper, action_bound_upper, "animate_bound_upper_icon", _("Set upper playback bound"));

		repeat.signal_toggled().connect(
			sigc::mem_fun(*this, &FrameDial::on_repeat_toggled) );
		bounds_enable.signal_toggled().connect(
			sigc::mem_fun(*this, &FrameDial::on_bounds_toggled) );
	}

	seek_begin.set_relief(Gtk::RELIEF_NONE);
	add(seek_begin);
	add(seek_prev_keyframe);
	add(seek_prev_frame);
	add(play);
	add(pause);
	add(seek_next_frame);
	add(seek_next_keyframe);
	add(seek_end);
	init_end_time_entry(end_time);
	create_separator();
	add(repeat);
	create_separator();
	add(bound_lower);
	add(bounds_enable);
	add(bound_upper);

	show_all();
	toggle_play_pause_button(false);
}

void
FrameDial::on_repeat_toggled()
	{ signal_repeat()(repeat.get_active()); }

void
FrameDial::on_bounds_toggled()
	{ signal_bounds_enable()(bounds_enable.get_active()); }

void
FrameDial::create_separator()
{
	auto separator = manage(new Gtk::Separator());
	separator->show();
	add(*separator);
}

void
FrameDial::init_end_time_entry(Widget_Time& end_time)
{
	end_time.set_width_chars(6);
	end_time.set_tooltip_text(_("End Time"));
	end_time.show();
	add(end_time);
}

void
FrameDial::toggle_play_pause_button(bool is_playing)
{
	if (is_playing) {
		play.hide();
		pause.show();
	} else {
		pause.hide();
		play.show();
	}
}

void
FrameDial::toggle_repeat(bool enable)
	{ repeat.set_active(enable); }

void
FrameDial::toggle_bounds_enable(bool enable)
	{ bounds_enable.set_active(enable); }

void
FrameDial::set_end_time(float fps, float value)
{
	end_time.set_fps(fps);
	end_time.set_value(value);
}

float
FrameDial::get_end_time()
{
	return end_time.get_value();
}

void
FrameDial::on_end_time_widget_changed()
{
	end_time.set_position(-1);
}
