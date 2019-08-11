/* === S Y N F I G ========================================================= */
/*!	\file state_bline.cpp
**	\brief Template File
**
**	$Id$
**
**	\legal
**	Copyright (c) 2002-2005 Robert B. Quattlebaum Jr., Adrian Bentley
**	Copyright (c) 2007, 2008 Chris Moore
**  Copyright (c) 2010, 2011 Carlos López
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

#include <synfig/general.h>

#include <synfig/valuenodes/valuenode_dynamiclist.h>

#include "state_bline.h"
#include "state_normal.h"
#include "canvasview.h"
#include "workarea.h"
#include "app.h"
#include <synfig/valuenodes/valuenode_bline.h>
#include <ETL/hermite>
#include <ETL/calculus>
#include <utility>
#include "event_mouse.h"
#include "event_layerclick.h"
#include "docks/dock_toolbox.h"
#include "docks/dialog_tooloptions.h"
#include "widgets/widget_enum.h"
#include "widgets/widget_distance.h"
#include <synfig/transform.h>
#include <synfigapp/main.h>

#include <gui/localization.h>

#include <gtkmm/separatormenuitem.h>
#include <gtkmm/imagemenuitem.h>
#endif

/* === U S I N G =========================================================== */

using namespace std;
using namespace etl;
using namespace synfig;
using namespace studio;

/* === M A C R O S ========================================================= */

// if defined, show the first duck as green while drawing
#define DISTINGUISH_FIRST_DUCK

#ifndef LAYER_CREATION
#define LAYER_CREATION(button, stockid, tooltip)	\
	{ \
		Gtk::Image *icon = manage(new Gtk::Image(Gtk::StockID(stockid), \
			Gtk::ICON_SIZE_SMALL_TOOLBAR)); \
		button.add(*icon); \
	} \
	button.set_relief(Gtk::RELIEF_NONE); \
	button.set_tooltip_text(tooltip) ;\
	button.signal_toggled().connect(sigc::mem_fun(*this, \
		&studio::StateBLine_Context::toggle_layer_creation))
#endif

// indentation for options layout
#ifndef SPACING
#define SPACING(name, px) \
	Gtk::Alignment *name = Gtk::manage(new Gtk::Alignment()); \
	name->set_size_request(px)
#endif

#define GAP	(3)
#define INDENTATION (6)

/* === G L O B A L S ======================================================= */

StateBLine studio::state_bline;

/* === C L A S S E S & S T R U C T S ======================================= */

class studio::StateBLine_Context : public sigc::trackable
{
	etl::handle<CanvasView> canvas_view_;
	CanvasView::IsWorking is_working;

	bool prev_table_status;
	bool loop_;
	bool prev_workarea_layer_status_;

	int depth;
	Canvas::Handle canvas;

	Gtk::Menu menu;

	Duckmatic::Push duckmatic_push;

	etl::handle<Duck> curr_duck;

	etl::handle<Duck> next_duck;

	std::list<synfig::ValueNode_Const::Handle> bline_point_list;

	bool on_vertex_change(const studio::Duck &duck, synfig::ValueNode_Const::Handle value_node);
	bool on_tangent1_change(const studio::Duck &duck, handle<WorkArea::Duck> other_duck, synfig::ValueNode_Const::Handle value_node);
	bool on_tangent2_change(const studio::Duck &duck, handle<WorkArea::Duck> other_duck, synfig::ValueNode_Const::Handle value_node);

	void popup_handle_menu(synfig::ValueNode_Const::Handle value_node);
	void popup_vertex_menu(synfig::ValueNode_Const::Handle value_node);
	void popup_bezier_menu(float location, synfig::ValueNode_Const::Handle value_node);

	void bline_set_split_handle(synfig::ValueNode_Const::Handle value_node, bool merge_radius, bool merge_angle);
	void bline_delete_vertex(synfig::ValueNode_Const::Handle value_node);
	void bline_insert_vertex(synfig::ValueNode_Const::Handle value_node,float origin=0.5);
	void loop_bline();
	void unloop_bline();

	void refresh_ducks(bool x=true);

	//Toolbox settings
	synfigapp::Settings& settings;

	// holder of options
	Gtk::Table options_table;

	// title
	Gtk::Label title_label;

	// layer name:
	Gtk::Label id_label;
	Gtk::HBox id_box;
	Gtk::Entry id_entry;

	// layer types to create:
	Gtk::Label layer_types_label;
	Gtk::ToggleButton layer_region_togglebutton;
	Gtk::ToggleButton layer_outline_togglebutton;
	Gtk::ToggleButton layer_advanced_outline_togglebutton;
	Gtk::ToggleButton layer_curve_gradient_togglebutton;
	Gtk::ToggleButton layer_plant_togglebutton;
	Gtk::HBox layer_types_box;

	// blend method
	Gtk::Label blend_label;
	Gtk::HBox blend_box;
	Widget_Enum blend_enum;

	// opacity
	Gtk::Label opacity_label;
	Gtk::HScale opacity_hscl;

	// brush size
	Gtk::Label bline_width_label;
	Widget_Distance bline_width_dist;

	// feather size
	Gtk::Label feather_label;
	Widget_Distance feather_dist;

	// link origins
	Gtk::Label link_origins_label;
	Gtk::CheckButton layer_link_origins_checkbutton;
	Gtk::HBox link_origins_box;

	// auto export
	Gtk::Label auto_export_label;
	Gtk::CheckButton auto_export_checkbutton;
	Gtk::HBox auto_export_box;

public:

	int layers_to_create()const
	{
		return
			get_layer_region_flag() +
			get_layer_outline_flag() +
			get_layer_advanced_outline_flag()+
			get_layer_curve_gradient_flag() +
			get_layer_plant_flag();
	}

	void sanity_check()
	{
		if(layers_to_create()==0)
			set_layer_region_flag(true);
	}

	synfig::String get_id()const { return id_entry.get_text(); }
	void set_id(const synfig::String& x) { return id_entry.set_text(x); }

		int get_blend()const { return blend_enum.get_value(); }
	void set_blend(int x) { return blend_enum.set_value(x); }

	Real get_opacity()const { return opacity_hscl.get_value(); }
	void set_opacity(Real x) { opacity_hscl.set_value(x); }

	Real get_bline_width() const {
		return bline_width_dist.get_value().get(
			Distance::SYSTEM_UNITS,
			get_canvas_view()->get_canvas()->rend_desc()
		);
	}
	void set_bline_width(Distance x) { return bline_width_dist.set_value(x);}

	Real get_feather_size() const {
		return feather_dist.get_value().get(
			Distance::SYSTEM_UNITS,
			get_canvas_view()->get_canvas()->rend_desc()
		);
	}
	void set_feather_size(Distance x) { return feather_dist.set_value(x);}

	bool get_layer_region_flag()const { return layer_region_togglebutton.get_active(); }
	void set_layer_region_flag(bool x) { return layer_region_togglebutton.set_active(x); }

	bool get_layer_outline_flag()const { return layer_outline_togglebutton.get_active(); }
	void set_layer_outline_flag(bool x) { return layer_outline_togglebutton.set_active(x); }

	bool get_layer_advanced_outline_flag()const { return layer_advanced_outline_togglebutton.get_active(); }
	void set_layer_advanced_outline_flag(bool x) { return layer_advanced_outline_togglebutton.set_active(x); }

	bool get_layer_curve_gradient_flag()const { return layer_curve_gradient_togglebutton.get_active(); }
	void set_layer_curve_gradient_flag(bool x) { return layer_curve_gradient_togglebutton.set_active(x); }

	bool get_layer_plant_flag()const { return layer_plant_togglebutton.get_active(); }
	void set_layer_plant_flag(bool x) { return layer_plant_togglebutton.set_active(x); }

	bool get_layer_link_origins_flag()const { return layer_link_origins_checkbutton.get_active(); }
	void set_layer_link_origins_flag(bool x) { return layer_link_origins_checkbutton.set_active(x); }

	bool get_auto_export_flag()const { return auto_export_checkbutton.get_active(); }
	void set_auto_export_flag(bool x) { return auto_export_checkbutton.set_active(x); }

  bool layer_region_flag;
  bool layer_outline_flag;
  bool layer_advanced_outline_flag;
  bool layer_curve_gradient_flag;
  bool layer_plant_flag;

	Smach::event_result event_stop_handler(const Smach::event& x);

	Smach::event_result event_refresh_handler(const Smach::event& x);

	Smach::event_result event_mouse_click_handler(const Smach::event& x);
	Smach::event_result event_mouse_release_handler(const Smach::event& x);
	Smach::event_result event_mouse_motion_handler(const Smach::event& x);
	Smach::event_result event_refresh_tool_options(const Smach::event& x);

