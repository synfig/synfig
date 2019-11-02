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
#include <synfig/general.h>

#include <gui/helpers.h>

#include "widget_curves.h"
#include "gui/timeplotdata.h"

#include "gui/waypointrenderer.h"
#include <synfig/layers/layer_pastecanvas.h>
#include <synfig/valuenodes/valuenode_dynamiclist.h>

#endif

/* === U S I N G =========================================================== */

using namespace synfig;
using namespace synfigapp;
using namespace studio;

/* === M A C R O S ========================================================= */

#define MAX_CHANNELS 15
#define ZOOM_CHANGING_FACTOR 1.25
#define DEFAULT_PAGE_SIZE 2.0

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
			return i->second;

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

		return channels[channel].values[time];
	}
};

/* === M E T H O D S ======================================================= */

Widget_Curves::Widget_Curves():
	range_adjustment(Gtk::Adjustment::create(-1.0, -2.0, 2.0, 0.1, 0.1, DEFAULT_PAGE_SIZE)),
	waypoint_edge_length(16),
	pointer_state(POINTER_NONE)
{
	set_size_request(64, 64);

	add_events(Gdk::BUTTON_PRESS_MASK | Gdk::BUTTON_RELEASE_MASK | Gdk::SCROLL_MASK | Gdk::POINTER_MOTION_MASK);

	time_plot_data = new TimePlotData(*this, range_adjustment);
	time_plot_data->set_extra_time_margin(16/2);
}

Widget_Curves::~Widget_Curves() {
	clear();
	set_time_model(etl::handle<TimeModel>());
	delete time_plot_data;
}

const etl::handle<TimeModel>&
Widget_Curves::get_time_model() const
{
	return time_plot_data->time_model;
}

void
Widget_Curves::set_time_model(const etl::handle<TimeModel> &x)
{
	time_plot_data->set_time_model(x);
}

void
Widget_Curves::clear() {
	while(!value_desc_changed.empty()) {
		value_desc_changed.back().disconnect();
		value_desc_changed.pop_back();
	}
	curve_list.clear();
	hovered_point.invalidate();
}

void
Widget_Curves::refresh()
{
	for(std::list<CurveStruct>::iterator i = curve_list.begin(); i != curve_list.end(); ++i)
		i->clear_all_values();
	hovered_point.invalidate();
	queue_draw();
}

void Widget_Curves::zoom_in()
{
	set_zoom(get_zoom() * ZOOM_CHANGING_FACTOR);
}

void Widget_Curves::zoom_out()
{
	set_zoom(get_zoom() / ZOOM_CHANGING_FACTOR);
}

void Widget_Curves::zoom_100()
{
	set_zoom(1.0);
}

void Widget_Curves::set_zoom(double new_zoom_factor)
{
	int x, y;
	get_pointer(x, y);
	double perc_y = y/(get_height()+0.0);
	double y_value = perc_y * range_adjustment->get_page_size() + range_adjustment->get_value();
	double new_range_page_size = DEFAULT_PAGE_SIZE / new_zoom_factor;
	double new_range_value = y_value - perc_y * new_range_page_size;
	ConfigureAdjustment(range_adjustment)
		.set_page_size(new_range_page_size)
		.set_value(new_range_value)
		.finish();
}

double Widget_Curves::get_zoom() const
{
	return DEFAULT_PAGE_SIZE / range_adjustment->get_page_size();
}

void Widget_Curves::scroll_up()
{
	ConfigureAdjustment(range_adjustment)
		.set_value(range_adjustment->get_value() - range_adjustment->get_step_increment())
			.finish();
}

void Widget_Curves::scroll_down()
{
	ConfigureAdjustment(range_adjustment)
		.set_value(range_adjustment->get_value() + range_adjustment->get_step_increment())
		.finish();
}

