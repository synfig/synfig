/* === S Y N F I G ========================================================= */
/*!	\file state_polygon.cpp
**	\brief Template File
**
**	$Id$
**
**	\legal
**	Copyright (c) 2002-2005 Robert B. Quattlebaum Jr., Adrian Bentley
**	Copyright (c) 2007, 2008 Chris Moore
**  Copyright (c) 2010 Carlos López
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
#include <synfigapp/action_system.h>
#include <synfig/valuenodes/valuenode_bline.h>

#include "state_polygon.h"
#include "state_normal.h"
#include "canvasview.h"
#include "workarea.h"
#include "app.h"

#include <synfigapp/action.h>
#include "event_mouse.h"
#include "event_layerclick.h"
#include "docks/dock_toolbox.h"
#include "docks/dialog_tooloptions.h"
#include "widgets/widget_enum.h"
#include "widgets/widget_distance.h"
#include <synfigapp/main.h>

#include <gui/localization.h>

#endif

/* === U S I N G =========================================================== */

using namespace std;
using namespace etl;
using namespace synfig;
using namespace studio;

/* === M A C R O S ========================================================= */

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
		&studio::StatePolygon_Context::toggle_layer_creation))
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

StatePolygon studio::state_polygon;

/* === C L A S S E S & S T R U C T S ======================================= */

class studio::StatePolygon_Context : public sigc::trackable
{
	etl::handle<CanvasView> canvas_view_;
	CanvasView::IsWorking is_working;

	bool prev_table_status;
	bool prev_workarea_layer_status_;

	Duckmatic::Push duckmatic_push;

	std::list<synfig::Point> polygon_point_list;

	bool on_polygon_duck_change(const studio::Duck &duck, std::list<synfig::Point>::iterator iter);
	void popup_handle_menu(synfigapp::ValueDesc value_desc);
	void refresh_ducks();

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
	Gtk::ToggleButton layer_polygon_togglebutton;
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

	// invert
	Gtk::Label invert_label;
	Gtk::CheckButton invert_checkbutton;
	Gtk::HBox invert_box;

	// feather size
	Gtk::Label feather_label;
	Widget_Distance feather_dist;

	// link origins
	Gtk::Label link_origins_label;
	Gtk::CheckButton layer_link_origins_checkbutton;
	Gtk::HBox link_origins_box;

	// spline origins at center
	Gtk::Label origins_at_center_label;
	Gtk::CheckButton layer_origins_at_center_checkbutton;
	Gtk::HBox origins_at_center_box;

public:

