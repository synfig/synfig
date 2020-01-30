/* === S Y N F I G ========================================================= */
/*!	\file docks/dock_timetrack2.cpp
**	\brief Dock to displaying layer parameters timetrack
**
**	$Id$
**
**	\legal
**	Copyright (c) 2002-2005 Robert B. Quattlebaum Jr., Adrian Bentley
**  ......... ... 2020 Rodolfo Ribeiro Gomes
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

#include "dock_timetrack2.h"

#include <widgets/widget_timetrack.h>
#include <gui/canvasview.h>
#include <gui/localization.h>

#include <synfig/general.h>

#endif

using namespace studio;

Dock_Timetrack2::Dock_Timetrack2()
	: Dock_CanvasSpecific("timetrack2", _("Timetrack"), Gtk::StockID("synfig-timetrack")),
	  current_widget_timetrack(nullptr)
{
	set_use_scrolled(false);

	widget_kf_list.set_hexpand();
	widget_kf_list.show();
	widget_timeslider.set_hexpand();
	widget_timeslider.show();

	vscrollbar.set_vexpand();
	vscrollbar.set_hexpand(false);
	vscrollbar.show();
	hscrollbar.set_hexpand();
	hscrollbar.show();

	grid.set_column_homogeneous(false);
	grid.set_row_homogeneous(false);
	// for letting user click/drag waypoint or keyframe mark of time zero
	grid.set_margin_left(2);
	grid.set_margin_right(2);

	add(grid);
}

void Dock_Timetrack2::init_canvas_view_vfunc(etl::loose_handle<CanvasView> canvas_view)
{
	Widget_Timetrack *widget_timetrack = new Widget_Timetrack();
	widget_timetrack->use_canvas_view(canvas_view);
	widget_timetrack->show();
	widget_timetrack->set_hexpand(true);
	widget_timetrack->set_vexpand(true);

	canvas_view->set_ext_widget(get_name(), widget_timetrack);

	// sync with Parameters Dock
	//  scrolling
	vscrollbar.set_adjustment(widget_timetrack->get_range_adjustment());
	canvas_view->get_adjustment_group("params")->add(vscrollbar.get_adjustment());
	//  TreeView header
	studio::LayerTree *tree_layer = dynamic_cast<studio::LayerTree*>(canvas_view->get_ext_widget("layers_cmp") );
	assert(tree_layer);
	tree_layer->signal_param_tree_header_height_changed().connect(
		sigc::mem_fun(*this, &studio::Dock_Timetrack2::on_update_header_height)
	);

	widget_timetrack->signal_waypoint_clicked().connect([=](synfigapp::ValueDesc value_desc, std::set<synfig::Waypoint,std::less<synfig::UniqueID>> waypoint_set, int button) {
		if (button != 3)
			return;
		button = 2;
		canvas_view->on_waypoint_clicked_canvasview(value_desc, waypoint_set, button);
	});

	widget_timetrack->signal_waypoint_double_clicked().connect([=](synfigapp::ValueDesc value_desc, std::set<synfig::Waypoint,std::less<synfig::UniqueID>> waypoint_set, int button) {
		if (button != 1)
			return;
		button = -1;
		canvas_view->on_waypoint_clicked_canvasview(value_desc, waypoint_set, button);
	});
}

void Dock_Timetrack2::changed_canvas_view_vfunc(etl::loose_handle<CanvasView> canvas_view)
{
	const std::vector<Gtk::Widget*> children = grid.get_children();
	for (Gtk::Widget * widget : children) {
		// CanvasView and Dock_Timetrack2 will delete widgets when needed
		grid.remove(*widget);
	}

	if( !canvas_view ) {
		widget_kf_list.set_time_model( etl::handle<TimeModel>() );
		widget_kf_list.set_canvas_interface( etl::loose_handle<synfigapp::CanvasInterface>() );

		widget_timeslider.set_canvas_view( CanvasView::Handle() );

		current_widget_timetrack = nullptr; // deleted by its studio::CanvasView::~CanvasView()

		hscrollbar.unset_adjustment();
	} else {
		widget_kf_list.set_time_model(canvas_view->time_model());
		widget_kf_list.set_canvas_interface(canvas_view->canvas_interface());

		widget_timeslider.set_canvas_view(canvas_view);

		current_widget_timetrack = dynamic_cast<Widget_Timetrack*>( canvas_view->get_ext_widget(get_name()) );
		current_widget_timetrack->set_size_request(100, 100);
		current_widget_timetrack->set_hexpand(true);
		current_widget_timetrack->set_vexpand(true);

		hscrollbar.set_adjustment(canvas_view->time_model()->scroll_time_adjustment());

		grid.attach(widget_kf_list,            0, 0, 1, 1);
		grid.attach(widget_timeslider,         0, 1, 1, 1);
		grid.attach(*current_widget_timetrack, 0, 2, 1, 1);
		grid.attach(hscrollbar,                0, 4, 2, 1);
		grid.attach(vscrollbar,                1, 0, 1, 4);
		grid.show();
	}

}

void Dock_Timetrack2::on_update_header_height(int height)
{
	int w = 0, h = 0;
	widget_kf_list.get_size_request(w, h);
	int ts_height = std::max(1, height - h);

	widget_timeslider.get_size_request(w, h);
	if (h != ts_height)
		widget_timeslider.set_size_request(-1, ts_height);
}