void
Widget_Curves::set_value_descs(etl::handle<synfigapp::CanvasInterface> canvas_interface_, const std::list<ValueDesc> &value_descs)
{
	canvas_interface = canvas_interface_;
	clear();
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
					zoom_in();
				} else {
					// Scroll up
					scroll_up();
				}
				return true;
			}
			case GDK_SCROLL_DOWN:
			case GDK_SCROLL_LEFT: {
				if (event->scroll.state & GDK_CONTROL_MASK) {
					// Ctrl+scroll , perform zoom out
					zoom_out();
				} else {
					// Scroll down
					scroll_down();
				}
				return true;
			}
			default:
				break;
		}
		break;
	}
	case GDK_MOTION_NOTIFY: {
		auto previous_hovered_point = hovered_point;
		hovered_point.invalidate();

		int pointer_x, pointer_y;
		get_pointer(pointer_x, pointer_y);
		if (pointer_state != POINTER_DRAGGING)
			find_channelpoint_at_position(pointer_x, pointer_y, hovered_point);

		if (previous_hovered_point != hovered_point)
			queue_draw();

		if (pointer_state == POINTER_DRAGGING) {
			drag(pointer_x, pointer_y);
		}
		if (pointer_state != POINTER_NONE) {
			queue_draw();
		}
		break;
	}
	case GDK_BUTTON_PRESS: {
		if (event->button.button == 3) {
			// cancel/undo current action
			if (pointer_state != POINTER_NONE) {
				cancel_dragging();
				pointer_state = POINTER_NONE;
				queue_draw();
			}
		} else if (event->button.button == 1) {
			if (pointer_state == POINTER_NONE) {
				get_pointer(pointer_tracking_start_x, pointer_tracking_start_y);
				ChannelPoint pointed_item;
				find_channelpoint_at_position(pointer_tracking_start_x, pointer_tracking_start_y, pointed_item);
				if (pointed_item.is_valid()) {
					bool is_already_selected = std::find(selected_points.begin(), selected_points.end(), pointed_item) != selected_points.end();
					if (is_already_selected) {
						if ((event->button.state & (GDK_CONTROL_MASK|GDK_SHIFT_MASK)) == 0) {
							start_dragging(pointed_item);
							pointer_state = POINTER_DRAGGING;
						} else
							pointer_state = POINTER_SELECTING;
					} else {
						selected_points.clear();
						selected_points.push_back(pointed_item);
						start_dragging(pointed_item);
						pointer_state = POINTER_DRAGGING;
					}
				} else {
					pointer_state = POINTER_SELECTING;
				}
			}
		}
		break;
	}
	case GDK_BUTTON_RELEASE: {
		int pointer_x, pointer_y;
		get_pointer(pointer_x, pointer_y);

		if (event->button.button == 1) {
			bool selection_changed = false;

			if (pointer_state == POINTER_SELECTING) {
				std::vector<ChannelPoint> cps;
				int x0 = std::min(pointer_tracking_start_x, pointer_x);
				int width = std::abs(pointer_tracking_start_x - pointer_x);
				int y0 = std::min(pointer_tracking_start_y, pointer_y);
				int height = std::abs(pointer_tracking_start_y - pointer_y);
				if (width < 1 && height < 1) {
					width = 1;
					height = 1;
				}
				Gdk::Rectangle rect(x0, y0, width, height);
				bool found = find_channelpoints_in_rect(rect, cps);
				if (!found) {
					if (selected_points.size() > 0 && (event->button.state & (GDK_SHIFT_MASK | GDK_CONTROL_MASK)) == 0) {
						selection_changed = true;
						selected_points.clear();
					}
				} else {
					if ((event->button.state & GDK_CONTROL_MASK) == GDK_CONTROL_MASK) {
						for (ChannelPoint cp : cps) {
							std::vector<ChannelPoint>::iterator already_selection_it = std::find(selected_points.begin(), selected_points.end(), cp);
							bool already_selected = already_selection_it != selected_points.end();
							if (already_selected) {
								selected_points.erase(already_selection_it);
								selection_changed = true;
							}
						}
					} else if ((event->button.state & GDK_SHIFT_MASK) == GDK_SHIFT_MASK) {
						for (ChannelPoint cp : cps) {
							std::vector<ChannelPoint>::iterator already_selection_it = std::find(selected_points.begin(), selected_points.end(), cp);
							bool already_selected = already_selection_it != selected_points.end();
							if (!already_selected) {
								selected_points.push_back(cp);
								selection_changed = true;
							}
						}
					} else {
						selected_points.clear();
						selected_points = cps;
						selection_changed = true;
					}
				}
			}
			else if (pointer_state == POINTER_DRAGGING) {
				if (event->button.button == 1) {
					finish_dragging();
				}
			}

			if (selection_changed)
				queue_draw();
		}

		if (event->button.button == 1 || event->button.button == 3) {
			if (pointer_state != POINTER_NONE) {
				pointer_state = POINTER_NONE;
				queue_draw();
			}
		}

		break;
	}
	default:
		break;
	}

	return Gtk::DrawingArea::on_event(event);
}

