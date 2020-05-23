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
#include <synfig/timepointcollect.h>
#endif

using namespace studio;

Widget_Timetrack::Widget_Timetrack()
	: waypoint_sd(*this),
	  params_treeview(nullptr),
	  param_list_height(0),
	  is_rebuild_param_info_list_queued(false),
	  is_update_param_list_geometries_queued(false),
	  action_state(NONE)
{
	add_events(Gdk::BUTTON_PRESS_MASK | Gdk::BUTTON_RELEASE_MASK | Gdk::SCROLL_MASK | Gdk::POINTER_MOTION_MASK | Gdk::KEY_PRESS_MASK | Gdk::KEY_RELEASE_MASK);
	set_can_focus(true);
	time_plot_data->set_extra_time_margin(16/2);
	setup_mouse_handler();

	setup_adjustment();

	waypoint_sd.signal_action_changed().connect([=](){
		update_cursor();
		action_state = waypoint_sd.get_action();
		signal_action_state_changed().emit();
	});
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
		synfig::error(_("TreeView must use model based on LayerParamTreeStore"));
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
		synfig::error(_("No canvas_view for timetrack"));
		return false;
	}
	Gtk::TreeView* params_treeview = dynamic_cast<Gtk::TreeView*>(canvas_view->get_ext_widget("params"));
	if (!params_treeview) {
		synfig::error(_("Params treeview widget doesn't exist"));
		return false;
	}

	set_time_model(canvas_view->time_model());
	set_canvas_interface(canvas_view->canvas_interface());
	return set_params_view(params_treeview);
}

void Widget_Timetrack::delete_selected()
{
	std::lock_guard<std::mutex> lock(param_list_mutex);

	std::vector<WaypointItem*> selection = waypoint_sd.get_selected_items();
	if (selection.size() == 0)
		return;

	// From CellRenderer_TimeTrack
	synfigapp::Action::Handle action(synfigapp::Action::create("TimepointsDelete"));
	if(!action)
		return;

	synfigapp::Action::ParamList param_list;
	for (WaypointItem *wi : selection) {
		param_list.add("canvas", canvas_interface->get_canvas());
		param_list.add("canvas_interface", canvas_interface);

		const synfigapp::ValueDesc &value_desc = param_info_map[wi->path.to_string()]->get_value_desc();
		if (value_desc.get_value_type() == synfig::type_canvas && !getenv("SYNFIG_SHOW_CANVAS_PARAM_WAYPOINTS")) {
			param_list.add("addcanvas", value_desc.get_value().get(synfig::Canvas::Handle()));
		} else {
			param_list.add("addvaluedesc", value_desc);
		}

		param_list.add("addtime", wi->time_point.get_time());
	}
	action->set_param_list(param_list);
	canvas_interface->get_instance()->perform_action(action);
}

bool Widget_Timetrack::move_selected(synfig::Time delta_time)
{
	std::lock_guard<std::mutex> lock(param_list_mutex);

	std::vector<WaypointItem*> selection = waypoint_sd.get_selected_items();
	if (selection.size() == 0)
		return true;

	// From CellRenderer_TimeTrack
	synfigapp::Action::Handle action(synfigapp::Action::create("TimepointsMove"));
	if (!action)
		return false;

	synfigapp::Action::ParamList param_list;
	for (WaypointItem *wi : selection) {
		param_list.add("canvas", canvas_interface->get_canvas());
		param_list.add("canvas_interface", canvas_interface);

		const synfigapp::ValueDesc &value_desc = param_info_map[wi->path.to_string()]->get_value_desc();
		if (value_desc.get_value_type() == synfig::type_canvas && !getenv("SYNFIG_SHOW_CANVAS_PARAM_WAYPOINTS")) {
			param_list.add("addcanvas", value_desc.get_value().get(synfig::Canvas::Handle()));
		} else {
			param_list.add("addvaluedesc", value_desc);
		}

		param_list.add("addtime", wi->time_point.get_time());
		param_list.add("deltatime", delta_time);
	}
	action->set_param_list(param_list);
	bool ok = canvas_interface->get_instance()->perform_action(action);
	if (ok)
		displace_selected_waypoint_items(delta_time);
	return ok;
}

