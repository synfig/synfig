/* === S Y N F I G ========================================================= */
/*!	\file timeplotdata.h
**	\brief Template Header
**
**	$Id$
**
**	\legal
**	Copyright (c) 2002-2005 Robert B. Quattlebaum Jr., Adrian Bentley
**	......... ... 2019 Rodolfo R. Gomes
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

#ifndef __SYNFIG_STUDIO_TIMEPLOTDATA_H
#define __SYNFIG_STUDIO_TIMEPLOTDATA_H

/* === H E A D E R S ======================================================= */

#include "gui/timemodel.h"
#include <synfig/time.h>

#include <gtkmm/widget.h>
#include <gtkmm/adjustment.h>

/* === M A C R O S ========================================================= */

/* === T Y P E D E F S ===================================================== */

/* === C L A S S E S & S T R U C T S ======================================= */

namespace studio {

/// Helper class that connects widget geometry to visible timeline and vice versa
/**
 * Some widgets show data that must be in sync to the project timeline, eg.
 * the waypoint 'editor'.
 * This class helps to map pixel coordinate <-> time, being updated automatically
 * when the timeline changes (by zooming or scrolling, for example) or widget size
 * changes.
 *
 * For widgets that uses another axis that can be scrolled and/or zoomed (like
 * Widget_Curves), \ref vertical_adjustment should be used.
 *
 * \sa TimeModel
 */
class TimePlotData {
	bool invalid;

public:
	etl::handle<TimeModel> time_model;

	/// Current Time \sa TimeModel::get_time
	synfig::Time time;
	/// Start visible Time \sa TimeModel::get_visible_lower()
	synfig::Time lower;
	/// Final visible Time \sa TimeModel::get_visible_upper()
	synfig::Time upper;
	/// How many pixels per second
	double k;
	/// How long a pixel last. Inverse of \ref k
	synfig::Time dt;

	/// How many extra pixels beyond regular bounds \sa set_extra_time_margin()
	double extra_margin;
	/// How long last the extra pixels
	synfig::Time extra_time;
	synfig::Time lower_ex;
	synfig::Time upper_ex;

private:
	/// If vertical adjustment is set
	bool has_vertical;
	synfig::Real range_lower;
	synfig::Real range_upper;
	double range_k;

	sigc::connection widget_resized;
	sigc::connection time_model_changed;
	sigc::connection vertical_changed;
	sigc::connection vertical_value_changed;

	Gtk::Widget &widget;
	Glib::RefPtr<Gtk::Adjustment> vertical_adjustment;

public:
	/**
	 * \param widget The widget that must zoom and scroll in sync with the timeline
	 * \param vertical_adjustment If widget displays info in another axis that can be scrolled/zoomed
	 */
	TimePlotData(Gtk::Widget & widget, Glib::RefPtr<Gtk::Adjustment> vertical_adjustment = Glib::RefPtr<Gtk::Adjustment>());
	virtual ~TimePlotData();

	void set_time_model(const etl::handle<TimeModel> &time_model);

	/// Sets an extra margin, creating a new and wider range. \sa is_time_visible_extra()
	void set_extra_time_margin(double margin);

	/// Checks whether this data is valid (it has valid size and valid time info)
	bool is_invalid() const;

	/// If \ref t is in \ref lower - \ref upper range
	bool is_time_visible(const synfig::Time & t) const;
	/// If \ref t is in expanded range (\ref lower_ex - \ref upper_ex)
	bool is_time_visible_extra(const synfig::Time & t) const;

	bool is_y_visible(synfig::Real y) const;

	/// What pixel time t is mapped to. Uses ::etl::round_to_int()
	int get_pixel_t_coord(const synfig::Time & t) const;

	/// Similar to get_pixel_t_coord(), but rounded by regular round()
	/** Maybe it was an error and should be replaced by get_pixel_t_coord() */
	double get_double_pixel_t_coord(const synfig::Time & t) const;

	int get_pixel_y_coord(synfig::Real y) const;

	/// What time a pixel represents.
	synfig::Time get_t_from_pixel_coord(double pixel) const;

	double get_y_from_pixel_coord(double pixel) const;

private:
	bool on_widget_resize(GdkEventConfigure * /*configure*/);

	void recompute_time_bounds();
	void recompute_geometry_data();
	void recompute_extra_time();
	void recompute_vertical();
}; // END of class TimePlotData

}; // END of namespace studio

/* === E N D =============================================================== */

#endif // TIMEPLOTDATA_H
