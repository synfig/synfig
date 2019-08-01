/* === S Y N F I G ========================================================= */
/*!	\file widget_curves.cpp
**	\brief Template File
**
**	$Id$
**
**	\legal
**	Copyright (c) 2002-2005 Robert B. Quattlebaum Jr., Adrian Bentley
**  Copyright (c) 2008 Gerco Ballintijn
**  Copyright (c) 2011 Carlos López
**  ......... ... 2018 Ivan Mahonin
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

#include <map>
#include <vector>

#include <gdkmm/general.h>

#include <ETL/misc>

#include <synfig/blinepoint.h>
#include <synfig/widthpoint.h>
#include <synfig/dashitem.h>

#include <gui/helpers.h>

#include "widget_curves.h"

#endif

/* === U S I N G =========================================================== */

using namespace synfig;
using namespace synfigapp;
using namespace studio;

/* === M A C R O S ========================================================= */

#define MAX_CHANNELS 15

/* === G L O B A L S ======================================================= */

/* === P R O C E D U R E S ================================================= */

/* === C L A S S E S ======================================================= */

struct Widget_Curves::Channel
{
	String name;
	Gdk::Color color;
	std::map<Real, Real> values;
	explicit Channel(const String &name = String(), const Gdk::Color &color = Gdk::Color()):
		name(name), color(color) { }
};

struct Widget_Curves::CurveStruct: sigc::trackable
{
	ValueDesc value_desc;
	std::vector<Channel> channels;

	void add_channel(const String &name, const Gdk::Color &color)
		{ channels.push_back(Channel(name, color)); }
	void add_channel(const String &name, const String &color)
		{ add_channel(name, Gdk::Color(color)); }

	CurveStruct() { }

	explicit CurveStruct(const ValueDesc& x)
		{ init(x); }

	bool init(const ValueDesc& x) {
		value_desc = x;
		channels.clear();

		Type &type = value_desc.get_value_type();
		if (type == type_real) {
			add_channel("real",     "#007f7f");
		} else
		if (type == type_time) {
			add_channel("time",     "#7f7f00");
		} else
		if (type == type_integer) {
			add_channel("int",      "#7f0000");
		} else
		if (type == type_bool) {
			add_channel("bool",     "#ff7f00");
		} else
		if (type == type_angle) {
			add_channel("theta",    "#004f4f");
		} else
		if (type == type_color) {
			add_channel("red",      "#7f0000");
			add_channel("green",    "#007f00");
			add_channel("blue",     "#00007f");
			add_channel("alpha",    "#000000");
		} else
		if (type == type_vector) {
			add_channel("x",        "#7f007f");
			add_channel("y",        "#007f7f");
		} else
		if (type == type_bline_point) {
			add_channel("v.x",      "#ff7f00");
			add_channel("v.y",      "#7f3f00");
			add_channel("width",    "#000000");
			add_channel("origin",   "#ffffff");
			add_channel("tsplit",   "#ff00ff");
			add_channel("t1.x",     "#ff0000");
			add_channel("t1.y",     "#7f0000");
			add_channel("t2.x",     "#ffff00");
			add_channel("t2.y",     "#7f7f00");
			add_channel("rsplit",   "#ff00ff");
			add_channel("asplit",   "#ff00ff");
		} else
		if (type == type_width_point) {
			add_channel("position", "#ff0000");
			add_channel("width",    "#00ff00");
		} else
		if (type == type_dash_item) {
			add_channel("offset",   "#ff0000");
			add_channel("length",   "#00ff00");
		}

		return !channels.empty();
	}

	void clear_all_values() {
		for(std::vector<Channel>::iterator i = channels.begin(); i != channels.end(); ++i)
			i->values.clear();
	}