bool Widget_Curves::find_channelpoint_at_position(int pos_x, int pos_y, ChannelPoint & cp)
{
	cp.invalidate();
	bool found = false;
	for(auto curve_it = curve_list.begin(); curve_it != curve_list.end(); ++curve_it) {
		int channels = (int)curve_it->channels.size();

		WaypointRenderer::foreach_visible_waypoint(curve_it->value_desc, *time_plot_data,
			[&](const synfig::TimePoint &tp, const synfig::Time &t, void *data) -> bool
		{
			int px = time_plot_data->get_pixel_t_coord(t);
			for (int c = 0; c < channels; ++c) {
				Real y = curve_it->get_value(c, t, time_plot_data->dt);
				int py = time_plot_data->get_pixel_y_coord(y);

				if (pos_x > px - waypoint_edge_length/2 && pos_x <= px + waypoint_edge_length/2) {
					if (pos_y > py - waypoint_edge_length/2 && pos_y <= py + waypoint_edge_length/2) {
						cp.curve_it = curve_it;
						cp.time_point = tp;
						cp.channel_idx = c;
						*static_cast<bool*>(data) = true;
						return true;
					}
				}
			}
			*static_cast<bool*>(data) = false;
			return false;
		}, &found);

		if (found)
			break;
	}
	return found;
}

bool Widget_Curves::find_channelpoints_in_rect(Gdk::Rectangle rect, std::vector<ChannelPoint> & list)
{
	list.clear();

	int x0 = rect.get_x();
	int x1 = rect.get_x() + rect.get_width();
	if (x0 > x1)
		std::swap(x0, x1);
	int y0 = rect.get_y();
	int y1 = rect.get_y() + rect.get_height();
	if (y0 > y1)
		std::swap(y0, y1);

	for(auto curve_it = curve_list.begin(); curve_it != curve_list.end(); ++curve_it) {
		int channels = (int)curve_it->channels.size();

		WaypointRenderer::foreach_visible_waypoint(curve_it->value_desc, *time_plot_data,
			[&](const synfig::TimePoint &tp, const synfig::Time &t, void *data) -> bool
		{
			int px = time_plot_data->get_pixel_t_coord(t);
			for (int c = 0; c < channels; ++c) {
				Real y = curve_it->get_value(c, t, time_plot_data->dt);
				int py = time_plot_data->get_pixel_y_coord(y);

				if (x0 < px + waypoint_edge_length/2 && x1 >= px - waypoint_edge_length/2) {
					if (y0 < py + waypoint_edge_length/2 && y1 >= py - waypoint_edge_length/2) {
						list.push_back(ChannelPoint(curve_it, tp, c));
					}
				}
			}
			return false;
		});
	}
	return list.size() > 0;
}

void Widget_Curves::start_dragging(const ChannelPoint& pointed_item)
{
	active_point = pointed_item;
	active_point_initial_y = time_plot_data->get_pixel_y_coord(pointed_item.get_value(time_plot_data->dt));
	is_dragging_initial_edit_mode_animate = canvas_interface->get_mode() & EditMode::MODE_ANIMATE;
	canvas_interface->set_mode(canvas_interface->get_mode() | EditMode::MODE_ANIMATE);
	pointer_state = POINTER_DRAGGING;
}

