/* === S Y N F I G ========================================================= */
/*!	\file widgets/widget_timetrack.cpp
**	\brief Widget to displaying layer parameter waypoints along time
**
**	$Id$
**
**	\legal
**	Copyright (c) 2002-2005 Robert B. Quattlebaum Jr., Adrian Bentley
**	......... ... 2020 Rodolfo Ribeiro Gomes
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

#include "widget_timetrack.h"

#include <gui/canvasview.h>
#include <gui/timeplotdata.h>
#include <gui/waypointrenderer.h>

#include <gui/localization.h>

#include <cairomm/cairomm.h>
#include <gdkmm.h>

#include <synfig/general.h>
#endif

using namespace studio;

Widget_Timetrack::Widget_Timetrack()
	: params_treeview(nullptr),
	  is_rebuild_param_info_list_queued(false),
	  is_update_param_list_geometries_queued(false)
{
	add_events(Gdk::BUTTON_PRESS_MASK | Gdk::BUTTON_RELEASE_MASK | Gdk::SCROLL_MASK | Gdk::POINTER_MOTION_MASK | Gdk::KEY_PRESS_MASK | Gdk::KEY_RELEASE_MASK);
	set_can_focus(true);
	time_plot_data->set_extra_time_margin(16/2);
	setup_mouse_handler();

	setup_adjustment();
}

Widget_Timetrack::~Widget_Timetrack()
{
	for (auto item : param_info_map) {
		delete item.second;
		item.second = nullptr;
	}
	teardown_params_view();
	teardown_params_store();
}

bool Widget_Timetrack::set_params_view(Gtk::TreeView* treeview)
{
	teardown_params_view();
	teardown_params_store();

	params_treeview = nullptr;
	params_store.reset();

	if (!treeview)
		return true;

	Glib::RefPtr<LayerParamTreeStore> treestore = Glib::RefPtr<LayerParamTreeStore>::cast_dynamic( treeview->get_model() );
	if (!treestore) {
		synfig::error("TreeView must use model based on LayerParamTreeStore");
		return false;
	}
	params_store = treestore;
	params_treeview = treeview;
	setup_params_store();
	setup_params_view();

	rebuild_param_info_list();

	return true;
}

Gtk::TreeView* Widget_Timetrack::get_params_view() const
{
	return params_treeview;
}

Glib::RefPtr<LayerParamTreeStore> Widget_Timetrack::get_params_model() const
{
	return params_store;
}

bool Widget_Timetrack::use_canvas_view(etl::loose_handle<CanvasView> canvas_view)
{
	if (!canvas_view) {
		synfig::error("No canvas_view for timetrack");
		return false;
	}
	Gtk::TreeView* params_treeview = dynamic_cast<Gtk::TreeView*>(canvas_view->get_ext_widget("params"));
	if (!params_treeview) {
		synfig::error("Params treeview widget doesn't exist");
		return false;
	}

	set_time_model(canvas_view->time_model());
	set_canvas_interface(canvas_view->canvas_interface());
	return set_params_view(params_treeview);
}

bool Widget_Timetrack::on_event(GdkEvent* event)
{
	if (waypoint_sd.process_event(event))
		return true;
	return Widget_TimeGraphBase::on_event(event);
}

bool Widget_Timetrack::on_draw(const Cairo::RefPtr<Cairo::Context>& cr)
{
	std::lock_guard<std::mutex> lock(param_list_mutex);

	if (Widget_TimeGraphBase::on_draw(cr))
		return true;

	// Draw waypoints

	// Maybe it's possible to be more efficient by redrawing only for the visible paths,
	// instead of iterate over entire param list
	//	Gtk::TreePath start_path, end_path;
	//	params_treeview->get_visible_range(start_path, end_path);

	params_store->foreach_path([=](const Gtk::TreeModel::Path &path) -> bool {
		RowInfo * row_info = param_info_map[path.to_string()];
		if (!row_info) {
			queue_rebuild_param_info_list();
			return true;
		}

		if (row_info->get_geometry().h == 0)
			return false;

		bool is_draggable = row_info->get_value_desc().is_animated() || row_info->get_value_desc().parent_is_linkable_value_node();
		if (!is_draggable) {
			cr->push_group();
		}

		std::vector<std::pair<synfig::TimePoint, synfig::Time>> visible_waypoints;
		WaypointRenderer::foreach_visible_waypoint(row_info->get_value_desc(), *time_plot_data,
			[&](const synfig::TimePoint &tp, const synfig::Time &t, void *) -> bool
		{
			visible_waypoints.push_back(std::pair<synfig::TimePoint, synfig::Time>(tp, t));
			return false;
		});

		// Draw static intervals
		draw_static_intervals_for_row(cr, row_info, visible_waypoints);

		draw_waypoints(cr, row_info, visible_waypoints);

		if (!is_draggable) {
			cr->pop_group_to_source();
			cr->paint_with_alpha(0.5);
		}

		return false;
	});


	// Draw selection rectangle
	if (waypoint_sd.get_state() == WaypointSD::State::POINTER_SELECTING) {
		// set up a dashed solid-color stroke
		static const std::vector<double>dashed3 = {5.0};
		cr->set_dash(dashed3, 0);

		int x0, y0;
		int x1, y1;
		waypoint_sd.get_initial_tracking_point(x0, y0);
		get_pointer(x1, y1);

		cr->rectangle(x0, y0, x1 - x0, y1 - y0);
		Gdk::RGBA color = get_style_context()->get_color();
		cr->set_source_rgb(color.get_red(), color.get_green(), color.get_blue());
		cr->stroke();
	}


	if (canvas_interface) {
		synfig::Canvas::Handle canvas = canvas_interface->get_canvas();

		if (canvas)
			for(synfig::KeyframeList::const_iterator i = canvas->keyframe_list().begin(); i != canvas->keyframe_list().end(); ++i)
				draw_keyframe_line(cr, *i);
	}
	draw_current_time(cr);
	return true;
}

void Widget_Timetrack::on_size_allocate(Gtk::Allocation& allocation)
{
	double upper = range_adjustment->get_upper();
	Widget_TimeGraphBase::on_size_allocate(allocation);
	set_default_page_size(allocation.get_height());
	ConfigureAdjustment(range_adjustment)
			.set_page_size(allocation.get_height())
			.set_step_increment(allocation.get_height()/10)
			.set_lower(0)
			.set_upper(upper)
			.finish();
}

void Widget_Timetrack::setup_mouse_handler()
{
	waypoint_sd.set_pan_enabled(true);
	waypoint_sd.set_scroll_enabled(true);
	waypoint_sd.set_zoom_enabled(false);
	waypoint_sd.set_canvas_interface(canvas_interface);
	waypoint_sd.signal_redraw_needed().connect(sigc::mem_fun(*this, &Gtk::Widget::queue_draw));
	waypoint_sd.signal_focus_requested().connect(sigc::mem_fun(*this, &Gtk::Widget::grab_focus));
	waypoint_sd.signal_scroll_up_requested().connect(sigc::mem_fun(*this, &Widget_Timetrack::scroll_up));
	waypoint_sd.signal_scroll_down_requested().connect(sigc::mem_fun(*this, &Widget_Timetrack::scroll_down));
	waypoint_sd.signal_panning_requested().connect(sigc::mem_fun(*this, &Widget_Timetrack::pan));
}

void Widget_Timetrack::setup_params_store()
{
	sigc::connection conn;

	conn = params_store->signal_row_inserted().connect([&](const Gtk::TreeModel::Path&, const Gtk::TreeModel::iterator&){
		queue_rebuild_param_info_list();
	});
	treestore_connections.push_back(conn);

	conn = params_store->signal_row_deleted().connect([&](const Gtk::TreeModel::Path&){
		queue_rebuild_param_info_list();
	});
	treestore_connections.push_back(conn);

	conn = params_store->signal_rows_reordered().connect([&](const Gtk::TreeModel::Path&, const Gtk::TreeModel::iterator&, int*){
		queue_rebuild_param_info_list();
	});
	treestore_connections.push_back(conn);
}

void Widget_Timetrack::teardown_params_store()
{
	for (sigc::connection &conn : treestore_connections)
		conn.disconnect();
	treestore_connections.clear();
}

void Widget_Timetrack::setup_params_view()
{
	sigc::connection conn;
	conn = params_treeview->signal_row_expanded().connect([&](const Gtk::TreeModel::iterator&,const Gtk::TreeModel::Path&) {
		queue_update_param_list_geometries();
	});
	treeview_connections.push_back(conn);

	conn = params_treeview->signal_row_collapsed().connect([&](const Gtk::TreeModel::iterator&,const Gtk::TreeModel::Path&) {
		queue_update_param_list_geometries();
	});
	treeview_connections.push_back(conn);

	conn = params_treeview->signal_style_updated().connect(sigc::mem_fun(*this, &Widget_Timetrack::queue_update_param_list_geometries));
	treeview_connections.push_back(conn);
}

void Widget_Timetrack::teardown_params_view()
{
	for (sigc::connection &conn : treeview_connections)
		conn.disconnect();
	treeview_connections.clear();
}

void Widget_Timetrack::setup_adjustment()
{
	range_adjustment->signal_value_changed().connect([&](){
		// wait for all members of Adjustment group to synchronize
		queue_update_param_list_geometries();
	});
	range_adjustment->signal_changed().connect([&](){
		set_default_page_size(range_adjustment->get_page_size());
		queue_update_param_list_geometries();
	});
}

void Widget_Timetrack::queue_rebuild_param_info_list()
{
	if (is_rebuild_param_info_list_queued)
		return;
	is_rebuild_param_info_list_queued = true;
	Glib::signal_idle().connect_once(sigc::mem_fun(*this, &Widget_Timetrack::rebuild_param_info_list));
}

void Widget_Timetrack::rebuild_param_info_list()
{
	std::lock_guard<std::mutex> lock(param_list_mutex);

	is_rebuild_param_info_list_queued = false;

	param_info_map.clear();

	if (!params_treeview || !params_treeview->get_realized() || !params_store) {
		queue_draw();
		return;
	}

	Gtk::TreeViewColumn *col = params_treeview->get_column(0);
	params_store.get()->foreach([&](const Gtk::TreeModel::Path &path, const Gtk::TreeModel::iterator &iter) -> bool {
		Gdk::Rectangle rect;
		params_treeview->get_cell_area(path, *col, rect);
		Geometry geometry;
		geometry.y = rect.get_y();
		geometry.h = rect.get_height();
		synfigapp::ValueDesc value_desc = iter->get_value(params_store->model.value_desc);
		RowInfo *row_info = new RowInfo(value_desc, geometry);
		param_info_map[path.to_string()] = row_info;
		// It could be optimized to make only this row dirty - and only redraw its rectangle
		row_info->signal_changed().connect(sigc::mem_fun(*this, &Widget_Timetrack::queue_draw));
		return false;
	});

	queue_draw();
}

void Widget_Timetrack::queue_update_param_list_geometries()
{
	if (is_update_param_list_geometries_queued)
		return;
	is_update_param_list_geometries_queued = true;
	Glib::signal_idle().connect_once(sigc::mem_fun(*this, &Widget_Timetrack::update_param_list_geometries));
}

void Widget_Timetrack::update_param_list_geometries()
{
	std::lock_guard<std::mutex> lock(param_list_mutex);
	is_update_param_list_geometries_queued = false;

	if (param_info_map.size() == 0 ||
		!params_treeview ||
		!params_treeview->get_realized() ||
		!params_store)
	{
		queue_draw();
		return;
	}

	Gtk::TreeViewColumn *col = params_treeview->get_column(0);
	params_store.get()->foreach([&](const Gtk::TreeModel::Path &path, const Gtk::TreeModel::iterator &/*iter*/) -> bool {
		Gdk::Rectangle rect;
		params_treeview->get_cell_area(path, *col, rect);
		Geometry geometry;
		geometry.y = rect.get_y();
		geometry.h = rect.get_height();
		param_info_map[path.to_string()]->set_geometry(geometry);

		return false;
	});

	queue_draw();
}

