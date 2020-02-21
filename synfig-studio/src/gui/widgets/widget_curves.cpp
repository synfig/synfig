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
#include <synfig/timepointcollect.h>

#include <gui/helpers.h>

#include "widget_curves.h"
#include "gui/timeplotdata.h"

#include "gui/waypointrenderer.h"
#include <synfig/layers/layer_pastecanvas.h>
#include <synfig/valuenodes/valuenode_dynamiclist.h>

#include "instance.h"
#include <synfigapp/action_system.h>

#include <gui/localization.h>

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
	std::string name;
	ValueDesc value_desc;
	std::vector<Channel> channels;

	void add_channel(const String &name, const Gdk::Color &color)
		{ channels.push_back(Channel(name, color)); }
	void add_channel(const String &name, const String &color)
		{ add_channel(name, Gdk::Color(color)); }

	CurveStruct() { }

	explicit CurveStruct(const ValueDesc& x, std::string name)
		: name(name)
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

	Real get_value(size_t channel, Real time, Real tolerance) {
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
		std::vector<Real> channel_values;
		bool ok = get_value_base_channel_values(value, channel_values);
		if (!ok)
			return Real(0.0);

		for (size_t c = 0; c < channel_values.size(); c++) {
			channels[c].values[time] = channel_values[c];
		}

		return channels[channel].values[time];
	}

	static bool get_value_base_channel_values(const ValueBase &value_base, std::vector<Real>& channels) {
		channels.clear();
		Type &type(value_base.get_type());
		if (type == type_real) {
			channels.push_back(value_base.get(Real()));
		} else
		if (type == type_time) {
			channels.push_back(value_base.get(Time()));
		} else
		if (type == type_integer) {
			channels.push_back(value_base.get(int()));
		} else
		if (type == type_bool) {
			channels.push_back(value_base.get(bool()));
		} else
		if (type == type_angle) {
			channels.push_back(Angle::rad(value_base.get(Angle())).get());
		} else
		if (type == type_color) {
			const Color & color = value_base.get(Color());
			channels.push_back(color.get_r());
			channels.push_back(color.get_g());
			channels.push_back(color.get_b());
			channels.push_back(color.get_a());
		} else
		if (type == type_vector) {
			const Vector& vector = value_base.get(Vector());
			channels.push_back(vector[0]);
			channels.push_back(vector[1]);
		} else
		if (type == type_bline_point) {
			const BLinePoint &bline_point = value_base.get(BLinePoint());
			channels.push_back(bline_point.get_vertex()[0]);
			channels.push_back(bline_point.get_vertex()[1]);
			channels.push_back(bline_point.get_width());
			channels.push_back(bline_point.get_origin());
			channels.push_back(bline_point.get_split_tangent_both());
			channels.push_back(bline_point.get_tangent1()[0]);
			channels.push_back(bline_point.get_tangent1()[1]);
			channels.push_back(bline_point.get_tangent2()[0]);
			channels.push_back(bline_point.get_tangent2()[1]);
			channels.push_back(bline_point.get_split_tangent_radius());
			channels.push_back(bline_point.get_split_tangent_angle());
		} else
		if (type == type_width_point) {
			const WidthPoint &width_point = value_base.get(WidthPoint());
			channels.push_back(width_point.get_position());
			channels.push_back(width_point.get_width());
		} else
		if (type == type_dash_item) {
			const DashItem &dash_item = value_base.get(DashItem());
			channels.push_back(dash_item.get_offset());
			channels.push_back(dash_item.get_length());
		} else {
			return false;
		}

		return true;
	}

	static bool set_value_base_channel_value(ValueBase& value_base, size_t channel_idx, Real v)
	{
		Type& type = value_base.get_type();
		if (type == type_real) {
			if (channel_idx > 0) {
				synfig::error("Invalid index for Real curve channel: %d", channel_idx);
				return false;
			} else {
				value_base.set(v);
			}
		} else
		if (type == type_time) {
			if (channel_idx > 0) {
				synfig::error("Invalid index for Time curve channel: %d", channel_idx);
				return false;
			} else {
				value_base.set(Time(v));
			}
		} else
		if (type == type_integer) {
			if (channel_idx > 0) {
				synfig::error("Invalid index for Integer curve channel: %d", channel_idx);
				return false;
			} else {
				value_base.set((int)v);
			}
		} else
		if (type == type_bool) {
			if (channel_idx > 0) {
				synfig::error("Invalid index for Bool curve channel: %d", channel_idx);
				return false;
			} else {
				value_base.set(v > 0.5);
			}
		} else
		if (type == type_angle) {
			if (channel_idx > 0) {
				synfig::error("Invalid index for Real curve channel: %d", channel_idx);
				return false;
			} else {
				value_base.set(Angle::rad(v));
			}
		} else
		if (type == type_color) {
			v = clamp(v, 0.0, 1.0);
			auto color = value_base.get(Color());
			switch (channel_idx) {
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
				synfig::error("Invalid index for Color curve channel: %d", channel_idx);
				return false;
			}

			value_base.set(color);
		} else
		if (type == type_vector) {
			if (channel_idx > 1) {
				synfig::error("Invalid index for Vector curve channel: %d", channel_idx);
				return false;
			} else {
				auto vector = value_base.get(Vector());
				vector[channel_idx] = v;
				value_base.set(vector);
			}
		} else
		if (type == type_bline_point) {
			BLinePoint bline_point = value_base.get(BLinePoint());
			switch (channel_idx) {
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
				synfig::error("Invalid index for BLinePoint curve channel: %d", channel_idx);
				return false;
			}
			value_base.set(bline_point);
		} else
		if (type == type_width_point) {
			WidthPoint width_point = value_base.get(WidthPoint());
			if (channel_idx == 0) {
				width_point.set_position(v);
			} else if (channel_idx == 1) {
				if (v < 0)
					v = 0;
				width_point.set_width(v);
			} else {
				synfig::error("Invalid index for WidthPoint curve channel: %d", channel_idx);
				return false;
			}
			value_base.set(width_point);
		} else
		if (type == type_dash_item) {
			DashItem dash_item = value_base.get(DashItem());
			if (channel_idx == 0) {
				dash_item.set_offset(v);
			} else if (channel_idx == 1) {
				if (v < 0)
					v = 0;
				dash_item.set_length(v);
			} else {
				synfig::error("Invalid index for DashItem curve channel: %d", channel_idx);
				return false;
			}
			value_base.set(dash_item);
		}
		return true;
	}

};