bool Widget_Timetrack::copy_selected(synfig::Time delta_time)
{
	std::lock_guard<std::mutex> lock(param_list_mutex);

	std::vector<WaypointItem*> selection = waypoint_sd.get_selected_items();
	if (selection.size() == 0)
		return true;

	// From CellRenderer_TimeTrack
	synfigapp::Action::Handle action(synfigapp::Action::create("TimepointsCopy"));
	if (!action)
		return false;

	synfigapp::Action::ParamList param_list;
	for (WaypointItem *wi : selection) {
		param_list.add("canvas", canvas_interface->get_canvas());
		param_list.add("canvas_interface", canvas_interface);

		const synfigapp::ValueDesc &value_desc = param_info_map[wi->path.to_string()]->get_value_desc();
		if (value_desc.get_value_type() == synfig::type_canvas && !getenv("SYNFIG_SHOW_CANVAS_PARAM_WAYPOINTS")) {
			param_list.add("addcanvas", value_desc.get_value().get(synfig::Canvas::Handle()));
		} else {
			param_list.add("addvaluedesc", value_desc);
		}

		param_list.add("addtime", wi->time_point.get_time());
		param_list.add("deltatime", delta_time);
	}
	action->set_param_list(param_list);
	bool ok = canvas_interface->get_instance()->perform_action(action);
	if (ok)
		displace_selected_waypoint_items(delta_time);
	return ok;
}

void Widget_Timetrack::scale_selected()
{
	std::lock_guard<std::mutex> lock(param_list_mutex);

	synfigapp::Action::PassiveGrouper group(canvas_interface->get_instance().get(), _("Scale waypoints"));
	WaypointScaleInfo scale_info = compute_scale_params();

	for (WaypointItem *wi : waypoint_sd.get_selected_items()) {
		synfigapp::Action::Handle action(synfigapp::Action::create("TimepointsMove"));
		if (!action)
			return;
		synfigapp::Action::ParamList param_list;
		param_list.add("canvas", canvas_interface->get_canvas());
		param_list.add("canvas_interface", canvas_interface);

		const synfigapp::ValueDesc &value_desc = param_info_map[wi->path.to_string()]->get_value_desc();
		if (value_desc.get_value_type() == synfig::type_canvas && !getenv("SYNFIG_SHOW_CANVAS_PARAM_WAYPOINTS")) {
			param_list.add("addcanvas", value_desc.get_value().get(synfig::Canvas::Handle()));
		} else {
			param_list.add("addvaluedesc", value_desc);
		}

		const synfig::Time t0 = wi->time_point.get_time();
		const synfig::Time t = compute_scaled_time(*wi, scale_info);
		const synfig::Time delta_time = t - t0;

		param_list.add("addtime", t0);
		param_list.add("deltatime", delta_time);
		action->set_param_list(param_list);
		bool ok = canvas_interface->get_instance()->perform_action(action);

		if (ok)
			wi->time_point.set_time(t);
	}
}

void Widget_Timetrack::goto_next_waypoint(long n)
{
	std::vector<WaypointItem *> selection = waypoint_sd.get_selected_items();
	if (selection.size() != 1 || n == 0)
		return;
	const WaypointItem wi = *selection.front();

	const synfig::Node::time_set& time_set = WaypointRenderer::get_times_from_valuedesc(param_info_map[wi.path.to_string()]->get_value_desc());
	if (time_set.size() == 1)
		return;

	std::vector<synfig::TimePoint> time_vector;
	time_vector.insert(time_vector.end(), time_set.begin(), time_set.end());
	std::sort(time_vector.begin(), time_vector.end());

	auto item = std::find(time_vector.begin(), time_vector.end(), wi.time_point);

	if (n > 0) {
		long max = std::distance(item, time_vector.end()-1);
		if (n > max)
			n = max;
		if (n <= 0)
			return;
	} else {
		long min = std::distance(item, time_vector.begin());
		if (n < min)
			n = min;
		if (n == 0)
			return;
	}

	item += n;
	if (item == time_vector.end())
		return;

	waypoint_sd.deselect(wi);
	waypoint_sd.select(WaypointItem(*item, wi.path));
}

void Widget_Timetrack::goto_previous_waypoint(long n)
{
	goto_next_waypoint(-n);
}

std::string Widget_Timetrack::get_action_state_name(Widget_Timetrack::ActionState action_state)
{
	switch (action_state) {
	case NONE:
		return "none";
	case MOVE:
		return "move";
	case COPY:
		return "copy";
	case SCALE:
		return "scale";
	}
	return "";
}

