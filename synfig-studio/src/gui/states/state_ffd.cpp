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
#include <gui/event_layerclick.h>
#include <gdk/gdkkeysyms.h>
#include <gui/localization.h>
#include <gui/workarea.h>

#include <synfig/general.h>
#include <synfig/layers/layer_freeformdeform.h>
#include <synfig/layers/layer_bitmap.h>
#include <synfig/layers/layer_pastecanvas.h>

#include <synfigapp/blineconvert.h>
#include <synfigapp/main.h>
#include <synfig/transformation.h>
#include <synfigapp/instance.h>
#include <synfig/valuenodes/valuenode_dynamiclist.h>
#include <synfig/valuenodes/valuenode_const.h>
#include <synfig/surface.h>
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

#include "devicetracker.h"

static bool group_has_image(const synfig::Layer::Handle& layer);
static bool is_valid_group_for_ffd(const synfig::Layer::Handle& layer);

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
	Gtk::Scale cull_threshold_hscl;

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

	// Auto-mesh (Moho-style edge contour tracing) widgets
	Gtk::Button regenerate_button;
	Gtk::Label mesh_margin_label;
	Glib::RefPtr<Gtk::Adjustment> mesh_margin_adj;
	Gtk::Scale mesh_margin_hscl;
	Gtk::Label mesh_edge_length_label;
	Glib::RefPtr<Gtk::Adjustment> mesh_edge_length_adj;
	Gtk::Scale mesh_edge_length_hscl;

	// Cached surface for live auto-mesh re-trace during creation
	synfig::Surface auto_mesh_surface_;
	synfig::Rect auto_mesh_bounds_;
	synfig::Transformation auto_mesh_transform_;
	synfig::Vector auto_mesh_origin_;
	bool auto_mesh_cached_;

	std::list<synfig::Point> polygon_point_list;
	std::list<synfig::Point> redo_point_list;
	synfig::Point preview_tl;
	synfig::Point preview_br;
	synfig::Angle preview_angle;
	bool editing_existing_mesh_;
	std::vector<etl::handle<WorkArea::Duck> > duck_list_;

	void on_grid_x_changed();
	void on_grid_y_changed();
	void on_smoothness_changed();
	void on_cull_threshold_changed();
	void on_reset_pressed();
	void on_auto_mesh_slider_changed();
	void cache_auto_mesh_surface();
	void retrace_auto_mesh();
	void update_grid_preview();
	void update_creation_controls_visibility();

	void on_make_ffd_pressed();
	void on_edit_mesh_pressed();
	void on_update_ffd_pressed();
	void on_regenerate_pressed();
	void reset();
	bool on_polygon_duck_change(const studio::Duck &duck, std::list<synfig::Point>::iterator iter);
	void on_duck_right_click(std::list<synfig::Point>::iterator iter);
	void refresh_ducks();
	void refresh_beziers();

	synfig::Layer::Handle get_selected_ffd_layer() const;
	void update_controls_from_layer();
	void on_mesh_mode_changed();

