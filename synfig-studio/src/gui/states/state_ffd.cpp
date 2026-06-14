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
#include <gui/event_keyboard.h>
#include <gdk/gdkkeysyms.h>
#include <gui/localization.h>
#include <gui/workarea.h>

#include <synfig/general.h>
#include <synfig/layers/layer_freeformdeform.h>

#include <synfigapp/blineconvert.h>
#include <synfigapp/main.h>
#include <synfig/transformation.h>
#include <synfigapp/instance.h>
#include <synfig/valuenodes/valuenode_dynamiclist.h>
#include <synfig/valuenodes/valuenode_const.h>
#include <gui/widgets/widget_enum.h>

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

	Gtk::Label cull_threshold_label;
	Glib::RefPtr<Gtk::Adjustment> cull_threshold_adj;
	Gtk::SpinButton cull_threshold_spin;

	Gtk::Label status_label;

	// Signal blocking flag
	bool updating_from_layer_;

	sigc::connection layer_param_changed_connection;
	void on_layer_param_changed(synfig::Layer::Handle layer, synfig::String param_name);

	Gtk::Button reset_button;

	// Creation mode widgets
	Gtk::Label mesh_mode_label;
	Widget_Enum mesh_mode_enum;
	Gtk::Label create_grid_x_label;
	Gtk::SpinButton create_grid_x_spin;
	Gtk::Label create_grid_y_label;
	Gtk::SpinButton create_grid_y_spin;
	Gtk::Button make_ffd_button;
	Gtk::Button clear_button;
	
	Gtk::Button edit_mesh_button;
	Gtk::Button update_ffd_button;

	std::list<synfig::Point> polygon_point_list;
	std::list<synfig::Point> redo_point_list;
	bool editing_existing_mesh_;

	void on_grid_x_changed();
	void on_grid_y_changed();
	void on_smoothness_changed();
	void on_cull_threshold_changed();
	void on_reset_pressed();

	void on_make_ffd_pressed();
	void on_edit_mesh_pressed();
	void on_update_ffd_pressed();
	void reset();
	bool on_polygon_duck_change(const studio::Duck &duck, std::list<synfig::Point>::iterator iter);
	void on_duck_right_click(std::list<synfig::Point>::iterator iter);
	void refresh_ducks();

	synfig::Layer::Handle get_selected_ffd_layer() const;
	void update_controls_from_layer();
	void on_mesh_mode_changed();