	// this counts the layers we create - they all have origins we can link
	int layers_to_create()const
	{
		return
			get_layer_polygon_flag() +
			get_layer_region_flag() +
			get_layer_outline_flag() +
			get_layer_advanced_outline_flag() +
			get_layer_curve_gradient_flag() +
			get_layer_plant_flag();
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

	bool get_invert()const { return invert_checkbutton.get_active(); }
	void set_invert(bool i) { invert_checkbutton.set_active(i); }

	bool get_layer_polygon_flag()const { return layer_polygon_togglebutton.get_active(); }
	void set_layer_polygon_flag(bool x) { return layer_polygon_togglebutton.set_active(x); }

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

  bool layer_polygon_flag;
  bool layer_region_flag;
  bool layer_outline_flag;
  bool layer_advanced_outline_flag;
  bool layer_curve_gradient_flag;
  bool layer_plant_flag;

	Smach::event_result event_stop_handler(const Smach::event& x);

	Smach::event_result event_refresh_handler(const Smach::event& x);

	Smach::event_result event_mouse_click_handler(const Smach::event& x);
	Smach::event_result event_refresh_tool_options(const Smach::event& x);
	void refresh_tool_options();

	StatePolygon_Context(CanvasView* canvas_view);

	~StatePolygon_Context();

	const etl::handle<CanvasView>& get_canvas_view()const{return canvas_view_;}
	etl::handle<synfigapp::CanvasInterface> get_canvas_interface()const{return canvas_view_->canvas_interface();}
	synfig::Canvas::Handle get_canvas()const{return canvas_view_->get_canvas();}
	WorkArea * get_work_area()const{return canvas_view_->get_work_area();}

	//void on_user_click(synfig::Point point);
	void load_settings();
	void save_settings();
	void reset();
	void increment_id();
	bool egress_on_selection_change;
	Smach::event_result event_layer_selection_changed_handler(const Smach::event& /*x*/)
	{
		if(egress_on_selection_change)
			throw &state_normal;
		return Smach::RESULT_OK;
	}

	void run();
	void toggle_layer_creation();

};	// END of class StatePolygon_Context

/* === M E T H O D S ======================================================= */

StatePolygon::StatePolygon():
	Smach::state<StatePolygon_Context>("polygon")
{
	insert(event_def(EVENT_LAYER_SELECTION_CHANGED,&StatePolygon_Context::event_layer_selection_changed_handler));
	insert(event_def(EVENT_STOP,&StatePolygon_Context::event_stop_handler));
	insert(event_def(EVENT_REFRESH,&StatePolygon_Context::event_refresh_handler));
	insert(event_def(EVENT_REFRESH_DUCKS,&StatePolygon_Context::event_refresh_handler));
	insert(event_def(EVENT_WORKAREA_MOUSE_BUTTON_DOWN,&StatePolygon_Context::event_mouse_click_handler));
	insert(event_def(EVENT_REFRESH_TOOL_OPTIONS,&StatePolygon_Context::event_refresh_tool_options));
}

StatePolygon::~StatePolygon()
{
}

void
StatePolygon_Context::load_settings()
{
	try
	{
		synfig::ChangeLocale change_locale(LC_NUMERIC, "C");
		String value;

		if(settings.get_value("polygon.id",value))
			set_id(value);
		else
			set_id("Polygon");

		if(settings.get_value("polygon.blend",value) && value != "")
			set_blend(atoi(value.c_str()));
		else
			set_blend(0);//(int)Color::BLEND_COMPOSITE); //0 should be blend composites value

		if(settings.get_value("polygon.opacity",value))
			set_opacity(atof(value.c_str()));
		else
			set_opacity(1);

		if(settings.get_value("polygon.bline_width",value) && value != "")
			set_bline_width(Distance(atof(value.c_str()), App::distance_system));
		else
			set_bline_width(Distance(1, App::distance_system)); // default width

		if(settings.get_value("polygon.feather",value))
			set_feather_size(Distance(atof(value.c_str()), App::distance_system));
		else
			set_feather_size(Distance(0, App::distance_system)); // default feather

		if(settings.get_value("polygon.invert",value) && value != "0")
			set_invert(true);
		else
			set_invert(false);

		if(settings.get_value("polygon.layer_polygon",value) && value=="0")
			set_layer_polygon_flag(false);
		else
			set_layer_polygon_flag(true);

		if(settings.get_value("polygon.layer_region",value) && value=="1")
			set_layer_region_flag(true);
		else
			set_layer_region_flag(false);

		if(settings.get_value("polygon.layer_outline",value) && value=="1")
			set_layer_outline_flag(true);
		else
			set_layer_outline_flag(false);

		if(settings.get_value("polygon.layer_advanced_outline",value) && value=="1")
			set_layer_advanced_outline_flag(true);
		else
			set_layer_advanced_outline_flag(false);

		if(settings.get_value("polygon.layer_curve_gradient",value) && value=="1")
			set_layer_curve_gradient_flag(true);
		else
			set_layer_curve_gradient_flag(false);

		if(settings.get_value("polygon.layer_plant",value) && value=="1")
			set_layer_plant_flag(true);
		else
			set_layer_plant_flag(false);

		if(settings.get_value("polygon.layer_link_origins",value) && value=="0")
			set_layer_link_origins_flag(false);
		else
			set_layer_link_origins_flag(true);

	  // determine layer flags
		layer_polygon_flag = get_layer_polygon_flag();
	  layer_region_flag = get_layer_region_flag();
	  layer_outline_flag = get_layer_outline_flag();
	  layer_advanced_outline_flag = get_layer_outline_flag();
	  layer_curve_gradient_flag = get_layer_curve_gradient_flag();
	  layer_plant_flag = get_layer_plant_flag();
	}
	catch(...)
	{
		synfig::warning("State Polygon: Caught exception when attempting to load settings.");
	}
}

void
StatePolygon_Context::save_settings()
{
	try
	{
		synfig::ChangeLocale change_locale(LC_NUMERIC, "C");
		settings.set_value("polygon.id",get_id().c_str());
		settings.set_value("polygon.blend",strprintf("%d",get_blend()));
		settings.set_value("polygon.opacity",strprintf("%f",(float)get_opacity()));
		settings.set_value("polygon.bline_width", bline_width_dist.get_value().get_string());
		settings.set_value("polygon.feather", feather_dist.get_value().get_string());
		settings.set_value("polygon.invert",get_invert()?"1":"0");
		settings.set_value("polygon.layer_polygon",get_layer_polygon_flag()?"1":"0");
		settings.set_value("polygon.layer_outline",get_layer_outline_flag()?"1":"0");
		settings.set_value("polygon.layer_advanced_outline",get_layer_advanced_outline_flag()?"1":"0");
		settings.set_value("polygon.layer_region",get_layer_region_flag()?"1":"0");
		settings.set_value("polygon.layer_curve_gradient",get_layer_curve_gradient_flag()?"1":"0");
		settings.set_value("polygon.layer_plant",get_layer_plant_flag()?"1":"0");
		settings.set_value("polygon.layer_link_origins",get_layer_link_origins_flag()?"1":"0");
	}
	catch(...)
	{
		synfig::warning("State Polygon: Caught exception when attempting to save settings.");
	}
}

void
StatePolygon_Context::reset()
{
	polygon_point_list.clear();
	refresh_ducks();
}

void
StatePolygon_Context::increment_id()
{
	String id(get_id());
	int number=1;
	int digits=0;

	if(id.empty())
		id="Polygon";

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

StatePolygon_Context::StatePolygon_Context(CanvasView* canvas_view):
	canvas_view_(canvas_view),
	is_working(*canvas_view),
	prev_workarea_layer_status_(get_work_area()->get_allow_layer_clicks()),
	duckmatic_push(get_work_area()),
	settings(synfigapp::Main::get_selected_input_device()->settings()),
	opacity_hscl(0.0f, 1.0125f, 0.0125f)
{
	egress_on_selection_change=true;


	/* Set up the tool options dialog */

	// 0, title
	title_label.set_label(_("Polygon Creation"));
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

	LAYER_CREATION(layer_polygon_togglebutton,
		("synfig-layer_geometry_polygon"), _("Create a polygon layer"));

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
	layer_types_box.pack_start(layer_polygon_togglebutton, Gtk::PACK_SHRINK);
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
		.set_description(_("Defines the blend method to be used for polygons")));

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

	// 6, invert
	invert_label.set_label(_("Invert"));
	invert_label.set_alignment(Gtk::ALIGN_START, Gtk::ALIGN_CENTER);

	invert_box.pack_start(invert_label);
	invert_box.pack_end(invert_checkbutton, Gtk::PACK_SHRINK);
	invert_box.set_sensitive(false);

	// 7, feather
	feather_label.set_label(_("Feather:"));
	feather_label.set_alignment(Gtk::ALIGN_START, Gtk::ALIGN_CENTER);
	feather_label.set_sensitive(false);

	feather_dist.set_digits(2);
	feather_dist.set_range(0,10000000);
	feather_dist.set_sensitive(false);

	// 8, link origins
	link_origins_label.set_label(_("Link Origins"));
	link_origins_label.set_alignment(Gtk::ALIGN_START, Gtk::ALIGN_CENTER);

	link_origins_box.pack_start(link_origins_label);
	link_origins_box.pack_end(layer_link_origins_checkbutton, Gtk::PACK_SHRINK);
	link_origins_box.set_sensitive(false);

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
	// 6, invert
	options_table.attach(invert_box,
		0, 2, 7, 8, Gtk::EXPAND|Gtk::FILL, Gtk::FILL, 0, 0
		);
	// 7, feather
	options_table.attach(feather_label,
		0, 1, 8, 9, Gtk::EXPAND|Gtk::FILL, Gtk::FILL, 0, 0
		);
	options_table.attach(feather_dist,
		1, 2, 8, 9, Gtk::EXPAND|Gtk::FILL, Gtk::FILL, 0, 0
		);
	// 8, link origins
	options_table.attach(link_origins_box,
		0, 2, 9, 10, Gtk::FILL, Gtk::FILL, 0, 0
		);

	// fine-tune options layout
	options_table.set_border_width(GAP*2); // border width
	options_table.set_row_spacings(GAP); // row gap
	options_table.set_row_spacing(0, GAP*2); // the gap between first and second row.
	options_table.set_row_spacing(2, 1); // row gap between label and icon of layer type
	//options_table.set_row_spacing(9, 0); // the final row using border width of table
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

	get_work_area()->set_cursor(Gdk::CROSSHAIR);

	// Hide the tables if they are showing
	prev_table_status=get_canvas_view()->tables_are_visible();
	if(prev_table_status)get_canvas_view()->hide_tables();

	// Disable the time bar
	get_canvas_view()->set_sensitive_timebar(false);

	App::dock_toolbox->refresh();
}

void
StatePolygon_Context::refresh_tool_options()
{
	App::dialog_tool_options->clear();
	App::dialog_tool_options->set_widget(options_table);

	App::dialog_tool_options->set_local_name(_("Polygon Tool"));
	App::dialog_tool_options->set_name("polygon");

	App::dialog_tool_options->add_button(
		Gtk::StockID("gtk-execute"),
		_("Make Polygon")
	)->signal_clicked().connect(
		sigc::mem_fun(
			*this,
			&StatePolygon_Context::run
		)
	);

	App::dialog_tool_options->add_button(
		Gtk::StockID("gtk-clear"),
		_("Clear current Polygon")
	)->signal_clicked().connect(
		sigc::mem_fun(
			*this,
			&StatePolygon_Context::reset
		)
	);
}

Smach::event_result
StatePolygon_Context::event_refresh_tool_options(const Smach::event& /*x*/)
{
	refresh_tool_options();
	return Smach::RESULT_ACCEPT;
}

StatePolygon_Context::~StatePolygon_Context()
{
	run();

	save_settings();
	// Restore layer clicking
	get_work_area()->set_allow_layer_clicks(prev_workarea_layer_status_);

	App::dialog_tool_options->clear();

	get_work_area()->reset_cursor();

	// Enable the time bar
	get_canvas_view()->set_sensitive_timebar(true);

	// Bring back the tables if they were out before
	if(prev_table_status)get_canvas_view()->show_tables();

	// Refresh the work area
	get_work_area()->queue_draw();

	App::dock_toolbox->refresh();
}

Smach::event_result
StatePolygon_Context::event_stop_handler(const Smach::event& /*x*/)
{
	reset();
	return Smach::RESULT_ACCEPT;

}

Smach::event_result
StatePolygon_Context::event_refresh_handler(const Smach::event& /*x*/)
{
	refresh_ducks();
	return Smach::RESULT_ACCEPT;
}

void
StatePolygon_Context::run()
{
	if(polygon_point_list.empty())
		return;

	if(polygon_point_list.size()<3)
	{
		get_canvas_view()->get_ui_interface()->error("You need at least 3 points to create a polygon");
		return;
	}

	synfigapp::Action::PassiveGrouper group(get_canvas_interface()->get_instance().get(),_("New Polygon"));

	Layer::Handle layer;

	Canvas::Handle canvas;
	int depth(0);

	// we are temporarily using the layer to hold something
	layer=get_canvas_view()->get_selection_manager()->get_selected_layer();
	if(layer)
	{
		depth=layer->get_depth();
		canvas=layer->get_canvas();
	}

	synfigapp::SelectionManager::LayerList layer_selection;
	if (!getenv("SYNFIG_TOOLS_CLEAR_SELECTION"))
		layer_selection = get_canvas_view()->get_selection_manager()->get_selected_layers();

	const synfig::TransformStack& transform(get_work_area()->get_curr_transform_stack());

	std::vector<BLinePoint> new_list;
	std::list<synfig::Point>::iterator iter;
	int i;
	for(i=0,iter=polygon_point_list.begin();iter!=polygon_point_list.end();++iter,++i)
	{
		*iter = transform.unperform(*iter);
		//new_list.push_back(*(new BLinePoint));
		new_list.push_back(BLinePoint());
		new_list[i].set_width(1);
		new_list[i].set_vertex(*iter);
		new_list[i].set_tangent(Point(0,0));
	}

	ValueNode_BLine::Handle value_node_bline(ValueNode_BLine::create(new_list));
	assert(value_node_bline);

	ValueNode::Handle value_node_origin(ValueNode_Const::create(Vector()));
	assert(value_node_origin);

	// Set the looping flag
	value_node_bline->set_loop(true);

	if(!canvas)
		canvas=get_canvas_view()->get_canvas();

	value_node_bline->set_member_canvas(canvas);

	// count how many layers we're going to be creating
	int layers_to_create = this->layers_to_create();

	///////////////////////////////////////////////////////////////////////////
	//   P O L Y G O N
	///////////////////////////////////////////////////////////////////////////

	if (get_layer_polygon_flag())
	{
		egress_on_selection_change=false;
		layer=get_canvas_interface()->add_layer_to("polygon",canvas,depth);
		egress_on_selection_change=true;
		if (!layer)
		{
			get_canvas_view()->get_ui_interface()->error(_("Unable to create layer"));
			group.cancel();
			return;
		}
		layer_selection.push_back(layer);

		layer->set_param("invert",get_invert());
		get_canvas_interface()->signal_layer_param_changed()(layer,"invert");

		if(get_feather_size())
		{
			layer->set_param("feather",get_feather_size());
			get_canvas_interface()->signal_layer_param_changed()(layer,"feather");
		}

		layer->set_description(get_id());
		get_canvas_interface()->signal_layer_new_description()(layer,layer->get_description());

		layer->disconnect_dynamic_param("vector_list");
		if(!layer->set_param("vector_list",ValueBase::List(polygon_point_list.begin(), polygon_point_list.end())))
		{
			group.cancel();
			get_canvas_view()->get_ui_interface()->error("Unable to set layer parameter");
			return;
		}

		{
			synfigapp::Action::Handle action(synfigapp::Action::create("ValueDescConvert"));
			synfigapp::ValueDesc value_desc(layer,"vector_list");
			action->set_param("canvas",get_canvas());
			action->set_param("canvas_interface",get_canvas_interface());
			action->set_param("value_desc",value_desc);
			action->set_param("type","dynamic_list");
			action->set_param("time",get_canvas_interface()->get_time());
			if(!get_canvas_interface()->get_instance()->perform_action(action))
			{
				group.cancel();
				get_canvas_view()->get_ui_interface()->error("Unable to execute action \"ValueDescConvert\"");
				return;
			}
		}

		// only link the polygon's origin parameter if the option is selected and we're creating more than one layer
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
				get_canvas_view()->get_ui_interface()->error(_("Unable to create Polygon layer"));
				group.cancel();
				throw String(_("Unable to create Polygon layer"));
				return;
			}
		}
	}

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
			get_canvas_view()->get_ui_interface()->error(_("Unable to create layer"));
			group.cancel();
			return;
		}
		layer_selection.push_back(layer);
		layer->set_description(get_id()+_(" Gradient"));
		get_canvas_interface()->signal_layer_new_description()(layer,layer->get_description());

		layer->set_param("blend_method",get_blend());
		get_canvas_interface()->signal_layer_param_changed()(layer,"blend_method");

		layer->set_param("amount",get_opacity());
		get_canvas_interface()->signal_layer_param_changed()(layer,"amount");

		layer->set_param("width",get_bline_width());
		get_canvas_interface()->signal_layer_param_changed()(layer,"width");

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
				return;
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
				group.cancel();
				throw String(_("Unable to create Gradient layer"));
				return;
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
			get_canvas_view()->get_ui_interface()->error(_("Unable to create layer"));
			group.cancel();
			return;
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
				return;
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
				return;
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
			get_canvas_view()->get_ui_interface()->error(_("Unable to create layer"));
			group.cancel();
			return;
		}
		layer_selection.push_back(layer);
		layer->set_description(get_id()+_(" Region"));
		get_canvas_interface()->signal_layer_new_description()(layer,layer->get_description());

		layer->set_param("blend_method",get_blend());
		get_canvas_interface()->signal_layer_param_changed()(layer,"blend_method");

		layer->set_param("amount",get_opacity());
		get_canvas_interface()->signal_layer_param_changed()(layer,"amount");

		layer->set_param("feather",get_feather_size());
		get_canvas_interface()->signal_layer_param_changed()(layer,"feather");

		layer->set_param("invert",get_invert());
		get_canvas_interface()->signal_layer_param_changed()(layer,"invert");

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
				return;
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
				return;
			}
		}
	}

	///////////////////////////////////////////////////////////////////////////
	//   O U T L I N E
	///////////////////////////////////////////////////////////////////////////

	if (get_layer_outline_flag())
	{
		synfigapp::PushMode push_mode(get_canvas_interface(),synfigapp::MODE_NORMAL);

		egress_on_selection_change=false;
		Layer::Handle layer(get_canvas_interface()->add_layer_to("outline",canvas,depth));
		egress_on_selection_change=true;
		if (!layer)
		{
			get_canvas_view()->get_ui_interface()->error(_("Unable to create layer"));
			group.cancel();
			return;
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

		layer->set_param("feather",get_feather_size());
		get_canvas_interface()->signal_layer_param_changed()(layer,"feather");

		layer->set_param("invert",get_invert());
		get_canvas_interface()->signal_layer_param_changed()(layer,"invert");

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
				return;
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
				return;
			}
		}
	}

	///////////////////////////////////////////////////////////////////////////
	//   A D V A N C E D   O U T L I N E
	///////////////////////////////////////////////////////////////////////////

	if (get_layer_advanced_outline_flag())
	{
		synfigapp::PushMode push_mode(get_canvas_interface(),synfigapp::MODE_NORMAL);

		egress_on_selection_change=false;
		Layer::Handle layer(get_canvas_interface()->add_layer_to("advanced_outline",canvas,depth));
		egress_on_selection_change=true;
		if (!layer)
		{
			get_canvas_view()->get_ui_interface()->error(_("Unable to create layer"));
			group.cancel();
			return;
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

		layer->set_param("feather",get_feather_size());
		get_canvas_interface()->signal_layer_param_changed()(layer,"feather");

		layer->set_param("invert",get_invert());
		get_canvas_interface()->signal_layer_param_changed()(layer,"invert");

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
				return;
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
				return;
			}
		}
	}

	egress_on_selection_change=false;
	get_canvas_interface()->get_selection_manager()->clear_selected_layers();
	get_canvas_interface()->get_selection_manager()->set_selected_layers(layer_selection);
	egress_on_selection_change=true;

	//post clean up stuff...
	reset();
	increment_id();
}

