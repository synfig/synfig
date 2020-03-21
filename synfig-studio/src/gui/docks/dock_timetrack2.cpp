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
	: Dock_CanvasSpecific("timetrack", _("Timetrack"), Gtk::StockID("synfig-timetrack")),
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

	setup_tool_palette();
	tool_palette.show_all();

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
	widget_timetrack->set_valign(Gtk::ALIGN_FILL);

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

	widget_timetrack->signal_action_state_changed().connect([=](){
		update_tool_palette_action();
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

		tool_palette.hide();
	} else {
		widget_kf_list.set_time_model(canvas_view->time_model());
		widget_kf_list.set_canvas_interface(canvas_view->canvas_interface());

		widget_timeslider.set_canvas_view(canvas_view);

		current_widget_timetrack = dynamic_cast<Widget_Timetrack*>( canvas_view->get_ext_widget(get_name()) );
		current_widget_timetrack->set_size_request(100, 100);
		current_widget_timetrack->set_hexpand(true);
		current_widget_timetrack->set_vexpand(true);

		hscrollbar.set_adjustment(canvas_view->time_model()->scroll_time_adjustment());

		update_tool_palette_action();
		tool_palette.show();

		grid.attach(widget_kf_list,            0, 0, 1, 1);
		grid.attach(widget_timeslider,         0, 1, 1, 1);
		grid.attach(*current_widget_timetrack, 0, 2, 1, 1);
		grid.attach(hscrollbar,                0, 4, 2, 1);
		grid.attach(vscrollbar,                1, 0, 1, 4);
		grid.attach(tool_palette,              2, 0, 1, 4);
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

void Dock_Timetrack2::setup_tool_palette()
{
	Gtk::ToolItemGroup *tool_item_group = Gtk::manage(new Gtk::ToolItemGroup());
	gtk_tool_item_group_set_label(tool_item_group->gobj(), nullptr);
	struct ActionButtonInfo {
		std::string name;
		std::string tooltip;
		std::string shortcut;
		Widget_Timetrack::ActionState action_state ;
	};

	const std::vector<ActionButtonInfo> tools_info {
		{"synfig-smooth_move", _("Move waypoints\n\nSelect waypoints and drag them along the timetrack."),
					std::string(""), Widget_Timetrack::ActionState::MOVE},
		{"synfig-duplicate", _("Duplicate waypoints\n\nAfter selecting waypoints, drag to duplicate them and place them in another time point."),
					_("Shift"), Widget_Timetrack::ActionState::COPY},
		{"synfig-scale", _("Scale waypoints\n\nAfter selecting more than one waypoint, drag them to change their timepoint regarding current time."),
// This should be a function like get_key_name() to be reused
#ifdef __APPLE__
					_("Option"),
#else
					_("Alt"),
#endif
					Widget_Timetrack::ActionState::SCALE}
	};

	Gtk::RadioButtonGroup button_group;
	for (const auto & tool_info : tools_info) {
		const std::string &name = tool_info.name;
		std::string tooltip = tool_info.tooltip;
		const std::string &shortcut = tool_info.shortcut;
		Widget_Timetrack::ActionState action_state = tool_info.action_state;

		Gtk::StockItem stock_item;
		Gtk::Stock::lookup(Gtk::StockID(name),stock_item);

		Gtk::RadioToolButton *tool_button = manage(new Gtk::RadioToolButton(
														*manage(new Gtk::Image(
																	stock_item.get_stock_id(),
																	Gtk::IconSize::from_name("synfig-small_icon_16x16") )),
														stock_item.get_label() ));
		tool_button->set_name(Widget_Timetrack::get_action_state_name(action_state));
		if (!shortcut.empty()) {
			std::string shortcut_text = _("Shortcut: ") + shortcut;
			tooltip += "\n\n" + shortcut_text;
		}
		tool_button->set_tooltip_text(tooltip);
		tool_button->set_group(button_group);
		tool_button->signal_toggled().connect([this, tool_button, action_state](){
			if (tool_button->get_active())
				current_widget_timetrack->set_action_state(action_state);
		});
		action_button_map[tool_button->get_name()] = tool_button;
		tool_item_group->add(*tool_button);
	}
	tool_palette.add(*tool_item_group);
	tool_palette.set_sensitive(true);
}

void Dock_Timetrack2::update_tool_palette_action()
{
	if (!current_widget_timetrack)
		return;
	Widget_Timetrack::ActionState action_state = current_widget_timetrack->get_action_state();
	std::string action_state_name = Widget_Timetrack::get_action_state_name(action_state);

	Gtk::RadioToolButton * button = action_button_map[action_state_name];
	if (!button)
		button = action_button_map[Widget_Timetrack::get_action_state_name(Widget_Timetrack::NONE)];
	if (!button)
		button = action_button_map[Widget_Timetrack::get_action_state_name(Widget_Timetrack::MOVE)];

	if (button)
		button->set_active(true);
}