Widget_Timetrack::ActionState Widget_Timetrack::get_action_state() const
{
	return action_state;
}

void Widget_Timetrack::set_action_state(Widget_Timetrack::ActionState action_state)
{
	waypoint_sd.set_action(action_state);
}

bool Widget_Timetrack::on_event(GdkEvent* event)
{
	if (waypoint_sd.process_event(event))
		return true;

	switch (event->type) {
	case GDK_KEY_PRESS:
		if (waypoint_sd.get_state() == waypoint_sd.POINTER_DRAGGING)
			return true;
		switch (event->key.keyval) {
		case GDK_KEY_Delete:
			delete_selected();
			return true;
		case GDK_KEY_n:
			goto_next_waypoint(1);
			return true;
		case GDK_KEY_N:
			goto_next_waypoint(5);
			return true;
		case GDK_KEY_b:
			goto_previous_waypoint(1);
			return true;
		case GDK_KEY_B:
			goto_previous_waypoint(5);
			return true;
		default:
			break;
		}
	default:
		break;
	}

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

		const Geometry& geometry = row_info->get_geometry();
		if (geometry.h == 0)
			return false;

		if (geometry.y + geometry.h < 0 || geometry.y > get_height())
			return false;

		// is param selected?
		draw_selected_background(cr, path, row_info);

		const synfigapp::ValueDesc &value_desc = row_info->get_value_desc();

		const bool is_dragging = waypoint_sd.get_state() == waypoint_sd.POINTER_DRAGGING;
		const bool is_user_moving_waypoints = is_dragging && waypoint_sd.get_action() == MOVE;
		const bool is_user_scaling_waypoints = is_dragging && waypoint_sd.get_action() == SCALE;

		std::vector<std::pair<synfig::TimePoint, synfig::Time>> visible_waypoints;
		WaypointRenderer::foreach_visible_waypoint(row_info->get_value_desc(), *time_plot_data,
			[&](const synfig::TimePoint &tp, const synfig::Time &t, void *) -> bool
		{
			// Don't draw it if it's being moved by user
			if ((is_user_moving_waypoints || is_user_scaling_waypoints) && waypoint_sd.is_selected(WaypointItem(tp, path)))
				return false;
			visible_waypoints.push_back(std::pair<synfig::TimePoint, synfig::Time>(tp, t));
			return false;
		});

		// Draw static intervals
		if (value_desc.get_value_type() != synfig::type_canvas)
			draw_static_intervals_for_row(cr, row_info, visible_waypoints);

		draw_waypoints(cr, path, row_info, visible_waypoints);

		return false;
	});


	// draw dragging waypoints
	if (waypoint_sd.get_state() == WaypointSD::State::POINTER_DRAGGING) {
		WaypointScaleInfo scale_info = compute_scale_params();
		bool is_user_scaling = waypoint_sd.get_action() == SCALE;

		for (const WaypointItem *item : waypoint_sd.get_selected_items()) {
			const int margin = 1;
			RowInfo * row_info = param_info_map[item->path.to_string()];
			if (!row_info)
				continue;

			const Geometry &geometry = row_info->get_geometry();
			const int waypoint_edge_length = geometry.h;
			const int py = geometry.y;

			const synfig::TimePoint &tp = item->time_point;
			synfig::Time t;
			if (is_user_scaling) {
				t = compute_scaled_time(*item, scale_info);
			} else {
				t = item->time_point.get_time() + waypoint_sd.get_deltatime();
			}

			int px = time_plot_data->get_pixel_t_coord(t);
			Gdk::Rectangle area(
						0 - waypoint_edge_length/2 + margin + px,
						0 + margin + py,
						waypoint_edge_length - 2*margin,
						waypoint_edge_length - 2*margin);

			bool hover = false;
			bool selected = true;
			const synfigapp::ValueDesc &value_desc = row_info->get_value_desc();
			bool is_static_value_node = (value_desc.is_value_node() && !synfig::ValueNode_Animated::Handle::cast_dynamic(value_desc.get_value_node()));

			WaypointRenderer::render_time_point_to_window(cr, area, tp, selected, hover, is_static_value_node);
		}
	}

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

	synfig::Canvas::Handle canvas;
	if (params_treeview && param_info_map.size() > 0) {
		const RowInfo * row_info = param_info_map.begin()->second;
//		If Parameters Panel shows data of more than one layer, maybe we should do something like:
//		Gtk::TreeIter selected_iter = params_treeview->get_selection()->get_selected();
//		Gtk::TreePath path = params_treeview->get_model()->get_path(selected_iter);
//		const RowInfo *row_info = param_info_map[path.to_string()];
		if (row_info) {
			// A different canvas? (eg. imported layer)
			canvas = row_info->get_value_desc().get_canvas();
		}
	}

	if (!canvas && canvas_interface)
		canvas = canvas_interface->get_canvas();

	if (canvas) {
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
	int height = allocation.get_height();
	set_default_page_size(height);
	ConfigureAdjustment(range_adjustment)
			.set_page_size(height)
			.set_step_increment(height/10)
			.set_lower(0)
			.set_upper(param_list_height)
			.finish();
}