void Widget_Curves::drag(int pointer_x, int pointer_y)
{
	int pointer_dy = pointer_y - pointer_tracking_start_y;
	int current_y = time_plot_data->get_pixel_y_coord(active_point.get_value(time_plot_data->dt));
	int waypoint_dy = current_y - active_point_initial_y;
	int dy = pointer_dy - waypoint_dy;
	for (auto point : selected_points) {
		if (!point.curve_it->value_desc.is_animated())
			continue;
		Time time = point.time_point.get_time();
		Real v = point.get_value(time_plot_data->dt);

		int pix_y = time_plot_data->get_pixel_y_coord(v);
		pix_y += dy;
		v = time_plot_data->get_y_from_pixel_coord(pix_y);
		ValueBase value_base = point.curve_it->value_desc.get_value(time);

		set_value_base_for_channel_point(value_base, point, v);

		bool ok = canvas_interface->change_value_at_time(point.curve_it->value_desc, value_base, time);
		if (!ok) {
			cancel_dragging();
			pointer_state = POINTER_NONE;
		}
	}
	queue_draw();
}

void Widget_Curves::finish_dragging()
{
	if (!is_dragging_initial_edit_mode_animate)
		canvas_interface->set_mode(canvas_interface->get_mode() - EditMode::MODE_ANIMATE);
	pointer_state = POINTER_NONE;
}

void Widget_Curves::cancel_dragging()
{
	if (pointer_state != POINTER_DRAGGING)
		return;
	int current_y = time_plot_data->get_pixel_y_coord(active_point.get_value(time_plot_data->dt));
	int waypoint_dy = current_y - active_point_initial_y;
	int dy = - waypoint_dy;
	for (auto point : selected_points) {
		if (!point.curve_it->value_desc.is_animated())
			continue;
		auto time = point.time_point.get_time();
		auto v = point.get_value(time_plot_data->dt);

		auto pix_y = time_plot_data->get_pixel_y_coord(v);
		pix_y += dy;
		v = time_plot_data->get_y_from_pixel_coord(pix_y);
		auto value_base = point.curve_it->value_desc.get_value(time);

		set_value_base_for_channel_point(value_base, point, v);

		canvas_interface->change_value_at_time(point.curve_it->value_desc, value_base, time);
	}
	if (!is_dragging_initial_edit_mode_animate)
		canvas_interface->set_mode(canvas_interface->get_mode() - EditMode::MODE_ANIMATE);
	queue_draw();
}