struct Tooltip
{
	const int outline_width = 2;
	const Color bg_color{0.9f, 0.9f, 0, 0.8f};
	const Color fg_color{0.5f, 0.5f, 0.5f, 1};
	const Color text_color{0.0f, 0.0f, 0.0f};

	int popup_margin = 0;
	int padding = 5;

	Gtk::Widget *widget;

	std::string text;

	Tooltip(Gtk::Widget *widget_)
		: widget(widget_)
	{}

	void draw(const Cairo::RefPtr<Cairo::Context>& cr, int pos_x, int pos_y) {

		guint cursor_w, cursor_h;
		cursor_w = cursor_h = widget->get_display()->get_default_cursor_size();
		int info_pos_x = pos_x;
		info_pos_x += cursor_w + popup_margin + padding;
		int info_pos_y = pos_y + padding;
//		info_pos_y += cursor_h;

		Glib::RefPtr<Pango::Layout> layout(Pango::Layout::create(widget->get_pango_context()));
		layout->set_markup(text);
		Pango::Rectangle text_rect = layout->get_pixel_ink_extents();
		if (info_pos_y + popup_margin + text_rect.get_height() > widget->get_height())
			info_pos_y = widget->get_height() - text_rect.get_height() - popup_margin - padding;
		else if (info_pos_y < popup_margin + padding)
			info_pos_y = popup_margin + padding;

		if (info_pos_x + padding + popup_margin + text_rect.get_width() > widget->get_width()) {
			info_pos_x = std::min(pos_x, widget->get_width());
			info_pos_x -= text_rect.get_width() + popup_margin + padding;
		} else if (info_pos_x < popup_margin) {
			info_pos_x = std::max(0, pos_x);
			info_pos_x += popup_margin + padding;
		}

		cr->set_source_rgba(bg_color.get_r(), bg_color.get_g(), bg_color.get_b(), bg_color.get_alpha());
		cr->rectangle(info_pos_x - outline_width - padding, info_pos_y - outline_width - padding, text_rect.get_width() + 2*outline_width + 2*padding, text_rect.get_height() + 2*outline_width + 2*padding);
		cr->fill_preserve();
		cr->set_source_rgba(fg_color.get_r(), fg_color.get_g(), fg_color.get_b(), fg_color.get_alpha());
		cr->set_line_width(1);
		cr->stroke();

		cr->move_to(info_pos_x, info_pos_y);
		cr->set_source_rgb(text_color.get_r(), text_color.get_g(), text_color.get_b());
		layout->show_in_cairo_context(cr);
	}
};

/* === M E T H O D S ======================================================= */

void Widget_Curves::on_waypoint_clicked(const Widget_Curves::ChannelPoint& cp, unsigned int button, Gdk::Point)
{
	std::set<synfig::Waypoint, std::less<UniqueID> > waypoint_set;
	synfig::waypoint_collect(waypoint_set, cp.time_point.get_time(), cp.curve_it->value_desc.get_value_node());
	signal_waypoint_clicked().emit(cp.curve_it->value_desc, waypoint_set, button);
}

