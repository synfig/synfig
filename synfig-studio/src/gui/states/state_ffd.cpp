/* === S Y N F I G ========================================================= */
/*!	\file state_ffd.cpp
**	\brief FFD Tool Implementation
**
**	\legal
**	Copyright (c) 2026 Ahmed Fathy
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
/* ========================================================================= */

/* === H E A D E R S ======================================================= */

#ifdef USING_PCH
#	include "pch.h"
#else
#ifdef HAVE_CONFIG_H
#	include <config.h>
#endif

#include <gui/states/state_ffd.h>
#include <gui/states/state_normal.h>

#include <gui/app.h>
#include <gui/canvasview.h>
#include <gui/docks/dialog_tooloptions.h>
#include <gui/docks/dock_toolbox.h>
#include <gui/event_mouse.h>
#include <gui/localization.h>
#include <gui/workarea.h>

#include <synfig/general.h>
#include <synfig/layers/layer_freeformdeform.h>

#include <synfigapp/main.h>

#endif

/* === U S I N G =========================================================== */

using namespace studio;
using namespace synfig;

/* === M A C R O S ========================================================= */

#define GAP	(3)

/* === G L O B A L S ======================================================= */

StateFFD studio::state_ffd;

/* === C L A S S E S & S T R U C T S ======================================= */

class studio::StateFFD_Context : public sigc::trackable
{
	CanvasView::Handle canvas_view_;
	CanvasView::IsWorking is_working;

	bool prev_workarea_layer_status_;

	// Tool options widgets
	Gtk::Grid options_table;
	Gtk::Label title_label;

	Gtk::Label grid_x_label;
	Glib::RefPtr<Gtk::Adjustment> grid_x_adj;
	Gtk::SpinButton grid_x_spin;

	Gtk::Label grid_y_label;
	Glib::RefPtr<Gtk::Adjustment> grid_y_adj;
	Gtk::SpinButton grid_y_spin;

	Gtk::Label smoothness_label;
	Glib::RefPtr<Gtk::Adjustment> smoothness_adj;
	Gtk::Scale smoothness_hscl;

	Gtk::Label status_label;

	// Signal blocking flag
	bool updating_from_layer_;

	void on_grid_x_changed();
	void on_grid_y_changed();
	void on_smoothness_changed();

	synfig::Layer::Handle get_selected_ffd_layer() const;
	void update_controls_from_layer();

public:

	Smach::event_result event_refresh_tool_options(const Smach::event& x);
	Smach::event_result event_layer_selection_changed_handler(const Smach::event& x);
	Smach::event_result event_stop_handler(const Smach::event& x);

	void refresh_tool_options();

	StateFFD_Context(CanvasView* canvas_view);
	~StateFFD_Context();

	const CanvasView::Handle& get_canvas_view() const { return canvas_view_; }
	etl::handle<synfigapp::CanvasInterface> get_canvas_interface() const { return canvas_view_->canvas_interface(); }
	synfig::Canvas::Handle get_canvas() const { return canvas_view_->get_canvas(); }
	WorkArea* get_work_area() const { return canvas_view_->get_work_area(); }

}; // END of class StateFFD_Context

/* === M E T H O D S ======================================================= */

StateFFD::StateFFD() :
	Smach::state<StateFFD_Context>("ffd", N_("FFD Tool"))
{
	insert(event_def(EVENT_LAYER_SELECTION_CHANGED, &StateFFD_Context::event_layer_selection_changed_handler));
	insert(event_def(EVENT_REFRESH_TOOL_OPTIONS,    &StateFFD_Context::event_refresh_tool_options));
	insert(event_def(EVENT_STOP,                    &StateFFD_Context::event_stop_handler));
	insert(event_def(EVENT_REFRESH_DUCKS,           &StateFFD_Context::event_refresh_tool_options));
}

StateFFD::~StateFFD()
= default;