void Widget_Timetrack::draw_static_intervals_for_row(const Cairo::RefPtr<Cairo::Context>& cr, const Widget_Timetrack::RowInfo* row_info, const std::vector<std::pair<synfig::TimePoint, synfig::Time> >& waypoints)
{
	const int waypoint_edge_length = row_info->get_geometry().h;
	const int py = row_info->get_geometry().y;
	const double static_line_thickness = 4;

	synfig::ValueBase previous_value;
	synfig::Time previous_time;

	Gdk::RGBA color = get_style_context()->get_color();
	cr->set_source_rgba(color.get_red(), color.get_green(), color.get_blue(), 0.7);

	for (const auto& pair : waypoints) {
		const synfig::Time &t = pair.second;
		int px = time_plot_data->get_pixel_t_coord(t);

		synfig::ValueBase value(row_info->get_value_desc().get_value(t));
		if (value == previous_value) {
			int previous_px = time_plot_data->get_pixel_t_coord(previous_time);
			cr->rectangle(previous_px, py + waypoint_edge_length/2 - static_line_thickness/2, px - previous_px, static_line_thickness);
			cr->fill();
		}
		previous_value = value;
		previous_time = t;

	}
}

void Widget_Timetrack::draw_waypoints(const Cairo::RefPtr<Cairo::Context>& cr, const Widget_Timetrack::RowInfo* row_info, const std::vector<std::pair<synfig::TimePoint, synfig::Time> >& waypoints)
{
	const int margin = 1;
	const int waypoint_edge_length = row_info->get_geometry().h;
	const int py = row_info->get_geometry().y;

	for (const auto& pair : waypoints) {
		const synfig::Time &t = pair.second;
		int px = time_plot_data->get_pixel_t_coord(t);
		Gdk::Rectangle area(
					0 - waypoint_edge_length/2 + margin + px,
					0 + margin + py,
					waypoint_edge_length - 2*margin,
					waypoint_edge_length - 2*margin);

		const auto & hovered_point = waypoint_sd.get_hovered_item();
		bool hover = false;
	//	bool hover = hovered_point.is_valid() && tp == hovered_point.time_point && hovered_point.curve_it == curve_it;
		bool selected = false;
	//	bool selected = waypoint_sd.is_selected(ChannelPoint(curve_it, tp, c));
		WaypointRenderer::render_time_point_to_window(cr, area, pair.first, selected, hover);
	}
}

