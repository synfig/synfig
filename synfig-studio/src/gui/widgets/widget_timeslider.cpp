/* === S Y N F I G ========================================================= */
/*!	\file widget_timeslider.cpp
**	\brief Time Slider Widget Implementation File
**
**	$Id$
**
**	\legal
**	Copyright (c) 2004 Adrian Bentley
**	Copyright (c) 2007, 2008 Chris Moore
**	Copyright (c) 2012, Carlos López
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

/* === H E A D E R S ======================================================= */

#ifdef USING_PCH
#	include "pch.h"
#else
#ifdef HAVE_CONFIG_H
#	include <config.h>
#endif

#include <cmath>

#include <gdkmm/general.h>

#include <ETL/misc>

#include <synfig/general.h>

#include <gui/app.h>

#include "widget_timeslider.h"

#include <gui/localization.h>

#endif

/* === U S I N G =========================================================== */

using namespace synfig;
using namespace studio;

/* === M A C R O S ========================================================= */

/* === G L O B A L S ======================================================= */

const double zoominfactor = 1.25;
const double zoomoutfactor = 1/zoominfactor;
const int fullheight = 20;

/* === P R O C E D U R E S ================================================= */

/* === M E T H O D S ======================================================= */

static void
calc_divisions(float fps, double range, double sub_range, double &out_step, int &out_subdivisions)
{
	int ifps = etl::round_to_int(fps);
	if (ifps < 1) ifps = 1;

	// build a list of all the factors of the frame rate
	int pos = 0;
	std::vector<double> ranges;
	for(int i = 1; i*i <= ifps; i++)
		if (ifps % i == 0) {
			ranges.insert(ranges.begin() + pos, i/fps);
			if (i*i != ifps)
				ranges.insert(ranges.begin() + pos + 1, ifps/i/fps);
			pos++;
		}

	{ // fill in any gaps where one factor is more than 2 times the previous
		std::vector<double>::iterator iter, next;
		pos = 0;
		for(int pos = 0; pos < (int)ranges.size()-1; pos++) {
			next = ranges.begin() + pos;
			iter = next++;
			if (*iter*2 < *next)
				ranges.insert(next, *iter*2);
		}
	}

	double more_ranges[] = {
		2, 3, 5, 10, 20, 30, 60, 90, 120, 180,
		300, 600, 1200, 1800, 2700, 3600, 3600*2,
		3600*4, 3600*8, 3600*16, 3600*32, 3600*64,
		3600*128, 3600*256, 3600*512, 3600*1024 };
	ranges.insert(ranges.end(), more_ranges, more_ranges + sizeof(more_ranges)/sizeof(double));

	double mid_range = (range + sub_range)/2;

	// find most ideal scale
	double scale;
	{
		std::vector<double>::iterator next = etl::binary_find(ranges.begin(), ranges.end(), mid_range);
		std::vector<double>::iterator iter = next++;
		if (iter == ranges.end()) iter--;
		if (next == ranges.end()) next--;
		if (fabs(*next - mid_range) < fabs(*iter - mid_range))
			iter = next;
		scale = *iter;
	}

	// subdivide into this many tick marks (8 or less)
	int subdiv = etl::round_to_int(scale * ifps);
	if (subdiv > 8) {
		const int ideal = subdiv;

		// find a number of tick marks that nicely divides the scale
		// (5 minutes divided by 6 is 50s, but that's not 'nice' -
		//  5 ticks of 1m each is much simpler than 6 ticks of 50s)
		for (subdiv = 8; subdiv > 0; subdiv--)
			if ((ideal <= ifps*2       && (ideal % (subdiv           )) == 0) ||
				(ideal <= ifps*2*60    && (ideal % (subdiv*ifps      )) == 0) ||
				(ideal <= ifps*2*60*60 && (ideal % (subdiv*ifps*60   )) == 0) ||
				(true                  && (ideal % (subdiv*ifps*60*60)) == 0))
				break;

		// if we didn't find anything, use 4 ticks
		if (!subdiv)
			subdiv = 4;
	}

	out_step = scale;
	out_subdivisions = subdiv;
}

/* === E N T R Y P O I N T ================================================= */

