/* === S Y N F I G ========================================================= */
/*!	\file widget_curves.cpp
**	\brief Template File
**
**	$Id$
**
**	\legal
**	Copyright (c) 2002-2005 Robert B. Quattlebaum Jr., Adrian Bentley
**  ......... ... 2019 Rodolfo R. Gomes
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

/* === H E A D E R S ======================================================= */

#ifdef USING_PCH
#	include "pch.h"
#else
# ifdef HAVE_CONFIG_H
#  include <config.h>
# endif

# include "timeplotdata.h"
# include <ETL/misc>

#endif

/* === U S I N G =========================================================== */

/* === M A C R O S ========================================================= */

/* === G L O B A L S ======================================================= */

/* === P R O C E D U R E S ================================================= */

/* === C L A S S E S ======================================================= */

namespace studio {

TimePlotData::TimePlotData(Gtk::Widget& widget, Glib::RefPtr<Gtk::Adjustment> vertical_adjustment) :
	invalid(true),
	k(0),
	extra_margin(0),
	has_vertical(false),
	widget(widget),
	vertical_adjustment(vertical_adjustment)
{
	widget_resized = widget.signal_configure_event().connect(
				sigc::mem_fun(*this, &TimePlotData::on_widget_resize) );

	if (vertical_adjustment) {
		vertical_changed = vertical_adjustment->signal_changed().connect(
					sigc::mem_fun(*this, &TimePlotData::recompute_vertical) );
		vertical_value_changed = vertical_adjustment->signal_value_changed().connect(
					sigc::mem_fun(*this, &TimePlotData::recompute_vertical) );
		recompute_vertical();
	} else {
		range_k = 0.0;
		range_lower = 0.0;
		range_upper = 0.0;
	}
}

TimePlotData::~TimePlotData()
{
	widget_resized.disconnect();
	time_model_changed.disconnect();
	vertical_changed.disconnect();
	vertical_value_changed.disconnect();
}

void
TimePlotData::set_time_model(const etl::handle<studio::TimeModel>& time_model)
{
	if (this->time_model == time_model)
		return;

	time_model_changed.disconnect();

	this->time_model = time_model;

	if (time_model) {
		time_model_changed = this->time_model->signal_changed().connect(
					sigc::mem_fun(*this, &TimePlotData::recompute_time_bounds) );
		invalid = false;
	} else {
		invalid = true;
	}

	recompute_time_bounds();
}

void
TimePlotData::set_extra_time_margin(double margin)
{
	extra_margin = margin;

	recompute_extra_time();
}

bool
TimePlotData::on_widget_resize(GdkEventConfigure*)
{
	recompute_geometry_data();
	return false;
}

void
TimePlotData::recompute_time_bounds()
{
	if (!time_model) {
		time = lower = upper = 0;
		invalid = true;
		widget.queue_draw();
		return;
	}
	time  = time_model->get_time();
	lower = time_model->get_visible_lower();
	upper = time_model->get_visible_upper();

	if (lower >= upper) {
		invalid = true;
		widget.queue_draw();
		return;
	}

	invalid = false;
	recompute_geometry_data(); // lower and upper change other fields
}

void
TimePlotData::recompute_geometry_data()
{
	k = widget.get_width()/(upper - lower);
	dt = 1.0/k;

	if (has_vertical) {
		range_k = widget.get_height()/(range_upper - range_lower);
	}

	recompute_extra_time(); // k (and lower and upper) changes extra_time
}

void
TimePlotData::recompute_extra_time()
{
	extra_time = extra_margin/k;
	lower_ex = lower - extra_time;
	upper_ex = upper + extra_time;

	widget.queue_draw();
}

void
TimePlotData::recompute_vertical()
{
	if (!vertical_adjustment) {
		has_vertical = false;

		if (vertical_changed.connected())
			vertical_changed.disconnect();
		if (vertical_value_changed.connected())
			vertical_value_changed.disconnect();

		range_k = 0.0;
		range_lower = 0.0;
		range_upper = 0.0;
		return;
	}
	range_lower = vertical_adjustment->get_value();
	range_upper = range_lower + vertical_adjustment->get_page_size();
	range_k = widget.get_height()/(range_upper - range_lower);
	has_vertical = true;
	widget.queue_draw();
}

bool
TimePlotData::is_invalid() const
{
	return invalid;
}

bool
TimePlotData::is_time_visible(const synfig::Time& t) const
{
	return t >= lower && t <= upper;
}

bool
TimePlotData::is_time_visible_extra(const synfig::Time& t) const
{
	return t >= lower_ex && t <= upper_ex;
}

bool
TimePlotData::is_y_visible(synfig::Real y) const
{
	return y >= range_lower && y <= range_upper;
}

int
TimePlotData::get_pixel_t_coord(const synfig::Time& t) const
{
	return etl::round_to_int((t - lower) * k);
}

double
TimePlotData::get_double_pixel_t_coord(const synfig::Time& t) const
{
	return round((t - lower) * k);
}

int
TimePlotData::get_pixel_y_coord(synfig::Real y) const
{
	return etl::round_to_int(-(y + range_lower) * range_k);
}

synfig::Time
TimePlotData::get_t_from_pixel_coord(double pixel) const
{
	return lower + synfig::Time(pixel/k);
}

double TimePlotData::get_y_from_pixel_coord(double pixel) const
{
	return -(range_lower + pixel / range_k);
}

synfig::Time studio::TimePlotData::get_delta_t_from_delta_pixel_coord(int delta_pixel) const
{
	return synfig::Time(delta_pixel/k);
}

double TimePlotData::get_delta_y_from_delta_pixel_coord(int delta_pixel) const
{
	return -(delta_pixel / range_k);
}

int TimePlotData::get_delta_pixel_from_delta_t_coord(double delta_t) const
{
	return int(delta_t * k);
}

int TimePlotData::get_delta_pixel_from_delta_y_coord(double delta_y) const
{
	return int(-delta_y * range_k);
}

}