	Smach::event_result event_hijack(const Smach::event& /*x*/) { return Smach::RESULT_ACCEPT; }

	void refresh_tool_options();

	StateBLine_Context(CanvasView* canvas_view);

	~StateBLine_Context();

	const etl::handle<CanvasView>& get_canvas_view()const{return canvas_view_;}
	etl::handle<synfigapp::CanvasInterface> get_canvas_interface()const{return canvas_view_->canvas_interface();}
	synfig::Canvas::Handle get_canvas()const{return canvas_view_->get_canvas();}
	WorkArea * get_work_area()const{return canvas_view_->get_work_area();}
	const synfig::TransformStack& get_transform_stack()const { return get_work_area()->get_curr_transform_stack(); }

	void load_settings();
	void save_settings();
	void reset();
	void increment_id();

	bool run_();
	bool run();

	bool egress_on_selection_change;
	Smach::event_result event_layer_selection_changed_handler(const Smach::event& /*x*/)
	{
		if(egress_on_selection_change)
			throw &state_normal;
		return Smach::RESULT_OK;
	}

	void toggle_layer_creation();

};	// END of class StateBLine_Context


/* === M E T H O D S ======================================================= */

StateBLine::StateBLine():
	Smach::state<StateBLine_Context>("bline")
{
	insert(event_def(EVENT_LAYER_SELECTION_CHANGED,		&StateBLine_Context::event_layer_selection_changed_handler));
	insert(event_def(EVENT_STOP,						&StateBLine_Context::event_stop_handler));
	insert(event_def(EVENT_REFRESH,						&StateBLine_Context::event_refresh_handler));
	insert(event_def(EVENT_REFRESH_DUCKS,				&StateBLine_Context::event_hijack));
	insert(event_def(EVENT_WORKAREA_MOUSE_BUTTON_DOWN,	&StateBLine_Context::event_mouse_click_handler));
	insert(event_def(EVENT_WORKAREA_MOUSE_BUTTON_UP,	&StateBLine_Context::event_mouse_release_handler));
	insert(event_def(EVENT_WORKAREA_MOUSE_MOTION,		&StateBLine_Context::event_mouse_motion_handler));
	insert(event_def(EVENT_WORKAREA_MOUSE_BUTTON_DRAG,	&StateBLine_Context::event_mouse_motion_handler));
	insert(event_def(EVENT_REFRESH_TOOL_OPTIONS,		&StateBLine_Context::event_refresh_tool_options));
}

StateBLine::~StateBLine()
{
}

void
StateBLine_Context::load_settings()
{
	try
	{
		synfig::ChangeLocale change_locale(LC_NUMERIC, "C");
		String value;

		if(settings.get_value("bline.id",value))
			set_id(value);
		else
			set_id(_("NewSpline"));

		if(settings.get_value("bline.blend",value) && value != "")
			set_blend(atoi(value.c_str()));
		else
			set_blend(0);//(int)Color::BLEND_COMPOSITE); //0 should be blend composites value

		if(settings.get_value("bline.opacity",value))
			set_opacity(atof(value.c_str()));
		else
			set_opacity(1);

		if(settings.get_value("bline.bline_width",value) && value != "")
			set_bline_width(Distance(atof(value.c_str()), App::distance_system));
		else
			set_bline_width(Distance(1, App::distance_system)); // default width

		if(settings.get_value("bline.layer_region",value) && value=="0")
			set_layer_region_flag(false);
		else
			set_layer_region_flag(true);

		if(settings.get_value("bline.layer_outline",value) && value=="1")
			set_layer_outline_flag(true);
		else
			set_layer_outline_flag(false);

		if(settings.get_value("bline.layer_advanced_outline",value) && value=="0")
			set_layer_advanced_outline_flag(false);
		else
			set_layer_advanced_outline_flag(true);

		if(settings.get_value("bline.layer_curve_gradient",value) && value=="1")
			set_layer_curve_gradient_flag(true);
		else
			set_layer_curve_gradient_flag(false);

		if(settings.get_value("bline.layer_plant",value) && value=="1")
			set_layer_plant_flag(true);
		else
			set_layer_plant_flag(false);

		if(settings.get_value("bline.layer_link_origins",value) && value=="0")
			set_layer_link_origins_flag(false);
		else
			set_layer_link_origins_flag(true);

		if(settings.get_value("bline.auto_export",value) && value=="1")
			set_auto_export_flag(true);
		else
			set_auto_export_flag(false);

		if(settings.get_value("bline.feather",value))
			set_feather_size(Distance(atof(value.c_str()), App::distance_system));
		else
			set_feather_size(Distance(0, App::distance_system));

	  // determine layer flags
	  layer_region_flag = get_layer_region_flag();
	  layer_outline_flag = get_layer_outline_flag();
	  layer_advanced_outline_flag = get_layer_outline_flag();
	  layer_curve_gradient_flag = get_layer_curve_gradient_flag();
	  layer_plant_flag = get_layer_plant_flag();

		sanity_check();
	}
	catch(...)
	{
		synfig::warning("State Spline: Caught exception when attempting to load settings.");
	}
}

void
StateBLine_Context::save_settings()
{
	try
	{
		synfig::ChangeLocale change_locale(LC_NUMERIC, "C");
		sanity_check();
		settings.set_value("bline.id",get_id().c_str());
		settings.set_value("bline.layer_outline",get_layer_outline_flag()?"1":"0");
		settings.set_value("bline.layer_advanced_outline",get_layer_advanced_outline_flag()?"1":"0");
		settings.set_value("bline.layer_region",get_layer_region_flag()?"1":"0");
		settings.set_value("bline.layer_curve_gradient",get_layer_curve_gradient_flag()?"1":"0");
		settings.set_value("bline.layer_plant",get_layer_plant_flag()?"1":"0");
		settings.set_value("bline.layer_link_origins",get_layer_link_origins_flag()?"1":"0");
		settings.set_value("bline.blend",strprintf("%d",get_blend()));
		settings.set_value("bline.opacity",strprintf("%f",(float)get_opacity()));
		settings.set_value("bline.bline_width", bline_width_dist.get_value().get_string());
		settings.set_value("bline.feather", feather_dist.get_value().get_string());
		settings.set_value("bline.auto_export",get_auto_export_flag()?"1":"0");

	}
	catch(...)
	{
		synfig::warning("State Spline : Caught exception when attempting to save settings.");
	}
}

void
StateBLine_Context::reset()
{
	loop_=false;
	bline_point_list.clear();
	refresh_ducks();
}

void
StateBLine_Context::increment_id()
{
	String id(get_id());
	int number=1;
	int digits=0;

	if(id.empty())
		id="NewSpline";

	// If there is a number
	// already at the end of the
	// id, then remove it.
	if(id[id.size()-1]<='9' && id[id.size()-1]>='0')
	{
		// figure out how many digits it is
		for (digits = 0;
			 (int)id.size()-1 >= digits && id[id.size()-1-digits] <= '9' && id[id.size()-1-digits] >= '0';
			 digits++)
			;

		String str_number;
		str_number=String(id,id.size()-digits,id.size());
		id=String(id,0,id.size()-digits);
		number=atoi(str_number.c_str());
	}
	else
	{
		number=1;
		digits=3;
	}

	number++;

	// Add the number back onto the id
	{
		const String format(strprintf("%%0%dd",digits));
		id+=strprintf(format.c_str(),number);
	}

	// Set the ID
	set_id(id);
}