void Widget_Curves::on_waypoint_double_clicked(const Widget_Curves::ChannelPoint& cp, unsigned int button, Gdk::Point)
{
	std::set<synfig::Waypoint, std::less<UniqueID> > waypoint_set;
	synfig::waypoint_collect(waypoint_set, cp.time_point.get_time(), cp.curve_it->value_desc.get_value_node());
	signal_waypoint_double_clicked().emit(cp.curve_it->value_desc, waypoint_set, button);
}

Widget_Curves::Widget_Curves()
	: Widget_TimeGraphBase(),
	  channel_point_sd(*this),
	  waypoint_edge_length(16)
{
	set_size_request(64, 64);

	add_events(Gdk::BUTTON_PRESS_MASK | Gdk::BUTTON_RELEASE_MASK | Gdk::SCROLL_MASK | Gdk::POINTER_MOTION_MASK | Gdk::KEY_PRESS_MASK | Gdk::KEY_RELEASE_MASK);

	set_can_focus(true);

	time_plot_data->set_extra_time_margin(16/2);

	channel_point_sd.set_pan_enabled(true);
	channel_point_sd.set_zoom_enabled(true);
	channel_point_sd.set_scroll_enabled(true);
	channel_point_sd.set_canvas_interface(canvas_interface);
	channel_point_sd.signal_drag_canceled().connect([&]() {
		overlapped_waypoints.clear();
	});
	channel_point_sd.signal_drag_finished().connect([&](bool /*started_by_keys*/) {
//		overlapped_waypoints.clear();
	});
	channel_point_sd.signal_redraw_needed().connect(sigc::mem_fun(*this, &Gtk::Widget::queue_draw));
	channel_point_sd.signal_focus_requested().connect(sigc::mem_fun(*this, &Gtk::Widget::grab_focus));
	channel_point_sd.signal_selection_changed().connect([=](){
		overlapped_waypoints.clear();
		queue_draw();
	});
	channel_point_sd.signal_zoom_in_requested().connect(sigc::mem_fun(*this, &Widget_Curves::zoom_in));
	channel_point_sd.signal_zoom_out_requested().connect(sigc::mem_fun(*this, &Widget_Curves::zoom_out));
	channel_point_sd.signal_scroll_up_requested().connect(sigc::mem_fun(*this, &Widget_Curves::scroll_up));
	channel_point_sd.signal_scroll_down_requested().connect(sigc::mem_fun(*this, &Widget_Curves::scroll_down));
	channel_point_sd.signal_panning_requested().connect(sigc::mem_fun(*this, &Widget_Curves::pan));
	channel_point_sd.signal_item_clicked().connect(sigc::mem_fun(*this, &Widget_Curves::on_waypoint_clicked));
	channel_point_sd.signal_item_double_clicked().connect(sigc::mem_fun(*this, &Widget_Curves::on_waypoint_double_clicked));
}

Widget_Curves::~Widget_Curves() {
	clear();
}

void
Widget_Curves::clear() {
	while(!value_desc_changed.empty()) {
		value_desc_changed.back().disconnect();
		value_desc_changed.pop_back();
	}
	curve_list.clear();
	channel_point_sd.clear();
}

void
Widget_Curves::refresh()
{
	for(std::list<CurveStruct>::iterator i = curve_list.begin(); i != curve_list.end(); ++i)
		i->clear_all_values();
	channel_point_sd.refresh();
	queue_draw();
}

void Widget_Curves::select_all_points()
{
	channel_point_sd.select_all_items();
}