void Widget_Curves::set_value_base_for_channel_point(ValueBase& value_base, const Widget_Curves::ChannelPoint& channel_point, Real v)
{
	Type& type = channel_point.curve_it->value_desc.get_value_type();
	if (type == type_real) {
		value_base.set(v);
	} else
	if (type == type_time) {
		value_base.set(Time(v));
	} else
	if (type == type_integer) {
		value_base.set((int)v);
	} else
	if (type == type_bool) {
		value_base.set(v > 0.5);
	} else
	if (type == type_angle) {
		value_base.set(Angle::rad(v));
	} else
	if (type == type_color) {
		v = clamp(v, 0.0, 1.0);
		auto color = value_base.get(Color());
		switch (channel_point.channel_idx) {
		case 0:
			color.set_r(v);
			break;
		case 1:
			color.set_g(v);
			break;
		case 2:
			color.set_b(v);
			break;
		case 3:
			color.set_a(v);
			break;
		default:
			synfig::error("Invalid index for Color curve channel: %d", channel_point.channel_idx);
			break;
		}

		value_base.set(color);
	} else
	if (type == type_vector) {
		if (channel_point.channel_idx > 1) {
			synfig::error("Invalid index for Vector curve channel: %d", channel_point.channel_idx);
		} else {
			auto vector = value_base.get(Vector());
			vector[channel_point.channel_idx] = v;
			value_base.set(vector);
		}
	} else
	if (type == type_bline_point) {
		BLinePoint bline_point = value_base.get(BLinePoint());
		switch (channel_point.channel_idx) {
		case 0: {
			Vector vertex = bline_point.get_vertex();
			vertex[0] = v;
			bline_point.set_vertex(vertex);
			break;
		}
		case 1: {
			Vector vertex = bline_point.get_vertex();
			vertex[1] = v;
			bline_point.set_vertex(vertex);
			break;
		}
		case 2:
			bline_point.set_width( v < 0 ? 0 : v);
			break;
		case 3:
			bline_point.set_origin(v);
			break;
		case 4:
			bline_point.set_split_tangent_both(v > 0.5);
			break;
		case 5: {
			Vector tangent = bline_point.get_tangent1();
			tangent[0] = v;
			bline_point.set_tangent1(tangent);
			break;
		}
		case 6: {
			Vector tangent = bline_point.get_tangent1();
			tangent[1] = v;
			bline_point.set_tangent1(tangent);
			break;
		}
		case 7: {
			Vector tangent = bline_point.get_tangent2();
			tangent[0] = v;
			bline_point.set_tangent2(tangent);
			break;
		}
		case 8: {
			Vector tangent = bline_point.get_tangent2();
			tangent[1] = v;
			bline_point.set_tangent2(tangent);
			break;
		}
		case 9:
			bline_point.set_split_tangent_radius(v > 0.5);
			break;
		case 10:
			bline_point.set_split_tangent_angle(v > 0.5);
			break;
		default:
			synfig::error("Invalid index for BLinePoint curve channel: %d", channel_point.channel_idx);
		}
		value_base.set(bline_point);
	} else
	if (type == type_width_point) {
		WidthPoint width_point = value_base.get(WidthPoint());
		if (channel_point.channel_idx == 0) {
			width_point.set_position(v);
		} else if (channel_point.channel_idx == 1) {
			if (v < 0)
				v = 0;
			width_point.set_width(v);
		} else {
			synfig::error("Invalid index for WidthPoint curve channel: %d", channel_point.channel_idx);
		}
		value_base.set(width_point);
	} else
	if (type == type_dash_item) {
		DashItem dash_item = value_base.get(DashItem());
		if (channel_point.channel_idx == 0) {
			dash_item.set_offset(v);
		} else if (channel_point.channel_idx == 1) {
			if (v < 0)
				v = 0;
			dash_item.set_length(v);
		} else {
			synfig::error("Invalid index for DashItem curve channel: %d", channel_point.channel_idx);
		}
		value_base.set(dash_item);
	}
}

