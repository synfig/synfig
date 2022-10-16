/* === S Y N F I G ========================================================= */
/*!	\file docks/dock_timetrack2.cpp
**	\brief Dock to displaying layer parameters timetrack
**
**	\legal
**	Copyright (c) 2002-2005 Robert B. Quattlebaum Jr., Adrian Bentley
**  ......... ... 2020 Rodolfo Ribeiro Gomes
**
**	This file is part of Synfig.
**
**	Synfig is free software: you can redistribute it and/or modify
**	it under the terms of the GNU General Public License as published by
**	the Free Software Foundation, either version 2 of the License, or
**	(at your option) any later version.
**
**	Synfig is distributed in the hope that it will be useful,
**	but WITHOUT ANY WARRANTY; without even the implied warranty of
**	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
**	GNU General Public License for more details.
**
**	You should have received a copy of the GNU General Public License
**	along with Synfig.  If not, see <https://www.gnu.org/licenses/>.
**	\endlegal
*/

#ifdef USING_PCH
#	include "pch.h"
#else
#ifdef HAVE_CONFIG_H
#	include <config.h>
#endif

#include "dock_timetrack2.h"

#include <gtkmm/stock.h>

#include <gui/widgets/widget_timetrack.h>
#include <gui/canvasview.h>
#include <gui/localization.h>
#include <gui/iconcontroller.h>

#endif

using namespace studio;