void
Widget_Curves::set_value_descs(etl::handle<synfigapp::CanvasInterface> canvas_interface_, const std::list< std::pair<std::string, ValueDesc> > &data)
{
	if (canvas_interface_ != canvas_interface) {
		canvas_interface = canvas_interface_;
		channel_point_sd.set_canvas_interface(canvas_interface_);
	}
	clear();
	CurveStruct curve_struct;
	for(auto it = data.begin(); it != data.end(); ++it) {
		const ValueDesc *i = &it->second;

		curve_struct.name = it->first;
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
	if (channel_point_sd.process_event(event))
		return true;

	switch (event->type) {
	case GDK_KEY_PRESS:
		switch (event->key.keyval) {
		case GDK_KEY_Delete:
			delete_selected();
			return true;
		default:
			break;
		}
	case GDK_2BUTTON_PRESS:
		if (event->button.button == 1) {
			add_waypoint_to(event->button.x, event->button.y);
		}
		break;
	default:
		break;
	}

	return Widget_TimeGraphBase::on_event(event);
}

bool
Widget_Curves::on_draw(const Cairo::RefPtr<Cairo::Context> &cr)
{
	if (Widget_TimeGraphBase::on_draw(cr))
		return true;

	if (curve_list.size() == 0)
		return true;

	int w = get_width();
	int h = get_height();

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
	draw_current_time(cr);

	// reserve arrays for maximum number of channels
	size_t max_channels = 0;
	for(std::list<CurveStruct>::iterator i = curve_list.begin(); i != curve_list.end(); ++i)
		max_channels = std::max(max_channels, i->channels.size());
	std::vector< std::vector<Gdk::Point> > points(max_channels);

	Real range_max = -100000000.0;
	Real range_min =  100000000.0;

	// draw overlapped waypoints
	cr->set_line_width(.4);
	for (auto it : overlapped_waypoints) {
		const Waypoint &waypoint = it.first;
		const auto &curve_it = it.second;
		const size_t num_channels = curve_it->channels.size();

		const int x = time_plot_data->get_pixel_t_coord(waypoint.get_time());

		std::vector<Real> channel_values;
		CurveStruct::get_value_base_channel_values(waypoint.get_value(), channel_values);
		for (size_t c = 0; c < num_channels; c++) {
			Real value = curve_it->get_value(c, waypoint.get_time(), time_plot_data->dt);
			int y = time_plot_data->get_pixel_y_coord(value);

			Real old_value = channel_values[c];
			int old_y = time_plot_data->get_pixel_y_coord(old_value);

			range_max = std::max(range_max, old_value);
			range_min = std::min(range_min, old_value);

			Gdk::Cairo::set_source_color(cr, curve_it->channels[c].color);
			cr->move_to(x, y);
			cr->line_to(x, old_y);
			cr->stroke();
			cr->arc(x, old_y, waypoint_edge_length / 5, 0, 6.28);
			cr->fill_preserve();
			cr->stroke();
		}
	}

	// Draw curves for the valuenodes stored in the curve list
	for(std::list<CurveStruct>::iterator curve_it = curve_list.begin(); curve_it != curve_list.end(); ++curve_it) {
		size_t channels = curve_it->channels.size();
		if (channels > points.size())
			points.resize(channels);

		for(size_t c = 0; c < channels; ++c) {
			points[c].clear();
			points[c].reserve(w);
		}

		Time t = time_plot_data->lower;
		for(int j = 0; j < w; ++j, t += time_plot_data->dt) {
			for(size_t c = 0; c < channels; ++c) {
				Real y = curve_it->get_value(c, t, time_plot_data->dt);
				range_max = std::max(range_max, y);
				range_min = std::min(range_min, y);
				points[c].push_back( Gdk::Point(j, time_plot_data->get_pixel_y_coord(y)) );
			}
		}

		// Get last time point for this graph curve
		Time last_timepoint;
		const Node::time_set & tset = WaypointRenderer::get_times_from_valuedesc(curve_it->value_desc);
		for (const auto & timepoint : tset) {
			if (timepoint.get_time() > last_timepoint)
				last_timepoint = timepoint.get_time();
		}
		int last_timepoint_pixel = time_plot_data->get_pixel_t_coord(last_timepoint);

		// Draw the graph curves with 0.5 width
		cr->set_line_width(0.5);
		const std::vector<double> dashes4 = {4};
		const std::vector<double> no_dashes;
		for(size_t c = 0; c < channels; ++c) {
			// Draw the curve
			std::vector<Gdk::Point> &p = points[c];
			std::vector<Gdk::Point>::iterator p_it;
			for(p_it = p.begin(); p_it != p.end(); ++p_it) {
				if (p_it == p.begin())
					cr->move_to(p_it->get_x(), p_it->get_y());
				else
					cr->line_to(p_it->get_x(), p_it->get_y());
				if (p_it->get_x() >= last_timepoint_pixel)
					break;
			}
			Gdk::Cairo::set_source_color(cr, curve_it->channels[c].color);
			cr->stroke();

			// Draw the remaining curve
			if (p_it != p.end()) {
				for(; p_it != p.end(); ++p_it) {
					cr->line_to(p_it->get_x(), p_it->get_y());
				}
				cr->set_dash(dashes4, 0);
				cr->stroke();

				cr->set_dash(no_dashes, 0);
			}

			Glib::RefPtr<Pango::Layout> layout(Pango::Layout::create(get_pango_context()));
			layout->set_text(curve_it->channels[c].name);

			cr->move_to(1, points[c][0].get_y() + 1);
			layout->show_in_cairo_context(cr);
		}

		// Draw waypoints
		bool is_draggable = curve_it->value_desc.is_animated() || curve_it->value_desc.parent_is_linkable_value_node();
		if (!is_draggable) {
			cr->push_group();
		}
		WaypointRenderer::foreach_visible_waypoint(curve_it->value_desc, *time_plot_data,
			[&](const synfig::TimePoint &tp, const synfig::Time &t, void *_data) -> bool
		{
			int px = time_plot_data->get_pixel_t_coord(t);
			Gdk::Rectangle area(
						0 - waypoint_edge_length/2 + 1 + px,
						0, //0 - waypoint_edge_length/2 + 1 + py,
						waypoint_edge_length - 2,
						waypoint_edge_length - 2);
			const auto & hovered_point = channel_point_sd.get_hovered_item();
			bool hover = hovered_point.is_valid() && tp == hovered_point.time_point && hovered_point.curve_it == curve_it;
			for (size_t c = 0; c < channels; ++c) {
				Real y = curve_it->get_value(c, t, time_plot_data->dt);
				int py = time_plot_data->get_pixel_y_coord(y);
				area.set_y(0 - waypoint_edge_length/2 + 1 + py);

				bool selected = channel_point_sd.is_selected(ChannelPoint(curve_it, tp, c));
				WaypointRenderer::render_time_point_to_window(cr, area, tp, selected, hover);
			}
			return false;
		});
		if (!is_draggable) {
			cr->pop_group_to_source();
			cr->paint_with_alpha(0.5);
		}
	}

	// Draw selection rectangle
	if (channel_point_sd.get_state() == ChannelPointSD::State::POINTER_SELECTING) {
		// set up a dashed solid-color stroke
		static const std::vector<double>dashed3 = {5.0};
		cr->set_dash(dashed3, 0);

		int x0, y0;
		int x1, y1;
		channel_point_sd.get_initial_tracking_point(x0, y0);
		get_pointer(x1, y1);

		cr->rectangle(x0, y0, x1 - x0, y1 - y0);
		Gdk::RGBA color = get_style_context()->get_color();
		cr->set_source_rgb(color.get_red(), color.get_green(), color.get_blue());
		cr->stroke();
	}

	// Draw info about hovered item
	if (channel_point_sd.get_hovered_item().is_valid() || channel_point_sd.get_state() == channel_point_sd.POINTER_DRAGGING) {
		const ChannelPoint* inspected_item = &channel_point_sd.get_hovered_item();
		if (!inspected_item->is_valid())
			inspected_item = channel_point_sd.get_active_item();

		float fps = canvas_interface->get_canvas()->rend_desc().get_frame_rate();

		char buf[512];
		if (channel_point_sd.get_state() != channel_point_sd.POINTER_DRAGGING) {
			snprintf(buf, 511, _("%s:<b>%s</b>\n<b>Time:</b> %lfs (%if)\n<b>Value:</b> %lf"),
					inspected_item->curve_it->name.c_str(),
					inspected_item->curve_it->channels[inspected_item->channel_idx].name.c_str(),
					Real(inspected_item->time_point.get_time()),
					int(std::trunc(inspected_item->time_point.get_time()*fps)),
					inspected_item->get_value(time_plot_data->dt)
					);
		} else {
			int x0, y0;
			channel_point_sd.get_initial_tracking_point(x0, y0);
			Time t0 = time_plot_data->get_t_from_pixel_coord(x0).round(fps);
			Real value0 = time_plot_data->get_y_from_pixel_coord(y0);
			snprintf(buf, 511, _("%s:<b>%s</b>\n<b>Time:</b> %lfs (%if) <small>( \u0394 %if )</small>\n<b>Value:</b> %lf <small>( \u0394 %lf )</small>"),
					inspected_item->curve_it->name.c_str(),
					inspected_item->curve_it->channels[inspected_item->channel_idx].name.c_str(),
					Real(inspected_item->time_point.get_time()),
					int(std::trunc(inspected_item->time_point.get_time()*fps)),
					int(std::trunc((inspected_item->time_point.get_time() - t0)*fps)),
					inspected_item->get_value(time_plot_data->dt),
					inspected_item->get_value(time_plot_data->dt) - value0
					);
		}
		std::string text(buf);

		int item_pos_x = time_plot_data->get_pixel_t_coord(inspected_item->time_point.get_time());
		int item_pos_y = time_plot_data->get_pixel_y_coord(inspected_item->get_value(time_plot_data->dt));

		Tooltip tooltip(this);
		tooltip.text = text;
		tooltip.popup_margin = waypoint_edge_length;
		tooltip.draw(cr, item_pos_x, item_pos_y);
	}

	if (channel_point_sd.get_state() != channel_point_sd.POINTER_DRAGGING && channel_point_sd.get_state() != channel_point_sd.POINTER_PANNING && !curve_list.empty() && range_min < range_max)
		ConfigureAdjustment(range_adjustment)
			.set_lower(-range_max - 0.5*range_adjustment->get_page_size())
			.set_upper(-range_min + 0.5*range_adjustment->get_page_size())
			.set_step_increment(range_adjustment->get_page_size()*20.0/(double)h) // 20 pixels
			.finish();
	cr->restore();

	return true;
}

void
Widget_Curves::delete_selected()
{
	Action::PassiveGrouper group(canvas_interface->get_instance().get(), _("Remove Waypoints"));
	for (ChannelPoint *cp : channel_point_sd.get_selected_items()) {
		std::set<synfig::Waypoint, std::less<UniqueID> > waypoint_set;
		synfig::waypoint_collect(waypoint_set, cp->time_point.get_time(), cp->curve_it->value_desc.get_value_node());
		for (const Waypoint &waypoint : waypoint_set) {
			canvas_interface->waypoint_remove(cp->curve_it->value_desc, waypoint);
		}
	}
}

bool
Widget_Curves::add_waypoint_to(int point_x, int point_y)
{
	// Search for nearest curve within a certain limited margin of tolerance
	int tolerance = 5;
	Time time = time_plot_data->get_t_from_pixel_coord(point_x);
	CurveStruct * clicked_curve = nullptr;

	for (auto &curve : curve_list) {
		for (size_t cidx = 0; cidx < curve.channels.size(); cidx++) {
			auto pixel = time_plot_data->get_pixel_y_coord(curve.get_value(cidx, time, time_plot_data->dt));
			int diff = std::abs(point_y - pixel);
			if ( diff < tolerance) {
				clicked_curve = &curve;
				tolerance = diff;
			}
		}
	}

	if (!clicked_curve)
		return false;

	// Add waypoint

	Action::Handle 	action(Action::create("WaypointAdd"));

	assert(action);
	if(!action)
		return false;

	Waypoint waypoint(clicked_curve->value_desc.get_value(time), time);

	action->set_param("canvas", canvas_interface->get_canvas());
	action->set_param("widget.canvas_interface", canvas_interface);
	action->set_param("value_node", clicked_curve->value_desc.get_value_node());
	action->set_param("time", time);
	action->set_param("waypoint", waypoint);

	if(!canvas_interface->get_instance()->perform_action(action))
		canvas_interface->get_ui_interface()->error(_("Action Failed."));
	return true;
}

Widget_Curves::ChannelPoint::ChannelPoint()
{
	invalidate();
}

Widget_Curves::ChannelPoint::ChannelPoint(std::list<CurveStruct>::iterator& curve_it, const TimePoint time_point, size_t channel_idx) :
	curve_it(curve_it), time_point(time_point), channel_idx(channel_idx)
{
}

void Widget_Curves::ChannelPoint::invalidate()
{
	channel_idx = MAX_CHANNELS;
}

bool Widget_Curves::ChannelPoint::is_valid() const
{
	return channel_idx < MAX_CHANNELS;
}

bool Widget_Curves::ChannelPoint::is_draggable() const
{
	const ValueDesc &value_desc(curve_it->value_desc);
	return value_desc.is_animated() || value_desc.parent_is_linkable_value_node();
}

bool Widget_Curves::ChannelPoint::operator ==(const Widget_Curves::ChannelPoint& b) const
{
	return curve_it == b.curve_it && time_point == b.time_point && channel_idx == b.channel_idx;
}

Real Widget_Curves::ChannelPoint::get_value(Real time_tolerance) const
{
	return curve_it->get_value(channel_idx, time_point.get_time(), time_tolerance);
}

Widget_Curves::ChannelPointSD::ChannelPointSD(Widget_Curves& widget)
	: SelectDragHelper<ChannelPoint>(_("Change animation curve")),
	  widget(widget)
{
}

void Widget_Curves::ChannelPointSD::get_item_position(const Widget_Curves::ChannelPoint& item, Gdk::Point& position)
{
	const Time &time = item.time_point.get_time();
	Real value = item.get_value(widget.time_plot_data->dt);
	position.set_x(widget.time_plot_data->get_pixel_t_coord(time));
	position.set_y(widget.time_plot_data->get_pixel_y_coord(value));
}

bool Widget_Curves::ChannelPointSD::find_item_at_position(int pos_x, int pos_y, Widget_Curves::ChannelPoint& cp)
{
	cp.invalidate();
	for(auto curve_it = widget.curve_list.begin(); curve_it != widget.curve_list.end(); ++curve_it) {
		size_t channels = curve_it->channels.size();

		WaypointRenderer::foreach_visible_waypoint(curve_it->value_desc, *widget.time_plot_data,
			[&](const synfig::TimePoint &tp, const synfig::Time &t, void *data) -> bool
		{
			int px = widget.time_plot_data->get_pixel_t_coord(t);
			for (size_t c = 0; c < channels; ++c) {
				Real y = curve_it->get_value(c, t, widget.time_plot_data->dt);
				int py = widget.time_plot_data->get_pixel_y_coord(y);

				if (pos_x > px - widget.waypoint_edge_length/2 && pos_x <= px + widget.waypoint_edge_length/2) {
					if (pos_y > py - widget.waypoint_edge_length/2 && pos_y <= py + widget.waypoint_edge_length/2) {
						cp.curve_it = curve_it;
						cp.time_point = tp;
						cp.channel_idx = c;
						return true;
					}
				}
			}
			return false;
		});

		if (cp.is_valid())
			return true;
	}
	return false;
}

bool Widget_Curves::ChannelPointSD::find_items_in_rect(Gdk::Rectangle rect, std::vector<ChannelPoint>& list)
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

	for(auto curve_it = widget.curve_list.begin(); curve_it != widget.curve_list.end(); ++curve_it) {
		size_t channels = curve_it->channels.size();

		WaypointRenderer::foreach_visible_waypoint(curve_it->value_desc, *widget.time_plot_data,
			[&](const synfig::TimePoint &tp, const synfig::Time &t, void *data) -> bool
		{
			int px = widget.time_plot_data->get_pixel_t_coord(t);
			for (size_t c = 0; c < channels; ++c) {
				Real y = curve_it->get_value(c, t, widget.time_plot_data->dt);
				int py = widget.time_plot_data->get_pixel_y_coord(y);

				if (x0 < px + widget.waypoint_edge_length/2 && x1 >= px - widget.waypoint_edge_length/2) {
					if (y0 < py + widget.waypoint_edge_length/2 && y1 >= py - widget.waypoint_edge_length/2) {
						list.push_back(ChannelPoint(curve_it, tp, c));
					}
				}
			}
			return false;
		});
	}
	return list.size() > 0;
}