	Real get_value(int channel, Real time, Real tolerance) {
		// First check to see if we have a value
		// that is "close enough" to the time
		// we are looking for
		std::map<Real, Real>::iterator i = channels[channel].values.lower_bound(time);
		if (i != channels[channel].values.end() && i->first - time <= tolerance)
			return -i->second;

		// Since that didn't work, we now need
		// to go ahead and figure out what the
		// actual value is at that time.
		ValueBase value(value_desc.get_value(time));
		Type &type(value.get_type());
		if (type == type_real) {
			channels[0].values[time] = value.get(Real());
		} else
		if (type == type_time) {
			channels[0].values[time] = value.get(Time());
		} else
		if (type == type_integer) {
			channels[0].values[time] = value.get(int());
		} else
		if (type == type_bool) {
			channels[0].values[time] = value.get(bool());
		} else
		if (type == type_angle) {
			channels[0].values[time] = Angle::rad(value.get(Angle())).get();
		} else
		if (type == type_color) {
			channels[0].values[time] = value.get(Color()).get_r();
			channels[1].values[time] = value.get(Color()).get_g();
			channels[2].values[time] = value.get(Color()).get_b();
			channels[3].values[time] = value.get(Color()).get_a();
		} else
		if (type == type_vector) {
			channels[0].values[time] = value.get(Vector())[0];
			channels[1].values[time] = value.get(Vector())[1];
		} else
		if (type == type_bline_point) {
			channels[0].values[time] = value.get(BLinePoint()).get_vertex()[0];
			channels[1].values[time] = value.get(BLinePoint()).get_vertex()[1];
			channels[2].values[time] = value.get(BLinePoint()).get_width();
			channels[3].values[time] = value.get(BLinePoint()).get_origin();
			channels[4].values[time] = value.get(BLinePoint()).get_split_tangent_both();
			channels[5].values[time] = value.get(BLinePoint()).get_tangent1()[0];
			channels[6].values[time] = value.get(BLinePoint()).get_tangent1()[1];
			channels[7].values[time] = value.get(BLinePoint()).get_tangent2()[0];
			channels[8].values[time] = value.get(BLinePoint()).get_tangent2()[1];
			channels[9].values[time] = value.get(BLinePoint()).get_split_tangent_radius();
			channels[10].values[time]= value.get(BLinePoint()).get_split_tangent_angle();
		} else
		if (type == type_width_point) {
			channels[0].values[time] = value.get(WidthPoint()).get_position();
			channels[1].values[time] = value.get(WidthPoint()).get_width();
		} else
		if (type == type_dash_item) {
			channels[0].values[time] = value.get(DashItem()).get_offset();
			channels[1].values[time] = value.get(DashItem()).get_length();
		} else {
			return Real(0.0);
		}

		return -channels[channel].values[time];
	}
};

/* === M E T H O D S ======================================================= */

Widget_Curves::Widget_Curves():
	range_adjustment(Gtk::Adjustment::create(-1.0, -2.0, 2.0, 0.1, 0.1, 2))
{
	set_size_request(64, 64);

	range_adjustment->signal_changed().connect(
		sigc::mem_fun(*this, &Widget_Curves::queue_draw) );
	range_adjustment->signal_value_changed().connect(
		sigc::mem_fun(*this, &Widget_Curves::queue_draw) );

	add_events(Gdk::BUTTON_PRESS_MASK | Gdk::BUTTON_RELEASE_MASK | Gdk::SCROLL_MASK);
}

Widget_Curves::~Widget_Curves() {
	clear();
	set_time_model(etl::handle<TimeModel>());
}

void
Widget_Curves::set_time_model(const etl::handle<TimeModel> &x)
{
	if (time_model == x) return;
	time_changed.disconnect();
	time_model = x;
	if (time_model)
		time_changed = time_model->signal_time_changed().connect(
			sigc::mem_fun(*this, &Widget_Curves::queue_draw) );
}

void
Widget_Curves::clear() {
	while(!value_desc_changed.empty()) {
		value_desc_changed.back().disconnect();
		value_desc_changed.pop_back();
	}
	curve_list.clear();
}

void
Widget_Curves::refresh()
{
	for(std::list<CurveStruct>::iterator i = curve_list.begin(); i != curve_list.end(); ++i)
		i->clear_all_values();
	queue_draw();
}

void
Widget_Curves::set_value_descs(const std::list<ValueDesc> &value_descs)
{
	curve_list.clear();
	CurveStruct curve_struct;
	for(std::list<ValueDesc>::const_iterator i = value_descs.begin(); i != value_descs.end(); ++i) {
		curve_struct.init(*i);
		if (curve_struct.channels.empty())
			continue;

		curve_list.push_back(curve_struct);

		if (i->is_value_node())
			value_desc_changed.push_back(
				i->get_value_node()->signal_changed().connect(
						sigc::mem_fun(*this, &Widget_Curves::refresh )));
		if (i->parent_is_value_node())
			value_desc_changed.push_back(
				i->get_parent_value_node()->signal_changed().connect(
					sigc::mem_fun(*this, &Widget_Curves::refresh )));
		if (i->parent_is_layer())
			value_desc_changed.push_back(
				i->get_layer()->signal_changed().connect(
					sigc::mem_fun(*this, &Widget_Curves::refresh )));
	}
	queue_draw();
}

bool
Widget_Curves::on_event(GdkEvent *event)
{
	switch(event->type) {
	case GDK_SCROLL: {
		switch(event->scroll.direction) {
			case GDK_SCROLL_UP:
			case GDK_SCROLL_RIGHT: {
				if (event->scroll.state & GDK_CONTROL_MASK) {
					// Ctrl+scroll , perform zoom in
					ConfigureAdjustment(range_adjustment)
						.set_page_size(range_adjustment->get_page_size()/1.25)
						.finish();
				} else {
					// Scroll up
					ConfigureAdjustment(range_adjustment)
						.set_value(range_adjustment->get_value() - range_adjustment->get_step_increment())
						.finish();
				}
				return true;
			}
			case GDK_SCROLL_DOWN:
			case GDK_SCROLL_LEFT: {
				if (event->scroll.state & GDK_CONTROL_MASK) {
					// Ctrl+scroll , perform zoom out
					ConfigureAdjustment(range_adjustment)
						.set_page_size(range_adjustment->get_page_size()*1.25)
						.finish();
				} else {
					// Scroll down
					ConfigureAdjustment(range_adjustment)
						.set_value(range_adjustment->get_value() + range_adjustment->get_step_increment())
						.finish();
				}
				return true;
			}
			default:
				break;
		}
		break;
	}
	default:
		break;
	}

	return Gtk::DrawingArea::on_event(event);
}

