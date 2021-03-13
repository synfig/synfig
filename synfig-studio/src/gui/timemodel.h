/* === S Y N F I G ========================================================= */
/*!	\file timemodel.h
**	\brief TimeModel Header
**
**	$Id$
**
**	\legal
**	Copyright (c) 2004 Adrian Bentley
**	......... ... 2018 Ivan Mahonin
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

/* === S T A R T =========================================================== */

#ifndef __SYNFIG_TIMEMODEL_H
#define __SYNFIG_TIMEMODEL_H

/* === H E A D E R S ======================================================= */

#include <ETL/handle>
#include <gtkmm/adjustment.h>
#include <list>
#include <synfig/real.h>
#include <synfig/time.h>

/* === M A C R O S ========================================================= */

/* === T Y P E D E F S ===================================================== */

/* === C L A S S E S & S T R U C T S ======================================= */

namespace studio {

//! Sets up an adjustment that controls
//!  - full time track
//!  - visible part of time track (when it zoomed/scrolled)
//!  - playing bounds at time track
class TimeModel: public etl::shared_object, public sigc::trackable
{
private:
	bool in_sync;

	float fps;
	bool play_bounds_enabled;
	bool play_repeat;
	synfig::Time time;
	synfig::Time play_time;
	synfig::Time lower;
	synfig::Time upper;
	synfig::Time visible_lower;
	synfig::Time visible_upper;
	synfig::Time play_bounds_lower;
	synfig::Time play_bounds_upper;

	Glib::RefPtr<Gtk::Adjustment> full_time_adjustment_;
	Glib::RefPtr<Gtk::Adjustment> scroll_time_adjustment_;
	Glib::RefPtr<Gtk::Adjustment> play_bounds_adjustment_;

	sigc::signal<void> signal_bounds_changed_;
	sigc::signal<void> signal_visible_changed_;
	sigc::signal<void> signal_play_bounds_changed_;
	sigc::signal<void> signal_time_changed_;
	sigc::signal<void> signal_play_time_changed_;
	sigc::signal<void> signal_changed_;

	void on_changed(Glib::RefPtr<Gtk::Adjustment> *source);
	void on_value_changed(Glib::RefPtr<Gtk::Adjustment> *source);

	//! put internal values to adjustments and emit the adjustments signals if need
	void sync();

	// all parameters will be edited infide functions so pass them by copy instead of const reference
	bool set_time_silent(synfig::Time time, bool *is_play_time_changed);
	bool set_bounds_silent(synfig::Time lower, synfig::Time upper, float fps);
	bool set_visible_bounds_silent(synfig::Time lower, synfig::Time upper);
	bool set_play_bounds_silent(synfig::Time lower, synfig::Time upper, bool enabled, bool repeat);

public:
	TimeModel();

	float get_frame_rate() const { return fps; }
	synfig::Time round_time(const synfig::Time &time) const
		{ return get_frame_rate() ? time.round(get_frame_rate()) : time; }

	const synfig::Time& get_time() const { return time; }
	const synfig::Time& get_play_time() const { return play_time; } //!< time clamped by playing bounds
	void set_time(const synfig::Time &time) {
		bool is_play_time_changed = false;
		if (set_time_silent(time, &is_play_time_changed)) time_changed();
		if (is_play_time_changed) play_time_changed();
	}

	bool almost_equal(const synfig::Time &a, const synfig::Time &b, const synfig::Time &range = synfig::Time()) const;
	bool almost_equal_to_current(const synfig::Time &time, const synfig::Time &range = synfig::Time()) const
		{ return almost_equal(get_time(), time, range); }

	const synfig::Time& get_lower() const { return lower; }
	const synfig::Time& get_upper() const { return upper; }
	void set_bounds(const synfig::Time &lower, const synfig::Time &upper, float fps)
		{ if (set_bounds_silent(lower, upper, fps)) bounds_changed(); }

	const synfig::Time& get_visible_lower() const { return visible_lower; }
	const synfig::Time& get_visible_upper() const { return visible_upper; }
	void set_visible_bounds(const synfig::Time &lower, const synfig::Time &upper)
		{ if (set_visible_bounds_silent(lower, upper)) visible_changed(); }

	synfig::Real get_zoom() const;
	void set_zoom(synfig::Real x, const synfig::Time &center);
	void zoom(synfig::Real x, const synfig::Time &center); //!< relative zoom in/out

	synfig::Time get_frame_duration() const; //!< duration of one frame
	synfig::Time get_page_increment() const; //!< depends on visible duration
	synfig::Time get_step_increment() const
		{ return get_frame_duration(); }
	synfig::Time get_page_size() const //!< visible duration
		{ return get_visible_upper() - get_visible_lower(); }
	synfig::Time get_size() const //!< full duration
		{ return round_time(get_upper() - get_lower()); }