public:

	Smach::event_result event_refresh_tool_options(const Smach::event& x);
	Smach::event_result event_layer_selection_changed_handler(const Smach::event& x);
	Smach::event_result event_stop_handler(const Smach::event& x);
	Smach::event_result event_mouse_click_handler(const Smach::event& x);
	Smach::event_result event_mouse_doubleclick_handler(const Smach::event& x);
	Smach::event_result event_key_press_handler(const Smach::event& x);
	Smach::event_result event_refresh_handler(const Smach::event& x);

	void refresh_tool_options();

	explicit StateFFD_Context(CanvasView* canvas_view);
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
	insert(event_def(EVENT_REFRESH,                 &StateFFD_Context::event_refresh_handler));
	insert(event_def(EVENT_REFRESH_DUCKS,           &StateFFD_Context::event_refresh_handler));
	insert(event_def(EVENT_WORKAREA_KEY_DOWN,      &StateFFD_Context::event_key_press_handler));
	insert(event_def(EVENT_WORKAREA_MOUSE_BUTTON_DOWN,   &StateFFD_Context::event_mouse_click_handler));
	insert(event_def(EVENT_WORKAREA_MOUSE_2BUTTON_DOWN,  &StateFFD_Context::event_mouse_doubleclick_handler));
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
	cull_threshold_adj(Gtk::Adjustment::create(0.0, 0.0, 1000.0, 0.01, 1.0)),
	cull_threshold_spin(cull_threshold_adj, 0.01, 2),
	reset_button(_("Reset Grid")),
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

	cull_threshold_label.set_label(_("Cull Threshold:"));
	cull_threshold_label.set_halign(Gtk::ALIGN_START);
	cull_threshold_spin.set_tooltip_text(_("Removes triangles larger than this threshold (0 = disable)"));
	cull_threshold_spin.set_hexpand(true);

	// Status label
	status_label.set_label(_("Select a Free-Form Deformation layer"));
	status_label.set_halign(Gtk::ALIGN_START);

	// Creation UI
	mesh_mode_label.set_label(_("Creation Mode:"));
	mesh_mode_label.set_halign(Gtk::ALIGN_START);
	mesh_mode_enum.set_param_desc(ParamDesc("mesh_mode")
		.set_local_name(_("Mode"))
		.set_description(_("0 = Grid, 1 = Custom Mesh"))
		.add_enum_value(0, "grid", _("Grid"))
		.add_enum_value(1, "custom", _("Custom Mesh"))
	);
	mesh_mode_enum.set_value(1);

	create_grid_x_label.set_label(_("Cols (Grid):"));
	create_grid_x_label.set_halign(Gtk::ALIGN_START);
	create_grid_x_spin.set_adjustment(grid_x_adj); // reuse adjustment
	create_grid_x_spin.set_hexpand(true);

	create_grid_y_label.set_label(_("Rows (Grid):"));
	create_grid_y_label.set_halign(Gtk::ALIGN_START);
	create_grid_y_spin.set_adjustment(grid_y_adj);
	create_grid_y_spin.set_hexpand(true);

	make_ffd_button.set_label(_("Make FFD Layer"));
	make_ffd_button.set_hexpand(true);
	make_ffd_button.set_halign(Gtk::ALIGN_FILL);
	
	clear_button.set_label(_("Clear Points"));
	clear_button.set_hexpand(true);
	clear_button.set_halign(Gtk::ALIGN_FILL);
	
	edit_mesh_button.set_label(_("Edit Custom Mesh"));
	edit_mesh_button.set_hexpand(true);
	edit_mesh_button.set_halign(Gtk::ALIGN_FILL);
	
	update_ffd_button.set_label(_("Update Mesh"));
	update_ffd_button.set_hexpand(true);
	update_ffd_button.set_halign(Gtk::ALIGN_FILL);

	editing_existing_mesh_ = false;

	// Layout
	options_table.attach(title_label,       0, 0, 2, 1);
	options_table.attach(status_label,      0, 1, 2, 1);
	// Edit UI
	options_table.attach(grid_x_label,      0, 2, 1, 1);
	options_table.attach(grid_x_spin,       1, 2, 1, 1);
	options_table.attach(grid_y_label,      0, 3, 1, 1);
	options_table.attach(grid_y_spin,       1, 3, 1, 1);
	options_table.attach(smoothness_label,  0, 4, 1, 1);
	options_table.attach(smoothness_hscl,   1, 4, 1, 1);
	options_table.attach(cull_threshold_label, 0, 5, 1, 1);
	options_table.attach(cull_threshold_spin,  1, 5, 1, 1);
	options_table.attach(reset_button,      0, 6, 2, 1);

	// Creation UI
	options_table.attach(mesh_mode_label,     0, 7, 1, 1);
	options_table.attach(mesh_mode_enum,      1, 7, 1, 1);
	options_table.attach(create_grid_x_label, 0, 8, 1, 1);
	options_table.attach(create_grid_x_spin,  1, 8, 1, 1);
	options_table.attach(create_grid_y_label, 0, 9, 1, 1);
	options_table.attach(create_grid_y_spin,  1, 9, 1, 1);
	options_table.attach(make_ffd_button,     0, 10, 2, 1);
	options_table.attach(clear_button,        0, 11, 2, 1);
	options_table.attach(edit_mesh_button,    0, 12, 2, 1);
	options_table.attach(update_ffd_button,   0, 13, 2, 1);

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
	cull_threshold_spin.signal_value_changed().connect(
		sigc::mem_fun(*this, &StateFFD_Context::on_cull_threshold_changed));
	reset_button.signal_clicked().connect(
		sigc::mem_fun(*this, &StateFFD_Context::on_reset_pressed));
	make_ffd_button.signal_clicked().connect(
		sigc::mem_fun(*this, &StateFFD_Context::on_make_ffd_pressed));
	edit_mesh_button.signal_clicked().connect(
		sigc::mem_fun(*this, &StateFFD_Context::on_edit_mesh_pressed));
	update_ffd_button.signal_clicked().connect(
		sigc::mem_fun(*this, &StateFFD_Context::on_update_ffd_pressed));
	clear_button.signal_clicked().connect(
		sigc::mem_fun(*this, &StateFFD_Context::reset));
	mesh_mode_enum.signal_changed().connect(
		sigc::mem_fun(*this, &StateFFD_Context::on_mesh_mode_changed));

	// Update controls from current selection
	update_controls_from_layer();
	on_mesh_mode_changed(); // Initialize sensitivity

	refresh_tool_options();
	App::dialog_tool_options->present();

	// clear out the ducks
	get_work_area()->clear_ducks();
	get_work_area()->queue_draw();

	// Disallow layer clicks so the user doesn't accidentally select the underlying image
	get_work_area()->set_allow_layer_clicks(false);

	get_work_area()->set_cursor(Gdk::CROSSHAIR);

	App::dock_toolbox->refresh();

	layer_param_changed_connection = get_canvas_interface()->signal_layer_param_changed().connect(
		sigc::mem_fun(*this, &StateFFD_Context::on_layer_param_changed));
}

void
StateFFD_Context::refresh_tool_options()
{
	App::dialog_tool_options->clear();
	App::dialog_tool_options->set_widget(options_table);
	App::dialog_tool_options->set_local_name(_("FFD Tool"));
	App::dialog_tool_options->set_icon("tool_ffd_icon");
}

void
StateFFD_Context::on_layer_param_changed(synfig::Layer::Handle layer, synfig::String param_name)
{
	synfig::Layer::Handle ffd = get_selected_ffd_layer();
	if (ffd && layer == ffd) {
		if (param_name == "grid_size_x" || param_name == "grid_size_y" || param_name == "smoothness" || param_name == "mesh_mode" || param_name == "cull_threshold") {
			update_controls_from_layer();
		}
	}
}

void
StateFFD_Context::on_mesh_mode_changed()
{
	bool is_grid = (mesh_mode_enum.get_value() == 0);
	if (is_grid) {
		create_grid_x_label.show();
		create_grid_x_spin.show();
		create_grid_y_label.show();
		create_grid_y_spin.show();
		cull_threshold_label.hide();
		cull_threshold_spin.hide();
	} else {
		create_grid_x_label.hide();
		create_grid_x_spin.hide();
		create_grid_y_label.hide();
		create_grid_y_spin.hide();
		cull_threshold_label.show();
		cull_threshold_spin.show();
	}
	refresh_ducks();
}