public:

	Smach::event_result event_refresh_tool_options(const Smach::event& x);
	Smach::event_result event_layer_selection_changed_handler(const Smach::event& x);
	Smach::event_result event_layer_click_handler(const Smach::event& x);
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
	insert(event_def(EVENT_WORKAREA_LAYER_CLICKED,  &StateFFD_Context::event_layer_click_handler));
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
	cull_threshold_adj(Gtk::Adjustment::create(0.0, 0.0, 20.0, 0.01, 1.0)),
	cull_threshold_hscl(cull_threshold_adj, Gtk::ORIENTATION_HORIZONTAL),
	reset_button(_("Reset Grid")),
	updating_from_layer_(false),
	auto_mesh_cached_(false)
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
	cull_threshold_hscl.set_digits(2);
	cull_threshold_hscl.set_value_pos(Gtk::POS_LEFT);
	cull_threshold_hscl.set_tooltip_text(_("Removes triangles larger than this threshold (0 = disable)"));
	cull_threshold_hscl.set_hexpand(true);

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

	create_grid_x_label.set_label(_("Grid Size X:"));
	create_grid_x_label.set_halign(Gtk::ALIGN_START);
	create_grid_x_spin.set_adjustment(grid_x_adj); // reuse adjustment
	create_grid_x_spin.set_hexpand(true);

	create_grid_y_label.set_label(_("Grid Size Y:"));
	create_grid_y_label.set_halign(Gtk::ALIGN_START);
	create_grid_y_spin.set_adjustment(grid_y_adj);

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

	// Auto-mesh sliders
	regenerate_button.set_label(_("Regenerate Edge Points"));
	regenerate_button.set_hexpand(true);
	regenerate_button.set_halign(Gtk::ALIGN_FILL);
	regenerate_button.set_tooltip_text(_("Re-trace image contour with current settings"));

	mesh_margin_label.set_label(_("Mesh Margin (px):"));
	mesh_margin_label.set_halign(Gtk::ALIGN_START);
	mesh_margin_adj = Gtk::Adjustment::create(5, 0, 100, 1, 5);
	mesh_margin_hscl.set_adjustment(mesh_margin_adj);
	mesh_margin_hscl.set_digits(0);
	mesh_margin_hscl.set_value_pos(Gtk::POS_LEFT);
	mesh_margin_hscl.set_hexpand(true);

	mesh_edge_length_label.set_label(_("Edge Length (units):"));
	mesh_edge_length_label.set_halign(Gtk::ALIGN_START);
	mesh_edge_length_adj = Gtk::Adjustment::create(0.5, 0.05, 100.0, 0.05, 1.0);
	mesh_edge_length_hscl.set_adjustment(mesh_edge_length_adj);
	mesh_edge_length_hscl.set_digits(2);
	mesh_edge_length_hscl.set_value_pos(Gtk::POS_LEFT);
	mesh_edge_length_hscl.set_hexpand(true);

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
	options_table.attach(cull_threshold_hscl,  1, 5, 1, 1);
	options_table.attach(reset_button,      0, 6, 2, 1);

	// Creation UI
	options_table.attach(mesh_mode_label,        0, 7, 1, 1);
	options_table.attach(mesh_mode_enum,         1, 7, 1, 1);
	options_table.attach(create_grid_x_label,    0, 8, 1, 1);
	options_table.attach(create_grid_x_spin,     1, 8, 1, 1);
	options_table.attach(create_grid_y_label,    0, 9, 1, 1);
	options_table.attach(create_grid_y_spin,     1, 9, 1, 1);
	options_table.attach(mesh_margin_label,      0, 10, 1, 1);
	options_table.attach(mesh_margin_hscl,       1, 10, 1, 1);
	options_table.attach(mesh_edge_length_label, 0, 11, 1, 1);
	options_table.attach(mesh_edge_length_hscl,  1, 11, 1, 1);
	options_table.attach(make_ffd_button,        0, 12, 2, 1);
	options_table.attach(clear_button,           0, 13, 2, 1);
	options_table.attach(edit_mesh_button,       0, 14, 2, 1);
	options_table.attach(update_ffd_button,      0, 15, 2, 1);
	options_table.attach(regenerate_button,      0, 16, 2, 1);

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
	cull_threshold_hscl.signal_value_changed().connect(
		sigc::mem_fun(*this, &StateFFD_Context::on_cull_threshold_changed));
	reset_button.signal_clicked().connect(
		sigc::mem_fun(*this, &StateFFD_Context::on_reset_pressed));
	make_ffd_button.signal_clicked().connect(
		sigc::mem_fun(*this, &StateFFD_Context::on_make_ffd_pressed));
	edit_mesh_button.signal_clicked().connect(
		sigc::mem_fun(*this, &StateFFD_Context::on_edit_mesh_pressed));
	update_ffd_button.signal_clicked().connect(
		sigc::mem_fun(*this, &StateFFD_Context::on_update_ffd_pressed));
	regenerate_button.signal_clicked().connect(
		sigc::mem_fun(*this, &StateFFD_Context::on_regenerate_pressed));
	clear_button.signal_clicked().connect(
		sigc::mem_fun(*this, &StateFFD_Context::reset));
	mesh_mode_enum.signal_changed().connect(
		sigc::mem_fun(*this, &StateFFD_Context::on_mesh_mode_changed));

	mesh_margin_hscl.signal_value_changed().connect(
		sigc::mem_fun(*this, &StateFFD_Context::on_auto_mesh_slider_changed));
	mesh_edge_length_hscl.signal_value_changed().connect(
		sigc::mem_fun(*this, &StateFFD_Context::on_auto_mesh_slider_changed));
	create_grid_x_spin.signal_value_changed().connect(
		sigc::mem_fun(*this, &StateFFD_Context::update_grid_preview));
	create_grid_y_spin.signal_value_changed().connect(
		sigc::mem_fun(*this, &StateFFD_Context::update_grid_preview));
	
	update_controls_from_layer();

	refresh_tool_options();
	App::dialog_tool_options->present();

	// clear out the ducks
	get_work_area()->clear_ducks();
	get_work_area()->queue_draw();

	get_work_area()->set_allow_layer_clicks(true);

	get_work_area()->set_cursor(Gdk::CROSSHAIR);

	App::dock_toolbox->refresh();

	layer_param_changed_connection = get_canvas_interface()->signal_layer_param_changed().connect(
		sigc::mem_fun(*this, &StateFFD_Context::on_layer_param_changed));
	
	refresh_ducks();
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
StateFFD_Context::update_creation_controls_visibility()
	{
		bool is_grid = (mesh_mode_enum.get_value() == 0);

		if (is_grid) {
			create_grid_x_label.show();
			create_grid_x_spin.show();
			create_grid_y_label.show();
			create_grid_y_spin.show();
			cull_threshold_label.hide();
			cull_threshold_hscl.hide();
			mesh_margin_label.show();
			mesh_margin_hscl.show();
			mesh_edge_length_label.hide();
			mesh_edge_length_hscl.hide();
		} else {
			create_grid_x_label.hide();
			create_grid_x_spin.hide();
			create_grid_y_label.hide();
			create_grid_y_spin.hide();
			cull_threshold_label.show();
			cull_threshold_hscl.show();
			mesh_margin_label.show();
			mesh_margin_hscl.show();
			mesh_edge_length_label.show();
			mesh_edge_length_hscl.show();
		}
	}

void
StateFFD_Context::on_mesh_mode_changed()
{
	update_creation_controls_visibility();
	if (mesh_mode_enum.get_value() == 1) {
		auto_mesh_cached_ = false;
		retrace_auto_mesh();
	} else {
		update_grid_preview();
	}
}

void
StateFFD_Context::on_auto_mesh_slider_changed()
{
	if (updating_from_layer_) return;

	synfig::Layer::Handle ffd = get_selected_ffd_layer();
	if (ffd) {
		ffd->set_param("auto_mesh_margin", synfig::ValueBase((synfig::Real)mesh_margin_hscl.get_value()));
		ffd->set_param("auto_mesh_edge_length", synfig::ValueBase(mesh_edge_length_hscl.get_value()));
		get_canvas_interface()->signal_layer_param_changed()(ffd, "auto_mesh_margin");
		get_canvas_interface()->signal_layer_param_changed()(ffd, "auto_mesh_edge_length");
		return;
	}

	if (mesh_mode_enum.get_value() == 1) {
		retrace_auto_mesh();
	} else {
		update_grid_preview();
	}
}