Widget_Timeslider::Widget_Timeslider():
	layout(Pango::Layout::create(get_pango_context())),
	lastx()
{
	set_size_request(-1, fullheight);

	{ // prepare pattern for play bounds
		const int pattern_step = 32;
		Cairo::RefPtr<Cairo::ImageSurface> surface = Cairo::ImageSurface::create(Cairo::FORMAT_ARGB32, pattern_step, pattern_step);
		Cairo::RefPtr<Cairo::Context> cr = Cairo::Context::create(surface);
		cr->set_source_rgba(0.0, 0.0, 0.0, 0.25);
		cr->scale((double)pattern_step, (double)pattern_step);
		cr->set_line_width(0.375);
		cr->move_to(1.75, -1.0);
		cr->line_to(-1.0, 1.75);
		cr->stroke();
		cr->move_to(2.0, -0.25);
		cr->line_to(-0.25, 2.0);
		cr->stroke();
		surface->flush();

		play_bounds_pattern = Cairo::SurfacePattern::create(surface);
		play_bounds_pattern->set_filter(Cairo::FILTER_NEAREST);
		play_bounds_pattern->set_extend(Cairo::EXTEND_REPEAT);
	}

	// click / scroll / zoom
	add_events( Gdk::BUTTON_PRESS_MASK
			  | Gdk::BUTTON_RELEASE_MASK
			  | Gdk::BUTTON_MOTION_MASK
			  | Gdk::SCROLL_MASK );
}

Widget_Timeslider::~Widget_Timeslider()
{
	time_change.disconnect();
	time_bounds_change.disconnect();
}

void
Widget_Timeslider::set_time_model(const etl::handle<TimeModel> &x)
{
	if (time_model == x) return;

	//disconnect old connections
	time_change.disconnect();
	time_bounds_change.disconnect();

	//connect update function to new adjustment
	time_model = x;

	if (time_model) {
		time_change = time_model->signal_time_changed().connect(sigc::mem_fun(*this,&Widget_Timeslider::queue_draw));
		time_bounds_change = time_model->signal_play_time_changed().connect(sigc::mem_fun(*this,&Widget_Timeslider::queue_draw));
	}

	queue_draw();
}

void
Widget_Timeslider::draw_background(const Cairo::RefPtr<Cairo::Context> &cr)
{
	//draw grey rectangle
	cr->save();
	cr->set_source_rgb(0.5, 0.5, 0.5);
	cr->rectangle(0.0, 0.0, (double)get_width(), (double)get_height());
	cr->fill();
	cr->restore();
}

bool
Widget_Timeslider::on_draw(const Cairo::RefPtr<Cairo::Context> &cr)
{
	const double mark_height = 12.0;
	const double sub_mark_height = 4.0;

	draw_background(cr);

	if (!time_model || get_width() <= 0 || get_height() <= 0) return true;

	// Get the time information since we now know it's valid
	Time time = time_model->get_time();
	Time lower = time_model->get_visible_lower();
	Time upper = time_model->get_visible_upper();

	if (lower >= upper) return true;
	double k = (double)get_width()/(double)(upper - lower);

	Time extra_time = (double)get_height()/k;
	Time lower_ex = lower - extra_time;
	Time upper_ex = upper + extra_time;

	// Draw the time line...
	double tpx = etl::round_to_int((time - lower)*k) + 0.5;
	cr->save();
	cr->set_source_rgb(1.0, 175.0/255.0, 0.0);
	cr->set_line_width(1.0);
	cr->move_to(tpx, 0.0);
	cr->line_to(tpx, fullheight);
	cr->stroke();
	cr->restore();

	// draw marks

	// get divisions
	double big_step_value;
	int subdivisions;
	calc_divisions(
		time_model->get_frame_rate(),
		140.0/k,
		280.0/k,
		big_step_value,
		subdivisions );

	step = time_model->round_time(Time(big_step_value/(double)subdivisions));
	step = std::max(time_model->get_step_increment(), step);

	Time big_step = step * (double)subdivisions;
	Time current = big_step * floor((double)lower_ex/(double)big_step);
	current = time_model->round_time(current);

	// draw
	cr->save();
	cr->set_source_rgb(51.0/255.0,51.0/255.0,51.0/255.0);
	cr->set_line_width(1.0);
	for(int i = 0; current <= upper_ex; ++i, current = time_model->round_time(current + step)) {
		double x = etl::round_to_int((double)(current - lower)*k) + 0.5;
		if (i % subdivisions == 0) {
			// draw big
			cr->move_to(x, 0.0);
			cr->line_to(x, mark_height);
			cr->stroke();

			layout->set_text(
				current.get_string(
					time_model->get_frame_rate(),
					App::get_time_format() ));

			// Approximately a font size of 8 pixels.
			// Pango::SCALE = 1024
			// create_attr_size waits a number in 1000th of pixels.
			// Should be user customizable in the future. Now it is fixed to 10
			Pango::AttrList attr_list;
			Pango::AttrInt pango_size(Pango::Attribute::create_attr_size(Pango::SCALE*10));
			pango_size.set_start_index(0);
			pango_size.set_end_index(64);
			attr_list.change(pango_size);
			layout->set_attributes(attr_list);
			cr->move_to(x + 1.0, 0.0);
			layout->show_in_cairo_context(cr);
		} else {
			// draw small
			cr->move_to(x, 0.0);
			cr->line_to(x, sub_mark_height);
			cr->stroke();
		}
	}
	cr->restore();

	// Draw the time line
	Gdk::Cairo::set_source_color(cr, Gdk::Color("#ffaf00"));
	cr->set_line_width(3.0);
	double x = round((double)(time - lower)*k);
	cr->move_to(x, 0.0);
	cr->line_to(x, fullheight);
	cr->stroke();

	// Draw play bounds
	if (time_model->get_play_bounds_enabled()) {
		double offset = -round((double)lower*k);
		Time bounds[2][2] {
			{ lower_ex, time_model->get_play_bounds_lower() },
			{ time_model->get_play_bounds_upper(), upper_ex } };
		for(int i = 0; i < 2; ++i) {
			if (bounds[i][0] < bounds[i][1]) {
				double x0 = round((double)(bounds[i][0] - lower)*k);
				double x1 = round((double)(bounds[i][1] - lower)*k);
				double w = x1 - x0;

				cr->save();
				cr->rectangle(x0, 0.0, w, (double)get_height());
				cr->clip();
				cr->translate(offset, 0.0);
				cr->set_source(play_bounds_pattern);
				cr->paint();
				cr->restore();

				cr->save();
				cr->set_line_width(1.0);
				cr->set_source_rgba(0.0, 0.0, 0.0, 0.25);
				cr->rectangle(x0 + 1.0, 1.0, w - 2.0, (double)get_height() - 2.0);
				cr->stroke();
				cr->set_source_rgba(0.0, 0.0, 0.0, 0.3);
				cr->rectangle(x0 + 0.5, 0.5, w - 1.0, (double)get_height() - 1.0);
				cr->stroke();
				cr->restore();
			}
		}
	}

	return true;
}

