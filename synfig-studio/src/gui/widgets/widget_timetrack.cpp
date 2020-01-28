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

#include <gui/localization.h>

#include <cairomm/cairomm.h>
#include <gdkmm.h>

#include <synfig/general.h>
#endif

using namespace studio;

Widget_Timetrack::Widget_Timetrack()
	: params_treeview(nullptr),
	  update_param_tree_queued(false),
	  update_param_height_queued(false)
{
	add_events(Gdk::BUTTON_PRESS_MASK | Gdk::BUTTON_RELEASE_MASK | Gdk::SCROLL_MASK | Gdk::POINTER_MOTION_MASK | Gdk::KEY_PRESS_MASK | Gdk::KEY_RELEASE_MASK);
	set_can_focus(true);
	time_plot_data->set_extra_time_margin(16/2);
	setup_mouse_handler();

	setup_adjustment();
}

Widget_Timetrack::~Widget_Timetrack()
{
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

	update_param_tree();

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
	{
		std::lock_guard<std::mutex> lock(param_list_mutex);
		if (params_info_list.size() > 0) {
			upper = params_info_list.back().second.y + params_info_list.back().second.h;
		}
	}
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
		queue_update_param_tree();
	});
	treestore_connections.push_back(conn);

	conn = params_store->signal_row_deleted().connect([&](const Gtk::TreeModel::Path&){
		queue_update_param_tree();
	});
	treestore_connections.push_back(conn);

	conn = params_store->signal_rows_reordered().connect([&](const Gtk::TreeModel::Path&, const Gtk::TreeModel::iterator&, int*){
		queue_update_param_tree();
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
		queue_update_param_heights();
	});
	treeview_connections.push_back(conn);

	conn = params_treeview->signal_row_collapsed().connect([&](const Gtk::TreeModel::iterator&,const Gtk::TreeModel::Path&) {
		queue_update_param_heights();
	});
	treeview_connections.push_back(conn);

	conn = params_treeview->signal_style_updated().connect(sigc::mem_fun(*this, &Widget_Timetrack::queue_update_param_heights));
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
		queue_update_param_heights();
	});
	range_adjustment->signal_changed().connect([&](){
		set_default_page_size(range_adjustment->get_page_size());
		queue_update_param_heights();
	});
}

void Widget_Timetrack::queue_update_param_tree()
{
	if (update_param_tree_queued)
		return;
	update_param_tree_queued = true;
	Glib::signal_idle().connect_once(sigc::mem_fun(*this, &Widget_Timetrack::update_param_tree));
}

void Widget_Timetrack::update_param_tree()
{
	std::lock_guard<std::mutex> lock(param_list_mutex);

	update_param_tree_queued = false;

	params_info_list.clear();

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
		params_info_list.push_back(std::pair<synfigapp::ValueDesc, Geometry>(value_desc, geometry));

		return false;
	});

	queue_draw();
}

void Widget_Timetrack::queue_update_param_heights()
{
	if (update_param_height_queued)
		return;
	update_param_height_queued = true;
	Glib::signal_idle().connect_once(sigc::mem_fun(*this, &Widget_Timetrack::update_param_heights));
}

void Widget_Timetrack::update_param_heights()
{
	std::lock_guard<std::mutex> lock(param_list_mutex);
	update_param_height_queued = false;

	if (params_info_list.size() == 0 ||
		!params_treeview ||
		!params_treeview->get_realized() ||
		!params_store)
	{
		queue_draw();
		return;
	}

	size_t idx = 0;
	Gtk::TreeViewColumn *col = params_treeview->get_column(0);
	params_store.get()->foreach([&](const Gtk::TreeModel::Path &path, const Gtk::TreeModel::iterator &/*iter*/) -> bool {
		Gdk::Rectangle rect;
		params_treeview->get_cell_area(path, *col, rect);
		Geometry geometry;
		geometry.y = rect.get_y();
		geometry.h = rect.get_height();
		params_info_list[idx++].second = geometry;

		return false;
	});

	queue_draw();
}

