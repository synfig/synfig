/* === S Y N F I G ========================================================= */
/*!	\file widgets/widget_timegraphbase.cpp
**	\brief Base class for widgets that are graph-like representations with time axis
**
**	$Id$
**
**	\legal
**	Copyright (c) 2002-2005 Robert B. Quattlebaum Jr., Adrian Bentley
**	......... ... 2019 Rodolfo Ribeiro Gomes
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

#ifdef USING_PCH
#	include "pch.h"
#else
#ifdef HAVE_CONFIG_H
#	include <config.h>
#endif

#include "widget_timegraphbase.h"

#include <gui/canvasview.h>
#include <gui/timeplotdata.h>

#include <cairomm/cairomm.h>
#include <gdkmm.h>
#include <cstring>
#endif

using namespace studio;

#define DEFAULT_ZOOM_CHANGING_FACTOR 1.25
#define DEFAULT_PAGE_SIZE 2.0

Widget_TimeGraphBase::Widget_TimeGraphBase()
	: Gtk::DrawingArea(),
	  range_adjustment(Gtk::Adjustment::create(-1.0, -1.0, 1.0, 0.1, 0.1, DEFAULT_PAGE_SIZE)),
	  zoom_changing_factor(DEFAULT_ZOOM_CHANGING_FACTOR),
	  default_page_size(DEFAULT_PAGE_SIZE)
{
	time_plot_data = new TimePlotData(*this, range_adjustment);
}

Widget_TimeGraphBase::~Widget_TimeGraphBase()
{
	if (time_model_changed_connection.connected())
		time_model_changed_connection.disconnect();

	delete time_plot_data;
}

const etl::handle<TimeModel>& Widget_TimeGraphBase::get_time_model() const
{
	return time_plot_data->time_model;
}

void Widget_TimeGraphBase::set_time_model(const etl::handle<TimeModel>& x)
{
	if (x == time_plot_data->time_model)
		return;

	if (time_model_changed_connection.connected())
		time_model_changed_connection.disconnect();

	time_plot_data->set_time_model(x);

	if (x)
		time_model_changed_connection = x->signal_changed().connect(sigc::mem_fun(*this, &Widget_TimeGraphBase::on_time_model_changed));
}

void Widget_TimeGraphBase::zoom_in()
{
	set_zoom(get_zoom() * zoom_changing_factor);
}

void Widget_TimeGraphBase::zoom_out()
{
	set_zoom(get_zoom() / zoom_changing_factor);
}

void Widget_TimeGraphBase::zoom_100()
{
	set_zoom(1.0);
}

void Widget_TimeGraphBase::set_zoom(double new_zoom_factor)
{
	int x, y;
	get_pointer(x, y);
	double perc_y = y/(get_height()+0.0);
	double y_value = perc_y * range_adjustment->get_page_size() + range_adjustment->get_value();
	double new_range_page_size = default_page_size / new_zoom_factor;
	double new_range_value = y_value - perc_y * new_range_page_size;
	ConfigureAdjustment(range_adjustment)
			.set_page_size(new_range_page_size)
			.set_value(new_range_value)
			.finish();
}

double Widget_TimeGraphBase::get_zoom() const {
	return default_page_size / range_adjustment->get_page_size();
}

void Widget_TimeGraphBase::scroll_up()
{
	ConfigureAdjustment(range_adjustment)
			.set_value(range_adjustment->get_value() - range_adjustment->get_step_increment())
			.finish();
}

void Widget_TimeGraphBase::scroll_down()
{
	ConfigureAdjustment(range_adjustment)
			.set_value(range_adjustment->get_value() + range_adjustment->get_step_increment())
			.finish();
}

void Widget_TimeGraphBase::pan(int dx, int dy, int /*total_dx*/, int /*total_dy*/)
{
	synfig::Time dt(-dx*time_plot_data->dt);
	time_plot_data->time_model->move_by(dt);

	double real_dy = (range_adjustment->get_page_size()*dy)/get_height();

	ConfigureAdjustment(range_adjustment)
			.set_value(range_adjustment->get_value() - real_dy)
			.finish();
}

etl::handle<synfigapp::CanvasInterface> Widget_TimeGraphBase::get_canvas_interface() const
{
	return canvas_interface;
}

void Widget_TimeGraphBase::set_canvas_interface(const etl::handle<synfigapp::CanvasInterface>& value)
{
	if (canvas_interface.get() == value.get())
		return;
	canvas_interface = value;
	on_canvas_interface_changed();
}

void Widget_TimeGraphBase::on_canvas_interface_changed()
{

}

void Widget_TimeGraphBase::set_default_page_size(double new_value)
{
	if (new_value < 0 || synfig::approximate_zero(new_value))
		return;
	double current_zoom = get_zoom();
	default_page_size = new_value;
	set_zoom(current_zoom);
}

double Widget_TimeGraphBase::get_default_page_size() const
{
	return default_page_size;
}

bool Widget_TimeGraphBase::on_draw(const Cairo::RefPtr<Cairo::Context>& cr)
{
	int w = get_width();
	int h = get_height();
	if (w <= 0 || h <= 0)
		return true;

	get_style_context()->render_background(cr, 0, 0, w, h);

	if (!time_plot_data->time_model)
		return true;

	if (time_plot_data->is_invalid())
		return true;

	return false;
}

void Widget_TimeGraphBase::on_time_model_changed()
{

}

void Widget_TimeGraphBase::draw_current_time(const Cairo::RefPtr<Cairo::Context>& cr) const
{
	cr->save();
	cr->set_line_width(1.0);
	cr->set_source_rgb(0, 0, 1);
	cr->rectangle(time_plot_data->get_pixel_t_coord(time_plot_data->time), 0, 0, get_height());
	cr->stroke();
	cr->restore();
}

void Widget_TimeGraphBase::draw_keyframe_line(const Cairo::RefPtr<Cairo::Context>& cr, const synfig::Keyframe& keyframe) const
{
	const synfig::Time &keyframe_time = keyframe.get_time();
	if (keyframe_time < time_plot_data->lower_ex || keyframe_time >= time_plot_data->upper_ex)
		return;
	const Gdk::Color keyframe_color("#a07f7f");
	cr->save();
	Gdk::Cairo::set_source_color(cr, keyframe_color);
	cr->rectangle(time_plot_data->get_pixel_t_coord(keyframe_time), 0, 1.0, get_height());
	cr->fill();
	cr->restore();
}