StateBLine_Context::StateBLine_Context(CanvasView* canvas_view):
	canvas_view_(canvas_view),
	is_working(*canvas_view),
	prev_table_status(false),
	loop_(false),
	prev_workarea_layer_status_(get_work_area()->get_allow_layer_clicks()),
	depth(-1),
	duckmatic_push(get_work_area()),
	settings(synfigapp::Main::get_selected_input_device()->settings()),
	opacity_hscl(0.0f, 1.0125f, 0.0125f)
{
	egress_on_selection_change=true;

	/* Set up the tool options dialog */

	// 0, title
	title_label.set_label(_("Spline Creation"));
	Pango::AttrList list;
	Pango::AttrInt attr = Pango::Attribute::create_attr_weight(Pango::WEIGHT_BOLD);
	list.insert(attr);
	title_label.set_attributes(list);
	title_label.set_alignment(Gtk::ALIGN_START, Gtk::ALIGN_CENTER);

	// 1, layer name label and entry
	id_label.set_label(_("Name:"));
	id_label.set_alignment(Gtk::ALIGN_START, Gtk::ALIGN_CENTER);
	SPACING(id_gap, GAP);
	id_box.pack_start(id_label, Gtk::PACK_SHRINK);
	id_box.pack_start(*id_gap, Gtk::PACK_SHRINK);

	id_box.pack_start(id_entry);

	// 2, layer types creation
	layer_types_label.set_label(_("Layer Type:"));
	layer_types_label.set_alignment(Gtk::ALIGN_START, Gtk::ALIGN_CENTER);

	LAYER_CREATION(layer_region_togglebutton,
		("synfig-layer_geometry_region"), _("Create a region layer"));

	LAYER_CREATION(layer_outline_togglebutton,
		("synfig-layer_geometry_outline"), _("Create a outline layer"));

	LAYER_CREATION(layer_advanced_outline_togglebutton,
		("synfig-layer_geometry_advanced_outline"), _("Create a advanced outline layer"));

	LAYER_CREATION(layer_plant_togglebutton,
		("synfig-layer_other_plant"), _("Create a plant layer"));

	LAYER_CREATION(layer_curve_gradient_togglebutton,
		("synfig-layer_gradient_curve"), _("Create a gradient layer"));

	SPACING(layer_types_indent, INDENTATION);

	layer_types_box.pack_start(*layer_types_indent, Gtk::PACK_SHRINK);
	layer_types_box.pack_start(layer_region_togglebutton, Gtk::PACK_SHRINK);
	layer_types_box.pack_start(layer_outline_togglebutton, Gtk::PACK_SHRINK);
	layer_types_box.pack_start(layer_advanced_outline_togglebutton, Gtk::PACK_SHRINK);
	layer_types_box.pack_start(layer_plant_togglebutton, Gtk::PACK_SHRINK);
	layer_types_box.pack_start(layer_curve_gradient_togglebutton, Gtk::PACK_SHRINK);

	// 3, blend method label and dropdown list
	blend_label.set_label(_("Blend Method:"));
	blend_label.set_alignment(Gtk::ALIGN_START, Gtk::ALIGN_CENTER);
	SPACING(blend_gap, GAP);
	blend_box.pack_start(blend_label, Gtk::PACK_SHRINK);
	blend_box.pack_start(*blend_gap, Gtk::PACK_SHRINK);

	blend_enum.set_param_desc(ParamDesc(Color::BLEND_COMPOSITE,"blend_method")
		.set_local_name(_("Blend Method"))
		.set_description(_("Defines the blend method to be used for splines")));

	// 4, opacity label and slider
	opacity_label.set_label(_("Opacity:"));
	opacity_label.set_alignment(Gtk::ALIGN_START, Gtk::ALIGN_CENTER);

	opacity_hscl.set_digits(2);
	opacity_hscl.set_value_pos(Gtk::POS_LEFT);
	opacity_hscl.set_tooltip_text(_("Opacity"));

	// 5, brush size
	bline_width_label.set_label(_("Brush Size:"));
	bline_width_label.set_alignment(Gtk::ALIGN_START, Gtk::ALIGN_CENTER);
	bline_width_label.set_sensitive(false);

	bline_width_dist.set_digits(2);
	bline_width_dist.set_range(0,10000000);
	bline_width_dist.set_sensitive(false);

	// 6, feather
	feather_label.set_label(_("Feather:"));
	feather_label.set_alignment(Gtk::ALIGN_START, Gtk::ALIGN_CENTER);
	feather_label.set_sensitive(false);

	feather_dist.set_digits(2);
	feather_dist.set_range(0,10000000);
	feather_dist.set_sensitive(false);

	// 7, link origins
	link_origins_label.set_label(_("Link Origins"));
	link_origins_label.set_alignment(Gtk::ALIGN_START, Gtk::ALIGN_CENTER);

	link_origins_box.pack_start(link_origins_label);
	link_origins_box.pack_end(layer_link_origins_checkbutton, Gtk::PACK_SHRINK);
	link_origins_box.set_sensitive(false);

	// 8, auto export
	auto_export_label.set_label(_("Auto Export"));
	auto_export_label.set_alignment(Gtk::ALIGN_START, Gtk::ALIGN_CENTER);

	auto_export_box.pack_start(auto_export_label);
	auto_export_box.pack_end(auto_export_checkbutton, Gtk::PACK_SHRINK);
	auto_export_box.set_sensitive(true);

	load_settings();

	// pack all options to the options_table

	// 0, title
	options_table.attach(title_label,
		0, 2, 0, 1, Gtk::FILL, Gtk::FILL, 0, 0
		);
	// 1, name
	options_table.attach(id_box,
		0, 2, 1, 2, Gtk::FILL, Gtk::FILL, 0, 0
		);
	// 2, layer types creation
	options_table.attach(layer_types_label,
		0, 2, 2, 3, Gtk::FILL, Gtk::FILL, 0, 0
		);
	options_table.attach(layer_types_box,
		0, 2, 3, 4, Gtk::FILL, Gtk::FILL, 0, 0
		);
	// 3, blend method
	options_table.attach(blend_box,
		0, 1, 4, 5, Gtk::EXPAND|Gtk::FILL, Gtk::FILL, 0, 0
		);
	options_table.attach(blend_enum,
		1, 2, 4, 5, Gtk::EXPAND|Gtk::FILL, Gtk::FILL, 0, 0
		);
	// 4, opacity
	options_table.attach(opacity_label,
		0, 1, 5, 6, Gtk::EXPAND|Gtk::FILL, Gtk::FILL, 0, 0
		);
	options_table.attach(opacity_hscl,
		1, 2, 5, 6, Gtk::EXPAND|Gtk::FILL, Gtk::FILL, 0, 0
		);
	// 5, brush size
	options_table.attach(bline_width_label,
		0, 1, 6, 7, Gtk::EXPAND|Gtk::FILL, Gtk::FILL, 0, 0
		);
	options_table.attach(bline_width_dist,
		1, 2, 6, 7, Gtk::EXPAND|Gtk::FILL, Gtk::FILL, 0, 0
		);
	// 6, feather
	options_table.attach(feather_label,
		0, 1, 7, 8, Gtk::EXPAND|Gtk::FILL, Gtk::FILL, 0, 0
		);
	options_table.attach(feather_dist,
		1, 2, 7, 8, Gtk::EXPAND|Gtk::FILL, Gtk::FILL, 0, 0
		);
	// 7, link origins
	options_table.attach(link_origins_box,
		0, 2, 8, 9, Gtk::FILL, Gtk::FILL, 0, 0
		);
	// 8, auto export
	options_table.attach(auto_export_box,
		0, 2, 9, 10, Gtk::FILL, Gtk::FILL, 0, 0
		);

	// fine-tune options layout
	options_table.set_border_width(GAP*2); // border width
	options_table.set_row_spacings(GAP); // row gap
	options_table.set_row_spacing(0, GAP*2); // the gap between first and second row.
	options_table.set_row_spacing(2, 1); // row gap between label and icon of layer type
	//options_table.set_row_spacing(10, 0); //// the final row using border width of table
	options_table.set_margin_bottom(0);
	options_table.show_all();

	refresh_tool_options();
	App::dialog_tool_options->present();

	// Turn off layer clicking
	get_work_area()->set_allow_layer_clicks(false);

	// clear out the ducks
	get_work_area()->clear_ducks();

	// Refresh the work area
	get_work_area()->queue_draw();

	// Hide the tables if they are showing
	prev_table_status=get_canvas_view()->tables_are_visible();
	if(prev_table_status)get_canvas_view()->hide_tables();

	// Disable the time bar
	get_canvas_view()->set_sensitive_timebar(false);

	get_work_area()->set_cursor(Gdk::CROSSHAIR);

	App::dock_toolbox->refresh();
}

void
StateBLine_Context::refresh_tool_options()
{
	App::dialog_tool_options->clear();
	App::dialog_tool_options->set_widget(options_table);
	App::dialog_tool_options->set_local_name(_("Spline Tool"));
	App::dialog_tool_options->set_name("bline");

	App::dialog_tool_options->add_button(
		Gtk::StockID("gtk-execute"),
		_("Make Spline")
	)->signal_clicked().connect(
		sigc::hide_return(sigc::mem_fun(
			*this,
			&StateBLine_Context::run
		))
	);

	App::dialog_tool_options->add_button(
		Gtk::StockID("gtk-clear"),
		_("Clear current Spline")
	)->signal_clicked().connect(
		sigc::mem_fun(
			*this,
			&StateBLine_Context::reset
		)
	);
}

Smach::event_result
StateBLine_Context::event_refresh_tool_options(const Smach::event& /*x*/)
{
	refresh_tool_options();
	return Smach::RESULT_ACCEPT;
}