Smach::event_result
StatePolygon_Context::event_mouse_click_handler(const Smach::event& x)
{
	const EventMouse& event(*reinterpret_cast<const EventMouse*>(&x));
	switch(event.button)
	{
	case BUTTON_LEFT:
		polygon_point_list.push_back(get_work_area()->snap_point_to_grid(event.pos));
		refresh_ducks();
		return Smach::RESULT_ACCEPT;

	default:
		return Smach::RESULT_OK;
	}
}


void
StatePolygon_Context::refresh_ducks()
{
	get_work_area()->clear_ducks();
	get_work_area()->queue_draw();

	if(polygon_point_list.empty()) return;

	std::list<synfig::Point>::iterator iter=polygon_point_list.begin();

	etl::handle<WorkArea::Duck> duck;
	duck=new WorkArea::Duck(*iter);
	duck->set_editable(true);
	duck->signal_edited().connect(
		sigc::bind(sigc::mem_fun(*this,&studio::StatePolygon_Context::on_polygon_duck_change),iter)
	);
	duck->signal_user_click(0).connect(sigc::mem_fun(*this,&StatePolygon_Context::run));

	get_work_area()->add_duck(duck);

	for(++iter;iter!=polygon_point_list.end();++iter)
	{
		etl::handle<WorkArea::Bezier> bezier(new WorkArea::Bezier());
		bezier->p1=bezier->c1=duck;

		duck=new WorkArea::Duck(*iter);
		duck->set_editable(true);
		duck->set_name(strprintf("%x",&*iter));
		duck->signal_edited().connect(
			sigc::bind(sigc::mem_fun(*this,&studio::StatePolygon_Context::on_polygon_duck_change),iter)
		);

		get_work_area()->add_duck(duck);

		bezier->p2=bezier->c2=duck;
		get_work_area()->add_bezier(bezier);
	}
	get_work_area()->queue_draw();
}