void Widget_Curves::ChannelPointSD::get_all_items(std::vector<Widget_Curves::ChannelPoint>& items)
{
	for (std::list<CurveStruct>::iterator curve_it = widget.curve_list.begin(); curve_it != widget.curve_list.end(); ++curve_it) {
		const auto &time_set = WaypointRenderer::get_times_from_valuedesc(curve_it->value_desc);
		for (size_t channel_idx = 0; channel_idx < curve_it->channels.size(); channel_idx++) {
			for (const TimePoint &time : time_set) {
				items.push_back(ChannelPoint(curve_it, time, channel_idx));
			}
		}
	}
}

void Widget_Curves::ChannelPointSD::delta_drag(int total_dx, int total_dy, bool by_keys)
{
	int dx = 0, dy = 0;
	if (total_dx == 0 && total_dy == 0)
		return;

	if (by_keys) {
		// snap to frames
		dx = total_dx * widget.time_plot_data->k/widget.canvas_interface->get_canvas()->rend_desc().get_frame_rate();
		dy = total_dy;
	} else {
		Gdk::Point current_pos;
		get_item_position(*get_active_item(), current_pos);
		int x0, y0;
		get_active_item_initial_point(x0, y0);

		int y1 = y0 + total_dy;
		dy = y1 - current_pos.get_y();

		int x1 = x0 + total_dx;
		// snap to frames
		float fps = widget.canvas_interface->get_canvas()->rend_desc().get_frame_rate();
		Time next_t = widget.time_plot_data->get_t_from_pixel_coord(x1).round(fps);
		x1 = widget.time_plot_data->get_pixel_t_coord(next_t);
		dx = x1 - current_pos.get_x();
	}

	std::vector<ChannelPoint*> selection = get_selected_items();
	// Move Y value
	if (dy != 0) {
		// If the active point (ie. the point clicked and dragged by cursor) is a converted parameter,
		// it can't be moved vertically.
		// So, any selected channel point shouldn't be moved vertically: it is strange in UX point of view
		if (get_active_item()->is_draggable()) {
			std::map< long, std::map< Time, std::vector<ChannelPoint*> > > selection_tree;
			for (auto point : selection) {
				// If it is a converted parameter (and not its inner parameter), its Y value can't be changed
				if (point->is_draggable())
					selection_tree[std::distance(widget.curve_list.begin(), point->curve_it)][point->time_point.get_time()].push_back(point);
			}

			for (const auto &curve : selection_tree) {
				for (auto tt : curve.second) {
					auto time = tt.first;
					auto channels = tt.second;
					auto refpoint = channels.front();
					ValueBase value_base = refpoint->curve_it->value_desc.get_value(time);

					for (auto point : channels) {
						// Clear cached values due to precision error while dragging multiple points
						point->curve_it->channels[point->channel_idx].values.clear();

						Real v = point->get_value(widget.time_plot_data->dt);
						int pix_y = widget.time_plot_data->get_pixel_y_coord(v);
						pix_y += dy;
						v = widget.time_plot_data->get_y_from_pixel_coord(pix_y);
						CurveStruct::set_value_base_channel_value(value_base, point->channel_idx, v);
					}

					const ValueDesc &value_desc = refpoint->curve_it->value_desc;
					ValueNode::Handle value_node = value_desc.get_value_node();
					std::set<synfig::Waypoint, std::less<UniqueID> > waypoint_set;
					synfig::waypoint_collect(waypoint_set, time, value_node);
					if (waypoint_set.size() < 1)
						break;

					Waypoint waypoint(*(waypoint_set.begin()));
					waypoint.set_value(value_base);
					widget.canvas_interface->waypoint_set_value_node(value_node, waypoint);
				}
			}
		}
	}

	// Move along time
	if (dx == 0)
		return;
	std::vector<std::pair<const ValueDesc&, Time>> times_to_move;

	const float fps = widget.canvas_interface->get_canvas()->rend_desc().get_frame_rate();
	const Time base_time = get_active_item()->time_point.get_time();
	const Time next_time = widget.time_plot_data->get_t_from_pixel_coord(widget.time_plot_data->get_pixel_t_coord(base_time) + dx).round(fps);
	const Time deltatime = next_time - base_time;

	if (deltatime != 0) {
		// new dragging position allow us to restore previouly overlapped waypoints
		auto waypoints_to_restore = widget.overlapped_waypoints;
		// let it store new overlapped waypoints until next drag motion
		widget.overlapped_waypoints.clear();

		bool ignore_move = false;
		std::vector<std::pair<ChannelPoint*, Time> > timepoints_to_update;

		// sort in order to avoid overlapping issues
		std::sort(selection.begin(), selection.end(), [=](const ChannelPoint * const a, const ChannelPoint * const b)->bool {
			if (dx < 0)
				return a->time_point < b->time_point;
			else
				return b->time_point < a->time_point;
		});

		for (ChannelPoint * point : selection) {
			const ValueDesc &value_desc = point->curve_it->value_desc;
			const Time &time = point->time_point.get_time();
			const Time new_time = Time(time+deltatime).round(fps);

			std::pair<const ValueDesc&, Time> meta_data(value_desc, time);
			if (std::find(times_to_move.begin(), times_to_move.end(), meta_data) == times_to_move.end()) {
				auto time_set = WaypointRenderer::get_times_from_valuedesc(value_desc);

				// are we overlapping existing waypoints while moving in time?
				auto new_timepoint = time_set.find(new_time);
				if (new_timepoint != time_set.end()) {
					// Converted layer parameter can't overlap waypoints... So, nobody moves this time
					if (!point->is_draggable()) {
						ignore_move = true;
						times_to_move.clear();
						break;
					}
					// Check if the point is selected to move. If so, it won't overlap
					ChannelPoint test_point(*point);
					test_point.time_point = *new_timepoint;
					if (!is_waypoint_selected(test_point)) {
						std::set<synfig::Waypoint, std::less<UniqueID> > waypoint_set;
						synfig::waypoint_collect(waypoint_set, new_time, value_desc.get_value_node());
						for (const Waypoint &waypoint : waypoint_set) {
							widget.overlapped_waypoints.push_back(std::pair<Waypoint, std::list<CurveStruct>::iterator>(waypoint, point->curve_it));
							widget.canvas_interface->waypoint_remove(value_desc, waypoint);
						}
					}
				}

				times_to_move.push_back(meta_data);
			}

			timepoints_to_update.push_back(std::pair<ChannelPoint*, Time>(point, new_time));
		}

		if (!ignore_move) {
			// first we move waypoints
			for (const auto &info : times_to_move)
				widget.canvas_interface->waypoint_move(info.first, info.second, deltatime);
			// now we update cached values in select-drag handler
			for (auto pair : timepoints_to_update)
				pair.first->time_point = TimePoint(pair.second);
		}

		// Now we can restore previously overlapped waypoints
		for (auto it : waypoints_to_restore) {
			Action::Handle 	action(Action::create("WaypointAdd"));

			assert(action);
			if(!action)
				return;

			action->set_param("canvas", widget.canvas_interface->get_canvas());
			action->set_param("widget.canvas_interface", widget.canvas_interface);
			action->set_param("value_node", it.second->value_desc.get_value_node());
//			action->set_param("time", i.first.get_time());
			action->set_param("waypoint", it.first);

			if(!widget.canvas_interface->get_instance()->perform_action(action))
				widget.canvas_interface->get_ui_interface()->error(_("Action Failed."));
		}
	}

	widget.queue_draw();
}

bool Widget_Curves::ChannelPointSD::is_waypoint_selected(const Widget_Curves::ChannelPoint& point) const
{
	ChannelPoint test_point(point);
	const size_t n_channels = point.curve_it->channels.size();
	for (size_t cidx = 0; cidx < n_channels; cidx++) {
		test_point.channel_idx = cidx;
		if (is_selected(test_point))
			return true;
	}
	return false;
}