StateBLine_Context::~StateBLine_Context()
{
	run();

	save_settings();
	App::dialog_tool_options->clear();

	get_work_area()->reset_cursor();

	// Restore layer clicking
	get_work_area()->set_allow_layer_clicks(prev_workarea_layer_status_);

	// Enable the time bar
	get_canvas_view()->set_sensitive_timebar(true);

	// Bring back the tables if they were out before
	if(prev_table_status)get_canvas_view()->show_tables();

	// Refresh the work area
	get_work_area()->queue_draw();

	App::dock_toolbox->refresh();
}

Smach::event_result
StateBLine_Context::event_stop_handler(const Smach::event& /*x*/)
{
	reset();
	return Smach::RESULT_ACCEPT;
}

Smach::event_result
StateBLine_Context::event_refresh_handler(const Smach::event& /*x*/)
{
	refresh_ducks();
	return Smach::RESULT_ACCEPT;
}

bool
StateBLine_Context::run()
{
	sanity_check();

	String err;
	bool success(false);
	for(int i=5;i>0 && !success;i--)try
	{
		success=run_();
	}
	catch(String s)
	{
		err=s;
	}
	if(!success && !err.empty())
	{
		get_canvas_view()->get_ui_interface()->error(err);
	}
	return success;
}

bool
StateBLine_Context::run_()
{
	curr_duck=0;
	next_duck=0;

	// Now we need to generate it
	if(bline_point_list.empty())
	{
		return false;
	}
	if(bline_point_list.size()<2)
	{
		get_canvas_view()->get_ui_interface()->task(_("Information: You need at least two (2) points to create a spline"));
		return false;
	}

	do
	{

		// Create the action group
		synfigapp::Action::PassiveGrouper group(get_canvas_interface()->get_instance().get(),_("New Spline"));

		std::vector<BLinePoint> new_list;
		std::list<synfig::ValueNode_Const::Handle>::iterator iter;
		const synfig::TransformStack& transform(get_transform_stack());

		for(iter=bline_point_list.begin();iter!=bline_point_list.end();++iter)
		{
			BLinePoint bline_point((*iter)->get_value().get(BLinePoint()));
			Point new_vertex(transform.unperform(bline_point.get_vertex()));

			bline_point.set_tangent1(
				transform.unperform(
					bline_point.get_tangent1()+bline_point.get_vertex()
				) -new_vertex
			);

			bline_point.set_tangent2(
				transform.unperform(
					bline_point.get_tangent2()+bline_point.get_vertex()
				) -new_vertex
			);

			bline_point.set_vertex(new_vertex);

			new_list.push_back(bline_point);
		}

		ValueNode_BLine::Handle value_node_bline(ValueNode_BLine::create(new_list));
		assert(value_node_bline);

		ValueNode::Handle value_node_origin(ValueNode_Const::create(Vector()));
		assert(value_node_origin);

		// Set the looping flag
		value_node_bline->set_loop(loop_);

		// Add the BLine to the canvas
		if(get_auto_export_flag() && !get_canvas_interface()->add_value_node(value_node_bline,get_id()))
		{
			group.cancel();
			increment_id();
			throw String(_("Unable to add value node"));
			return false;
		}

		Layer::Handle layer;

		// we are temporarily using the layer to hold something
		layer=get_canvas_view()->get_selection_manager()->get_selected_layer();

		if(layer)
		{
			if(depth<0)
				depth=layer->get_depth();
			if(!canvas)
				canvas=layer->get_canvas();
		}
		else
			depth=0;

		if(!canvas)
			canvas=get_canvas_view()->get_canvas();

		value_node_bline->set_member_canvas(canvas);

		synfigapp::SelectionManager::LayerList layer_selection;
		if (!getenv("SYNFIG_TOOLS_CLEAR_SELECTION"))
			layer_selection = get_canvas_view()->get_selection_manager()->get_selected_layers();

		// count how many layers we're going to be creating
		int layers_to_create = this->layers_to_create();

		///////////////////////////////////////////////////////////////////////////
		//   C U R V E   G R A D I E N T
		///////////////////////////////////////////////////////////////////////////

		if(get_layer_curve_gradient_flag())
		{
			synfigapp::PushMode push_mode(get_canvas_interface(),synfigapp::MODE_NORMAL);

			egress_on_selection_change=false;
			Layer::Handle layer(get_canvas_interface()->add_layer_to("curve_gradient",canvas,depth));
			egress_on_selection_change=true;
			if (!layer)
			{
				group.cancel();
				throw String(_("Unable to create layer"));
			}
			layer_selection.push_back(layer);
			layer->set_description(get_id()+_(" Gradient"));
			get_canvas_interface()->signal_layer_new_description()(layer,layer->get_description());

			layer->set_param("blend_method",get_blend());
			get_canvas_interface()->signal_layer_param_changed()(layer,"blend_method");

			layer->set_param("amount",get_opacity());
			get_canvas_interface()->signal_layer_param_changed()(layer,"amount");

			{
				synfigapp::Action::Handle action(synfigapp::Action::create("LayerParamConnect"));
				assert(action);

				action->set_param("canvas",get_canvas());
				action->set_param("canvas_interface",get_canvas_interface());
				action->set_param("layer",layer);
				if(!action->set_param("param",String("bline")))
					synfig::error("LayerParamConnect didn't like \"param\"");
				if(!action->set_param("value_node",ValueNode::Handle(value_node_bline)))
					synfig::error("LayerParamConnect didn't like \"value_node\"");

				if(!get_canvas_interface()->get_instance()->perform_action(action))
				{
					group.cancel();
					throw String(_("Unable to create Gradient layer"));
					return false;
				}
			}

			// only link the curve gradient's origin parameter if the option is selected and we're creating more than one layer
			if (get_layer_link_origins_flag() && layers_to_create > 1)
			{
				synfigapp::Action::Handle action(synfigapp::Action::create("LayerParamConnect"));
				assert(action);

				action->set_param("canvas",get_canvas());
				action->set_param("canvas_interface",get_canvas_interface());
				action->set_param("layer",layer);
				if(!action->set_param("param",String("origin")))
					synfig::error("LayerParamConnect didn't like \"param\"");
				if(!action->set_param("value_node",ValueNode::Handle(value_node_origin)))
					synfig::error("LayerParamConnect didn't like \"value_node\"");

				if(!get_canvas_interface()->get_instance()->perform_action(action))
				{
					//get_canvas_view()->get_ui_interface()->error(_("Unable to create BLine layer"));
					group.cancel();
					throw String(_("Unable to create Gradient layer"));
					return false;
				}
			}
		}

		///////////////////////////////////////////////////////////////////////////
		//   P L A N T
		///////////////////////////////////////////////////////////////////////////

		if(get_layer_plant_flag())
		{
			synfigapp::PushMode push_mode(get_canvas_interface(),synfigapp::MODE_NORMAL);

			egress_on_selection_change=false;
			Layer::Handle layer(get_canvas_interface()->add_layer_to("plant",canvas,depth));
			egress_on_selection_change=true;
			if (!layer)
			{
				group.cancel();
				throw String(_("Unable to create layer"));
			}
			layer_selection.push_back(layer);

			layer->set_description(get_id()+_(" Plant"));
			get_canvas_interface()->signal_layer_new_description()(layer,layer->get_description());

			layer->set_param("blend_method",get_blend());
			get_canvas_interface()->signal_layer_param_changed()(layer,"blend_method");

			layer->set_param("amount",get_opacity());
			get_canvas_interface()->signal_layer_param_changed()(layer,"amount");

			{
				synfigapp::Action::Handle action(synfigapp::Action::create("LayerParamConnect"));
				assert(action);

				action->set_param("canvas",get_canvas());
				action->set_param("canvas_interface",get_canvas_interface());
				action->set_param("layer",layer);
				if(!action->set_param("param",String("bline")))
					synfig::error("LayerParamConnect didn't like \"param\"");
				if(!action->set_param("value_node",ValueNode::Handle(value_node_bline)))
					synfig::error("LayerParamConnect didn't like \"value_node\"");

				if(!get_canvas_interface()->get_instance()->perform_action(action))
				{
					group.cancel();
					throw String(_("Unable to create Plant layer"));
					return false;
				}
			}

			// only link the plant's origin parameter if the option is selected and we're creating more than one layer
			if (get_layer_link_origins_flag() && layers_to_create > 1)
			{
				synfigapp::Action::Handle action(synfigapp::Action::create("LayerParamConnect"));
				assert(action);

				action->set_param("canvas",get_canvas());
				action->set_param("canvas_interface",get_canvas_interface());
				action->set_param("layer",layer);
				if(!action->set_param("param",String("origin")))
					synfig::error("LayerParamConnect didn't like \"param\"");
				if(!action->set_param("value_node",ValueNode::Handle(value_node_origin)))
					synfig::error("LayerParamConnect didn't like \"value_node\"");

				if(!get_canvas_interface()->get_instance()->perform_action(action))
				{
					group.cancel();
					throw String(_("Unable to create Plant layer"));
					return false;
				}
			}
		}

		///////////////////////////////////////////////////////////////////////////
		//   R E G I O N
		///////////////////////////////////////////////////////////////////////////

		if(get_layer_region_flag())
		{
			synfigapp::PushMode push_mode(get_canvas_interface(),synfigapp::MODE_NORMAL);

			egress_on_selection_change=false;
			Layer::Handle layer(get_canvas_interface()->add_layer_to("region",canvas,depth));
			egress_on_selection_change=true;
			if (!layer)
			{
				group.cancel();
				throw String(_("Unable to create layer"));
			}
			layer_selection.push_back(layer);

			layer->set_description(get_id()+_(" Region"));
			get_canvas_interface()->signal_layer_new_description()(layer,layer->get_description());

			layer->set_param("blend_method",get_blend());
			get_canvas_interface()->signal_layer_param_changed()(layer,"blend_method");

			layer->set_param("amount",get_opacity());
			get_canvas_interface()->signal_layer_param_changed()(layer,"amount");

			if(get_feather_size())
			{
				layer->set_param("feather",get_feather_size());
				get_canvas_interface()->signal_layer_param_changed()(layer,"feather");
			}

			// I don't know if it's safe to reuse the same LayerParamConnect action, so I'm
			// using 2 separate ones.
			{
				synfigapp::Action::Handle action(synfigapp::Action::create("LayerParamConnect"));
				assert(action);

				action->set_param("canvas",get_canvas());
				action->set_param("canvas_interface",get_canvas_interface());
				action->set_param("layer",layer);
				if(!action->set_param("param",String("bline")))
					synfig::error("LayerParamConnect didn't like \"param\"");
				if(!action->set_param("value_node",ValueNode::Handle(value_node_bline)))
					synfig::error("LayerParamConnect didn't like \"value_node\"");

				if(!get_canvas_interface()->get_instance()->perform_action(action))
				{
					group.cancel();
					throw String(_("Unable to create Region layer"));
					return false;
				}
			}

			// only link the region's origin parameter if the option is selected and we're creating more than one layer
			if (get_layer_link_origins_flag() && layers_to_create > 1)
			{
				synfigapp::Action::Handle action(synfigapp::Action::create("LayerParamConnect"));
				assert(action);

				action->set_param("canvas",get_canvas());
				action->set_param("canvas_interface",get_canvas_interface());
				action->set_param("layer",layer);
				if(!action->set_param("param",String("origin")))
					synfig::error("LayerParamConnect didn't like \"param\"");
				if(!action->set_param("value_node",ValueNode::Handle(value_node_origin)))
					synfig::error("LayerParamConnect didn't like \"value_node\"");

				if(!get_canvas_interface()->get_instance()->perform_action(action))
				{
					group.cancel();
					throw String(_("Unable to create Region layer"));
					return false;
				}
			}
		}

		///////////////////////////////////////////////////////////////////////////
		//   O U T L I N E
		///////////////////////////////////////////////////////////////////////////

		if(get_layer_outline_flag())
		{
			synfigapp::PushMode push_mode(get_canvas_interface(),synfigapp::MODE_NORMAL);

			egress_on_selection_change=false;
			Layer::Handle layer(get_canvas_interface()->add_layer_to("outline",canvas,depth));
			egress_on_selection_change=true;
			if (!layer)
			{
				group.cancel();
				throw String(_("Unable to create layer"));
			}
			layer_selection.push_back(layer);

			layer->set_description(get_id()+_(" Outline"));
			get_canvas_interface()->signal_layer_new_description()(layer,layer->get_description());

			layer->set_param("blend_method",get_blend());
			get_canvas_interface()->signal_layer_param_changed()(layer,"blend_method");

			layer->set_param("amount",get_opacity());
			get_canvas_interface()->signal_layer_param_changed()(layer,"amount");

			layer->set_param("width",get_bline_width());
			get_canvas_interface()->signal_layer_param_changed()(layer,"width");

			if(get_feather_size())
			{
				layer->set_param("feather",get_feather_size());
				get_canvas_interface()->signal_layer_param_changed()(layer,"feather");
			}

			{
				synfigapp::Action::Handle action(synfigapp::Action::create("LayerParamConnect"));
				assert(action);

				action->set_param("canvas",get_canvas());
				action->set_param("canvas_interface",get_canvas_interface());
				action->set_param("layer",layer);
				if(!action->set_param("param",String("bline")))
					synfig::error("LayerParamConnect didn't like \"param\"");
				if(!action->set_param("value_node",ValueNode::Handle(value_node_bline)))
					synfig::error("LayerParamConnect didn't like \"value_node\"");

				if(!get_canvas_interface()->get_instance()->perform_action(action))
				{
					group.cancel();
					throw String(_("Unable to create Outline layer"));
					return false;
				}
			}

			// only link the outline's origin parameter if the option is selected and we're creating more than one layer
			if (get_layer_link_origins_flag() && layers_to_create > 1)
			{
				synfigapp::Action::Handle action(synfigapp::Action::create("LayerParamConnect"));
				assert(action);

				action->set_param("canvas",get_canvas());
				action->set_param("canvas_interface",get_canvas_interface());
				action->set_param("layer",layer);
				if(!action->set_param("param",String("origin")))
					synfig::error("LayerParamConnect didn't like \"param\"");
				if(!action->set_param("value_node",ValueNode::Handle(value_node_origin)))
					synfig::error("LayerParamConnect didn't like \"value_node\"");

				if(!get_canvas_interface()->get_instance()->perform_action(action))
				{
					group.cancel();
					throw String(_("Unable to create Outline layer"));
					return false;
				}
			}
		}

		///////////////////////////////////////////////////////////////////////////
		//   A D V A N C E D   O U T L I N E
		///////////////////////////////////////////////////////////////////////////

		if(get_layer_advanced_outline_flag())
		{
			synfigapp::PushMode push_mode(get_canvas_interface(),synfigapp::MODE_NORMAL);

			egress_on_selection_change=false;
			Layer::Handle layer(get_canvas_interface()->add_layer_to("advanced_outline",canvas,depth));
			egress_on_selection_change=true;
			if (!layer)
			{
				group.cancel();
				throw String(_("Unable to create layer"));
			}
			layer_selection.push_back(layer);

			layer->set_description(get_id()+_(" Advanced Outline"));
			get_canvas_interface()->signal_layer_new_description()(layer,layer->get_description());

			layer->set_param("blend_method",get_blend());
			get_canvas_interface()->signal_layer_param_changed()(layer,"blend_method");

			layer->set_param("amount",get_opacity());
			get_canvas_interface()->signal_layer_param_changed()(layer,"amount");

			layer->set_param("width",get_bline_width());
			get_canvas_interface()->signal_layer_param_changed()(layer,"width");

			if(get_feather_size())
			{
				layer->set_param("feather",get_feather_size());
				get_canvas_interface()->signal_layer_param_changed()(layer,"feather");
			}

			{
				synfigapp::Action::Handle action(synfigapp::Action::create("LayerParamConnect"));
				assert(action);

				action->set_param("canvas",get_canvas());
				action->set_param("canvas_interface",get_canvas_interface());
				action->set_param("layer",layer);
				if(!action->set_param("param",String("bline")))
					synfig::error("LayerParamConnect didn't like \"param\"");
				if(!action->set_param("value_node",ValueNode::Handle(value_node_bline)))
					synfig::error("LayerParamConnect didn't like \"value_node\"");

				if(!get_canvas_interface()->get_instance()->perform_action(action))
				{
					group.cancel();
					throw String(_("Unable to create Advanced Outline layer"));
					return false;
				}
			}

			// only link the advanced outline's origin parameter if the option is selected and we're creating more than one layer
			if (get_layer_link_origins_flag() && layers_to_create > 1)
			{
				synfigapp::Action::Handle action(synfigapp::Action::create("LayerParamConnect"));
				assert(action);

				action->set_param("canvas",get_canvas());
				action->set_param("canvas_interface",get_canvas_interface());
				action->set_param("layer",layer);
				if(!action->set_param("param",String("origin")))
					synfig::error("LayerParamConnect didn't like \"param\"");
				if(!action->set_param("value_node",ValueNode::Handle(value_node_origin)))
					synfig::error("LayerParamConnect didn't like \"value_node\"");

				if(!get_canvas_interface()->get_instance()->perform_action(action))
				{
					group.cancel();
					throw String(_("Unable to create Advanced Outline layer"));
					return false;
				}
			}
		}

		egress_on_selection_change=false;
		get_canvas_interface()->get_selection_manager()->clear_selected_layers();
		get_canvas_interface()->get_selection_manager()->set_selected_layers(layer_selection);
		egress_on_selection_change=true;

	} while(0);

	reset();
	increment_id();
	return true;
}