void
StateFFD_Context::cache_auto_mesh_surface()
{
	auto_mesh_cached_ = false;

	synfig::Layer::Handle selected;
	auto selection = get_canvas_interface()->get_selection_manager()->get_selected_layers();
	if (!selection.empty()) {
		selected = selection.front();
	}
	if (!is_valid_group_for_ffd(selected)) return;

	synfig::Canvas::Handle sub_canvas = selected->get_param("canvas").get(synfig::Canvas::Handle());
	if (!sub_canvas) return;

	auto_mesh_transform_ = selected->get_param("transformation").get(synfig::Transformation());
	auto_mesh_origin_ = selected->get_param("origin").get(synfig::Vector(0,0));

	auto_mesh_bounds_ = sub_canvas->get_context(synfig::ContextParams()).get_full_bounding_rect();
	if (!auto_mesh_bounds_.is_valid()) return;

	synfig::Vector size(auto_mesh_bounds_.maxx - auto_mesh_bounds_.minx,
	                    auto_mesh_bounds_.maxy - auto_mesh_bounds_.miny);
	auto_mesh_bounds_.minx -= size[0] * 0.1; auto_mesh_bounds_.maxx += size[0] * 0.1;
	auto_mesh_bounds_.miny -= size[1] * 0.1; auto_mesh_bounds_.maxy += size[1] * 0.1;

	int max_res = 512;
	double aspect = (double)std::max(size[0], 1e-6) / std::max(size[1], 1e-6);
	int res_w, res_h;
	if (aspect >= 1.0) { res_w = max_res; res_h = std::max(10, (int)(max_res / aspect)); }
	else { res_h = max_res; res_w = std::max(10, (int)(max_res * aspect)); }

	synfig::RendDesc desc;
	desc.set_tl(synfig::Point(auto_mesh_bounds_.minx, auto_mesh_bounds_.maxy));
	desc.set_br(synfig::Point(auto_mesh_bounds_.maxx, auto_mesh_bounds_.miny));
	desc.set_w(res_w);
	desc.set_h(res_h);
	desc.set_antialias(1);

	auto_mesh_surface_.set_wh(res_w, res_h);
	auto_mesh_surface_.clear();

	if (!sub_canvas->get_context(synfig::ContextParams()).accelerated_render(&auto_mesh_surface_, 0, desc, 0)) return;

	auto_mesh_cached_ = true;
}

void
StateFFD_Context::retrace_auto_mesh()
{
	if (!auto_mesh_cached_) {
		cache_auto_mesh_surface();
		if (!auto_mesh_cached_) return;
	}

	int margin = (int)mesh_margin_hscl.get_value();
	synfig::Real edge_length = mesh_edge_length_hscl.get_value();
	std::vector<synfig::Point> points = Layer_FreeFormDeform::generate_edge_points(
		auto_mesh_surface_, auto_mesh_bounds_, edge_length, margin);

	if (points.size() < 3) return;

	auto to_world = [&](synfig::Point local_p) {
		local_p -= auto_mesh_origin_;
		local_p[0] *= auto_mesh_transform_.scale[0];
		local_p[1] *= auto_mesh_transform_.scale[1];
		local_p[0] += local_p[1] * synfig::Angle::tan(auto_mesh_transform_.skew_angle).get();
		synfig::Real sn = synfig::Angle::sin(auto_mesh_transform_.angle).get();
		synfig::Real cs = synfig::Angle::cos(auto_mesh_transform_.angle).get();
		synfig::Point rot;
		rot[0] = local_p[0] * cs - local_p[1] * sn;
		rot[1] = local_p[0] * sn + local_p[1] * cs;
		local_p = rot;
		local_p += auto_mesh_transform_.offset;
		return local_p;
	};

	polygon_point_list.clear();
	for (auto& p : points)
		polygon_point_list.push_back(to_world(p));

	refresh_ducks();
}