bool
Widget_Curves::on_draw(const Cairo::RefPtr<Cairo::Context> &cr)
{
	int w = get_width();
	int h = get_height();
	if (w <= 0 || h <= 0)
		return Gtk::DrawingArea::on_draw(cr);

	get_style_context()->render_background(cr, 0, 0, w, h);

	if (!time_plot_data->time_model || !curve_list.size())
		return true;

	if (time_plot_data->is_invalid())
		return true;

	cr->save();

	// Draw zero mark
	cr->set_source_rgb(0.31, 0.31, 0.31);
	cr->rectangle(0, time_plot_data->get_pixel_y_coord(0.0), w, 0);
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
			if (time_plot_data->is_time_visible(i->get_time())) {
				int x = time_plot_data->get_pixel_t_coord(i->get_time());
				cr->set_source_rgb(0.63, 0.5, 0.5);
				cr->rectangle(x, 0, 1, h);
				cr->fill();
			}
		}
	}

	// Draw current time
	cr->set_source_rgb(0, 0, 1);
	cr->rectangle(time_plot_data->get_pixel_t_coord(time_plot_data->time), 0, 0, h);
	cr->stroke();

	// reserve arrays for maximum number of channels
	int max_channels = 0;
	for(std::list<CurveStruct>::iterator i = curve_list.begin(); i != curve_list.end(); ++i)
		max_channels = std::max(max_channels, (int)i->channels.size());
	std::vector< std::vector<Gdk::Point> > points(max_channels);

	Real range_max = -100000000.0;
	Real range_min =  100000000.0;

	// Draw curves for the valuenodes stored in the curve list
	for(std::list<CurveStruct>::iterator curve_it = curve_list.begin(); curve_it != curve_list.end(); ++curve_it) {
		int channels = (int)curve_it->channels.size();
		if (channels > (int)points.size())
			points.resize(channels);

		for(int c = 0; c < channels; ++c) {
			points[c].clear();
			points[c].reserve(w);
		}

		Time t = time_plot_data->lower;
		for(int j = 0; j < w; ++j, t += time_plot_data->dt) {
			for(int c = 0; c < channels; ++c) {
				Real y = curve_it->get_value(c, t, time_plot_data->dt);
				range_max = std::max(range_max, y);
				range_min = std::min(range_min, y);
				points[c].push_back( Gdk::Point(j, time_plot_data->get_pixel_y_coord(y)) );
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
			Gdk::Cairo::set_source_color(cr, curve_it->channels[c].color);
			cr->stroke();

			Glib::RefPtr<Pango::Layout> layout(Pango::Layout::create(get_pango_context()));
			layout->set_text(curve_it->channels[c].name);

			cr->move_to(1, points[c][0].get_y() + 1);
			layout->show_in_cairo_context(cr);
		}

		// Draw waypoints
		WaypointRenderer::foreach_visible_waypoint(curve_it->value_desc, *time_plot_data,
			[&](const synfig::TimePoint &tp, const synfig::Time &t, void *_data) -> bool
		{
			int px = time_plot_data->get_pixel_t_coord(t);
			Gdk::Rectangle area(
						0 - waypoint_edge_length/2 + 1 + px,
						0, //0 - waypoint_edge_length/2 + 1 + py,
						waypoint_edge_length - 2,
						waypoint_edge_length - 2);
			bool hover = hovered_point.is_valid() && tp == hovered_point.time_point && hovered_point.curve_it == curve_it;
			for (int c = 0; c < channels; ++c) {
				Real y = curve_it->get_value(c, t, time_plot_data->dt);
				int py = time_plot_data->get_pixel_y_coord(y);
				area.set_y(0 - waypoint_edge_length/2 + 1 + py);

				std::vector<ChannelPoint>::iterator selection_it = std::find(selected_points.begin(), selected_points.end(), ChannelPoint(curve_it, tp, c));
				bool selected = selection_it != selected_points.end();
				WaypointRenderer::render_time_point_to_window(cr, area, tp, selected, hover);
			}
			return false;
		});
	}

	// Draw selection rectangle
	if (pointer_state == POINTER_SELECTING) {
		static const std::vector<double>dashed3 = {5.0};
		cr->set_dash(dashed3, 0);
		int x1, y1;
		get_pointer(x1, y1);
		cr->rectangle(pointer_tracking_start_x, pointer_tracking_start_y, x1 - pointer_tracking_start_x, y1 - pointer_tracking_start_y);
		 // set up a dashed solid-color stroke
		cr->stroke();
	}

	if (!curve_list.empty() && range_min < range_max)
		ConfigureAdjustment(range_adjustment)
			.set_lower(-range_max - 0.5*range_adjustment->get_page_size())
			.set_upper(-range_min + 0.5*range_adjustment->get_page_size())
			.set_step_increment(range_adjustment->get_page_size()*20.0/(double)h) // 20 pixels
			.finish();
	cr->restore();

	return true;
}

Widget_Curves::ChannelPoint::ChannelPoint()
{
	invalidate();
}

Widget_Curves::ChannelPoint::ChannelPoint(std::list<CurveStruct>::iterator& curve_it, const TimePoint time_point, int channel_idx) :
	curve_it(curve_it), time_point(time_point), channel_idx(channel_idx)
{
}

void Widget_Curves::ChannelPoint::invalidate()
{
	channel_idx = -1;
}

bool Widget_Curves::ChannelPoint::is_valid() const
{
	return channel_idx >= 0;
}

bool Widget_Curves::ChannelPoint::operator ==(const Widget_Curves::ChannelPoint& b) const
{
	return curve_it == b.curve_it && time_point == b.time_point && channel_idx == b.channel_idx;
}

Real Widget_Curves::ChannelPoint::get_value(Real time_tolerance) const
{
	return curve_it->get_value(channel_idx, time_point.get_time(), time_tolerance);
}