Smach::event_result
StateBLine_Context::event_mouse_motion_handler(const Smach::event& x)
{
	const EventMouse& event(*reinterpret_cast<const EventMouse*>(&x));

	if(curr_duck)
	{
		Point p(get_work_area()->snap_point_to_grid(event.pos));
		curr_duck->set_trans_point(p);
		curr_duck->signal_edited()(*curr_duck);
		if(next_duck)
			next_duck->set_trans_point(p);
		get_work_area()->queue_draw();
		return Smach::RESULT_ACCEPT;
	}

	return Smach::RESULT_OK;
}

Smach::event_result
StateBLine_Context::event_mouse_release_handler(const Smach::event& /*x*/)
{
	if(curr_duck)
	{
		curr_duck->signal_edited()(*curr_duck);
		if(next_duck)
		{
			curr_duck=next_duck;
			next_duck=0;
		}
		refresh_ducks(false);
		return Smach::RESULT_ACCEPT;
	}
	return Smach::RESULT_OK;
}

Smach::event_result
StateBLine_Context::event_mouse_click_handler(const Smach::event& x)
{
	const EventMouse& event(*reinterpret_cast<const EventMouse*>(&x));
	switch(event.button)
	{
	case BUTTON_LEFT:
		{
			// If we are already looped up, then don't try to add anything else
			if(loop_)
				return Smach::RESULT_OK;
			BLinePoint bline_point;
			bline_point.set_vertex(get_work_area()->snap_point_to_grid(event.pos));
			bline_point.set_width(1.0f);
			bline_point.set_origin(0.5f);
			bline_point.set_split_tangent_radius(false);
			bline_point.set_split_tangent_angle(false);
			bline_point.set_tangent(Vector(0,0));
			bline_point_list.push_back(ValueNode_Const::Handle::cast_dynamic(ValueNode_Const::create(bline_point)));

			refresh_ducks();
			return Smach::RESULT_ACCEPT;
		}

	default:
		return Smach::RESULT_OK;
	}
}

