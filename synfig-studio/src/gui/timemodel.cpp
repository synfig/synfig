/* === S Y N F I G ========================================================= */
/*!	\file timemodel.cpp
**	\brief TimeModel Implementation File
**
**	$Id$
**
**	\legal
**	Copyright (c) 2004 Adrian Bentley
**	......... ... 2018 Ivan Mahonin
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

#include <gui/timemodel.h>

#include <algorithm>
#include <gui/helpers.h>

#endif

/* === U S I N G =========================================================== */

using namespace synfig;
using namespace studio;

/* === M A C R O S ========================================================= */

/* === G L O B A L S ======================================================= */

/* === P R O C E D U R E S ================================================= */

/* === M E T H O D S ======================================================= */

/* === E N T R Y P O I N T ================================================= */

TimeModel::TimeModel():
	in_sync(),
	fps(),
	play_bounds_enabled(),
	play_repeat(),
	full_time_adjustment_(Gtk::Adjustment::create(0.0, 0.0, 0.0)),
	scroll_time_adjustment_(Gtk::Adjustment::create(0.0, 0.0, 0.0)),
	play_bounds_adjustment_(Gtk::Adjustment::create(0.0, 0.0, 0.0))
{
	full_time_adjustment_->signal_changed().connect(
		sigc::bind(sigc::mem_fun(*this, &TimeModel::on_changed), &full_time_adjustment_) );
	full_time_adjustment_->signal_value_changed().connect(
		sigc::bind(sigc::mem_fun(*this, &TimeModel::on_value_changed), &full_time_adjustment_) );

	scroll_time_adjustment_->signal_changed().connect(
		sigc::bind(sigc::mem_fun(*this, &TimeModel::on_changed), &scroll_time_adjustment_) );
	scroll_time_adjustment_->signal_value_changed().connect(
		sigc::bind(sigc::mem_fun(*this, &TimeModel::on_value_changed), &scroll_time_adjustment_) );

	play_bounds_adjustment_->signal_changed().connect(
		sigc::bind(sigc::mem_fun(*this, &TimeModel::on_changed), &play_bounds_adjustment_) );
	play_bounds_adjustment_->signal_value_changed().connect(
		sigc::bind(sigc::mem_fun(*this, &TimeModel::on_value_changed), &play_bounds_adjustment_) );
}

void
TimeModel::on_changed(Glib::RefPtr<Gtk::Adjustment> *source)
{
	if (in_sync) return;
	if (source == &play_bounds_adjustment_)
		set_play_bounds(Time((*source)->get_lower()), Time((*source)->get_upper()));
	sync();
}

void
TimeModel::on_value_changed(Glib::RefPtr<Gtk::Adjustment> *source)
{
	if (in_sync) return;
	if (source == &scroll_time_adjustment_) {
		Time lower((*source)->get_value());
		Time upper = lower + get_page_size();
		set_visible_bounds(lower, upper);
	} else
	if (source == &play_bounds_adjustment_) {
		Time t = round_time( Time((*source)->get_value()) );
		if (play_time == play_bounds_lower && t <= play_bounds_lower) return;
		if (play_time == play_bounds_upper && t >= play_bounds_upper) return;
		set_time(t);
	} else {
		set_time(Time((*source)->get_value()));
	}
	sync();
}

void
TimeModel::sync()
{
	in_sync = true;
	try {
		double precision = real_low_precision<double>();

		Time step_increment = get_step_increment();
		Time page_increment = get_page_increment();
		Time page_size = get_page_size();

		// raise events only when all changes will done
		FreezeNotify freeze_full_time(full_time_adjustment());
		FreezeNotify freeze_scroll_time(scroll_time_adjustment());
		FreezeNotify freeze_play_bounds(play_bounds_adjustment());

		configure_adjustment(
			full_time_adjustment(),
			(double)time,
			(double)lower,
			(double)upper,
			(double)step_increment,
			(double)page_increment,
			(double)page_size,
			precision );

		configure_adjustment(
			scroll_time_adjustment(),
			(double)visible_lower,
			(double)lower,
			(double)upper,
			(double)step_increment,
			(double)page_increment,
			(double)page_size,
			precision );

		configure_adjustment(
			play_bounds_adjustment(),
			(double)time,
			(double)play_bounds_lower,
			(double)play_bounds_upper,
			(double)step_increment,
			(double)step_increment,
			0.0,
			precision );
	} catch(...) {
		in_sync = false;
		throw;
	}
	in_sync = false;
}

bool
TimeModel::set_time_silent(Time time, bool *is_play_time_changed)
{
	time = round_time(time);
	Time t = std::max(lower, std::min(upper, time));
	Time pt = std::max(play_bounds_lower, std::min(play_bounds_upper, t));

	if (this->play_time != pt) {
		this->play_time = pt;
		if (is_play_time_changed) *is_play_time_changed = true;
	}

	if (this->time == t) return false;

	this->time = t;
	return true;
}