Dock_Timetrack2::Dock_Timetrack2()
	: Dock_CanvasSpecific("timetrack", _("Timetrack"), "time_track_icon"),
	  current_widget_timetrack(nullptr)
{
	set_use_scrolled(false);

	widget_kf_list.set_hexpand();
	widget_kf_list.show();
	widget_timeslider.set_hexpand();
	widget_timeslider.show();

	vscrollbar.set_vexpand();
	vscrollbar.set_hexpand(false);
	vscrollbar.set_orientation(Gtk::ORIENTATION_VERTICAL);
	vscrollbar.show();
	hscrollbar.set_hexpand();
	hscrollbar.show();

	setup_toolbar();
	toolbar->show_all();
	set_interp_buttons_sensitivity(false);

	grid.set_column_homogeneous(false);
	grid.set_row_homogeneous(false);
	// for letting user click/drag waypoint or keyframe mark of time zero
	grid.set_margin_start(2);
	grid.set_margin_end(2);

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

	widget_timetrack->signal_waypoint_clicked().connect(sigc::mem_fun(*this, &Dock_Timetrack2::on_widget_timetrack_waypoint_clicked));

	widget_timetrack->signal_waypoint_double_clicked().connect(sigc::mem_fun(*this, &Dock_Timetrack2::on_widget_timetrack_waypoint_double_clicked));

	widget_timetrack->signal_action_state_changed().connect(sigc::mem_fun(*this, &Dock_Timetrack2::update_toolbar_action));

	widget_timetrack->signal_waypoint_selection_changed().connect(sigc::mem_fun(*this, &Dock_Timetrack2::set_interp_buttons_sensitivity));
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

		toolbar->hide();
	} else {
		widget_kf_list.set_time_model(canvas_view->time_model());
		widget_kf_list.set_canvas_interface(canvas_view->canvas_interface());

		widget_timeslider.set_canvas_view(canvas_view);

		current_widget_timetrack = dynamic_cast<Widget_Timetrack*>( canvas_view->get_ext_widget(get_name()) );
		current_widget_timetrack->set_size_request(100, 100);
		current_widget_timetrack->set_hexpand(true);
		current_widget_timetrack->set_vexpand(true);

		hscrollbar.set_adjustment(canvas_view->time_model()->scroll_time_adjustment());

		update_toolbar_action();
		set_interp_buttons_sensitivity(current_widget_timetrack->get_num_waypoints_selected());

		grid.attach(widget_kf_list,            0, 0, 1, 1);
		grid.attach(widget_timeslider,         0, 1, 1, 1);
		grid.attach(*current_widget_timetrack, 0, 2, 1, 1);
		grid.attach(hscrollbar,                0, 4, 2, 1);
		grid.attach(vscrollbar,                1, 0, 1, 4);
		grid.attach(*toolbar,                  2, 0, 1, 4);
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

void Dock_Timetrack2::on_widget_timetrack_waypoint_clicked(synfigapp::ValueDesc value_desc, std::set<synfig::Waypoint, std::less<synfig::UniqueID> > waypoint_set, int button)
{
	if (button != 3)
		return;
	button = 2;
	CanvasView::LooseHandle canvas_view = get_canvas_view();
	if (canvas_view)
		canvas_view->on_waypoint_clicked_canvasview(value_desc, waypoint_set, button);
}

void Dock_Timetrack2::on_widget_timetrack_waypoint_double_clicked(synfigapp::ValueDesc value_desc, std::set<synfig::Waypoint, std::less<synfig::UniqueID> > waypoint_set, int button)
{
	if (button != 1)
		return;
	button = -1;
	CanvasView::LooseHandle canvas_view = get_canvas_view();
	if (canvas_view)
		canvas_view->on_waypoint_clicked_canvasview(value_desc, waypoint_set, button);
}

void Dock_Timetrack2::setup_toolbar()
{
	toolbar = manage(new Gtk::Toolbar());
	toolbar->set_icon_size(Gtk::IconSize::from_name("synfig-small_icon_16x16"));
	toolbar->set_toolbar_style(Gtk::TOOLBAR_ICONS);
	toolbar->set_property("orientation", Gtk::ORIENTATION_VERTICAL);
	toolbar->get_style_context()->add_class("synfigstudio-efficient-workspace");

	struct ActionButtonInfo {
		std::string icon;
		std::string tooltip;
		std::string shortcut;
		Widget_Timetrack::ActionState action_state ;
	};

	const std::vector<ActionButtonInfo> tools_info {
		{"tool_smooth_move_icon", _("Move waypoints\n\nSelect waypoints and drag them along the timetrack."),
					std::string(""), Widget_Timetrack::ActionState::MOVE},
		{"duplicate_icon", _("Duplicate waypoints\n\nAfter selecting waypoints, drag to duplicate them and place them in another time point."),
					_("Shift"), Widget_Timetrack::ActionState::COPY},
		{"tool_scale_icon", _("Scale waypoints\n\nAfter selecting more than one waypoint, drag them to change their timepoint regarding current time."),
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
		const std::string &icon_name = tool_info.icon;
		std::string tooltip = tool_info.tooltip;
		const std::string &shortcut = tool_info.shortcut;
		Widget_Timetrack::ActionState action_state = tool_info.action_state;

		Gtk::RadioToolButton *tool_button = manage(new Gtk::RadioToolButton());
		tool_button->set_icon_name(icon_name);
		tool_button->set_name(Widget_Timetrack::get_action_state_name(action_state));
		if (!shortcut.empty()) {
			std::string shortcut_text = _("Shortcut: ") + shortcut;
			tooltip += "\n\n" + shortcut_text;
		}
		tool_button->set_tooltip_text(tooltip);
		tool_button->set_group(button_group);
		tool_button->signal_toggled().connect(sigc::track_obj([this, tool_button, action_state](){
			if (tool_button->get_active())
				current_widget_timetrack->set_action_state(action_state);
		}, *this));
		action_button_map[tool_button->get_name()] = tool_button;
		toolbar->append(*tool_button);
	}

	Gtk::SeparatorToolItem* separator = Gtk::manage(new Gtk::SeparatorToolItem());
	separator->set_name("separator");
	toolbar->append(*separator);

	struct InterpolationButtonInfo {
		std::string name;
		synfig::Interpolation interpolation;
	};

	const std::vector<InterpolationButtonInfo> interp_buttons_info{
		{N_("Clamped"), synfig::INTERPOLATION_CLAMPED},
		{N_("TCB"), synfig::INTERPOLATION_TCB},
		{N_("Constant"), synfig::INTERPOLATION_CONSTANT},
		{N_("Ease In/Out"), synfig::INTERPOLATION_HALT},
		{N_("Linear"), synfig::INTERPOLATION_LINEAR}
	};

	for (const auto & interp_button_info: interp_buttons_info) {
		Gtk::Image* image= Gtk::manage(new Gtk::Image());
		image->set_from_icon_name(interpolation_icon_name(interp_button_info.interpolation), Gtk::IconSize::from_name("synfig-small_icon_16x16"));
		Gtk::ToolButton *tool_button = manage(new Gtk::ToolButton(*image, _((interp_button_info.name).c_str())));
		tool_button->signal_clicked().connect(sigc::track_obj([this, interp_button_info](){
			current_widget_timetrack->interpolate_selected(interp_button_info.interpolation);
		}, *this));
		tool_button->set_tooltip_text(synfig::strprintf(_("Change waypoint interpolation to %s"), interp_button_info.name.c_str()));
		tool_button->set_name(interp_button_info.name);
		toolbar->append(*tool_button);
	}
	toolbar->set_sensitive(true);
}

void Dock_Timetrack2::update_toolbar_action()
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

void Dock_Timetrack2::set_interp_buttons_sensitivity(bool sensitive)
{
	for (int i = 0; i < toolbar->get_n_items(); i++){
		std::string name = toolbar->get_nth_item(i)->get_name();
		if ( name == "Clamped" || name == "TCB" || name == "Constant" ||
			 name =="separator" || name == "Ease In/Out"|| name == "Linear")
			toolbar->get_nth_item(i)->set_sensitive(sensitive);
	}
}