	synfig::Time get_visible_center() const
		{ return (get_visible_lower() + get_visible_upper())*0.5; }
	void set_zoom(synfig::Real x)
		{ set_zoom(x, get_visible_center()); }
	void zoom(synfig::Real x) //!< relative zoom in/out
		{ zoom(x, get_visible_center()); }

	void move_to(const synfig::Time &time) //!< absolute scroll
		{ set_visible_bounds(time, time + get_page_size()); }
	void move_by(const synfig::Time &time) //!< relative scroll
		{ move_to(get_visible_lower() + time); }

	bool get_play_bounds_enabled() const { return play_bounds_enabled; }
	bool get_play_repeat() const { return play_repeat;}
	const synfig::Time& get_play_bounds_lower() const { return play_bounds_lower; }
	const synfig::Time& get_play_bounds_upper() const { return play_bounds_upper; }
	void set_play_bounds_enabled(bool enabled);
	void set_play_repeat(bool repeat);
	void set_play_bounds_lower(const synfig::Time &lower); //!< upper bound will be fixed to be >= (lower + two frames)
	void set_play_bounds_upper(const synfig::Time &upper); //!< lower bound will be fixed to be <= (upper - two frames)
	void set_play_bounds(const synfig::Time &lower, const synfig::Time &upper, bool enabled, bool repeat) //!< duration will be fixed to be >= two frames
		{ if (set_play_bounds_silent(lower, upper, enabled, repeat)) play_bounds_changed(); }
	void set_play_bounds(const synfig::Time &lower, const synfig::Time &upper, bool enabled)
		{ set_play_bounds(lower, upper, enabled, get_play_repeat()); }
	void set_play_bounds(const synfig::Time &lower, const synfig::Time &upper)
		{ set_play_bounds(lower, upper, get_play_bounds_enabled()); }
	void set_play_bounds_lower_to_current()
		{ set_play_bounds_lower(get_time()); }
	void set_play_bounds_upper_to_current()
		{ set_play_bounds_upper(get_time()); }

	//! returns play_time if play bounds is enabled, in other case returns time
	const synfig::Time& get_actual_play_time() const
		{ return get_play_bounds_enabled() ? get_play_time() : get_time(); }
	//! returns play_bounds_lower if play bounds is enabled, in other case returns lower
	const synfig::Time& get_actual_play_bounds_lower() const
		{ return get_play_bounds_enabled() ? get_play_bounds_lower() : get_lower(); }
	//! returns play_bounds_upper if play bounds is enabled, in other case returns upper
	const synfig::Time& get_actual_play_bounds_upper() const
		{ return get_play_bounds_enabled() ? get_play_bounds_upper() : get_upper(); }

	const Glib::RefPtr<Gtk::Adjustment>& full_time_adjustment()    { return full_time_adjustment_; }
	const Glib::RefPtr<Gtk::Adjustment>& scroll_time_adjustment()  { return scroll_time_adjustment_; }
	const Glib::RefPtr<Gtk::Adjustment>& play_bounds_adjustment()  { return play_bounds_adjustment_; }

	sigc::signal<void> signal_bounds_changed()      { return signal_bounds_changed_; }      // raises on bounds changed
	sigc::signal<void> signal_visible_changed()     { return signal_visible_changed_; }     // raises on visible bounds changed
	sigc::signal<void> signal_play_bounds_changed() { return signal_play_bounds_changed_; } // raises on play bounds changed
	sigc::signal<void> signal_time_changed()        { return signal_time_changed_; }        // raises on current time changed
	sigc::signal<void> signal_play_time_changed()   { return signal_play_time_changed_; }   // raises on play time changed

	sigc::signal<void> signal_changed()             { return signal_changed_; }             // raises on any change

	void bounds_changed() {
		sync();
		signal_bounds_changed()();
		visible_changed();
		play_bounds_changed();
		time_changed();
		signal_changed()();
	}

	void visible_changed() {
		sync();
		signal_visible_changed()();
		signal_changed()();
	}

	void play_bounds_changed() {
		sync();
		signal_play_bounds_changed()();
		play_time_changed();
		signal_changed()();
	}

	void time_changed() {
		sync();
		signal_time_changed()();
		signal_changed()();
	}

	void play_time_changed() {
		sync();
		signal_play_time_changed()();
		signal_changed()();
	}

	void all_changed()
		{ bounds_changed(); }
};

}; // END of namespace studio

/* === E N D =============================================================== */

#endif