bool
Widget_Timeslider::on_button_press_event(GdkEventButton *event) //for clicking
{
	lastx = (double)event->x;

	if (!time_model || get_width() <= 0 || get_height() <= 0)
		return false;

	Time lower = time_model->get_visible_lower();
	Time upper = time_model->get_visible_upper();
	if (lower >= upper)
		return false;

	if (event->button == 1) {
		double k = (upper - lower)/(double)get_width();
		Time time = Time((double)event->x*k) + lower;
		time_model->set_time(time);
	}

	return event->button == 1 || event->button == 2;
}

bool
Widget_Timeslider::on_button_release_event(GdkEventButton *event){
	lastx = (double)event->x;
	return event->button == 1 || event->button == 2;
}

bool
Widget_Timeslider::on_motion_notify_event(GdkEventMotion* event) //for dragging
{
	double dx = (double)event->x - lastx;
	lastx = (double)event->x;

	if (!time_model || get_width() <= 0 || get_height() <= 0)
		return false;

	Time lower = time_model->get_visible_lower();
	Time upper = time_model->get_visible_upper();
	if (lower >= upper)
		return false;
	double k = (upper - lower)/(double)get_width();

	Gdk::ModifierType mod = Gdk::ModifierType(event->state);
	if (mod & Gdk::BUTTON1_MASK) {
		// scrubbing
		Time time = Time((double)event->x*k) + lower;
		time_model->set_time(time);
		return true;
	} else
	if (mod & Gdk::BUTTON2_MASK) {
		// scrolling
		Time dt(-dx*k);
		time_model->move_by(dt);
		return true;
	}

	return false;
}

bool
Widget_Timeslider::on_scroll_event(GdkEventScroll* event) //for zooming
{
	if (!time_model || get_width() <= 0 || get_height() <= 0)
		return false;

	Time lower = time_model->get_visible_lower();
	Time upper = time_model->get_visible_upper();
	if (lower >= upper)
		return false;

	double k = (upper - lower)/(double)get_width();
	Time time = Time((double)event->x*k) + lower;

	switch(event->direction) {
	case GDK_SCROLL_UP: //zoom in
		time_model->zoom(zoominfactor, time);
		return true;
	case GDK_SCROLL_DOWN: //zoom out
		time_model->zoom(zoomoutfactor, time);
		return true;
	case GDK_SCROLL_RIGHT:
		time_model->move_by(step);
		return true;
	case GDK_SCROLL_LEFT:
		time_model->move_by(-step);
		return true;
	default:
		break;
	}

	return false;
}