void* StateFFD::enter_state(studio::CanvasView* machine_context) const
{
	return new StateFFD_Context(machine_context);
}

StateFFD_Context::StateFFD_Context(CanvasView* canvas_view) :
	canvas_view_(canvas_view),
	is_working(*canvas_view),
	prev_workarea_layer_status_(get_work_area()->get_allow_layer_clicks()),
	grid_x_adj(Gtk::Adjustment::create(3, 2, 20, 1, 1)),
	grid_x_spin(grid_x_adj, 1, 0),
	grid_y_adj(Gtk::Adjustment::create(3, 2, 20, 1, 1)),
	grid_y_spin(grid_y_adj, 1, 0),
	smoothness_adj(Gtk::Adjustment::create(1.0, 0.0, 1.0, 0.01, 0.1)),
	smoothness_hscl(smoothness_adj, Gtk::ORIENTATION_HORIZONTAL),
	updating_from_layer_(false)
{
	get_canvas_interface()->set_state("ffd");

	// Title
	title_label.set_label(_("FFD Tool"));
	Pango::AttrList list;
	Pango::AttrInt attr = Pango::Attribute::create_attr_weight(Pango::WEIGHT_BOLD);
	list.insert(attr);
	title_label.set_attributes(list);

	// Grid Size X
	grid_x_label.set_label(_("Grid Size X:"));
	grid_x_label.set_halign(Gtk::ALIGN_START);
	grid_x_spin.set_hexpand(true);

	// Grid Size Y
	grid_y_label.set_label(_("Grid Size Y:"));
	grid_y_label.set_halign(Gtk::ALIGN_START);
	grid_y_spin.set_hexpand(true);

	// Smoothness
	smoothness_label.set_label(_("Smoothness:"));
	smoothness_label.set_halign(Gtk::ALIGN_START);
	smoothness_hscl.set_digits(2);
	smoothness_hscl.set_value_pos(Gtk::POS_LEFT);
	smoothness_hscl.set_tooltip_text(_("0 = Bilinear (sharp), 1 = Catmull-Rom (smooth)"));
	smoothness_hscl.set_hexpand(true);

	// Status label
	status_label.set_label(_("Select a Free-Form Deformation layer"));
	status_label.set_halign(Gtk::ALIGN_START);

	// Layout
	options_table.attach(title_label,       0, 0, 2, 1);
	options_table.attach(status_label,      0, 1, 2, 1);
	options_table.attach(grid_x_label,      0, 2, 1, 1);
	options_table.attach(grid_x_spin,       1, 2, 1, 1);
	options_table.attach(grid_y_label,      0, 3, 1, 1);
	options_table.attach(grid_y_spin,       1, 3, 1, 1);
	options_table.attach(smoothness_label,  0, 4, 1, 1);
	options_table.attach(smoothness_hscl,   1, 4, 1, 1);

	options_table.set_border_width(GAP*2);
	options_table.set_row_spacing(GAP);
	options_table.set_margin_bottom(0);
	options_table.show_all();

	// Connect signals
	grid_x_spin.signal_value_changed().connect(
		sigc::mem_fun(*this, &StateFFD_Context::on_grid_x_changed));
	grid_y_spin.signal_value_changed().connect(
		sigc::mem_fun(*this, &StateFFD_Context::on_grid_y_changed));
	smoothness_hscl.signal_value_changed().connect(
		sigc::mem_fun(*this, &StateFFD_Context::on_smoothness_changed));

	// Update controls from current selection
	update_controls_from_layer();

	refresh_tool_options();
	App::dialog_tool_options->present();

	// Allow layer clicks so the user can select FFD layers
	get_work_area()->set_allow_layer_clicks(true);

	get_work_area()->set_cursor(Gdk::ARROW);

	App::dock_toolbox->refresh();
}

void
StateFFD_Context::refresh_tool_options()
{
	App::dialog_tool_options->clear();
	App::dialog_tool_options->set_widget(options_table);
	App::dialog_tool_options->set_local_name(_("FFD Tool"));
	App::dialog_tool_options->set_icon("tool_ffd_icon");
}