StateFFD_Context::~StateFFD_Context()
{
	layer_param_changed_connection.disconnect();

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
		synfig::Real ct = ffd->get_param("cull_threshold").get(synfig::Real());

		grid_x_adj->set_value(gx);
		grid_y_adj->set_value(gy);
		smoothness_hscl.set_value(sm);
		cull_threshold_adj->set_value(ct);

		updating_from_layer_ = false;

		status_label.set_label(editing_existing_mesh_ ? _("Updating FFD Mesh") : _("FFD layer selected"));
		
		int mesh_mode = ffd->get_param("mesh_mode").get(int());

		if (editing_existing_mesh_) {
			grid_x_label.hide();
			grid_x_spin.hide();
			grid_y_label.hide();
			grid_y_spin.hide();
			cull_threshold_label.show();
			cull_threshold_spin.show();
			smoothness_label.show();
			smoothness_hscl.show();
			reset_button.hide();
			edit_mesh_button.hide();

			mesh_mode_label.hide();
			mesh_mode_enum.hide();
			create_grid_x_label.hide();
			create_grid_x_spin.hide();
			create_grid_y_label.hide();
			create_grid_y_spin.hide();
			make_ffd_button.hide();
			
			update_ffd_button.show();
			clear_button.show();
			get_work_area()->set_cursor(Gdk::CROSSHAIR);
		} else {
			if (mesh_mode == 0) { // Grid
				grid_x_label.show();
				grid_x_spin.show();
				grid_y_label.show();
				grid_y_spin.show();
				cull_threshold_label.hide();
				cull_threshold_spin.hide();
				edit_mesh_button.hide();
			} else { // Custom Mesh
				grid_x_label.hide();
				grid_x_spin.hide();
				grid_y_label.hide();
				grid_y_spin.hide();
				cull_threshold_label.show();
				cull_threshold_spin.show();
				edit_mesh_button.show();
			}

			reset_button.show();
			smoothness_label.show();
			smoothness_hscl.show();

			mesh_mode_label.hide();
			mesh_mode_enum.hide();
			create_grid_x_label.hide();
			create_grid_x_spin.hide();
			create_grid_y_label.hide();
			create_grid_y_spin.hide();
			make_ffd_button.hide();
			update_ffd_button.hide();
			clear_button.hide();
			get_work_area()->set_cursor(Gdk::ARROW);
		}
	} else {
		editing_existing_mesh_ = false;
		status_label.set_label(_("Creation Settings"));
		grid_x_label.hide();
		grid_x_spin.hide();
		grid_y_label.hide();
		grid_y_spin.hide();
		smoothness_label.hide();
		smoothness_hscl.hide();
		reset_button.hide();
		edit_mesh_button.hide();
		update_ffd_button.hide();

		mesh_mode_label.show();
		mesh_mode_enum.show();
		on_mesh_mode_changed();
		make_ffd_button.show();
		clear_button.show();
		get_work_area()->set_cursor(Gdk::CROSSHAIR);
	}
}

void
StateFFD_Context::on_grid_x_changed()
{
	if (updating_from_layer_) return;

	synfig::Layer::Handle ffd = get_selected_ffd_layer();
	if (!ffd) return;

	int old_cols = ffd->get_param("grid_size_x").get(int());
	int old_rows = ffd->get_param("grid_size_y").get(int());
	int new_cols = (int)grid_x_adj->get_value();
	if (new_cols == old_cols) return;

	etl::handle<Layer_FreeFormDeform> ffd_typed = etl::handle<Layer_FreeFormDeform>::cast_dynamic(ffd);
	if (!ffd_typed) return;

	synfigapp::Action::PassiveGrouper group(get_canvas_interface()->get_instance().get(), _("Change FFD Grid Size X"));

	std::vector<synfig::Point> new_points = ffd_typed->get_interpolated_grid(new_cols, old_rows);

	std::vector<synfig::ValueBase> grid_points;
	for (auto& p : new_points) {
		grid_points.push_back(p);
	}
	synfig::ValueBase new_grid_points_value(grid_points);

	synfig::ValueNode::Handle dyn_list = synfig::ValueNode_DynamicList::create(new_grid_points_value, get_canvas());

	synfigapp::Action::Handle action_connect = synfigapp::Action::create("ValueDescConnect");
	action_connect->set_param("canvas", get_canvas());
	action_connect->set_param("canvas_interface", get_canvas_interface());
	action_connect->set_param("dest", synfigapp::ValueDesc(ffd, "grid_points"));
	action_connect->set_param("src", dyn_list);

	if(!action_connect->is_ready() || !get_canvas_interface()->get_instance()->perform_action(action_connect)) {
		group.cancel();
		return;
	}

	synfigapp::Action::Handle action_x = synfigapp::Action::create("ValueDescSet");
	action_x->set_param("canvas", get_canvas());
	action_x->set_param("canvas_interface", get_canvas_interface());
	action_x->set_param("value_desc", synfigapp::ValueDesc(ffd, "grid_size_x"));
	action_x->set_param("new_value", synfig::ValueBase(new_cols));
	action_x->set_param("time", get_canvas_interface()->get_time());

	if(!action_x->is_ready() || !get_canvas_interface()->get_instance()->perform_action(action_x)) {
		group.cancel();
		return;
	}

	get_canvas_view()->queue_rebuild_ducks();
	get_work_area()->queue_render();
}