bool
TimeModel::set_bounds_silent(Time lower, Time upper, float fps)
{
	if (approximate_less_or_equal_lp(fps, 0.f)) fps = 0.f;

	// try to keep minimum one frame range
	Time step = Time(fps ? 1.0/fps : 0.0);
	lower = round_time(lower);
	upper = std::max(round_time(lower + step), round_time(upper));

	if (this->lower == lower && this->upper == upper && this->fps == fps)
		return false;

	Time play_bounds_lower = this->play_bounds_lower <= this->lower ? lower : this->play_bounds_lower;
	Time play_bounds_upper = this->play_bounds_upper >= this->upper ? upper : this->play_bounds_upper;

	this->lower = lower;
	this->upper = upper;
	this->fps = fps;

	set_visible_bounds_silent(visible_lower, visible_upper);
	set_play_bounds_silent(play_bounds_lower, play_bounds_upper, play_bounds_enabled, play_repeat);
	set_time_silent(time, NULL);
	return true;
}

bool
TimeModel::set_visible_bounds_silent(Time lower, Time upper)
{
	// try to keep duration, minimum duration is a one frame
	Time duration = std::max(Time(fps ? 1.0/fps : 0.0), upper - lower);

	// this->lower bound have priority when this->upper < this->lower
	lower = std::max(this->lower, std::min(this->upper, lower));
	upper = std::max(lower, std::min(this->upper, upper));

	if (duration > Time() && this->lower < this->upper) {
		Time t = lower + duration;
		if (t > upper)
			upper = std::max(lower, std::min(this->upper, t));
		t = upper - duration;
		if (t < lower)
			lower = std::max(this->lower, t);
	}

	if (visible_lower == lower && visible_upper == upper) return false;

	visible_lower = lower;
	visible_upper = upper;
	return true;
}

bool
TimeModel::set_play_bounds_silent(Time lower, Time upper, bool enabled, bool repeat) {
	// this->lower bound have priority when this->upper < this->lower
	lower = std::max(this->lower, std::min(this->upper, round_time(lower)));
	upper = std::max(lower, std::min(this->upper, round_time(upper)));

	// try to keep minimum two frame range
	if (fps && this->lower < this->upper) {
		Time step = Time(2.0/fps);
		Time t = round_time(lower + step);
		if (t > upper)
			upper = std::max(lower, std::min(this->upper, t));
		t = round_time(upper - step);
		if (t < lower)
			lower = std::max(this->lower, t);
	}

	if ( play_bounds_lower == lower && play_bounds_upper == upper
	  && play_bounds_enabled == enabled && play_repeat == repeat )
		return false;

	play_bounds_lower = lower;
	play_bounds_upper = upper;
	play_bounds_enabled = enabled;
	play_repeat = repeat;
	set_time_silent(time, NULL);
	return true;
}

void
TimeModel::set_play_bounds_lower(const Time &lower) {
	// upper bound will be fixed to be >= (lower + two frames)
	Time step = Time(fps ? 2.0/fps : 0.0);
	Time t = round_time(lower);
	set_play_bounds(t, std::max(play_bounds_upper, t + step));
}

void
TimeModel::set_play_bounds_upper(const Time &upper) {
	// lower bound will be fixed to be <= (upper - two frames)
	Time step = Time(fps ? 2.0/fps : 0.0);
	Time t = round_time(upper);
	set_play_bounds(std::min(play_bounds_lower, t - step), t);
}

void
TimeModel::set_play_repeat(bool repeat)
{
	if (play_repeat == repeat) return;
	play_repeat = repeat;
	play_bounds_changed();
}

void
TimeModel::set_play_bounds_enabled(bool enabled)
{
	if (play_bounds_enabled == enabled) return;
	play_bounds_enabled = enabled;
	play_bounds_changed();
}

Real
TimeModel::get_zoom() const
	{ return (Real)get_size()/(Real)get_page_size(); }

void
TimeModel::set_zoom(Real x, const Time &center)
{
	x = std::max(Real(1.0), x);
	Time size = get_size();
	Time page_size = size/x;
	Time lower = center - page_size*0.5;
	Time upper = lower + page_size;
	set_visible_bounds(lower, upper);
}

void
TimeModel::zoom(Real x, const Time &center)
{
	// relative zoom in/out
	x = std::max(real_low_precision<Real>(), x);
	x = 1.0/x;
	Time lower = (visible_lower - center)*x + center;
	Time upper = (visible_upper - center)*x + center;
	set_visible_bounds(lower, upper);
}

bool
TimeModel::almost_equal(const synfig::Time &a, const synfig::Time &b, const synfig::Time &range) const
	{ return (a < b ? b - a : a - b) <= std::max(Time(), range) + get_frame_duration()*0.49999; }

Time
TimeModel::get_frame_duration() const
	{ return fps ? Time(1.0/fps).round(fps) : Time(); }

Time
TimeModel::get_page_increment() const
{
	Time s = get_step_increment();
	Time p = get_page_size();
	return std::max(s, std::min(p*0.8, p - s));
}