void
StateBLine_Context::refresh_ducks(bool button_down)
{
	get_work_area()->clear_ducks();
	get_work_area()->queue_draw();

	if(bline_point_list.empty())
		return;

	list<ValueNode_Const::Handle>::iterator iter;

	handle<WorkArea::Bezier> bezier;
	handle<WorkArea::Duck> duck,tduck1,tduck2,first_tduck1,first_tduck2;
	BLinePoint bline_point;

	for(iter=bline_point_list.begin();iter!=bline_point_list.end();++iter)
	{
		ValueNode_Const::Handle value_node(*iter);
		bline_point=(value_node->get_value().get(BLinePoint()));
		assert(value_node);

		// First add the duck associated with this vertex
		duck=new WorkArea::Duck(bline_point.get_vertex());
		duck->set_editable(true);
#ifdef DISTINGUISH_FIRST_DUCK
		if (iter!=bline_point_list.begin())
			duck->set_type(Duck::TYPE_VERTEX);
#else
		duck->set_type(Duck::TYPE_VERTEX);
#endif
		duck->set_name(strprintf("%x-vertex",value_node.get()));
		duck->signal_edited().connect(
			sigc::bind(sigc::mem_fun(*this,&studio::StateBLine_Context::on_vertex_change),value_node)
		);
		duck->signal_user_click(2).connect(
			sigc::bind(sigc::mem_fun(*this,&studio::StateBLine_Context::popup_vertex_menu),value_node)
		);
		duck->set_guid(value_node->get_guid()^synfig::GUID::hasher(0));

		get_work_area()->add_duck(duck);

		tduck1=new WorkArea::Duck(bline_point.get_tangent1());
		tduck2=new WorkArea::Duck(bline_point.get_tangent2());
		if (!first_tduck1) first_tduck1 = tduck1;
		if (!first_tduck2) first_tduck2 = tduck2;

		// Add the tangent1 duck
		tduck1->set_editable(true);
		tduck1->set_edit_immediatelly(true);
		tduck1->set_name(strprintf("%x-tangent1",value_node.get()));
		tduck1->set_origin(duck);
		tduck1->set_scalar(-0.33333333333333333);
		tduck1->set_tangent(true);
		tduck1->set_guid(value_node->get_guid()^synfig::GUID::hasher(3));
		tduck1->signal_edited().connect(
			sigc::bind(sigc::mem_fun(*this,&studio::StateBLine_Context::on_tangent1_change),tduck2,value_node)
		);
		tduck1->signal_user_click(2).connect(
			sigc::bind(sigc::mem_fun(*this,&studio::StateBLine_Context::popup_handle_menu),value_node)
		);

		// See if we need to add that duck to the previous bezier
		if(bezier)
		{
			get_work_area()->add_duck(tduck1);
			bezier->p2=duck;
			bezier->c2=tduck1;

			bezier->signal_user_click(2).connect(
				sigc::bind(
					sigc::mem_fun(
						*this,
						&studio::StateBLine_Context::popup_bezier_menu
					),
					value_node
				)
			);
			get_work_area()->add_bezier(bezier);

			bezier=0;
		}

		// Now we see if we need to create a bezier
		list<ValueNode_Const::Handle>::iterator next(iter);
		next++;

		bezier=new WorkArea::Bezier();

		// Add the tangent2 duck
		tduck2->set_editable(true);
		tduck2->set_edit_immediatelly(true);
		tduck2->set_origin(duck);
		tduck2->set_scalar(0.33333333333333333);
		tduck2->set_tangent(true);

		tduck2->set_name(strprintf("%x-tangent2",value_node.get()));
		tduck2->signal_edited().connect(
			sigc::bind(sigc::mem_fun(*this,&studio::StateBLine_Context::on_tangent2_change),tduck1,value_node)
		);

		tduck2->set_guid(value_node->get_guid()^synfig::GUID::hasher(4));
		tduck2->signal_user_click(2).connect(
			sigc::bind(sigc::mem_fun(*this,&studio::StateBLine_Context::popup_handle_menu),value_node)
		);

		// Setup the next bezier
		bezier->p1=duck;
		bezier->c1=tduck2;

		get_work_area()->add_duck(tduck2);
		curr_duck=tduck2;
	}

	// Add the loop, if requested
	if(bezier && loop_)
	{
		curr_duck=0;
		BLinePoint bline_point(bline_point_list.front()->get_value().get(BLinePoint()));

		duck=new WorkArea::Duck(bline_point.get_vertex());
		duck->set_editable(true);
#ifndef DISTINGUISH_FIRST_DUCK
		duck->set_type(Duck::TYPE_VERTEX);
#endif
		duck->set_name(strprintf("%x-vertex",bline_point_list.front().get()));
		duck->signal_edited().connect(
			sigc::bind(sigc::mem_fun(*this,&studio::StateBLine_Context::on_vertex_change),bline_point_list.front())
		);
		duck->signal_user_click(2).connect(
			sigc::bind(sigc::mem_fun(*this,&studio::StateBLine_Context::popup_vertex_menu),bline_point_list.front())
		);
		get_work_area()->add_duck(duck);

		/*
		tduck1=new WorkArea::Duck(bline_point.get_tangent1());

		// Add the tangent1 duck
		tduck1->set_editable(true);
		tduck1->set_name(strprintf("%x-tangent1",bline_point_list.front().get()));
		tduck1->set_origin(duck);
		tduck1->set_scalar(-0.33333333333333333);
		tduck1->set_tangent(true);
		tduck1->signal_edited().connect(
			sigc::bind(sigc::mem_fun(*this,&studio::StateBLine_Context::on_tangent1_change),first_tduck2,bline_point_list.front())
		);
		tduck1->signal_user_click(2).connect(
			sigc::bind(sigc::mem_fun(*this,&studio::StateBLine_Context::popup_handle_menu),bline_point_list.front())
		);
		get_work_area()->add_duck(tduck1);
		*/
		get_work_area()->add_duck(first_tduck1);

		bezier->p2=duck;
		bezier->c2=first_tduck1;

		bezier->signal_user_click(2).connect(
			sigc::bind(
				sigc::mem_fun(
					*this,
					&studio::StateBLine_Context::popup_bezier_menu
				),
				bline_point_list.front()
			)
		);

		get_work_area()->add_bezier(bezier);
	}
	if(bezier && !loop_)
	{
		duck=new WorkArea::Duck(bline_point.get_vertex());
		duck->set_ignore(true);
		duck->set_name("temp");

		// Add the tangent1 duck
		tduck1=new WorkArea::Duck(Vector(0,0));
		tduck1->set_ignore(true);
		tduck1->set_name("ttemp");
		tduck1->set_origin(duck);
		tduck1->set_scalar(-0.33333333333333333);

		tduck1->set_tangent(true);
		bezier->p2=duck;
		bezier->c2=tduck1;

		get_work_area()->add_duck(bezier->p2);
		get_work_area()->add_bezier(bezier);

		duck->set_guid(synfig::GUID());
		tduck1->set_guid(synfig::GUID());

		next_duck=duck;
	}

	if(!button_down)
	{
		if(curr_duck)
		{
			if(next_duck)
			{
				curr_duck=next_duck;
				next_duck=0;
			}
		}
	}
	get_work_area()->queue_draw();
}