Widget_Timetrack::RowInfo::RowInfo()
{
}

Widget_Timetrack::RowInfo::RowInfo(synfigapp::ValueDesc value_desc_, Widget_Timetrack::Geometry geometry)
	: value_desc(value_desc_), geometry(geometry)
{
	if (value_desc) {
		if (value_desc.is_value_node())
			value_desc_connections.push_back(
						value_desc.get_value_node()->signal_changed().connect(
							sigc::mem_fun(*this, &RowInfo::refresh )));
		if (value_desc.parent_is_value_node())
			value_desc_connections.push_back(
						value_desc.get_parent_value_node()->signal_changed().connect(
							sigc::mem_fun(*this, &RowInfo::refresh )));
		refresh();
	}
}

Widget_Timetrack::RowInfo::~RowInfo()
{
	for (sigc::connection &conn : value_desc_connections)
		conn.disconnect();
	value_desc_connections.clear();
}

synfigapp::ValueDesc Widget_Timetrack::RowInfo::get_value_desc() const
{
	return value_desc;
}

Widget_Timetrack::Geometry Widget_Timetrack::RowInfo::get_geometry() const
{
	return geometry;
}

void Widget_Timetrack::RowInfo::set_geometry(const Widget_Timetrack::Geometry& value)
{
	geometry = value;
}

void Widget_Timetrack::RowInfo::refresh()
{
	if (!value_desc) {
		synfig::error("ValueDesc invalid! Internal error");
		return;
	}

	signal_changed().emit();
}