void
StateFFD_Context::update_grid_preview()
{
	synfig::Layer::Handle selected;
	auto selection = get_canvas_interface()->get_selection_manager()->get_selected_layers();
	if (!selection.empty()) {
		selected = selection.front();
	}
	if (!is_valid_group_for_ffd(selected)) {
		polygon_point_list.clear();
		refresh_ducks();
		return;
	}

	synfig::Canvas::Handle switch_canvas = selected->get_param("canvas").get(synfig::Canvas::Handle());
	if (!switch_canvas) {
		polygon_point_list.clear();
		refresh_ducks();
		return;
	}

	synfig::ValueBase tr_val = selected->get_param("transformation");
	synfig::Transformation tr = tr_val.get(synfig::Transformation());
	synfig::ValueBase origin_val = selected->get_param("origin");
	synfig::Vector origin = origin_val.get(synfig::Vector());

	auto to_world = [&](synfig::Point local_p) {
		local_p -= origin;
		local_p[0] *= tr.scale[0];
		local_p[1] *= tr.scale[1];
		local_p[0] += local_p[1] * synfig::Angle::tan(tr.skew_angle).get();
		synfig::Real sn = synfig::Angle::sin(tr.angle).get();
		synfig::Real cs = synfig::Angle::cos(tr.angle).get();
		synfig::Point rot;
		rot[0] = local_p[0] * cs - local_p[1] * sn;
		rot[1] = local_p[0] * sn + local_p[1] * cs;
		local_p = rot;
		local_p += tr.offset;
		return local_p;
	};

	synfig::Rect bounds = switch_canvas->get_context(synfig::ContextParams()).get_full_bounding_rect();
	synfig::Point tl(bounds.get_min()[0], bounds.get_max()[1]);
	synfig::Point br(bounds.get_max()[0], bounds.get_min()[1]);

	synfig::RendDesc desc;
	desc.set_tl(tl);
	desc.set_br(br);
	desc.set_w(128);
	desc.set_h(128);

	synfig::Surface surface;
	surface.set_wh(128, 128);

	if (!switch_canvas->get_context(synfig::ContextParams()).accelerated_render(&surface, 0, desc, 0)) {
		polygon_point_list.clear();
		refresh_ducks();
		return;
	}

	double sum_x = 0, sum_y = 0, sum_w = 0;
	for (int y = 0; y < 128; ++y) {
		double cy_pos = tl[1] - (y / 127.0) * (tl[1] - br[1]);
		for (int x = 0; x < 128; ++x) {
			double a = surface[y][x].get_a();
			if (a > 0.05) {
				double cx_pos = tl[0] + (x / 127.0) * (br[0] - tl[0]);
				sum_x += cx_pos * a;
				sum_y += cy_pos * a;
				sum_w += a;
			}
		}
	}

	if (sum_w <= 0) {
		polygon_point_list.clear();
		refresh_ducks();
		return;
	}

	double cx = sum_x / sum_w;
	double cy = sum_y / sum_w;

	double sum_xx = 0, sum_yy = 0, sum_xy = 0;
	for (int y = 0; y < 128; ++y) {
		double cy_pos = tl[1] - (y / 127.0) * (tl[1] - br[1]);
		for (int x = 0; x < 128; ++x) {
			double a = surface[y][x].get_a();
			if (a > 0.05) {
				double cx_pos = tl[0] + (x / 127.0) * (br[0] - tl[0]);
				double dx = cx_pos - cx;
				double dy = cy_pos - cy;
				sum_xx += dx * dx * a;
				sum_yy += dy * dy * a;
				sum_xy += dx * dy * a;
			}
		}
	}

	sum_xx /= sum_w;
	sum_yy /= sum_w;
	sum_xy /= sum_w;

	double trace = sum_xx + sum_yy;
	double ev_x = 1.0, ev_y = 0.0;
	if (sum_xy != 0.0) {
		double det = sum_xx * sum_yy - sum_xy * sum_xy;
		double L1 = trace / 2.0 + std::sqrt(std::max(0.0, trace * trace / 4.0 - det));
		ev_x = L1 - sum_yy;
		ev_y = sum_xy;
	} else {
		ev_x = (sum_xx > sum_yy) ? 1.0 : 0.0;
		ev_y = (sum_xx > sum_yy) ? 0.0 : 1.0;
	}

	double len = std::sqrt(ev_x*ev_x + ev_y*ev_y);
	if (len > 0) { ev_x /= len; ev_y /= len; }

	double min_u = 1e10, max_u = -1e10;
	double min_v = 1e10, max_v = -1e10;
	for (int y = 0; y < 128; ++y) {
		double cy_pos = tl[1] - (y / 127.0) * (tl[1] - br[1]);
		for (int x = 0; x < 128; ++x) {
			if (surface[y][x].get_a() > 0.05) {
				double cx_pos = tl[0] + (x / 127.0) * (br[0] - tl[0]);
				double dx = cx_pos - cx;
				double cy_dy = cy_pos - cy;
				double u = dx * ev_x + cy_dy * ev_y;
				double v = -dx * ev_y + cy_dy * ev_x;
				min_u = std::min(min_u, u);
				max_u = std::max(max_u, u);
				min_v = std::min(min_v, v);
				max_v = std::max(max_v, v);
			}
		}
	}

	double center_u = (min_u + max_u) * 0.5;
	double center_v = (min_v + max_v) * 0.5;
	double final_cx = cx + center_u * ev_x - center_v * ev_y;
	double final_cy = cy + center_u * ev_y + center_v * ev_x;

	double w_fit = (max_u - min_u) * 1.05;
	double h_fit = (max_v - min_v) * 1.05;

	// Apply margin expansion (convert pixel margin to canvas units)
	double margin_units = (mesh_margin_hscl.get_value() / 100.0) * std::max(w_fit, h_fit);
	w_fit += margin_units * 2.0;
	h_fit += margin_units * 2.0;

	synfig::Point new_tl(final_cx - w_fit * 0.5, final_cy + h_fit * 0.5);
	synfig::Point new_br(final_cx + w_fit * 0.5, final_cy - h_fit * 0.5);

	double angle_rad = std::atan2(ev_y, ev_x);
	preview_tl = new_tl;
	preview_br = new_br;
	preview_angle = synfig::Angle::rad(angle_rad);

	polygon_point_list.clear();
	synfig::Vector v1 = synfig::Vector(new_br[0] - new_tl[0], 0);
	synfig::Vector v2 = synfig::Vector(0, new_br[1] - new_tl[1]);

	synfig::Real sn = synfig::Angle::sin(synfig::Angle::rad(angle_rad)).get();
	synfig::Real cs = synfig::Angle::cos(synfig::Angle::rad(angle_rad)).get();
	auto rotate = [&](synfig::Vector v) {
		return synfig::Vector(v[0] * cs - v[1] * sn, v[0] * sn + v[1] * cs);
	};

	synfig::Point p1 = new_tl;
	synfig::Point p2 = new_tl + v1;
	synfig::Point p3 = new_br;
	synfig::Point p4 = new_tl + v2;

	synfig::Point final_c(final_cx, final_cy);

	polygon_point_list.push_back(to_world(final_c + rotate(p1 - final_c)));
	polygon_point_list.push_back(to_world(final_c + rotate(p2 - final_c)));
	polygon_point_list.push_back(to_world(final_c + rotate(p3 - final_c)));
	polygon_point_list.push_back(to_world(final_c + rotate(p4 - final_c)));

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

static bool
group_has_image(const synfig::Layer::Handle& layer)
{
	using namespace synfig;
	Layer_PasteCanvas::Handle pc = Layer_PasteCanvas::Handle::cast_dynamic(layer);
	if (!pc) return false;
	Canvas::Handle sub = pc->get_sub_canvas();
	if (!sub) return false;
	for (IndependentContext i = sub->get_independent_context(); *i; ++i) {
		if (Layer_Bitmap::Handle::cast_dynamic(*i))
			return true;
		if (Layer_PasteCanvas::Handle::cast_dynamic(*i)) {
			if (group_has_image(*i))
				return true;
		}
	}
	return false;
}

static bool
is_valid_group_for_ffd(const synfig::Layer::Handle& layer)
{
	return layer && (layer->get_name() == "switch" || layer->get_name() == "group") && group_has_image(layer);
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
		synfig::Real am_margin = ffd->get_param("auto_mesh_margin").get(synfig::Real());
		synfig::Real am_edge = ffd->get_param("auto_mesh_edge_length").get(synfig::Real());

		grid_x_adj->set_value(gx);
		grid_y_adj->set_value(gy);
		smoothness_hscl.set_value(sm);
		cull_threshold_adj->set_value(ct);
		mesh_margin_adj->set_value(am_margin);
		mesh_edge_length_adj->set_value(am_edge);

		updating_from_layer_ = false;

		status_label.set_label(editing_existing_mesh_ ? _("Updating FFD Mesh") : _("FFD layer selected"));
		
		int mesh_mode = ffd->get_param("mesh_mode").get(int());

		if (editing_existing_mesh_) {
			grid_x_label.hide();
			grid_x_spin.hide();
			grid_y_label.hide();
			grid_y_spin.hide();
			cull_threshold_label.show();
			cull_threshold_hscl.show();
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
			mesh_margin_label.show();
			mesh_margin_hscl.show();
			mesh_edge_length_label.show();
			mesh_edge_length_hscl.show();

			update_ffd_button.show();
			clear_button.show();
			regenerate_button.show();
			get_work_area()->set_cursor(Gdk::CROSSHAIR);
		} else {
		if (mesh_mode == 0) { // Grid
			grid_x_label.show();
			grid_x_spin.show();
			grid_y_label.show();
			grid_y_spin.show();
			cull_threshold_label.hide();
			cull_threshold_hscl.hide();
			edit_mesh_button.hide();
			regenerate_button.hide();
			mesh_margin_label.show();
			mesh_margin_hscl.show();
			mesh_edge_length_label.hide();
			mesh_edge_length_hscl.hide();
		} else { // Custom Mesh
				grid_x_label.hide();
				grid_x_spin.hide();
				grid_y_label.hide();
				grid_y_spin.hide();
				cull_threshold_label.show();
				cull_threshold_hscl.show();
				mesh_margin_label.show();
				mesh_margin_hscl.show();
				mesh_edge_length_label.show();
				mesh_edge_length_hscl.show();
				edit_mesh_button.show();
				regenerate_button.show();
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

		// Check if the current selection is a group with an image
		synfig::Layer::Handle sel = get_canvas_interface()->get_selection_manager()->get_selected_layer();
		bool valid_target = is_valid_group_for_ffd(sel);

		grid_x_label.hide();
		grid_x_spin.hide();
		grid_y_label.hide();
		grid_y_spin.hide();
		smoothness_label.hide();
		smoothness_hscl.hide();
		reset_button.hide();
		edit_mesh_button.hide();
		update_ffd_button.hide();
		regenerate_button.hide();

		if (valid_target) {
			status_label.set_label(_("Group with image selected"));
			mesh_mode_label.show();
			mesh_mode_enum.show();
			on_mesh_mode_changed();
			make_ffd_button.show();
			clear_button.show();
			regenerate_button.hide();
			get_work_area()->set_cursor(Gdk::CROSSHAIR);
		} else {
			status_label.set_label(_("Select a Switch Group with an image"));
			mesh_mode_label.hide();
			mesh_mode_enum.hide();
			create_grid_x_label.hide();
			create_grid_x_spin.hide();
			create_grid_y_label.hide();
			create_grid_y_spin.hide();
			cull_threshold_label.hide();
			cull_threshold_hscl.hide();
			make_ffd_button.hide();
			clear_button.hide();
			mesh_margin_label.hide();
			mesh_margin_hscl.hide();
			mesh_edge_length_label.hide();
			mesh_edge_length_hscl.hide();
			regenerate_button.hide();
			get_work_area()->set_cursor(Gdk::ARROW);
		}
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
	get_canvas_view()->queue_rebuild_ducks();
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
StateFFD_Context::event_layer_click_handler(const Smach::event& x)
{
	const EventLayerClick& event(*reinterpret_cast<const EventLayerClick*>(&x));

	if(event.layer && (event.layer->get_name() == "bone" || event.layer->get_name() == "skeleton"))
	{
		if (event.button == BUTTON_LEFT && (event.modifier&GDK_CONTROL_MASK))
		{
			std::list<Layer::Handle> layer_list(canvas_view_->get_selection_manager()->get_selected_layers());
			std::set<Layer::Handle> layers(layer_list.begin(),layer_list.end());
			
			if(!layers.count(event.layer))
			{
				layer_list.push_back(event.layer);
				canvas_view_->get_selection_manager()->set_selected_layers(layer_list);
			}
			else
			{
				layers.erase(event.layer);
				layer_list=std::list<Layer::Handle>(layers.begin(),layers.end());
				canvas_view_->get_selection_manager()->set_selected_layers(layer_list);
			}
			return Smach::RESULT_ACCEPT;
		}
	}
	
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
	if (editing_existing_mesh_ || !polygon_point_list.empty()) {
		refresh_ducks();
		return Smach::RESULT_ACCEPT;
	} else {
		return Smach::RESULT_OK;
	}
}

Smach::event_result
StateFFD_Context::event_mouse_click_handler(const Smach::event& x)
{
	bool can_add_points = editing_existing_mesh_;
	if (!can_add_points && mesh_mode_enum.get_value() == 1) {
		synfig::Layer::Handle selected;
		auto selection = get_canvas_interface()->get_selection_manager()->get_selected_layers();
		if (!selection.empty()) {
			selected = selection.front();
		}
		if (is_valid_group_for_ffd(selected)) {
			can_add_points = true;
		}
	}

	if (!can_add_points) return Smach::RESULT_OK;

	const EventMouse& event(*reinterpret_cast<const EventMouse*>(&x));
	switch(event.button)
	{
	case BUTTON_LEFT:
	{
		if (!get_work_area()) return Smach::RESULT_OK;
		synfig::Point p = get_work_area()->snap_point_to_grid(event.pos);
		synfig::TransformStack t_stack = get_work_area()->get_curr_transform_stack();
		p = t_stack.unperform(p);
		
		polygon_point_list.push_back(p);
		redo_point_list.clear();
		refresh_ducks();
		return Smach::RESULT_ACCEPT;
	}

	case BUTTON_RIGHT:
		if (!polygon_point_list.empty()) {
			synfig::TransformStack t_stack;
			if (get_work_area()) t_stack = get_work_area()->get_curr_transform_stack();
			
			synfig::Real min_dist = 1e10;
			auto closest_it = polygon_point_list.end();
			for (auto it = polygon_point_list.begin(); it != polygon_point_list.end(); ++it) {
				synfig::Point world_p = t_stack.perform(*it);
				synfig::Real dist = (world_p - event.pos).mag_squared();
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
	bool can_add_points = editing_existing_mesh_;
	if (!can_add_points && mesh_mode_enum.get_value() == 1) {
		synfig::Layer::Handle selected;
		auto selection = get_canvas_interface()->get_selection_manager()->get_selected_layers();
		if (!selection.empty()) {
			selected = selection.front();
		}
		if (is_valid_group_for_ffd(selected)) {
			can_add_points = true;
		}
	}

	if (!can_add_points) return Smach::RESULT_OK;

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
	refresh_beziers();
	get_work_area()->queue_draw();
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
	if (!get_work_area()) return;

	get_work_area()->clear_ducks();
	get_work_area()->queue_draw();

	if (polygon_point_list.empty()) return;

	synfig::TransformStack transform_stack = get_work_area()->get_curr_transform_stack();

	std::vector<synfig::Point> pts;
	duck_list_.clear();

	std::list<synfig::Point>::iterator iter = polygon_point_list.begin();

	etl::handle<WorkArea::Duck> duck = new WorkArea::Duck(*iter);
	duck->set_editable(true);
	duck->set_name(strprintf("%p", &*iter));
	duck->set_type(Duck::TYPE_VERTEX);
	duck->set_transform_stack(transform_stack);
	duck->signal_edited().connect(
		sigc::bind(sigc::mem_fun(*this, &studio::StateFFD_Context::on_polygon_duck_change), iter)
	);
	duck->signal_user_click(2).connect(
		sigc::bind(sigc::mem_fun(*this, &studio::StateFFD_Context::on_duck_right_click), iter)
	);
	get_work_area()->add_duck(duck);
	duck_list_.push_back(duck);
	pts.push_back(*iter);

	for (++iter; iter != polygon_point_list.end(); ++iter)
	{
		duck = new WorkArea::Duck(*iter);
		duck->set_editable(true);
		duck->set_name(strprintf("%p", &*iter));
		duck->set_type(Duck::TYPE_VERTEX);
		duck->set_transform_stack(transform_stack);
		duck->signal_edited().connect(
			sigc::bind(sigc::mem_fun(*this, &studio::StateFFD_Context::on_polygon_duck_change), iter)
		);
		duck->signal_user_click(2).connect(
			sigc::bind(sigc::mem_fun(*this, &studio::StateFFD_Context::on_duck_right_click), iter)
		);
		get_work_area()->add_duck(duck);
		duck_list_.push_back(duck);
		pts.push_back(*iter);
	}

	refresh_beziers();
	get_work_area()->queue_draw();
}

void
StateFFD_Context::refresh_beziers()
{
	if (!get_work_area()) return;

	get_work_area()->clear_beziers();

	std::vector<synfig::Point> pts;
	for (const auto& d : duck_list_) {
		pts.push_back(d->get_point());
	}

	// Draw triangulation if Custom Mesh mode
	if (mesh_mode_enum.get_value() == 1 && duck_list_.size() > 2) {
		std::vector<rendering::Mesh::Triangle> tris = synfig::Layer_FreeFormDeform::triangulate(pts);

		std::vector<synfig::Point> local_pts;
		if (editing_existing_mesh_) {
			local_pts = pts; // Already local
		} else {
			synfig::Vector origin_offset(0,0);
			synfig::Transformation layer_transform;
			synfig::Layer::Handle target_layer;
			if (!get_canvas_view()->get_selection_manager()->get_selected_layers().empty()) {
				target_layer = *get_canvas_view()->get_selection_manager()->get_selected_layers().begin();
			}
			if (target_layer) {
				if (target_layer->get_param("origin").get_type() == synfig::type_vector)
					origin_offset = target_layer->get_param("origin").get(synfig::Vector(0,0));
				if (target_layer->get_param("transformation").get_type() == synfig::type_transformation)
					layer_transform = target_layer->get_param("transformation").get(synfig::Transformation());
			}

			for (auto& p : pts) {
				synfig::Point local_p = p - layer_transform.offset;
				synfig::Real sn = synfig::Angle::sin(-layer_transform.angle).get();
				synfig::Real cs = synfig::Angle::cos(-layer_transform.angle).get();
				synfig::Point unrot;
				unrot[0] = local_p[0] * cs - local_p[1] * sn;
				unrot[1] = local_p[0] * sn + local_p[1] * cs;
				local_p = unrot;
				local_p[0] -= local_p[1] * synfig::Angle::tan(layer_transform.skew_angle).get();
				if (layer_transform.scale[0] != 0.0) local_p[0] /= layer_transform.scale[0];
				if (layer_transform.scale[1] != 0.0) local_p[1] /= layer_transform.scale[1];
				local_p += origin_offset;
				local_pts.push_back(local_p);
			}
		}

		// Apply Cull Threshold to Preview Mesh using local points (matching actual FFD layer logic)
		tris = synfig::Layer_FreeFormDeform::cull_triangles(tris, local_pts, cull_threshold_adj->get_value());

		for (const auto& tri : tris) {
			etl::handle<WorkArea::Bezier> b1(new WorkArea::Bezier());
			b1->p1 = b1->c1 = duck_list_[tri.vertices[0]];
			b1->p2 = b1->c2 = duck_list_[tri.vertices[1]];
			get_work_area()->add_bezier(b1);

			etl::handle<WorkArea::Bezier> b2(new WorkArea::Bezier());
			b2->p1 = b2->c1 = duck_list_[tri.vertices[1]];
			b2->p2 = b2->c2 = duck_list_[tri.vertices[2]];
			get_work_area()->add_bezier(b2);

			etl::handle<WorkArea::Bezier> b3(new WorkArea::Bezier());
			b3->p1 = b3->c1 = duck_list_[tri.vertices[2]];
			b3->p2 = b3->c2 = duck_list_[tri.vertices[0]];
			get_work_area()->add_bezier(b3);
		}
	} else if (mesh_mode_enum.get_value() == 0 && duck_list_.size() == 4 && !editing_existing_mesh_) {
		int pts_x = (int)create_grid_x_spin.get_value();
		int pts_y = (int)create_grid_y_spin.get_value();
		if (pts_x < 2) pts_x = 2;
		if (pts_y < 2) pts_y = 2;
		
		int cols = pts_x - 1;
		int rows = pts_y - 1;

		synfig::Point tl = pts[0];
		synfig::Point tr = pts[1];
		synfig::Point br = pts[2];
		synfig::Point bl = pts[3];

		synfig::TransformStack transform_stack = get_work_area()->get_curr_transform_stack();

		for (int i = 0; i <= cols; ++i) {
			double u = (double)i / cols;
			synfig::Point p1 = tl + (tr - tl) * u;
			synfig::Point p2 = bl + (br - bl) * u;

			etl::handle<WorkArea::Bezier> b(new WorkArea::Bezier());
			etl::handle<WorkArea::Duck> d1 = new WorkArea::Duck(p1);
			etl::handle<WorkArea::Duck> d2 = new WorkArea::Duck(p2);
			d1->set_transform_stack(transform_stack);
			d2->set_transform_stack(transform_stack);
			b->p1 = b->c1 = d1;
			b->p2 = b->c2 = d2;
			get_work_area()->add_bezier(b);
		}

		for (int j = 0; j <= rows; ++j) {
			double v = (double)j / rows;
			synfig::Point p1 = tl + (bl - tl) * v;
			synfig::Point p2 = tr + (br - tr) * v;

			etl::handle<WorkArea::Bezier> b(new WorkArea::Bezier());
			etl::handle<WorkArea::Duck> d1 = new WorkArea::Duck(p1);
			etl::handle<WorkArea::Duck> d2 = new WorkArea::Duck(p2);
			d1->set_transform_stack(transform_stack);
			d2->set_transform_stack(transform_stack);
			b->p1 = b->c1 = d1;
			b->p2 = b->c2 = d2;
			get_work_area()->add_bezier(b);
		}
	}
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
	synfigapp::Action::PassiveGrouper group(get_canvas_interface()->get_instance().get(),_("Make FFD"));

	synfig::Point saved_preview_tl = preview_tl;
	synfig::Point saved_preview_br = preview_br;
	synfig::Angle saved_preview_angle = preview_angle;
	std::vector<synfig::Point> saved_polygon(polygon_point_list.begin(), polygon_point_list.end());

	synfig::Layer::Handle selected;
	if (!get_canvas_view()->get_selection_manager()->get_selected_layers().empty()) {
		selected = *get_canvas_view()->get_selection_manager()->get_selected_layers().begin();
	}
	
	int depth = 0;
	if (selected) depth = selected->get_depth();

	synfig::Layer::Handle layer;
	synfig::Canvas::Handle target_canvas;
	if (selected && selected->get_canvas()) {
		target_canvas = selected->get_canvas();
		if (target_canvas != get_canvas() && !target_canvas->is_inline()) {
			group.cancel();
			get_canvas_view()->get_ui_interface()->error(
				_("Cannot apply FFD to a layer that is inside an imported file. Please double-click the imported file to open it in a new tab and apply FFD there."));
			return;
		}
	} else {
		target_canvas = get_canvas();
	}

	synfig::Vector origin_offset(0,0);
	synfig::Transformation layer_transform; // Automatically defaults to identity

	if (is_valid_group_for_ffd(selected)) {
		synfig::Layer::Handle switch_layer = selected;
		
		synfig::Canvas::Handle sub_canvas = switch_layer->get_param("canvas").get(synfig::Canvas::Handle());
		if (sub_canvas && !sub_canvas->is_inline()) {
			group.cancel();
			get_canvas_view()->get_ui_interface()->error(
				_("Cannot apply FFD directly to an imported file. Please double-click the imported file to open it in a new tab and apply FFD to its internal groups."));
			return;
		}
		
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
			action->set_param("new_description", synfig::String("FFD ") + switch_layer->get_non_empty_description());
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
			group.cancel();
			return;
		}
	} else {
		// Not a valid group with an image — reject and inform the user
		group.cancel();
		get_canvas_view()->get_ui_interface()->error(
			_("FFD requires a Group or Switch Group that contains an image layer."));
		return;
	}

	if (!layer) {
		group.cancel();
		return;
	}

	int mode = mesh_mode_enum.get_value();
	layer->set_param("mesh_mode", synfig::ValueBase(mode));
	layer->set_param("cull_threshold", synfig::ValueBase(cull_threshold_adj->get_value()));
	layer->set_param("auto_mesh_margin", synfig::ValueBase((synfig::Real)mesh_margin_hscl.get_value()));
	layer->set_param("auto_mesh_edge_length", synfig::ValueBase(mesh_edge_length_hscl.get_value()));
	layer->set_param("auto_mesh_dpi", synfig::ValueBase(300));

	if (mode == 1) { // Custom Mesh
		if (saved_polygon.size() < 3) {
			get_canvas_view()->get_ui_interface()->error(_("Need at least 3 points to create a Custom Mesh FFD."));
			group.cancel();
			return;
		}

		std::vector<synfig::ValueBase> pts_vb;
		for (auto& p : saved_polygon) {
			// 1. Subtract offset
			synfig::Point local_p = p - layer_transform.offset;
			
			// 2. Un-rotate
			synfig::Real sn = synfig::Angle::sin(-layer_transform.angle).get();
			synfig::Real cs = synfig::Angle::cos(-layer_transform.angle).get();
			synfig::Point unrot;
			unrot[0] = local_p[0] * cs - local_p[1] * sn;
			unrot[1] = local_p[0] * sn + local_p[1] * cs;
			local_p = unrot;
			
			// 3. Un-skew
			local_p[0] -= local_p[1] * synfig::Angle::tan(layer_transform.skew_angle).get();
			
			// 4. Un-scale
			if (layer_transform.scale[0] != 0.0) local_p[0] /= layer_transform.scale[0];
			if (layer_transform.scale[1] != 0.0) local_p[1] /= layer_transform.scale[1];

			// 5. Add origin back
			local_p += origin_offset;

			pts_vb.push_back(local_p);
		}

		synfig::ValueNode::Handle dyn_list = synfig::ValueNode_DynamicList::create(synfig::ValueBase(pts_vb), get_canvas());
		layer->connect_dynamic_param("grid_points", dyn_list);
		layer->set_param("source_points", synfig::ValueBase(pts_vb));
	} else {
		int pts_x = (int)create_grid_x_spin.get_value();
		int pts_y = (int)create_grid_y_spin.get_value();
		if (pts_x < 2) pts_x = 2;
		if (pts_y < 2) pts_y = 2;
		layer->set_param("grid_size_x", synfig::ValueBase(pts_x));
		layer->set_param("grid_size_y", synfig::ValueBase(pts_y));

		if (saved_preview_tl != synfig::Point(0,0) || saved_preview_br != synfig::Point(0,0)) {
			layer->set_param("source_tl", synfig::ValueBase(saved_preview_tl));
			layer->set_param("source_br", synfig::ValueBase(saved_preview_br));
			layer->set_param("source_angle", synfig::ValueBase(saved_preview_angle));
		}

		etl::handle<Layer_FreeFormDeform> ffd_typed = etl::handle<Layer_FreeFormDeform>::cast_dynamic(layer);
		if (ffd_typed) ffd_typed->regenerate_grid_points();

		synfig::ValueBase grid_points_vb = layer->get_param("grid_points");
		layer->set_param("grid_points", grid_points_vb);
		
		synfig::ValueNode::Handle dyn_list = synfig::ValueNode_DynamicList::create(grid_points_vb, get_canvas());
		layer->connect_dynamic_param("grid_points", dyn_list);
	}

	auto canvas_interface = get_canvas_interface();
	Glib::signal_idle().connect_once([canvas_interface, layer]() {
		if (canvas_interface && canvas_interface->get_selection_manager()) {
			canvas_interface->get_selection_manager()->clear_selected_layers();
			synfigapp::SelectionManager::LayerList layer_selection;
			layer_selection.push_back(layer);
			canvas_interface->get_selection_manager()->set_selected_layers(layer_selection);
		}
	});
	
	reset();
	
	// Switch back to normal tool to allow user to immediately interact with grid/mesh
	get_canvas_view()->get_smach().process_event(EVENT_STOP);
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

void
StateFFD_Context::on_regenerate_pressed()
{
	synfig::Layer::Handle ffd_layer = get_selected_ffd_layer();
	etl::handle<Layer_FreeFormDeform> ffd = etl::handle<Layer_FreeFormDeform>::cast_dynamic(ffd_layer);
	if (!ffd || !ffd_layer) return;

	int margin = (int)mesh_margin_hscl.get_value();
	synfig::Real edge_length = mesh_edge_length_hscl.get_value();
	int dpi = ffd->get_param("auto_mesh_dpi").get(int());
	int max_res = std::max(64, std::min(dpi * 2, 1024));

	synfig::Surface surface;
	synfig::Rect bounds;
	if (!ffd->render_context_below(surface, bounds, max_res)) {
		get_canvas_view()->get_ui_interface()->error(_("Could not render image below FFD layer for regeneration."));
		return;
	}

	std::vector<synfig::Point> points = Layer_FreeFormDeform::generate_edge_points(surface, bounds, edge_length, margin);

	if (points.size() < 3) {
		get_canvas_view()->get_ui_interface()->error(_("Auto Mesh could not generate enough points. Try adjusting margin or edge length."));
		return;
	}

	synfigapp::Action::PassiveGrouper group(get_canvas_interface()->get_instance().get(), _("Regenerate Edge Points"));

	std::vector<synfig::ValueBase> pts_vb;
	for (auto& p : points) pts_vb.push_back(p);

	synfig::ValueNode::Handle dyn_list = synfig::ValueNode_DynamicList::create(synfig::ValueBase(pts_vb), ffd->get_canvas());

	synfigapp::Action::Handle action_connect = synfigapp::Action::create("ValueDescConnect");
	action_connect->set_param("canvas", ffd->get_canvas());
	action_connect->set_param("canvas_interface", get_canvas_interface());
	action_connect->set_param("dest", synfigapp::ValueDesc(ffd_layer, "grid_points"));
	action_connect->set_param("src", dyn_list);
	if (!action_connect->is_ready() || !get_canvas_interface()->get_instance()->perform_action(action_connect)) {
		group.cancel();
		return;
	}

	synfigapp::Action::Handle action_src = synfigapp::Action::create("LayerParamSet");
	action_src->set_param("canvas", ffd->get_canvas());
	action_src->set_param("canvas_interface", get_canvas_interface());
	action_src->set_param("layer", ffd_layer);
	action_src->set_param("param", synfig::String("source_points"));
	action_src->set_param("new_value", synfig::ValueBase(pts_vb));
	if (!action_src->is_ready() || !get_canvas_interface()->get_instance()->perform_action(action_src)) {
		group.cancel();
		return;
	}

	synfigapp::Action::Handle action_margin = synfigapp::Action::create("LayerParamSet");
	action_margin->set_param("canvas", ffd->get_canvas());
	action_margin->set_param("canvas_interface", get_canvas_interface());
	action_margin->set_param("layer", ffd_layer);
	action_margin->set_param("param", synfig::String("auto_mesh_margin"));
	action_margin->set_param("new_value", synfig::ValueBase((synfig::Real)margin));
	get_canvas_interface()->get_instance()->perform_action(action_margin);

	synfigapp::Action::Handle action_edge = synfigapp::Action::create("LayerParamSet");
	action_edge->set_param("canvas", ffd->get_canvas());
	action_edge->set_param("canvas_interface", get_canvas_interface());
	action_edge->set_param("layer", ffd_layer);
	action_edge->set_param("param", synfig::String("auto_mesh_edge_length"));
	action_edge->set_param("new_value", synfig::ValueBase(edge_length));
	get_canvas_interface()->get_instance()->perform_action(action_edge);

	if (editing_existing_mesh_) {
		polygon_point_list.clear();
		for (auto& p : points) polygon_point_list.push_back(p);
		refresh_ducks();
	}

	get_canvas_view()->queue_rebuild_ducks();
	get_work_area()->queue_render();
}