bool
StateBLine_Context::on_vertex_change(const studio::Duck &duck, synfig::ValueNode_Const::Handle value_node)
{
	BLinePoint bline_point(value_node->get_value().get(BLinePoint()));
	bline_point.set_vertex(duck.get_point());
	value_node->set_value(bline_point);
	return true;
}

bool
StateBLine_Context::on_tangent1_change(const studio::Duck &duck, handle<WorkArea::Duck> other_duck, synfig::ValueNode_Const::Handle value_node)
{
	BLinePoint bline_point(value_node->get_value().get(BLinePoint()));
	bline_point.set_tangent1(duck.get_point());
	value_node->set_value(bline_point);
	if (other_duck)
	{
		other_duck->set_point(bline_point.get_tangent2());
		get_work_area()->update_ducks();
		get_work_area()->queue_draw();
	}
	return true;
}

bool
StateBLine_Context::on_tangent2_change(const studio::Duck &duck, handle<WorkArea::Duck> other_duck, synfig::ValueNode_Const::Handle value_node)
{
	BLinePoint bline_point(value_node->get_value().get(BLinePoint()));

	bool split_angle = bline_point.get_split_tangent_angle();
	bool split_radius = bline_point.get_split_tangent_radius();

	if (split_angle && split_radius) {
		bline_point.set_tangent2(duck.get_point());
	} else
	if (split_angle && !split_radius) {
		bline_point.set_tangent1(Vector(duck.get_point().mag(), bline_point.get_tangent1().angle()));
		bline_point.set_tangent2(duck.get_point());
	} else
	if (!split_angle && split_radius) {
		bline_point.set_tangent1(Vector(bline_point.get_tangent1().mag(), duck.get_point().angle()));
		bline_point.set_tangent2(duck.get_point());
	} else
	if (!split_angle && !split_radius) {
		bline_point.set_tangent1(duck.get_point());
	}

	value_node->set_value(bline_point);
	if (other_duck)
	{
		other_duck->set_point(bline_point.get_tangent1());
		get_work_area()->update_ducks();
		get_work_area()->queue_draw();
	}
	return true;
}

void
StateBLine_Context::loop_bline()
{
	loop_=true;

	refresh_ducks(false);
}

void
StateBLine_Context::unloop_bline()
{
	loop_=false;

	refresh_ducks(false);
}

void
StateBLine_Context::popup_vertex_menu(synfig::ValueNode_Const::Handle value_node)
{
	std::vector<Gtk::Widget*> children = menu.get_children();
	for(std::vector<Gtk::Widget*>::iterator i = children.begin(); i != children.end(); ++i)
		menu.remove(**i);

	Gtk::MenuItem *item = NULL;
	Gtk::ImageMenuItem *item2 = NULL;

	BLinePoint bline_point(value_node->get_value().get(BLinePoint()));
	#define STATE_BLINE_ADD_MENU_ITEM(title, split_angle, split_radius, icon) \
	do {                                                                \
		item2 = manage(new Gtk::ImageMenuItem(                       \
				*manage(new Gtk::Image(			    \
					Gtk::StockID(icon),		\
					Gtk::ICON_SIZE_MENU )),			\
				_(title)));                     \
		item2->signal_activate().connect(                                \
				sigc::bind(                                             \
					sigc::mem_fun(*this,&studio::StateBLine_Context::bline_set_split_handle), \
					value_node, split_angle, split_radius ));           \
		item2->show();                                                   \
		menu.append(*item2);                                             \
	} while (false)

	bool split_angle = bline_point.get_split_tangent_angle();
	bool split_radius = bline_point.get_split_tangent_radius();
	
	if (split_angle && split_radius)
		STATE_BLINE_ADD_MENU_ITEM("Merge Tangents", false, false, "gtk-connect");
	else if (!split_angle && !split_radius)
		STATE_BLINE_ADD_MENU_ITEM("Split Tangents", true, true, "gtk-disconnect");
	else if (!split_angle && split_radius)
	{
		STATE_BLINE_ADD_MENU_ITEM("Split Tangents", true, true, "gtk-disconnect");
		STATE_BLINE_ADD_MENU_ITEM("Merge Tangents", false, false, "gtk-connect");
	}
	else if (split_angle && !split_radius)
	{
		STATE_BLINE_ADD_MENU_ITEM("Merge Tangents", false, false, "gtk-connect");
		STATE_BLINE_ADD_MENU_ITEM("Split Tangents", true, true, "gtk-disconnect");
	}
	
	item = manage(new Gtk::SeparatorMenuItem());
	item->show();
	menu.append(*item);

	if (split_angle)
		STATE_BLINE_ADD_MENU_ITEM("Merge Tangents's Angle", false, split_radius, "synfig-type_angle");
	else
		STATE_BLINE_ADD_MENU_ITEM("Split Tangents's Angle", true, split_radius, "synfig-type_angle");

	if (split_radius)
		STATE_BLINE_ADD_MENU_ITEM("Merge Tangents's Radius", split_angle, false, "synfig-type_vector");
	else
		STATE_BLINE_ADD_MENU_ITEM("Split Tangents's Radius", split_angle, true, "synfig-type_vector");

	#undef STATE_BLINE_ADD_MENU_ITEM
	
	item = manage(new Gtk::SeparatorMenuItem());
	item->show();
	menu.append(*item);
	
	if(loop_)
	{
		item = manage(new Gtk::MenuItem(_("Unloop Spline")));
		item->signal_activate().connect(
			sigc::mem_fun(*this,&studio::StateBLine_Context::unloop_bline) );
		item->show();
		menu.append(*item);
	} else {
		item = manage(new Gtk::MenuItem(_("Loop Spline")));
		item->signal_activate().connect(
			sigc::mem_fun(*this,&studio::StateBLine_Context::loop_bline) );
		item->show();
		menu.append(*item);
	}

	item = manage(new Gtk::SeparatorMenuItem());
	item->show();
	menu.append(*item);

	item = manage(new Gtk::MenuItem(_("Delete Vertex")));
	item->signal_activate().connect(
		sigc::bind(
				sigc::mem_fun(*this,&studio::StateBLine_Context::bline_delete_vertex),
				value_node ));
	item->show();
	menu.append(*item);

	menu.popup(0,0);
}

void
StateBLine_Context::popup_bezier_menu(float location, synfig::ValueNode_Const::Handle value_node)
{
	std::vector<Gtk::Widget*> children = menu.get_children();
	for(std::vector<Gtk::Widget*>::iterator i = children.begin(); i != children.end(); ++i)
		menu.remove(**i);

	Gtk::MenuItem *item = NULL;
	item = manage(new Gtk::MenuItem(_("Insert Vertex")));
	item->signal_activate().connect(
		sigc::bind(
			sigc::bind(
				sigc::mem_fun(*this,&studio::StateBLine_Context::bline_insert_vertex),
				location
			),
			value_node ));
	item->show();
	menu.append(*item);
	item = manage(new Gtk::SeparatorMenuItem());
	item->show();
	menu.append(*item);

	if(loop_)
	{
		item = manage(new Gtk::MenuItem(_("Unloop Spline")));
		item->signal_activate().connect(
			sigc::mem_fun(*this,&studio::StateBLine_Context::unloop_bline) );
		item->show();
		menu.append(*item);
	} else {
		item = manage(new Gtk::MenuItem(_("Loop Spline")));
		item->signal_activate().connect(
			sigc::mem_fun(*this,&studio::StateBLine_Context::loop_bline) );
		item->show();
		menu.append(*item);
	}
	menu.popup(0,0);
}