void Widget_Timetrack::on_canvas_interface_changed()
{
	waypoint_sd.set_canvas_interface(canvas_interface);
}

void Widget_Timetrack::displace_selected_waypoint_items(const synfig::Time& offset)
{
	const float fps = canvas_interface->get_canvas()->rend_desc().get_frame_rate();
	std::vector<WaypointItem*> selection = waypoint_sd.get_selected_items();
	for (WaypointItem * point : selection) {
		const synfig::Time &time = point->time_point.get_time();
		const synfig::Time new_time = synfig::Time(time+offset).round(fps);

		point->time_point.set_time(new_time);
	}
}

void Widget_Timetrack::update_cursor()
{
	const char * cursor_name = "default";
	switch (waypoint_sd.get_action()) {
	case NONE:
		cursor_name = "default";
		break;
	case MOVE:
		cursor_name = "move";
		break;
	case COPY:
		cursor_name = "copy";
		break;
	case SCALE:
		cursor_name = "ew-resize";
		break;
	}
	get_window()->set_cursor(Gdk::Cursor::create(get_display(), cursor_name));
}

void Widget_Timetrack::setup_mouse_handler()
{
	waypoint_sd.set_pan_enabled(true);
	waypoint_sd.set_scroll_enabled(true);
	waypoint_sd.set_zoom_enabled(false);
	waypoint_sd.set_multiple_selection_enabled(true);
	waypoint_sd.set_canvas_interface(canvas_interface);

	waypoint_sd.signal_redraw_needed().connect(sigc::mem_fun(*this, &Gtk::Widget::queue_draw));
	waypoint_sd.signal_focus_requested().connect(sigc::mem_fun(*this, &Gtk::Widget::grab_focus));
	waypoint_sd.signal_scroll_up_requested().connect(sigc::mem_fun(*this, &Widget_Timetrack::scroll_up));
	waypoint_sd.signal_scroll_down_requested().connect(sigc::mem_fun(*this, &Widget_Timetrack::scroll_down));
	waypoint_sd.signal_panning_requested().connect(sigc::mem_fun(*this, &Widget_Timetrack::pan));

	waypoint_sd.signal_selection_changed().connect(sigc::mem_fun(*this, &Gtk::Widget::queue_draw));
	waypoint_sd.signal_item_clicked().connect(sigc::mem_fun(*this, &Widget_Timetrack::on_waypoint_clicked));
	waypoint_sd.signal_item_double_clicked().connect(sigc::mem_fun(*this, &Widget_Timetrack::on_waypoint_double_clicked));
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

	conn = params_store->signal_row_changed().connect([&](const Gtk::TreeModel::Path &p, const Gtk::TreeModel::iterator&){
		// this is the way I found out how to redraw when a new value node is created by editor
		// Instead of on every row change, it should be called only when a value node is created...
		RowInfo * row_info = param_info_map[p.to_string()];
		if (row_info)
			row_info->recheck_for_value_nodes();
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

	conn = params_treeview->get_selection()->signal_changed().connect(sigc::mem_fun(*this, &Widget_Timetrack::queue_draw));
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
	params_store->foreach([&](const Gtk::TreeModel::Path &path, const Gtk::TreeModel::iterator &iter) -> bool {
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

	int previous_param_list_height = param_list_height;
	int first_param_y = 0;
	int last_param_y = 0;
	Gtk::TreeViewColumn *col = params_treeview->get_column(0);
	params_store->foreach_path([&](const Gtk::TreeModel::Path &path) -> bool {
		Gdk::Rectangle rect;
		params_treeview->get_cell_area(path, *col, rect);
		Geometry geometry;
		geometry.y = rect.get_y();
		geometry.h = rect.get_height();
		RowInfo *row_info = param_info_map[path.to_string()];
		if (row_info)
			row_info->set_geometry(geometry);
		else
			synfig::error("internal error: it shouldn't be without parameter row info!");

		last_param_y = std::max(last_param_y, geometry.y + geometry.h);
		first_param_y = std::min(first_param_y, geometry.y);
		return false;
	});

	param_list_height = last_param_y - first_param_y;

	if (previous_param_list_height != param_list_height) {
		ConfigureAdjustment(range_adjustment)
				.set_upper(param_list_height)
				.finish();
	}
	queue_draw();
}

void Widget_Timetrack::draw_static_intervals_for_row(const Cairo::RefPtr<Cairo::Context>& cr, const Widget_Timetrack::RowInfo* row_info, const std::vector<std::pair<synfig::TimePoint, synfig::Time> >& waypoints) const
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

void Widget_Timetrack::draw_waypoints(const Cairo::RefPtr<Cairo::Context>& cr, const Gtk::TreePath &path, const RowInfo *row_info, const std::vector<std::pair<synfig::TimePoint, synfig::Time> >& waypoints) const
{
	const int margin = 1;
	const Geometry &geometry = row_info->get_geometry();
	const int waypoint_edge_length = geometry.h;
	const int py = geometry.y;
	const auto & hovered_point = waypoint_sd.get_hovered_item();

	const synfigapp::ValueDesc &value_desc = row_info->get_value_desc();
	// a not-animated valuenode with waypoints? they should be from inner valubases/nodes (it's a converted parameter)
	const bool is_static_value_node = (value_desc.is_value_node() && !synfig::ValueNode_Animated::Handle::cast_dynamic(value_desc.get_value_node()));

	for (const auto& pair : waypoints) {
		const synfig::TimePoint &tp = pair.first;
		const synfig::Time &t = pair.second;
		int px = time_plot_data->get_pixel_t_coord(t);
		Gdk::Rectangle area(
					0 - waypoint_edge_length/2 + margin + px,
					0 + margin + py,
					waypoint_edge_length - 2*margin,
					waypoint_edge_length - 2*margin);

		bool hover = waypoint_sd.has_hovered_item() && tp == hovered_point.time_point && hovered_point.path == path;
		bool selected = waypoint_sd.is_selected(WaypointItem(tp, path));
		WaypointRenderer::render_time_point_to_window(cr, area, tp, selected, hover, is_static_value_node);
	}
}

void Widget_Timetrack::draw_selected_background(const Cairo::RefPtr<Cairo::Context>& cr, const Gtk::TreePath& path, const RowInfo *row_info) const
{
	if (!params_treeview)
		return;
	std::vector<Gtk::TreePath> path_list = params_treeview->get_selection()->get_selected_rows();
	size_t n_drawn_selected_rows = 0;

	if (n_drawn_selected_rows < path_list.size() // avoid searching if all selected rows have been drawn yet
		&& std::find(path_list.begin(), path_list.end(), path) != path_list.end())
	{
		Geometry geometry = row_info->get_geometry();
		auto foreign_context = params_treeview->get_style_context();
		Gtk::StateFlags old_state = foreign_context->get_state();
		foreign_context->set_state(Gtk::STATE_FLAG_SELECTED);
		foreign_context->render_background(cr, 0, geometry.y, get_width(), geometry.h);
		foreign_context->set_state(old_state);
		n_drawn_selected_rows++;
	}
}

void Widget_Timetrack::on_waypoint_clicked(const Widget_Timetrack::WaypointItem& wi, unsigned int button, Gdk::Point)
{
	std::set<synfig::Waypoint, std::less<synfig::UniqueID> > waypoint_set;
	const synfigapp::ValueDesc &value_desc = param_info_map[wi.path.to_string()]->get_value_desc();
	if (value_desc.is_value_node())
		synfig::waypoint_collect(waypoint_set, wi.time_point.get_time(), value_desc.get_value_node());
	if (waypoint_set.size() > 0)
		signal_waypoint_clicked().emit(value_desc, waypoint_set, button);
}

void Widget_Timetrack::on_waypoint_double_clicked(const Widget_Timetrack::WaypointItem& wi, unsigned int button, Gdk::Point)
{
	std::set<synfig::Waypoint, std::less<synfig::UniqueID> > waypoint_set;
	const synfigapp::ValueDesc &value_desc = param_info_map[wi.path.to_string()]->get_value_desc();
	if (value_desc.is_value_node())
		synfig::waypoint_collect(waypoint_set, wi.time_point.get_time(), value_desc.get_value_node());
	if (waypoint_set.size() > 0)
	signal_waypoint_double_clicked().emit(value_desc, waypoint_set, button);
}

Widget_Timetrack::WaypointScaleInfo Widget_Timetrack::compute_scale_params() const
{
	WaypointScaleInfo info;
	info.scale = 1.0;
	info.base_offset = 0.0;
	info.ref_time = time_plot_data->time_model->get_time();

	if (waypoint_sd.get_action() == SCALE) {
		info.scale = 1.0 + waypoint_sd.get_deltatime() / (waypoint_sd.get_active_item()->time_point.get_time() - info.ref_time);
		info.base_offset = (1 - info.scale) * info.ref_time;
	}
	return info;
}

synfig::Time Widget_Timetrack::compute_scaled_time(const Widget_Timetrack::WaypointItem& item, const Widget_Timetrack::WaypointScaleInfo& scale_info) const
{
//	synfig::Time t = scale_info.ref_time + (item.time_point.get_time() - scale_info.ref_time) * scale_info.scale;
	synfig::Time t = item.time_point.get_time() * scale_info.scale + scale_info.base_offset;
	t = t.round(time_plot_data->time_model->get_frame_rate());
	return t;
}

Widget_Timetrack::RowInfo::RowInfo()
	: geometry{0,0}
{
}

Widget_Timetrack::RowInfo::RowInfo(synfigapp::ValueDesc value_desc_, Widget_Timetrack::Geometry geometry)
	: value_desc(value_desc_), geometry(geometry)
{
	if (value_desc) {
		auto value_node = value_desc.get_value_node();
		if (value_node)
			value_node_connection =
						value_node->signal_changed().connect(
							sigc::mem_fun(*this, &RowInfo::refresh ));
		if (value_desc.parent_is_value_node())
			parent_value_node_connection =
						value_desc.get_parent_value_node()->signal_changed().connect(
							sigc::mem_fun(*this, &RowInfo::refresh ));
		refresh();
	}
}

Widget_Timetrack::RowInfo::~RowInfo()
{
	clear_connections();
}

const synfigapp::ValueDesc& Widget_Timetrack::RowInfo::get_value_desc() const
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

void Widget_Timetrack::RowInfo::recheck_for_value_nodes()
{
	bool changed = false;
	if (value_desc) {
		auto value_node = value_desc.get_value_node();
		if (value_node && !value_node_connection) {
			value_node_connection =
						value_node->signal_changed().connect(
							sigc::mem_fun(*this, &RowInfo::refresh ));
			changed = true;
		}
		if (value_desc.parent_is_value_node() && !parent_value_node_connection) {
			parent_value_node_connection =
						value_desc.get_parent_value_node()->signal_changed().connect(
							sigc::mem_fun(*this, &RowInfo::refresh ));
			changed = true;
		}
		if (changed) {
			// this is the way I found out how to redraw when a new value node is created by editor
			refresh();
		}
	}
}

void Widget_Timetrack::RowInfo::refresh()
{
	if (!value_desc) {
		synfig::error(_("ValueDesc invalid! Internal error"));
		return;
	}

	signal_changed().emit();
}

void Widget_Timetrack::RowInfo::clear_connections()
{
	value_node_connection.disconnect();
	parent_value_node_connection.disconnect();
}

Widget_Timetrack::WaypointItem::WaypointItem(const synfig::TimePoint time_point, const Gtk::TreePath& path)
	: time_point(time_point), path(path)
{
}

bool Widget_Timetrack::WaypointItem::operator ==(const Widget_Timetrack::WaypointItem& b) const
{
	return time_point == b.time_point && path == b.path;
}

Widget_Timetrack::WaypointSD::WaypointSD(Widget_Timetrack& widget)
	: SelectDragHelper<WaypointItem>(_("Move waypoints")),
	  widget(widget),
	  action(ActionState::NONE),
	  is_action_set_before_drag(false)
{
	signal_drag_started().connect([&]() {on_drag_started();});
	signal_drag_canceled().connect([&]() {on_drag_canceled();});
	signal_drag_finished().connect([&](bool started_by_keys) {on_drag_finish(started_by_keys);});
	signal_modifier_keys_changed().connect([&]() {on_modifier_keys_changed();});
}

Widget_Timetrack::WaypointSD::~WaypointSD()
{
}

void Widget_Timetrack::WaypointSD::get_item_position(const Widget_Timetrack::WaypointItem &item, Gdk::Point &p)
{
	p.set_x(widget.time_plot_data->get_pixel_t_coord(item.time_point.get_time()));
	RowInfo * row_info = widget.param_info_map[item.path.to_string()];
	if (!row_info) {
		synfig::warning(_("invalid item"));
		return;
	}
	Geometry geometry = row_info->get_geometry();
	p.set_y(geometry.y + geometry.h/2);
}

bool Widget_Timetrack::WaypointSD::find_item_at_position(int pos_x, int pos_y, Widget_Timetrack::WaypointItem &item)
{
	std::lock_guard<std::mutex> lock(widget.param_list_mutex);

	Gtk::TreePath path;
	bool ok = widget.params_treeview->get_path_at_pos(1, pos_y, path);
	if (!ok)
		return false;

	RowInfo *row_info = widget.param_info_map[path.to_string()];
	if (!row_info) {
		synfig::warning(_("couldn't find row info for path: internal error"));
		return false;
	}
	const synfigapp::ValueDesc &value_desc = row_info->get_value_desc();

	const int waypoint_edge_length = row_info->get_geometry().h;
	bool found = false;

	WaypointRenderer::foreach_visible_waypoint(value_desc, *widget.time_plot_data,
											   [&](const synfig::TimePoint &tp, const synfig::Time &t, void *) -> bool
	{
		const int px = widget.time_plot_data->get_pixel_t_coord(t);
		if (pos_x > px - waypoint_edge_length/2 && pos_x <= px + waypoint_edge_length/2) {
			item.path = path;
			item.time_point = tp;
			found = true;
			return true;
		}
		return false;
	});

	return found;
}

bool Widget_Timetrack::WaypointSD::find_items_in_rect(Gdk::Rectangle rect, std::vector<Widget_Timetrack::WaypointItem>& list)
{
	std::lock_guard<std::mutex> lock(widget.param_list_mutex);

	list.clear();

	int x0 = rect.get_x();
	int x1 = rect.get_x() + rect.get_width();
	if (x0 > x1)
		std::swap(x0, x1);
	int y0 = rect.get_y();
	int y1 = rect.get_y() + rect.get_height();
	if (y0 > y1)
		std::swap(y0, y1);

	std::set<Gtk::TreePath> path_list;
	for (int y = y0; y < y1; ) {
		Gtk::TreePath path;
		bool ok = widget.params_treeview->get_path_at_pos(1, y, path);
		// Did we reach the bottom of treeview? End loop
		// Otherwise the !ok is because user selected a rectangle area from bottom to top
		// and went beyond widget top y coordinate (0).
		if (!ok) {
			if (y >= 0)
				break;
			else {
				y++;
				continue;
			}
		}


		const RowInfo *row_info = widget.param_info_map[path.to_string()];
		if (!row_info) {
			synfig::warning(_("%s :\n\tcouldn't find row info for path: internal error"), __PRETTY_FUNCTION__);
			continue;
		}
		path_list.insert(path);
		y += row_info->get_geometry().h;
	}

	for(const Gtk::TreePath & path : path_list) {
		const RowInfo *row_info = widget.param_info_map[path.to_string()];
		const int waypoint_edge_length = row_info->get_geometry().h;
		const synfigapp::ValueDesc &value_desc = row_info->get_value_desc();

		WaypointRenderer::foreach_visible_waypoint(value_desc, *widget.time_plot_data,
			[&](const synfig::TimePoint &tp, const synfig::Time &t, void *) -> bool
		{
			int px = widget.time_plot_data->get_pixel_t_coord(t);
			if (x0 < px + waypoint_edge_length/2 && x1 >= px - waypoint_edge_length/2) {
				list.push_back(WaypointItem(tp, path));
			}
			return false;
		});
	}
	return list.size() > 0;
}

void Widget_Timetrack::WaypointSD::delta_drag(int total_dx, int /*total_dy*/, bool by_keys)
{
	int dx = 0;
	if (total_dx == 0)
		return;

	if (by_keys) {
		// snap to frames
		dx = total_dx * widget.time_plot_data->k/widget.canvas_interface->get_canvas()->rend_desc().get_frame_rate();
	} else {
		Gdk::Point current_pos;
		get_item_position(*get_active_item(), current_pos);
		int x0, y0;
		get_active_item_initial_point(x0, y0);

		int x1 = x0 + total_dx;
		// snap to frames
		float fps = widget.canvas_interface->get_canvas()->rend_desc().get_frame_rate();
		synfig::Time next_t = widget.time_plot_data->get_t_from_pixel_coord(x1).round(fps);
		x1 = widget.time_plot_data->get_pixel_t_coord(next_t);
		dx = x1 - current_pos.get_x();
	}

	// Move along time
	if (dx == 0) {
		deltatime = 0;
		return;
	}

	const float fps = widget.canvas_interface->get_canvas()->rend_desc().get_frame_rate();
	int x0, y0;
	get_active_item_initial_point(x0, y0);
	const synfig::Time base_time = widget.time_plot_data->get_t_from_pixel_coord(x0);
	const synfig::Time next_time = widget.time_plot_data->get_t_from_pixel_coord(widget.time_plot_data->get_pixel_t_coord(base_time) + dx).round(fps);

	deltatime = next_time - base_time;

	// actual move is done only on finishing drag
}

const synfig::Time& Widget_Timetrack::WaypointSD::get_deltatime() const
{
	return deltatime;
}

Widget_Timetrack::ActionState Widget_Timetrack::WaypointSD::get_action() const
{
	return action;
}

void Widget_Timetrack::WaypointSD::set_action(Widget_Timetrack::ActionState action_state)
{
	action = action_state;
}

sigc::signal<void>& Widget_Timetrack::WaypointSD::signal_action_changed()
{
	return signal_action_changed_;
}

void Widget_Timetrack::WaypointSD::on_drag_started()
{
	deltatime = 0;
	if (action == NONE)
		update_action();
	else
		is_action_set_before_drag = true;
}

void Widget_Timetrack::WaypointSD::on_drag_canceled()
{
	deltatime = 0;
	if (!is_action_set_before_drag)
		update_action();
}

void Widget_Timetrack::WaypointSD::on_drag_finish(bool /*started_by_keys*/)
{
	if (deltatime == 0) {
		update_action();
		return;
	}

	if (action == MOVE)
		widget.move_selected(deltatime);
	else if (action == COPY)
		widget.copy_selected(deltatime);
	else if (action == SCALE)
		widget.scale_selected();

	deltatime = 0;
	if (!is_action_set_before_drag)
		update_action();
}

void Widget_Timetrack::WaypointSD::on_modifier_keys_changed()
{
	update_action();

	if (action != NONE) {
		std::string action_name;
		switch (action) {
		case MOVE:
			action_name = _("Move waypoints");
			break;
		case COPY:
			action_name = _("Copy waypoints");
			break;
		case SCALE:
			action_name = _("Scale waypoints");
			break;
		}

		synfigapp::Action::PassiveGrouper * group = get_action_group_drag();
		if (group)
			group->set_name(action_name);
		else if (get_state() == POINTER_DRAGGING) {
			synfig::warning(_("no undo-action set when on_modifier_keys_changed() is called while dragging waypoints: internal error"));
		}

		widget.queue_draw();
	}
}

void Widget_Timetrack::WaypointSD::update_action()
{
	ActionState previous_action = action;
	if (get_state() != POINTER_DRAGGING) {
		action = NONE;
	} else {
		synfigapp::Action::PassiveGrouper * action_group = get_action_group_drag();
		if (action_group) {
			if (is_dragging_started_by_keys() || !get_modifiers())
				action = MOVE;
			else if (has_modifier(Gdk::SHIFT_MASK))
				action = COPY;
			else if (has_modifier(Gdk::MOD1_MASK))
				action = SCALE;
		} else {
			synfig::warning(_("can't find action group: internal error"));
			action = NONE;
		}
	}
	if (action != previous_action)
		signal_action_changed().emit();
}