void
StateFFD_Context::on_grid_y_changed()
{
	if (updating_from_layer_) return;

	synfig::Layer::Handle ffd = get_selected_ffd_layer();
	if (!ffd) return;

	int old_cols = ffd->get_param("grid_size_x").get(int());
	int old_rows = ffd->get_param("grid_size_y").get(int());
	int new_rows = (int)grid_y_adj->get_value();
	if (new_rows == old_rows) return;

	etl::handle<Layer_FreeFormDeform> ffd_typed = etl::handle<Layer_FreeFormDeform>::cast_dynamic(ffd);
	if (!ffd_typed) return;

	synfigapp::Action::PassiveGrouper group(get_canvas_interface()->get_instance().get(), _("Change FFD Grid Size Y"));

	std::vector<synfig::Point> new_points = ffd_typed->get_interpolated_grid(old_cols, new_rows);

	std::vector<synfig::ValueBase> grid_points;
	for (auto& p : new_points) {
		grid_points.push_back(p);
	}
	synfig::ValueBase new_grid_points_value(grid_points);

	synfig::ValueNode::Handle dyn_list = synfig::ValueNode_DynamicList::create(new_grid_points_value, get_canvas());

	synfigapp::Action::Handle action_connect = synfigapp::Action::create("ValueDescConnect");
	action_connect->set_param("canvas", get_canvas());
	action_connect->set_param("canvas_interface", get_canvas_interface());
	action_connect->set_param("dest", synfigapp::ValueDesc(ffd, "grid_points"));
	action_connect->set_param("src", dyn_list);

	if(!action_connect->is_ready() || !get_canvas_interface()->get_instance()->perform_action(action_connect)) {
		group.cancel();
		return;
	}

	synfigapp::Action::Handle action_y = synfigapp::Action::create("ValueDescSet");
	action_y->set_param("canvas", get_canvas());
	action_y->set_param("canvas_interface", get_canvas_interface());
	action_y->set_param("value_desc", synfigapp::ValueDesc(ffd, "grid_size_y"));
	action_y->set_param("new_value", synfig::ValueBase(new_rows));
	action_y->set_param("time", get_canvas_interface()->get_time());

	if(!action_y->is_ready() || !get_canvas_interface()->get_instance()->perform_action(action_y)) {
		group.cancel();
		return;
	}

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

void
StateFFD_Context::on_cull_threshold_changed()
{
	if (updating_from_layer_) return;

	// Update preview lines if in creation mode
	if (!polygon_point_list.empty()) {
		refresh_ducks();
	}

	synfig::Layer::Handle ffd = get_selected_ffd_layer();
	if (!ffd) return;

	synfig::Real val = cull_threshold_adj->get_value();
	ffd->set_param("cull_threshold", val);
	get_canvas_interface()->signal_layer_param_changed()(ffd, "cull_threshold");
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
	if (editing_existing_mesh_) {
		synfig::Layer::Handle ffd = get_selected_ffd_layer();
		if (!ffd) {
			editing_existing_mesh_ = false;
			reset();
		}
	}
	update_controls_from_layer();
	get_work_area()->queue_draw();
	get_canvas_view()->queue_rebuild_ducks();
	
	return Smach::RESULT_ACCEPT;
}

Smach::event_result
StateFFD_Context::event_stop_handler(const Smach::event& /*x*/)
{
	reset();
	throw &state_normal;
}

Smach::event_result
StateFFD_Context::event_refresh_handler(const Smach::event& /*x*/)
{
	if (!polygon_point_list.empty()) {
		refresh_ducks();
	} else {
		get_canvas_view()->queue_rebuild_ducks();
	}
	return Smach::RESULT_ACCEPT;
}

Smach::event_result
StateFFD_Context::event_mouse_click_handler(const Smach::event& x)
{
	if (get_selected_ffd_layer() && !editing_existing_mesh_) return Smach::RESULT_OK; // Ignore if editing (normal mode)

	const EventMouse& event(*reinterpret_cast<const EventMouse*>(&x));
	switch(event.button)
	{
	case BUTTON_LEFT:
		polygon_point_list.push_back(get_work_area()->snap_point_to_grid(event.pos));
		redo_point_list.clear();
		refresh_ducks();
		return Smach::RESULT_ACCEPT;

	case BUTTON_RIGHT:
		if (!polygon_point_list.empty()) {
			synfig::Real min_dist = 1e10;
			auto closest_it = polygon_point_list.end();
			for (auto it = polygon_point_list.begin(); it != polygon_point_list.end(); ++it) {
				synfig::Real dist = (*it - event.pos).mag_squared();
				if (dist < min_dist) {
					min_dist = dist;
					closest_it = it;
				}
			}

			// Approx 10 pixels screen distance in canvas coordinates
			synfig::Real pw = get_work_area()->get_pw();
			synfig::Real threshold_sq = (10.0 * pw) * (10.0 * pw);

			if (min_dist < threshold_sq && closest_it != polygon_point_list.end()) {
				redo_point_list.push_back(*closest_it);
				polygon_point_list.erase(closest_it);
				refresh_ducks();
				return Smach::RESULT_ACCEPT;
			}
		}
		return Smach::RESULT_OK;

	default:
		return Smach::RESULT_OK;
	}
}

Smach::event_result
StateFFD_Context::event_mouse_doubleclick_handler(const Smach::event& x)
{
	if (get_selected_ffd_layer()) return Smach::RESULT_OK;

	const EventMouse& event(*reinterpret_cast<const EventMouse*>(&x));
	switch(event.button)
	{
	case BUTTON_LEFT:
		on_make_ffd_pressed();
		return Smach::RESULT_ACCEPT;

	default:
		return Smach::RESULT_OK;
	}
}

Smach::event_result
StateFFD_Context::event_key_press_handler(const Smach::event& x)
{
	if (get_selected_ffd_layer()) return Smach::RESULT_OK;

	const EventKeyboard& event(*reinterpret_cast<const EventKeyboard*>(&x));
	switch(event.keyval)
	{
	case GDK_KEY_BackSpace:
	case GDK_KEY_Delete:
	case GDK_KEY_z:
	case GDK_KEY_Z:
		if ((event.keyval == GDK_KEY_z || event.keyval == GDK_KEY_Z) &&
			!(event.modifier & Gdk::CONTROL_MASK)) {
			return Smach::RESULT_OK;
		}
		if (!polygon_point_list.empty()) {
			redo_point_list.push_back(polygon_point_list.back());
			polygon_point_list.pop_back();
			refresh_ducks();
			return Smach::RESULT_ACCEPT;
		}
		return Smach::RESULT_OK;

	case GDK_KEY_y:
	case GDK_KEY_Y:
		if (!(event.modifier & Gdk::CONTROL_MASK)) {
			return Smach::RESULT_OK;
		}
		if (!redo_point_list.empty()) {
			polygon_point_list.push_back(redo_point_list.back());
			redo_point_list.pop_back();
			refresh_ducks();
			return Smach::RESULT_ACCEPT;
		}
		return Smach::RESULT_OK;

	default:
		return Smach::RESULT_OK;
	}
}

void
StateFFD_Context::reset()
{
	polygon_point_list.clear();
	redo_point_list.clear();
	refresh_ducks();
}

bool
StateFFD_Context::on_polygon_duck_change(const studio::Duck &duck, std::list<synfig::Point>::iterator iter)
{
	*iter = duck.get_point();
	return true;
}

void
StateFFD_Context::on_duck_right_click(std::list<synfig::Point>::iterator iter)
{
	redo_point_list.push_back(*iter);
	polygon_point_list.erase(iter);
	refresh_ducks();
}

void
StateFFD_Context::refresh_ducks()
{
	get_work_area()->clear_ducks();
	get_work_area()->queue_draw();

	if (polygon_point_list.empty()) return;

	std::vector<synfig::Point> pts;
	std::vector<etl::handle<WorkArea::Duck>> duck_list;

	std::list<synfig::Point>::iterator iter = polygon_point_list.begin();

	etl::handle<WorkArea::Duck> duck = new WorkArea::Duck(*iter);
	duck->set_editable(true);
	duck->set_name(strprintf("%p", &*iter));
	duck->set_type(Duck::TYPE_VERTEX);
	duck->signal_edited().connect(
		sigc::bind(sigc::mem_fun(*this, &studio::StateFFD_Context::on_polygon_duck_change), iter)
	);
	duck->signal_user_click(2).connect(
		sigc::bind(sigc::mem_fun(*this, &studio::StateFFD_Context::on_duck_right_click), iter)
	);
	get_work_area()->add_duck(duck);
	duck_list.push_back(duck);
	pts.push_back(*iter);

	for (++iter; iter != polygon_point_list.end(); ++iter)
	{
		duck = new WorkArea::Duck(*iter);
		duck->set_editable(true);
		duck->set_name(strprintf("%p", &*iter));
		duck->set_type(Duck::TYPE_VERTEX);
		duck->signal_edited().connect(
			sigc::bind(sigc::mem_fun(*this, &studio::StateFFD_Context::on_polygon_duck_change), iter)
		);
		duck->signal_user_click(2).connect(
			sigc::bind(sigc::mem_fun(*this, &studio::StateFFD_Context::on_duck_right_click), iter)
		);
		get_work_area()->add_duck(duck);
		duck_list.push_back(duck);
		pts.push_back(*iter);
	}

	// Draw triangulation if Custom Mesh mode
	if (mesh_mode_enum.get_value() == 1 && duck_list.size() > 2) {
		std::vector<rendering::Mesh::Triangle> tris = synfig::Layer_FreeFormDeform::triangulate(pts);

		// Apply Cull Threshold to Preview Mesh
		synfig::Real cull_threshold = cull_threshold_adj->get_value();
		if (cull_threshold > 0.0) {
			synfig::Real cull_sq = cull_threshold * cull_threshold;
			std::vector<rendering::Mesh::Triangle> culled_tris;
			for (const auto& tri : tris) {
				synfig::Point a = pts[tri.vertices[0]];
				synfig::Point b = pts[tri.vertices[1]];
				synfig::Point c = pts[tri.vertices[2]];
				
				synfig::Real D = 2.0 * (a[0]*(b[1]-c[1]) + b[0]*(c[1]-a[1]) + c[0]*(a[1]-b[1]));
				if (std::abs(D) > 1e-6) {
					synfig::Real ux = ((a[0]*a[0] + a[1]*a[1]) * (b[1] - c[1]) + 
							   (b[0]*b[0] + b[1]*b[1]) * (c[1] - a[1]) + 
							   (c[0]*c[0] + c[1]*c[1]) * (a[1] - b[1])) / D;
					synfig::Real uy = ((a[0]*a[0] + a[1]*a[1]) * (c[0] - b[0]) + 
							   (b[0]*b[0] + b[1]*b[1]) * (a[0] - c[0]) + 
							   (c[0]*c[0] + c[1]*c[1]) * (b[0] - a[0])) / D;
					synfig::Point center(ux, uy);
					if ((a - center).mag_squared() > cull_sq) {
						continue; // Cull this preview triangle
					}
				}
				culled_tris.push_back(tri);
			}
			tris = culled_tris;
		}

		for (const auto& tri : tris) {
			etl::handle<WorkArea::Bezier> b1(new WorkArea::Bezier());
			b1->p1 = b1->c1 = duck_list[tri.vertices[0]];
			b1->p2 = b1->c2 = duck_list[tri.vertices[1]];
			get_work_area()->add_bezier(b1);

			etl::handle<WorkArea::Bezier> b2(new WorkArea::Bezier());
			b2->p1 = b2->c1 = duck_list[tri.vertices[1]];
			b2->p2 = b2->c2 = duck_list[tri.vertices[2]];
			get_work_area()->add_bezier(b2);

			etl::handle<WorkArea::Bezier> b3(new WorkArea::Bezier());
			b3->p1 = b3->c1 = duck_list[tri.vertices[2]];
			b3->p2 = b3->c2 = duck_list[tri.vertices[0]];
			get_work_area()->add_bezier(b3);
		}
	}

	get_work_area()->queue_draw();
}

void
StateFFD_Context::on_reset_pressed()
{
	synfig::Layer::Handle ffd = get_selected_ffd_layer();
	if (!ffd) return;

	etl::handle<Layer_FreeFormDeform> ffd_typed = etl::handle<Layer_FreeFormDeform>::cast_dynamic(ffd);
	if (!ffd_typed) return;

	synfigapp::Action::PassiveGrouper group(get_canvas_interface()->get_instance().get(), _("Reset FFD Grid"));

	int mesh_mode = ffd_typed->get_param("mesh_mode").get(int());
	
	if (mesh_mode == 1) {
		// Custom Mesh: Reset grid_points to source_points
		synfig::ValueBase source_points_vb = ffd_typed->get_param("source_points");
		
		synfig::ValueNode::Handle dyn_list = synfig::ValueNode_DynamicList::create(source_points_vb, get_canvas());
		synfigapp::Action::Handle action_connect = synfigapp::Action::create("ValueDescConnect");
		action_connect->set_param("canvas", get_canvas());
		action_connect->set_param("canvas_interface", get_canvas_interface());
		action_connect->set_param("dest", synfigapp::ValueDesc(ffd, "grid_points"));
		action_connect->set_param("src", dyn_list);
		if (!action_connect->is_ready() || !get_canvas_interface()->get_instance()->perform_action(action_connect)) {
			group.cancel();
			return;
		}
	} else {
		// Grid Mode
		synfig::Rect bounds = ffd_typed->get_context_bounds();
		if (!bounds.is_valid() || bounds.area() <= 0.0001) return;

		int cols = ffd_typed->get_param("grid_size_x").get(int());
		int rows = ffd_typed->get_param("grid_size_y").get(int());

		std::vector<synfig::Point> grid_points = ffd_typed->compute_grid_for_bounds(bounds, cols, rows);
		std::vector<synfig::ValueBase> grid_points_vb;
		for (const auto& p : grid_points) grid_points_vb.push_back(synfig::ValueBase(p));

		synfig::ValueNode::Handle dyn_list = synfig::ValueNode_DynamicList::create(synfig::ValueBase(grid_points_vb), get_canvas());
		synfigapp::Action::Handle action_connect = synfigapp::Action::create("ValueDescConnect");
		action_connect->set_param("canvas", get_canvas());
		action_connect->set_param("canvas_interface", get_canvas_interface());
		action_connect->set_param("dest", synfigapp::ValueDesc(ffd, "grid_points"));
		action_connect->set_param("src", dyn_list);
		if (!action_connect->is_ready() || !get_canvas_interface()->get_instance()->perform_action(action_connect)) {
			group.cancel();
			return;
		}

		synfigapp::Action::Handle action_tl = synfigapp::Action::create("ValueDescSet");
		action_tl->set_param("canvas", get_canvas());
		action_tl->set_param("canvas_interface", get_canvas_interface());
		action_tl->set_param("value_desc", synfigapp::ValueDesc(ffd, "source_tl"));
		action_tl->set_param("new_value", synfig::ValueBase(synfig::Point(bounds.minx, bounds.maxy)));
		action_tl->set_param("time", get_canvas_interface()->get_time());
		if (!action_tl->is_ready() || !get_canvas_interface()->get_instance()->perform_action(action_tl)) {
			group.cancel();
			return;
		}

		synfigapp::Action::Handle action_br = synfigapp::Action::create("ValueDescSet");
		action_br->set_param("canvas", get_canvas());
		action_br->set_param("canvas_interface", get_canvas_interface());
		action_br->set_param("value_desc", synfigapp::ValueDesc(ffd, "source_br"));
		action_br->set_param("new_value", synfig::ValueBase(synfig::Point(bounds.maxx, bounds.miny)));
		action_br->set_param("time", get_canvas_interface()->get_time());
		if (!action_br->is_ready() || !get_canvas_interface()->get_instance()->perform_action(action_br)) {
			group.cancel();
			return;
		}
	}

	get_canvas_view()->queue_rebuild_ducks();
	get_work_area()->queue_render();
}

void
StateFFD_Context::on_make_ffd_pressed()
{
	synfigapp::Action::PassiveGrouper group(get_canvas_interface()->get_instance().get(), _("Make FFD Layer"));
	
	int depth = 0;
	synfig::Layer::Handle selected = get_canvas_interface()->get_selection_manager()->get_selected_layer();
	if (selected) depth = selected->get_depth();

	synfig::Layer::Handle layer;
	synfig::Canvas::Handle target_canvas = get_canvas();

	synfig::Vector origin_offset(0,0);
	synfig::Transformation layer_transform; // Automatically defaults to identity

	if (selected && selected->get_name() == "switch") {
		synfig::Layer::Handle switch_layer = selected;
		
		if (switch_layer->get_param("origin").get_type() == synfig::type_vector) {
			origin_offset = switch_layer->get_param("origin").get(synfig::Vector(0,0));
		}
		if (switch_layer->get_param("transformation").get_type() == synfig::type_transformation) {
			layer_transform = switch_layer->get_param("transformation").get(synfig::Transformation());
		}
		
		synfig::Layer::Handle new_group = get_canvas_interface()->add_layer_to("group", target_canvas, depth);
		if (new_group) {
			synfigapp::Action::Handle action(synfigapp::Action::create("LayerSetDesc"));
			action->set_param("canvas", target_canvas);
			action->set_param("canvas_interface", get_canvas_interface());
			action->set_param("layer", new_group);
			action->set_param("new_description", synfig::String("FFD Group"));
			get_canvas_interface()->get_instance()->perform_action(action);

			synfig::Canvas::Handle child_canvas = new_group->get_param("canvas").get(synfig::Canvas::Handle());

			synfigapp::Action::Handle rem_action(synfigapp::Action::create("LayerRemove"));
			rem_action->set_param("canvas", target_canvas);
			rem_action->set_param("canvas_interface", get_canvas_interface());
			rem_action->set_param("layer", switch_layer);
			get_canvas_interface()->get_instance()->perform_action(rem_action);

			synfigapp::Action::Handle add_action(synfigapp::Action::create("LayerAdd"));
			add_action->set_param("canvas", child_canvas);
			add_action->set_param("canvas_interface", get_canvas_interface());
			add_action->set_param("new", switch_layer);
			get_canvas_interface()->get_instance()->perform_action(add_action);

			auto transfer_param = [&](const synfig::String& param_name, const synfig::ValueBase& default_val) {
				synfig::ValueBase val = switch_layer->get_param(param_name);
				auto dyn_param = switch_layer->dynamic_param_list().find(param_name);
				
				if (dyn_param != switch_layer->dynamic_param_list().end()) {
					synfig::ValueNode::Handle vn = dyn_param->second;
					
					synfigapp::Action::Handle param_connect(synfigapp::Action::create("LayerParamConnect"));
					param_connect->set_param("canvas", target_canvas);
					param_connect->set_param("canvas_interface", get_canvas_interface());
					param_connect->set_param("layer", new_group);
					param_connect->set_param("param", param_name);
					param_connect->set_param("value_node", vn);
					get_canvas_interface()->get_instance()->perform_action(param_connect);

					synfigapp::Action::Handle param_disconnect(synfigapp::Action::create("LayerParamDisconnect"));
					param_disconnect->set_param("canvas", child_canvas);
					param_disconnect->set_param("canvas_interface", get_canvas_interface());
					param_disconnect->set_param("layer", switch_layer);
					param_disconnect->set_param("param", param_name);
					get_canvas_interface()->get_instance()->perform_action(param_disconnect);

					synfigapp::Action::Handle param_set(synfigapp::Action::create("LayerParamSet"));
					param_set->set_param("canvas", child_canvas);
					param_set->set_param("canvas_interface", get_canvas_interface());
					param_set->set_param("layer", switch_layer);
					param_set->set_param("param", param_name);
					param_set->set_param("new_value", default_val);
					get_canvas_interface()->get_instance()->perform_action(param_set);
				} else {
					synfigapp::Action::Handle param_set_group(synfigapp::Action::create("LayerParamSet"));
					param_set_group->set_param("canvas", target_canvas);
					param_set_group->set_param("canvas_interface", get_canvas_interface());
					param_set_group->set_param("layer", new_group);
					param_set_group->set_param("param", param_name);
					param_set_group->set_param("new_value", val);
					get_canvas_interface()->get_instance()->perform_action(param_set_group);

					synfigapp::Action::Handle param_set_switch(synfigapp::Action::create("LayerParamSet"));
					param_set_switch->set_param("canvas", child_canvas);
					param_set_switch->set_param("canvas_interface", get_canvas_interface());
					param_set_switch->set_param("layer", switch_layer);
					param_set_switch->set_param("param", param_name);
					param_set_switch->set_param("new_value", default_val);
					get_canvas_interface()->get_instance()->perform_action(param_set_switch);
				}
			};

			transfer_param("origin", synfig::ValueBase(synfig::Vector(0,0)));
			transfer_param("transformation", synfig::ValueBase(synfig::Transformation()));

			layer = get_canvas_interface()->add_layer_to("free_form_deform", child_canvas, 0);
		} else {
			layer = get_canvas_interface()->add_layer_to("free_form_deform", target_canvas, depth);
		}
	} else {
		layer = get_canvas_interface()->add_layer_to("free_form_deform", target_canvas, depth);
	}

	if (!layer) {
		group.cancel();
		return;
	}

	int mode = mesh_mode_enum.get_value();
	layer->set_param("mesh_mode", synfig::ValueBase(mode));
	layer->set_param("cull_threshold", synfig::ValueBase(cull_threshold_adj->get_value()));

	if (mode == 1) { // Custom Mesh
		if (polygon_point_list.size() < 3) {
			get_canvas_view()->get_ui_interface()->error(_("Need at least 3 points to create a Custom Mesh FFD."));
			group.cancel();
			return;
		}

		std::vector<synfig::ValueBase> pts_vb;
		for (auto& p : polygon_point_list) {
			// 1. Subtract the origin
			synfig::Point local_p = p - origin_offset;
			
			// 2. Apply Inverse Transformation (offset, angle, skew, scale)
			local_p -= layer_transform.offset;
			
			// Un-rotate
			synfig::Real sn = synfig::Angle::sin(-layer_transform.angle).get();
			synfig::Real cs = synfig::Angle::cos(-layer_transform.angle).get();
			synfig::Point unrot;
			unrot[0] = local_p[0] * cs - local_p[1] * sn;
			unrot[1] = local_p[0] * sn + local_p[1] * cs;
			local_p = unrot;
			
			// Un-skew
			local_p[0] -= local_p[1] * synfig::Angle::tan(layer_transform.skew_angle).get();
			
			// Un-scale
			if (layer_transform.scale[0] != 0.0) local_p[0] /= layer_transform.scale[0];
			if (layer_transform.scale[1] != 0.0) local_p[1] /= layer_transform.scale[1];

			pts_vb.push_back(local_p);
		}

		synfig::ValueNode::Handle dyn_list = synfig::ValueNode_DynamicList::create(synfig::ValueBase(pts_vb), get_canvas());
		layer->connect_dynamic_param("grid_points", dyn_list);
		layer->set_param("source_points", synfig::ValueBase(pts_vb));
	} else {
		layer->set_param("grid_size_x", synfig::ValueBase((int)create_grid_x_spin.get_value()));
		layer->set_param("grid_size_y", synfig::ValueBase((int)create_grid_y_spin.get_value()));
	}

	synfigapp::SelectionManager::LayerList layer_selection;
	layer_selection.push_back(layer);
	get_canvas_interface()->get_selection_manager()->set_selected_layers(layer_selection);
	
	reset();
}

void
StateFFD_Context::on_edit_mesh_pressed()
{
	synfig::Layer::Handle ffd = get_selected_ffd_layer();
	if (!ffd) return;

	editing_existing_mesh_ = true;

	// Extract existing source points
	polygon_point_list.clear();
	const synfig::ValueBase& src_vb = ffd->get_param("source_points");
	if (src_vb.get_type() == synfig::type_list) {
		const synfig::ValueBase::List& list = src_vb.get_list();
		for (const auto& item : list) {
			if (item.can_get(synfig::Point())) {
				polygon_point_list.push_back(item.get(synfig::Point()));
			}
		}
	}

	update_controls_from_layer();
	refresh_ducks();
	get_work_area()->queue_draw();
}

void
StateFFD_Context::on_update_ffd_pressed()
{
	synfig::Layer::Handle ffd = get_selected_ffd_layer();
	if (!ffd || !editing_existing_mesh_) return;

	if (polygon_point_list.size() < 3) {
		get_canvas_view()->get_ui_interface()->error(_("Need at least 3 points to update Custom Mesh FFD."));
		return;
	}

	synfigapp::Action::PassiveGrouper group(get_canvas_interface()->get_instance().get(), _("Update FFD Mesh"));

	std::vector<synfig::ValueBase> pts_vb;
	for (auto& p : polygon_point_list) {
		// Note: We assume points are in the same local space they were extracted from.
		pts_vb.push_back(p);
	}

	synfig::ValueNode::Handle dyn_list = synfig::ValueNode_DynamicList::create(synfig::ValueBase(pts_vb), get_canvas());

	synfigapp::Action::Handle action_connect = synfigapp::Action::create("ValueDescConnect");
	action_connect->set_param("canvas", get_canvas());
	action_connect->set_param("canvas_interface", get_canvas_interface());
	action_connect->set_param("dest", synfigapp::ValueDesc(ffd, "grid_points"));
	action_connect->set_param("src", dyn_list);

	if(!action_connect->is_ready() || !get_canvas_interface()->get_instance()->perform_action(action_connect)) {
		group.cancel();
		return;
	}

	synfigapp::Action::Handle action_src = synfigapp::Action::create("LayerParamSet");
	action_src->set_param("canvas", get_canvas());
	action_src->set_param("canvas_interface", get_canvas_interface());
	action_src->set_param("layer", ffd);
	action_src->set_param("param", synfig::String("source_points"));
	action_src->set_param("new_value", synfig::ValueBase(pts_vb));

	if(!action_src->is_ready() || !get_canvas_interface()->get_instance()->perform_action(action_src)) {
		group.cancel();
		return;
	}

	editing_existing_mesh_ = false;
	reset(); // clears polygon_point_list

	update_controls_from_layer();
	get_canvas_view()->queue_rebuild_ducks();
	get_work_area()->queue_render();
}