bool
Widget_Curves::on_draw(const Cairo::RefPtr<Cairo::Context> &cr)
{
	int w = get_width();
	int h = get_height();
	if (w <= 0 || h <= 0)
		return Gtk::DrawingArea::on_draw(cr);

	get_style_context()->render_background(cr, 0, 0, w, h);

	if (!time_model || !curve_list.size())
		return true;

	Time time  = time_model->get_time();
	Time lower = time_model->get_visible_lower();
	Time upper = time_model->get_visible_upper();
	double k = (double)w/(double)(upper - lower);
	Time dt(1.0/k);

	Real range_lower = range_adjustment->get_value();
	Real range_upper = range_lower + range_adjustment->get_page_size();
	double range_k = (double)h/(double)(range_upper - range_lower);

	cr->save();

	// Draw zero mark
	cr->set_source_rgb(0.31, 0.31, 0.31);
	cr->rectangle(0, etl::round_to_int((0.0 - range_lower)*range_k), w, 0);
	cr->stroke();

	// This try to find a valid canvas to show the keyframes of those
	// valuenodes. If not canvas found then no keyframes marks are shown.
	Canvas::Handle canvas;
	for(std::list<CurveStruct>::iterator i = curve_list.begin(); i != curve_list.end(); ++i) {
		canvas = i->value_desc.get_canvas();
		if (canvas) break;
	}

	if (canvas) {
		// draw vertical lines for the keyframes marks.
		for(KeyframeList::const_iterator i = canvas->keyframe_list().begin(); i != canvas->keyframe_list().end(); ++i) {
			if (!i->get_time().is_valid())
				continue;
			int x = (i->get_time() - lower)*k;
			if (i->get_time() >= lower && i->get_time() <= upper) {
				cr->set_source_rgb(0.63, 0.5, 0.5);
				cr->rectangle(x, 0, 1, h);
				cr->fill();
			}
		}
	}

	// Draw current time
	cr->set_source_rgb(0, 0, 1);
	cr->rectangle(etl::round_to_int((double)(time - lower)*k), 0, 0, h);
	cr->stroke();

	// reserve arrays for maximum number of channels
	int max_channels = 0;
	for(std::list<CurveStruct>::iterator i = curve_list.begin(); i != curve_list.end(); ++i)
		max_channels = std::max(max_channels, (int)i->channels.size());
	std::vector< std::vector<Gdk::Point> > points(max_channels);

	Real range_max = -100000000.0;
	Real range_min =  100000000.0;

	// Draw curves for the valuenodes stored in the curve list
	for(std::list<CurveStruct>::iterator i = curve_list.begin(); i != curve_list.end(); ++i) {
		int channels = (int)i->channels.size();
		if (channels > (int)points.size())
			points.resize(channels);

		for(int c = 0; c < channels; ++c) {
			points[c].clear();
			points[c].reserve(w);
		}

		Time t = lower;
		for(int j = 0; j < w; ++j, t += dt) {
			for(int c = 0; c < channels; ++c) {
				Real x = i->get_value(c, t, dt);
				range_max = std::max(range_max, x);
				range_min = std::min(range_min, x);
				points[c].push_back( Gdk::Point(j, etl::round_to_int((x - range_lower)*range_k)) );
			}
		}

		// Draw the graph curves with 0.5 width
		cr->set_line_width(0.5);
		for(int c = 0; c < channels; ++c) {
			// Draw the curve
			std::vector<Gdk::Point> &p = points[c];
			for(std::vector<Gdk::Point>::iterator j = p.begin(); j != p.end(); ++j) {
				if (j == p.begin())
					cr->move_to(j->get_x(), j->get_y());
				else
					cr->line_to(j->get_x(), j->get_y());
			}
			Gdk::Cairo::set_source_color(cr, i->channels[c].color);
			cr->stroke();

			Glib::RefPtr<Pango::Layout> layout(Pango::Layout::create(get_pango_context()));
			layout->set_text(i->channels[c].name);

			cr->move_to(1, points[c][0].get_y() + 1);
			layout->show_in_cairo_context(cr);
		}
	}

	if (!curve_list.empty() && range_min < range_max)
		ConfigureAdjustment(range_adjustment)
			.set_lower(range_min - 0.5*range_adjustment->get_page_size())
			.set_upper(range_max + 0.5*range_adjustment->get_page_size())
			.set_step_increment(range_adjustment->get_page_size()*20.0/(double)h) // 20 pixels
			.finish();
	cr->restore();

	return true;
}