void
StateBLine_Context::bline_insert_vertex(synfig::ValueNode_Const::Handle value_node, float origin)
{
	list<ValueNode_Const::Handle>::iterator iter;

	for(iter=bline_point_list.begin();iter!=bline_point_list.end();++iter)
		if(*iter==value_node)
		{
			BLinePoint bline_point;
			BLinePoint next_bline_point((*iter)->get_value().get(BLinePoint()));
			BLinePoint prev_bline_point;

			list<ValueNode_Const::Handle>::iterator prev(iter);
			if(iter==bline_point_list.begin())
			{
				assert(loop_);
				prev = bline_point_list.end();
			}
			prev--;

			prev_bline_point=(*prev)->get_value().get(BLinePoint());

			etl::hermite<Vector> curve(prev_bline_point.get_vertex(),
									   next_bline_point.get_vertex(),
									   prev_bline_point.get_tangent2(),
									   next_bline_point.get_tangent1());
			etl::derivative< etl::hermite<Vector> > deriv(curve);

			bline_point.set_split_tangent_angle(false);
			bline_point.set_split_tangent_radius(false);
			bline_point.set_vertex(curve(origin));
			bline_point.set_width((next_bline_point.get_width()-prev_bline_point.get_width())*origin+prev_bline_point.get_width());
			bline_point.set_tangent1(deriv(origin)*std::min(1.0f-origin,origin));
			bline_point.set_tangent2(bline_point.get_tangent1());
			bline_point.set_origin(origin);
			bline_point_list.insert(iter,ValueNode_Const::Handle::cast_dynamic(ValueNode_Const::create(bline_point)));

			break;
		}

	if(iter==bline_point_list.end())
	{
		get_canvas_view()->get_ui_interface()->error(_("Unable to find where to insert vertex, internal error, please report this bug"));
	}

	refresh_ducks(false);
}

void
StateBLine_Context::bline_delete_vertex(synfig::ValueNode_Const::Handle value_node)
{
	list<ValueNode_Const::Handle>::iterator iter;

	for(iter=bline_point_list.begin();iter!=bline_point_list.end();++iter)
		if(*iter==value_node)
		{
			bline_point_list.erase(iter);
			break;
		}
	if(iter==bline_point_list.end())
	{
		get_canvas_view()->get_ui_interface()->error(_("Unable to remove vertex, internal error, please report this bug"));
	}

	refresh_ducks(false);
}

void
StateBLine_Context::popup_handle_menu(synfig::ValueNode_Const::Handle value_node)
{
	std::vector<Gtk::Widget*> children = menu.get_children();
	for(std::vector<Gtk::Widget*>::iterator i = children.begin(); i != children.end(); ++i)
		menu.remove(**i);

	BLinePoint bline_point(value_node->get_value().get(BLinePoint()));

	Gtk::MenuItem *item = NULL;
	Gtk::ImageMenuItem *item2 = NULL;
	#define STATE_BLINE_ADD_MENU_ITEM(title, split_angle, split_radius, icon)	\
	do {                                                                \
		item2 = manage(new Gtk::ImageMenuItem(                       \
				*Gtk::manage(new Gtk::Image(			    \
					Gtk::StockID(icon),		\
					Gtk::ICON_SIZE_MENU )),			\
				_(title)));                     \
		item2->signal_activate().connect(                                \
			sigc::bind(													\
				sigc::mem_fun(*this,&studio::StateBLine_Context::bline_set_split_handle), \
				value_node, split_angle, split_radius ));               \
		item2->show();                                                   \
		menu.append(*item2);                                             \
	} while(false)

	bool split_angle = bline_point.get_split_tangent_angle();
	bool split_radius = bline_point.get_split_tangent_radius();
	
	if (split_angle && split_radius)
		STATE_BLINE_ADD_MENU_ITEM("Merge Tangents", false, false, "gtk-connect");
	else if (!split_angle && !split_radius)
		STATE_BLINE_ADD_MENU_ITEM("Split Tangents", true, true, "gtk-disconnect");
	else if (!split_angle && split_radius)
	{
		STATE_BLINE_ADD_MENU_ITEM("Split Tangents", true, true, "gtk-disconnect");
		STATE_BLINE_ADD_MENU_ITEM("Merge Tangents", false, false, "gtk-connect");
	}
	else if (split_angle && !split_radius)
	{
		STATE_BLINE_ADD_MENU_ITEM("Merge Tangents", false, false, "gtk-connect");
		STATE_BLINE_ADD_MENU_ITEM("Split Tangents", true, true, "gtk-disconnect");
	}
	
	item = manage(new Gtk::SeparatorMenuItem());
	item->show();
	menu.append(*item);

	if (split_angle)
		STATE_BLINE_ADD_MENU_ITEM("Merge Tangents's Angle", false, split_radius, "synfig-type_angle");
	else
		STATE_BLINE_ADD_MENU_ITEM("Split Tangents's Angle", true, split_radius, "synfig-type_angle");

	if (split_radius)
		STATE_BLINE_ADD_MENU_ITEM("Merge Tangents's Radius", split_angle, false, "synfig-type_vector");
	else
		STATE_BLINE_ADD_MENU_ITEM("Split Tangents's Radius", split_angle, true, "synfig-type_vector");

	#undef STATE_BLINE_ADD_MENU_ITEM

	item = manage(new Gtk::SeparatorMenuItem());
	item->show();
	menu.append(*item);
	if(loop_)
	{
		item = manage(new Gtk::MenuItem(_("Unloop Spline")));
		item->signal_activate().connect(
			sigc::mem_fun(*this,&studio::StateBLine_Context::unloop_bline) );
		item->show();
		menu.append(*item);
	} else {
		item = manage(new Gtk::MenuItem(_("Loop Spline")));
		item->signal_activate().connect(
			sigc::mem_fun(*this,&studio::StateBLine_Context::loop_bline) );
		item->show();
		menu.append(*item);
	}
	item = manage(new Gtk::SeparatorMenuItem());
	item->show();
	menu.append(*item);

	item = manage(new Gtk::MenuItem(_("Delete Vertex")));
	item->signal_activate().connect(
		sigc::bind(
			sigc::mem_fun(*this,&studio::StateBLine_Context::bline_delete_vertex),
			value_node ));
	item->show();
	menu.append(*item);

	menu.popup(0,0);
}

void
StateBLine_Context::bline_set_split_handle(synfig::ValueNode_Const::Handle value_node, bool split_angle, bool split_radius)
{
	BLinePoint bline_point(value_node->get_value().get(BLinePoint()));

	if (bline_point.get_split_tangent_angle() != split_angle)
	{
		bline_point.set_tangent2(Vector(bline_point.get_tangent2().mag(), bline_point.get_tangent1().angle()));
		bline_point.set_split_tangent_angle(split_angle);
	}

	if (bline_point.get_split_tangent_radius() != split_radius)
	{
		bline_point.set_tangent2(Vector(bline_point.get_tangent1().mag(), bline_point.get_tangent2().angle()));
		bline_point.set_split_tangent_radius(split_radius);
	}

	value_node->set_value(bline_point);
	refresh_ducks(false);
}


void
StateBLine_Context::toggle_layer_creation()
{
  // don't allow none layer creation
  if (get_layer_region_flag() +
     get_layer_outline_flag() +
     get_layer_advanced_outline_flag() +
     get_layer_curve_gradient_flag() +
     get_layer_plant_flag() == 0)
  {
    if(layer_region_flag) set_layer_region_flag(true);
    else if(layer_outline_flag) set_layer_outline_flag(true);
    else if(layer_advanced_outline_flag) set_layer_advanced_outline_flag(true);
    else if(layer_curve_gradient_flag) set_layer_curve_gradient_flag(true);
    else if(layer_plant_flag) set_layer_plant_flag(true);
  }

	// brush size
	if (get_layer_outline_flag() ||
		get_layer_advanced_outline_flag() ||
		get_layer_curve_gradient_flag())
	{
		bline_width_label.set_sensitive(true);
		bline_width_dist.set_sensitive(true);
	}
	else
	{
		bline_width_label.set_sensitive(false);
		bline_width_dist.set_sensitive(false);
	}

	// feather size
	if (get_layer_region_flag() ||
		get_layer_outline_flag() ||
		get_layer_advanced_outline_flag())
	{
		feather_label.set_sensitive(true);
		feather_dist.set_sensitive(true);
	}
	else
	{
		feather_label.set_sensitive(false);
		feather_dist.set_sensitive(false);
	}

	// link origins
	if (get_layer_region_flag() +
		get_layer_outline_flag() +
		get_layer_advanced_outline_flag() +
		get_layer_plant_flag() +
		get_layer_curve_gradient_flag() >= 2)
		{
			link_origins_box.set_sensitive(true);
		}
	else link_origins_box.set_sensitive(false);

  // update layer flags
  layer_region_flag = get_layer_region_flag();
  layer_outline_flag = get_layer_outline_flag();
  layer_advanced_outline_flag = get_layer_advanced_outline_flag();
  layer_curve_gradient_flag = get_layer_curve_gradient_flag();
  layer_plant_flag = get_layer_plant_flag();
}