StateFFD_Context::~StateFFD_Context()
{
	get_canvas_interface()->set_state("");

	App::dialog_tool_options->clear();

	get_work_area()->reset_cursor();

	// Restore layer clicking
	get_work_area()->set_allow_layer_clicks(prev_workarea_layer_status_);

	get_work_area()->queue_draw();

	App::dock_toolbox->refresh();
}

synfig::Layer::Handle
StateFFD_Context::get_selected_ffd_layer() const
{
	std::list<Layer::Handle> layers =
		get_canvas_interface()->get_selection_manager()->get_selected_layers();

	for (const auto& layer : layers) {
		if (layer && layer->get_name() == "free_form_deform")
			return layer;
	}
	return nullptr;
}

void
StateFFD_Context::update_controls_from_layer()
{
	synfig::Layer::Handle ffd = get_selected_ffd_layer();

	if (ffd) {
		updating_from_layer_ = true;

		int gx = ffd->get_param("grid_size_x").get(int());
		int gy = ffd->get_param("grid_size_y").get(int());
		synfig::Real sm = ffd->get_param("smoothness").get(synfig::Real());

		grid_x_adj->set_value(gx);
		grid_y_adj->set_value(gy);
		smoothness_hscl.set_value(sm);

		updating_from_layer_ = false;

		status_label.set_label(_("FFD layer selected"));
		grid_x_label.show();
		grid_x_spin.show();
		grid_y_label.show();
		grid_y_spin.show();
		smoothness_label.show();
		smoothness_hscl.show();
	} else {
		status_label.set_label(_("Select a Free-Form Deformation layer"));
		grid_x_label.hide();
		grid_x_spin.hide();
		grid_y_label.hide();
		grid_y_spin.hide();
		smoothness_label.hide();
		smoothness_hscl.hide();
	}
}

void
StateFFD_Context::on_grid_x_changed()
{
	if (updating_from_layer_) return;

	synfig::Layer::Handle ffd = get_selected_ffd_layer();
	if (!ffd) return;

	int val = (int)grid_x_adj->get_value();
	ffd->set_param("grid_size_x", val);
	get_canvas_interface()->signal_layer_param_changed()(ffd, "grid_size_x");
	get_canvas_view()->queue_rebuild_ducks();
	get_work_area()->queue_render();
}

void
StateFFD_Context::on_grid_y_changed()
{
	if (updating_from_layer_) return;

	synfig::Layer::Handle ffd = get_selected_ffd_layer();
	if (!ffd) return;

	int val = (int)grid_y_adj->get_value();
	ffd->set_param("grid_size_y", val);
	get_canvas_interface()->signal_layer_param_changed()(ffd, "grid_size_y");
	get_canvas_view()->queue_rebuild_ducks();
	get_work_area()->queue_render();
}

void
StateFFD_Context::on_smoothness_changed()
{
	if (updating_from_layer_) return;

	synfig::Layer::Handle ffd = get_selected_ffd_layer();
	if (!ffd) return;

	synfig::Real val = smoothness_hscl.get_value();
	ffd->set_param("smoothness", val);
	get_canvas_interface()->signal_layer_param_changed()(ffd, "smoothness");
	get_work_area()->queue_render();
}

Smach::event_result
StateFFD_Context::event_refresh_tool_options(const Smach::event& /*x*/)
{
	refresh_tool_options();
	return Smach::RESULT_ACCEPT;
}

Smach::event_result
StateFFD_Context::event_layer_selection_changed_handler(const Smach::event& /*x*/)
{
	update_controls_from_layer();
	return Smach::RESULT_ACCEPT;
}

Smach::event_result
StateFFD_Context::event_stop_handler(const Smach::event& /*x*/)
{
	throw &state_normal;
	return Smach::RESULT_OK;
}
