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

/// Helper class that handles widget geometry to related time and vice versa
struct TimePlotData {
	bool invalid;

	etl::handle<TimeModel> time_model;

	synfig::Time time;
	synfig::Time lower;
	synfig::Time upper;
	double k;
	synfig::Time dt;

	double extra_margin; //! pixels
private:
	synfig::Time extra_time;
	synfig::Time lower_ex;
	synfig::Time upper_ex;

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
	TimePlotData(Gtk::Widget & widget, Glib::RefPtr<Gtk::Adjustment> vertical_adjustment);

	~TimePlotData();

	void set_time_model(const etl::handle<TimeModel> &time_model);

	void set_extra_time_margin(double margin);

	bool is_time_visible(const synfig::Time & t) const;

	bool is_time_visible_extra(const synfig::Time & t) const;

	bool is_y_visible(synfig::Real y) const;

	int get_pixel_t_coord(const synfig::Time & t) const;

	int get_pixel_y_coord(synfig::Real y) const;

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