bool
StatePolygon_Context::on_polygon_duck_change(const studio::Duck &duck, std::list<synfig::Point>::iterator iter)
{
	*iter=duck.get_point();
	return true;
}


void
StatePolygon_Context::toggle_layer_creation()
{
  // don't allow none layer creation
  if (get_layer_polygon_flag() +
     get_layer_region_flag() +
     get_layer_outline_flag() +
     get_layer_advanced_outline_flag() +
     get_layer_curve_gradient_flag() +
     get_layer_plant_flag() == 0)
  {
    if(layer_polygon_flag) set_layer_polygon_flag(true);
    else if(layer_region_flag) set_layer_region_flag(true);
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

	// invert
	if (get_layer_polygon_flag() ||
		get_layer_region_flag() ||
		get_layer_outline_flag() ||
		get_layer_advanced_outline_flag())
	{
		invert_box.set_sensitive(true);
	}
	else
		invert_box.set_sensitive(false);

	// feather size
	if (get_layer_polygon_flag() ||
		get_layer_region_flag() ||
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
		get_layer_curve_gradient_flag() +
		get_layer_polygon_flag() >= 2)
		{
			link_origins_box.set_sensitive(true);
		}
	else link_origins_box.set_sensitive(false);

  // update layer flags
  layer_polygon_flag = get_layer_polygon_flag();
  layer_region_flag = get_layer_region_flag();
  layer_outline_flag = get_layer_outline_flag();
  layer_advanced_outline_flag = get_layer_advanced_outline_flag();
  layer_curve_gradient_flag = get_layer_curve_gradient_flag();
  layer_plant_flag = get_layer_plant_flag();
}
