/* === S Y N F I G ========================================================= */
/*!	\file state_star.cpp
**	\brief Template File
**
**	$Id$
**
**	\legal
**	Copyright (c) 2002-2005 Robert B. Quattlebaum Jr., Adrian Bentley
**	Copyright (c) 2008 Chris Moore
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

#include "state_star.h"
#include "state_normal.h"
#include "canvasview.h"
#include "workarea.h"
#include "app.h"

#include <synfigapp/action.h>
#include "event_mouse.h"
#include "event_layerclick.h"
#include "docks/dock_toolbox.h"
#include "docks/dialog_tooloptions.h"
#include "duck.h"
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
		&studio::StateStar_Context::toggle_layer_creation))
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

StateStar studio::state_star;

/* === C L A S S E S & S T R U C T S ======================================= */

class studio::StateStar_Context : public sigc::trackable
{
	etl::handle<CanvasView> canvas_view_;
	CanvasView::IsWorking is_working;

	Duckmatic::Push duckmatic_push;

	Point point_holder;

	etl::handle<Duck> point2_duck;

	void refresh_ducks();

	bool prev_workarea_layer_status_;

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
	Gtk::ToggleButton layer_star_togglebutton;
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

	// star points
	Gtk::Label number_of_points_label;
	Glib::RefPtr<Gtk::Adjustment> number_of_points_adj;
	Gtk::SpinButton	number_of_points_spin;

	// radius ratio
	Gtk::Label radius_ratio_label;
	Glib::RefPtr<Gtk::Adjustment> radius_ratio_adj;
	Gtk::SpinButton	radius_ratio_spin;

	// angle offset
	Gtk::Label angle_offset_label;
	Glib::RefPtr<Gtk::Adjustment> angle_offset_adj;
	Gtk::SpinButton	angle_offset_spin;

	// regular polygon
	Gtk::Label regular_polygon_label;
	Gtk::CheckButton regular_polygon_checkbutton;
	Gtk::HBox regular_polygon_box;

	// inner width
	Gtk::Label outer_width_label;
	Glib::RefPtr<Gtk::Adjustment> outer_width_adj;
	Gtk::SpinButton	outer_width_spin;

	// inner tangent
	Gtk::Label inner_tangent_label;
	Glib::RefPtr<Gtk::Adjustment> inner_tangent_adj;
	Gtk::SpinButton	inner_tangent_spin;

	// outer width
	Gtk::Label inner_width_label;
	Glib::RefPtr<Gtk::Adjustment> inner_width_adj;
	Gtk::SpinButton	inner_width_spin;

	// outer tangent
	Gtk::Label outer_tangent_label;
	Glib::RefPtr<Gtk::Adjustment> outer_tangent_adj;
	Gtk::SpinButton	outer_tangent_spin;

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

	// this only counts the layers which will have their origins linked
	int layers_to_create()const
	{
		return
			(get_layer_star_flag() && get_layer_origins_at_center_flag()) +
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

	Real get_number_of_points()const { return number_of_points_adj->get_value(); }
	void set_number_of_points(Real f) { number_of_points_adj->set_value(f); }

	Real get_inner_tangent()const { return inner_tangent_adj->get_value(); }
	void set_inner_tangent(Real f) { inner_tangent_adj->set_value(f); }

	Real get_outer_tangent()const { return outer_tangent_adj->get_value(); }
	void set_outer_tangent(Real f) { outer_tangent_adj->set_value(f); }

	Real get_inner_width()const { return inner_width_adj->get_value(); }
	void set_inner_width(Real f) { inner_width_adj->set_value(f); }

	Real get_outer_width()const { return outer_width_adj->get_value(); }
	void set_outer_width(Real f) { outer_width_adj->set_value(f); }

	Real get_radius_ratio()const { return radius_ratio_adj->get_value(); }
	void set_radius_ratio(Real f) { radius_ratio_adj->set_value(f); }

	Real get_angle_offset()const { return angle_offset_adj->get_value(); }
	void set_angle_offset(Real f) { angle_offset_adj->set_value(f); }

	bool get_invert()const { return invert_checkbutton.get_active(); }
	void set_invert(bool i) { invert_checkbutton.set_active(i); }

	bool get_regular_polygon()const { return regular_polygon_checkbutton.get_active(); }
	void set_regular_polygon(bool i) { regular_polygon_checkbutton.set_active(i); }

	bool get_layer_star_flag()const { return layer_star_togglebutton.get_active(); }
	void set_layer_star_flag(bool x) { return layer_star_togglebutton.set_active(x); }

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

	bool get_layer_origins_at_center_flag()const { return layer_origins_at_center_checkbutton.get_active(); }
	void set_layer_origins_at_center_flag(bool x) { return layer_origins_at_center_checkbutton.set_active(x); }

  bool layer_star_flag;
  bool layer_region_flag;
  bool layer_outline_flag;
  bool layer_advanced_outline_flag;
  bool layer_curve_gradient_flag;
  bool layer_plant_flag;

	void refresh_tool_options(); //to refresh the toolbox

	//events
	Smach::event_result event_stop_handler(const Smach::event& x);
	Smach::event_result event_refresh_handler(const Smach::event& x);
	Smach::event_result event_mouse_click_handler(const Smach::event& x);
	Smach::event_result event_refresh_tool_options(const Smach::event& x);

	//constructor destructor
	StateStar_Context(CanvasView* canvas_view);
	~StateStar_Context();

	//Canvas interaction
	const etl::handle<CanvasView>& get_canvas_view()const{return canvas_view_;}
	etl::handle<synfigapp::CanvasInterface> get_canvas_interface()const{return canvas_view_->canvas_interface();}
	synfig::Canvas::Handle get_canvas()const{return canvas_view_->get_canvas();}
	WorkArea * get_work_area()const{return canvas_view_->get_work_area();}

	//Modifying settings etc.
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

	void make_star(const Point& p1, const Point& p2);

	void toggle_layer_creation();

};	// END of class StateStar_Context

/* === M E T H O D S ======================================================= */

StateStar::StateStar():
	Smach::state<StateStar_Context>("star")
{
	insert(event_def(EVENT_LAYER_SELECTION_CHANGED,&StateStar_Context::event_layer_selection_changed_handler));
	insert(event_def(EVENT_STOP,&StateStar_Context::event_stop_handler));
	insert(event_def(EVENT_REFRESH,&StateStar_Context::event_refresh_handler));
	insert(event_def(EVENT_REFRESH_DUCKS,&StateStar_Context::event_refresh_handler));
	insert(event_def(EVENT_WORKAREA_MOUSE_BUTTON_DOWN,&StateStar_Context::event_mouse_click_handler));
	insert(event_def(EVENT_WORKAREA_MOUSE_BUTTON_DRAG,&StateStar_Context::event_mouse_click_handler));
	insert(event_def(EVENT_WORKAREA_MOUSE_BUTTON_UP,&StateStar_Context::event_mouse_click_handler));
	insert(event_def(EVENT_REFRESH_TOOL_OPTIONS,&StateStar_Context::event_refresh_tool_options));
}

StateStar::~StateStar()
{
}

void
StateStar_Context::load_settings()
{
	try
	{
		synfig::ChangeLocale change_locale(LC_NUMERIC, "C");
		String value;

		//parse the arguments yargh!
		if(settings.get_value("star.id",value))
			set_id(value);
		else
			set_id("Star");

		if(settings.get_value("star.blend",value) && value != "")
			set_blend(atoi(value.c_str()));
		else
			set_blend(0);//(int)Color::BLEND_COMPOSITE); //0 should be blend composites value

		if(settings.get_value("star.opacity",value))
			set_opacity(atof(value.c_str()));
		else
			set_opacity(1);

		if(settings.get_value("star.bline_width",value) && value != "")
			set_bline_width(Distance(atof(value.c_str()), App::distance_system));
		else
			set_bline_width(Distance(1, App::distance_system)); // default width

		if(settings.get_value("star.feather",value))
			set_feather_size(Distance(atof(value.c_str()), App::distance_system));
		else
			set_feather_size(Distance(0, App::distance_system)); // default feather

		if(settings.get_value("star.number_of_points",value))
			set_number_of_points(atof(value.c_str()));
		else
			set_number_of_points(5);

		if(settings.get_value("star.inner_tangent",value))
			set_inner_tangent(atof(value.c_str()));
		else
			set_inner_tangent(0);

		if(settings.get_value("star.outer_tangent",value))
			set_outer_tangent(atof(value.c_str()));
		else
			set_outer_tangent(0);

		if(settings.get_value("star.inner_width",value))
			set_inner_width(atof(value.c_str()));
		else
			set_inner_width(1);

		if(settings.get_value("star.outer_width",value))
			set_outer_width(atof(value.c_str()));
		else
			set_outer_width(1);

		if(settings.get_value("star.radius_ratio",value))
			set_radius_ratio(atof(value.c_str()));
		else
			set_radius_ratio(0.5);

		//if(settings.get_value("star.angle_offset",value))
		//	set_angle_offset(atof(value.c_str()));
		//else
			set_angle_offset(90.0);

		if(settings.get_value("star.invert",value) && value != "0")
			set_invert(true);
		else
			set_invert(false);

		if(settings.get_value("star.regular_polygon",value) && value != "0")
			set_regular_polygon(true);
		else
			set_regular_polygon(false);

		if(settings.get_value("star.layer_star",value) && value=="0")
			set_layer_star_flag(false);
		else
			set_layer_star_flag(true);

		if(settings.get_value("star.layer_region",value) && value=="1")
			set_layer_region_flag(true);
		else
			set_layer_region_flag(false);

		if(settings.get_value("star.layer_outline",value) && value=="1")
			set_layer_outline_flag(true);
		else
			set_layer_outline_flag(false);

		if(settings.get_value("star.layer_advanced_outline",value) && value=="1")
			set_layer_advanced_outline_flag(true);
		else
			set_layer_advanced_outline_flag(false);

		if(settings.get_value("star.layer_curve_gradient",value) && value=="1")
			set_layer_curve_gradient_flag(true);
		else
			set_layer_curve_gradient_flag(false);

		if(settings.get_value("star.layer_plant",value) && value=="1")
			set_layer_plant_flag(true);
		else
			set_layer_plant_flag(false);

		if(settings.get_value("star.layer_link_origins",value) && value=="0")
			set_layer_link_origins_flag(false);
		else
			set_layer_link_origins_flag(true);

		if(settings.get_value("star.layer_origins_at_center",value) && value=="0")
			set_layer_origins_at_center_flag(false);
		else
			set_layer_origins_at_center_flag(true);

	  // determine layer flags
		layer_star_flag = get_layer_star_flag();
	  layer_region_flag = get_layer_region_flag();
	  layer_outline_flag = get_layer_outline_flag();
	  layer_advanced_outline_flag = get_layer_outline_flag();
	  layer_curve_gradient_flag = get_layer_curve_gradient_flag();
	  layer_plant_flag = get_layer_plant_flag();
	}
	catch(...)
	{
		synfig::warning("State Star: Caught exception when attempting to load settings.");
	}
}

void
StateStar_Context::save_settings()
{
	try
	{
		synfig::ChangeLocale change_locale(LC_NUMERIC, "C");
		settings.set_value("star.id",get_id());
		settings.set_value("star.blend",strprintf("%d",get_blend()));
		settings.set_value("star.opacity",strprintf("%f",(float)get_opacity()));
		settings.set_value("star.bline_width", bline_width_dist.get_value().get_string());
		settings.set_value("star.feather", feather_dist.get_value().get_string());
		settings.set_value("star.number_of_points",strprintf("%d",(int)(get_number_of_points() + 0.5)));
		settings.set_value("star.inner_tangent",strprintf("%f",(float)get_inner_tangent()));
		settings.set_value("star.outer_tangent",strprintf("%f",(float)get_outer_tangent()));
		settings.set_value("star.inner_width",strprintf("%f",(float)get_inner_width()));
		settings.set_value("star.outer_width",strprintf("%f",(float)get_outer_width()));
		settings.set_value("star.radius_ratio",strprintf("%f",(float)get_radius_ratio()));
		//settings.set_value("star.angle_offset",strprintf("%f",(float)get_angle_offset()));
		settings.set_value("star.invert",get_invert()?"1":"0");
		settings.set_value("star.regular_polygon",get_regular_polygon()?"1":"0");
		settings.set_value("star.layer_star",get_layer_star_flag()?"1":"0");
		settings.set_value("star.layer_outline",get_layer_outline_flag()?"1":"0");
		settings.set_value("star.layer_advanced_outline",get_layer_advanced_outline_flag()?"1":"0");
		settings.set_value("star.layer_region",get_layer_region_flag()?"1":"0");
		settings.set_value("star.layer_curve_gradient",get_layer_curve_gradient_flag()?"1":"0");
		settings.set_value("star.layer_plant",get_layer_plant_flag()?"1":"0");
		settings.set_value("star.layer_link_origins",get_layer_link_origins_flag()?"1":"0");
		settings.set_value("star.layer_origins_at_center",get_layer_origins_at_center_flag()?"1":"0");
	}
	catch(...)
	{
		synfig::warning("State Star: Caught exception when attempting to save settings.");
	}
}

void
StateStar_Context::reset()
{
	refresh_ducks();
}

void
StateStar_Context::increment_id()
{
	String id(get_id());
	int number=1;
	int digits=0;

	if(id.empty())
		id="Star";

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

StateStar_Context::StateStar_Context(CanvasView* canvas_view):
	canvas_view_(canvas_view),
	is_working(*canvas_view),
	duckmatic_push(get_work_area()),
	prev_workarea_layer_status_(get_work_area()->get_allow_layer_clicks()),
	settings(synfigapp::Main::get_selected_input_device()->settings()),
	opacity_hscl(0.0f, 1.0125f, 0.0125f),
	number_of_points_adj(Gtk::Adjustment::create(0, 2, 120, 1, 1)),
	number_of_points_spin(number_of_points_adj,1,0),
	radius_ratio_adj(Gtk::Adjustment::create(0, -10, 10, 0.01, 0.1)),
	radius_ratio_spin(radius_ratio_adj,1,2),
	angle_offset_adj(Gtk::Adjustment::create(0, -360, 360, 0.1, 1)),
	angle_offset_spin(angle_offset_adj,1,1),
	outer_width_adj(Gtk::Adjustment::create(0, -10, 10, 0.01, 0.1)),
	outer_width_spin(outer_width_adj,1,2),
	inner_tangent_adj(Gtk::Adjustment::create(0,-10, 10, 0.01, 0.1)),
	inner_tangent_spin(inner_tangent_adj,1,2),
	inner_width_adj(Gtk::Adjustment::create(0, -10, 10, 0.01, 0.1)),
	inner_width_spin(inner_width_adj,1,2),
	outer_tangent_adj(Gtk::Adjustment::create(0,-10, 10, 0.01, 0.1)),
	outer_tangent_spin(outer_tangent_adj,1,2)
{
	egress_on_selection_change=true;

	/* Set up the tool options dialog */

	// 0, title
	title_label.set_label(_("Star Creation"));
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

	LAYER_CREATION(layer_star_togglebutton,
		("synfig-layer_geometry_star"), _("Create a star layer"));

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
	layer_types_box.pack_start(layer_star_togglebutton, Gtk::PACK_SHRINK);
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
		.set_description(_("Defines the blend method to be used for stars")));

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

	// 6, star points
	number_of_points_label.set_label(_("Star Points:"));
	number_of_points_label.set_alignment(Gtk::ALIGN_START, Gtk::ALIGN_CENTER);

	// 7, angle offset
	angle_offset_label.set_label(_("Offset:"));
	angle_offset_label.set_alignment(Gtk::ALIGN_START, Gtk::ALIGN_CENTER);

	// 8, radius ratio
	radius_ratio_label.set_label(_("Radius Ratio:"));
	radius_ratio_label.set_alignment(Gtk::ALIGN_START, Gtk::ALIGN_CENTER);

	// 9, regular polygon
	regular_polygon_label.set_label(_("Regular Polygon"));
	regular_polygon_label.set_alignment(Gtk::ALIGN_START, Gtk::ALIGN_CENTER);

	regular_polygon_box.pack_start(regular_polygon_label);
	regular_polygon_box.pack_end(regular_polygon_checkbutton, Gtk::PACK_SHRINK);

	// 10, inner width
	inner_width_label.set_label(_("Inner Width:"));
	inner_width_label.set_alignment(Gtk::ALIGN_START, Gtk::ALIGN_CENTER);

	// 11, inner tangent
	inner_tangent_label.set_label(_("Inner Tangent:"));
	inner_tangent_label.set_alignment(Gtk::ALIGN_START, Gtk::ALIGN_CENTER);

	// 12, outer width
	outer_width_label.set_label(_("Outer Width:"));
	outer_width_label.set_alignment(Gtk::ALIGN_START, Gtk::ALIGN_CENTER);

	// 13, outer tangent
	outer_tangent_label.set_label(_("Outer Tangent:"));
	outer_tangent_label.set_alignment(Gtk::ALIGN_START, Gtk::ALIGN_CENTER);

	// 14, invert
	invert_label.set_label(_("Invert"));
	invert_label.set_alignment(Gtk::ALIGN_START, Gtk::ALIGN_CENTER);

	invert_box.pack_start(invert_label);
	invert_box.pack_end(invert_checkbutton, Gtk::PACK_SHRINK);
	invert_box.set_sensitive(false);

	// 15, feather
	feather_label.set_label(_("Feather:"));
	feather_label.set_alignment(Gtk::ALIGN_START, Gtk::ALIGN_CENTER);
	feather_label.set_sensitive(false);

	feather_dist.set_digits(2);
	feather_dist.set_range(0,10000000);
	feather_dist.set_sensitive(false);

	// 16, link origins
	link_origins_label.set_label(_("Link Origins"));
	link_origins_label.set_alignment(Gtk::ALIGN_START, Gtk::ALIGN_CENTER);

	link_origins_box.pack_start(link_origins_label);
	link_origins_box.pack_end(layer_link_origins_checkbutton, Gtk::PACK_SHRINK);
	link_origins_box.set_sensitive(false);

	// 17, spline origins at center
	origins_at_center_label.set_label(_("Spline Origins at Center"));
	origins_at_center_label.set_alignment(Gtk::ALIGN_START, Gtk::ALIGN_CENTER);

	origins_at_center_box.pack_start(origins_at_center_label);
	origins_at_center_box.pack_end(layer_origins_at_center_checkbutton, Gtk::PACK_SHRINK);
	origins_at_center_box.set_sensitive(false);

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
	// 6, star points
	options_table.attach(number_of_points_label,
		0, 1, 7, 8, Gtk::EXPAND|Gtk::FILL, Gtk::FILL, 0, 0
		);
	options_table.attach(number_of_points_spin,
		1, 2, 7, 8, Gtk::EXPAND|Gtk::FILL, Gtk::FILL, 0, 0
		);
	// 7, star points offset
	options_table.attach(angle_offset_label,
		0, 1, 8, 9, Gtk::EXPAND|Gtk::FILL, Gtk::FILL, 0, 0
		);
	options_table.attach(angle_offset_spin,
		1, 2, 8, 9, Gtk::EXPAND|Gtk::FILL, Gtk::FILL, 0, 0
		);
	// 8, radius ratio
	options_table.attach(radius_ratio_label,
		0, 1, 9, 10, Gtk::EXPAND|Gtk::FILL, Gtk::FILL, 0, 0
		);
	options_table.attach(radius_ratio_spin,
		1, 2, 9, 10, Gtk::EXPAND|Gtk::FILL, Gtk::FILL, 0, 0
		);
	// 9, regular polygon
	options_table.attach(regular_polygon_box,
		0, 2, 10, 11, Gtk::EXPAND|Gtk::FILL, Gtk::FILL, 0, 0
		);
	// 10, inner width
	options_table.attach(inner_width_label,
		0, 1, 11, 12, Gtk::EXPAND|Gtk::FILL, Gtk::FILL, 0, 0
		);
	options_table.attach(inner_width_spin,
		1, 2, 11, 12, Gtk::EXPAND|Gtk::FILL, Gtk::FILL, 0, 0
		);
	// 11, inner tangent
	options_table.attach(inner_tangent_label,
		0, 1, 12, 13, Gtk::EXPAND|Gtk::FILL, Gtk::FILL, 0, 0
		);
	options_table.attach(inner_tangent_spin,
		1, 2, 12, 13, Gtk::EXPAND|Gtk::FILL, Gtk::FILL, 0, 0
		);
	// 12, outer width
	options_table.attach(outer_width_label,
		0, 1, 13, 14, Gtk::EXPAND|Gtk::FILL, Gtk::FILL, 0, 0
		);
	options_table.attach(outer_width_spin,
		1, 2, 13, 14, Gtk::EXPAND|Gtk::FILL, Gtk::FILL, 0, 0
		);
	// 13, outer tangent
	options_table.attach(outer_tangent_label,
		0, 1, 14, 15, Gtk::EXPAND|Gtk::FILL, Gtk::FILL, 0, 0
		);
	options_table.attach(outer_tangent_spin,
		1, 2, 14, 15, Gtk::EXPAND|Gtk::FILL, Gtk::FILL, 0, 0
		);
	// 14, invert
	options_table.attach(invert_box,
		0, 2, 15, 16, Gtk::EXPAND|Gtk::FILL, Gtk::FILL, 0, 0
		);
	// 15, feather
	options_table.attach(feather_label,
		0, 1, 16, 17, Gtk::EXPAND|Gtk::FILL, Gtk::FILL, 0, 0
		);
	options_table.attach(feather_dist,
		1, 2, 16, 17, Gtk::EXPAND|Gtk::FILL, Gtk::FILL, 0, 0
		);
	// 16, link origins
	options_table.attach(link_origins_box,
		0, 2, 17, 18, Gtk::FILL, Gtk::FILL, 0, 0
		);
	// 17, origins at center
	options_table.attach(origins_at_center_box,
		0, 2, 18, 19, Gtk::FILL, Gtk::FILL, 0, 0
		);

	// fine-tune options layout
	options_table.set_border_width(GAP*2); // border width
	options_table.set_row_spacings(GAP); // row gap
	options_table.set_row_spacing(0, GAP*2); // the gap between first and second row.
	options_table.set_row_spacing(2, 1); // row gap between label and icon of layer type
	//options_table.set_row_spacing(19, 0); // the final row using border width of table
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

	get_work_area()->set_cursor(Gdk::STAR);

	App::dock_toolbox->refresh();
}

void
StateStar_Context::refresh_tool_options()
{
	App::dialog_tool_options->clear();
	App::dialog_tool_options->set_widget(options_table);
	App::dialog_tool_options->set_local_name(_("Star Tool"));
	App::dialog_tool_options->set_name("star");
}

Smach::event_result
StateStar_Context::event_refresh_tool_options(const Smach::event& /*x*/)
{
	refresh_tool_options();
	return Smach::RESULT_ACCEPT;
}

StateStar_Context::~StateStar_Context()
{
	save_settings();

	// Restore layer clicking
	get_work_area()->set_allow_layer_clicks(prev_workarea_layer_status_);
	get_work_area()->reset_cursor();

	App::dialog_tool_options->clear();

	// Refresh the work area
	get_work_area()->queue_draw();

	get_canvas_view()->queue_rebuild_ducks();

	App::dock_toolbox->refresh();
}

Smach::event_result
StateStar_Context::event_stop_handler(const Smach::event& /*x*/)
{
	throw &state_normal;
	return Smach::RESULT_OK;
}

Smach::event_result
StateStar_Context::event_refresh_handler(const Smach::event& /*x*/)
{
	refresh_ducks();
	return Smach::RESULT_ACCEPT;
}

void
StateStar_Context::make_star(const Point& _p1, const Point& _p2)
{
	synfigapp::Action::PassiveGrouper group(get_canvas_interface()->get_instance().get(),_("New Star"));
	synfigapp::PushMode push_mode(get_canvas_interface(),synfigapp::MODE_NORMAL);

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
	const Point p1(transform.unperform(_p1));
	const Point p2(transform.unperform(_p2));

	Real radius_ratio(get_radius_ratio());
	Real radius1((p2-p1).mag());
	Real radius2(radius1 * radius_ratio);
	int points = get_number_of_points();
	Real inner_tangent = get_inner_tangent() * radius1;
	Real outer_tangent = get_outer_tangent() * radius2;
	Real inner_width = get_inner_width();
	Real outer_width = get_outer_width();
	Angle::deg offset(get_angle_offset());
	bool regular(get_regular_polygon());
	Angle::deg angle(360.0/points);
	Vector origin;
	Real x, y;

	if (get_layer_origins_at_center_flag())
	{
		x = y = 0;
		origin = p1;
	}
	else
	{
		x = p1[0];
		y = p1[1];
	}

	std::vector<BLinePoint> new_list;
	int point(0);
	for (int i = 0; i < points; i++)
	{
		new_list.push_back(*(new BLinePoint));
		new_list[point].set_width(outer_width);
		new_list[point].set_vertex(Point(radius1*Angle::cos(angle*i + offset).get() + x,
										 radius1*Angle::sin(angle*i + offset).get() + y));
		new_list[point++].set_tangent(Point(-Angle::sin(angle*i + offset).get(),
											 Angle::cos(angle*i + offset).get()) * outer_tangent);

		if (!regular)
		{
			new_list.push_back(*(new BLinePoint));
			new_list[point].set_width(inner_width);
			new_list[point].set_vertex(Point(radius2*Angle::cos(angle*i + angle/2 + offset).get() + x,
											 radius2*Angle::sin(angle*i + angle/2 + offset).get() + y));
			new_list[point++].set_tangent(Point(-Angle::sin(angle*i + angle/2 + offset).get(),
												 Angle::cos(angle*i + angle/2 + offset).get()) * inner_tangent);
		}
	}

	ValueNode_BLine::Handle value_node_bline(ValueNode_BLine::create(new_list));
	assert(value_node_bline);

	ValueNode::Handle value_node_origin(ValueNode_Const::create(origin));
	assert(value_node_origin);

	// Set the looping flag
	value_node_bline->set_loop(true);

	if(!canvas)
		canvas=get_canvas_view()->get_canvas();

	value_node_bline->set_member_canvas(canvas);

	// count how many layers we're going to be creating
	int layers_to_create = this->layers_to_create();

	///////////////////////////////////////////////////////////////////////////
	//   S T A R
	///////////////////////////////////////////////////////////////////////////

	if (get_layer_star_flag())
	{
		egress_on_selection_change=false;
		layer=get_canvas_interface()->add_layer_to("star",canvas,depth);
		egress_on_selection_change=true;
		if (!layer)
		{
			get_canvas_view()->get_ui_interface()->error(_("Unable to create layer"));
			group.cancel();
			return;
		}
		layer_selection.push_back(layer);

		layer->set_param("amount",get_opacity());
		get_canvas_interface()->signal_layer_param_changed()(layer,"amount");

		layer->set_param("blend_method",get_blend());
		get_canvas_interface()->signal_layer_param_changed()(layer,"blend_method");

		layer->set_param("radius1",radius1);
		get_canvas_interface()->signal_layer_param_changed()(layer,"radius1");

		layer->set_param("radius2",radius2);
		get_canvas_interface()->signal_layer_param_changed()(layer,"radius2");

		layer->set_param("angle",offset);
		get_canvas_interface()->signal_layer_param_changed()(layer,"angle");

		layer->set_param("points",points);
		get_canvas_interface()->signal_layer_param_changed()(layer,"points");

		layer->set_param("regular_polygon",regular);
		get_canvas_interface()->signal_layer_param_changed()(layer,"regular_polygon");

		layer->set_param("feather",get_feather_size());
		get_canvas_interface()->signal_layer_param_changed()(layer,"feather");

		layer->set_param("invert",get_invert());
		get_canvas_interface()->signal_layer_param_changed()(layer,"invert");

		layer->set_description(get_id());
		get_canvas_interface()->signal_layer_new_description()(layer,layer->get_description());

		// only link the star's origin parameter if the option is selected, we're putting bline
		// origins at their centers, and we're creating more than one layer
		if (get_layer_link_origins_flag() && get_layer_origins_at_center_flag() && layers_to_create > 1)
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
				throw String(_("Unable to create Star layer"));
				return;
			}
		}
		else
		{
			layer->set_param("origin",p1);
			get_canvas_interface()->signal_layer_param_changed()(layer,"origin");
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

		layer->set_param("amount",get_opacity());
		get_canvas_interface()->signal_layer_param_changed()(layer,"amount");

		layer->set_param("blend_method",get_blend());
		get_canvas_interface()->signal_layer_param_changed()(layer,"blend_method");

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
				//get_canvas_view()->get_ui_interface()->error(_("Unable to create BLine layer"));
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
		else
		{
			layer->set_param("origin",origin);
			get_canvas_interface()->signal_layer_param_changed()(layer,"origin");
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

		layer->set_param("amount",get_opacity());
		get_canvas_interface()->signal_layer_param_changed()(layer,"amount");

		layer->set_param("blend_method",get_blend());
		get_canvas_interface()->signal_layer_param_changed()(layer,"blend_method");

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
		else
		{
			layer->set_param("origin",origin);
			get_canvas_interface()->signal_layer_param_changed()(layer,"origin");
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

		layer->set_param("amount",get_opacity());
		get_canvas_interface()->signal_layer_param_changed()(layer,"amount");

		layer->set_param("blend_method",get_blend());
		get_canvas_interface()->signal_layer_param_changed()(layer,"blend_method");

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
		else
		{
			layer->set_param("origin",origin);
			get_canvas_interface()->signal_layer_param_changed()(layer,"origin");
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

		layer->set_param("amount",get_opacity());
		get_canvas_interface()->signal_layer_param_changed()(layer,"amount");

		layer->set_param("blend_method",get_blend());
		get_canvas_interface()->signal_layer_param_changed()(layer,"blend_method");

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
		else
		{
			layer->set_param("origin",origin);
			get_canvas_interface()->signal_layer_param_changed()(layer,"origin");
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

		layer->set_param("amount",get_opacity());
		get_canvas_interface()->signal_layer_param_changed()(layer,"amount");

		layer->set_param("blend_method",get_blend());
		get_canvas_interface()->signal_layer_param_changed()(layer,"blend_method");

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

		// only link the advanced_outline's origin parameter if the option is selected and we're creating more than one layer
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
		else
		{
			layer->set_param("origin",origin);
			get_canvas_interface()->signal_layer_param_changed()(layer,"origin");
		}
	}
	egress_on_selection_change=false;
	get_canvas_interface()->get_selection_manager()->clear_selected_layers();
	get_canvas_interface()->get_selection_manager()->set_selected_layers(layer_selection);
	egress_on_selection_change=true;

	reset();
	increment_id();
}

Smach::event_result
StateStar_Context::event_mouse_click_handler(const Smach::event& x)
{
	const EventMouse& event(*reinterpret_cast<const EventMouse*>(&x));

	if(event.key==EVENT_WORKAREA_MOUSE_BUTTON_DOWN && event.button==BUTTON_LEFT)
	{
		point_holder=get_work_area()->snap_point_to_grid(event.pos);
		etl::handle<Duck> duck=new Duck();
		duck->set_point(point_holder);
		duck->set_name("p1");
		duck->set_type(Duck::TYPE_POSITION);
		duck->set_editable(false);
		get_work_area()->add_duck(duck);

		point2_duck=new Duck();
		point2_duck->set_point(Vector(0,0));
		point2_duck->set_name("radius");
		point2_duck->set_origin(duck);
		point2_duck->set_radius(true);
		point2_duck->set_scalar(-1);
		point2_duck->set_type(Duck::TYPE_RADIUS);
		point2_duck->set_hover(true);
		get_work_area()->add_duck(point2_duck);

		return Smach::RESULT_ACCEPT;
	}

	if(event.key==EVENT_WORKAREA_MOUSE_BUTTON_DRAG && event.button==BUTTON_LEFT)
	{
		if (!point2_duck) return Smach::RESULT_OK;
		point2_duck->set_point(point_holder-get_work_area()->snap_point_to_grid(event.pos));
		get_work_area()->queue_draw();
		return Smach::RESULT_ACCEPT;
	}

	if(event.key==EVENT_WORKAREA_MOUSE_BUTTON_UP && event.button==BUTTON_LEFT)
	{
		Point point(get_work_area()->snap_point_to_grid(event.pos));

		if (App::restrict_radius_ducks)
		{
			if ((point[0] - point_holder[0]) < 0) point[0] = point_holder[0];
			if ((point[1] - point_holder[1]) < 0) point[1] = point_holder[1];
		}

		make_star(point_holder, point);
		get_work_area()->clear_ducks();
		return Smach::RESULT_ACCEPT;
	}

	return Smach::RESULT_OK;
}


void
StateStar_Context::refresh_ducks()
{
	get_work_area()->clear_ducks();
	get_work_area()->queue_draw();
}

void
StateStar_Context::toggle_layer_creation()
{
	// don't allow none layer creation
  if (get_layer_star_flag() +
     get_layer_region_flag() +
     get_layer_outline_flag() +
     get_layer_advanced_outline_flag() +
     get_layer_curve_gradient_flag() +
     get_layer_plant_flag() == 0)
  {
    if(layer_star_flag) set_layer_star_flag(true);
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

	// inner/outer width and tangent
	if (get_layer_region_flag() ||
		get_layer_outline_flag() ||
		get_layer_advanced_outline_flag() ||
		get_layer_plant_flag() ||
		get_layer_curve_gradient_flag())
	{
		inner_width_label.set_sensitive(true);
		inner_width_spin.set_sensitive(true);
		inner_tangent_label.set_sensitive(true);
		inner_tangent_spin.set_sensitive(true);
		outer_width_label.set_sensitive(true);
		outer_width_spin.set_sensitive(true);
		outer_tangent_label.set_sensitive(true);
		outer_tangent_spin.set_sensitive(true);
	}
	else
	{
		inner_width_label.set_sensitive(false);
		inner_width_spin.set_sensitive(false);
		inner_tangent_label.set_sensitive(false);
		inner_tangent_spin.set_sensitive(false);
		outer_width_label.set_sensitive(false);
		outer_width_spin.set_sensitive(false);
		outer_tangent_label.set_sensitive(false);
		outer_tangent_spin.set_sensitive(false);
	}

	// invert
	if (get_layer_star_flag() ||
		get_layer_region_flag() ||
		get_layer_outline_flag() ||
		get_layer_advanced_outline_flag())
	{
		invert_box.set_sensitive(true);
	}
	else
		invert_box.set_sensitive(false);

	// feather size
	if (get_layer_star_flag() ||
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

	// orignis at center
	if (get_layer_region_flag() ||
		get_layer_outline_flag() ||
		get_layer_advanced_outline_flag() ||
		get_layer_plant_flag() ||
		get_layer_curve_gradient_flag())
	{
		origins_at_center_box.set_sensitive(true);
	}
	else
		origins_at_center_box.set_sensitive(false);

	// link origins
	if (get_layer_region_flag() +
		get_layer_outline_flag() +
		get_layer_advanced_outline_flag() +
		get_layer_plant_flag() +
		get_layer_curve_gradient_flag() +
		get_layer_star_flag() >= 2)
	{
		link_origins_box.set_sensitive(true);
	}
	else link_origins_box.set_sensitive(false);

  // update layer flags
  layer_star_flag = get_layer_star_flag();
  layer_region_flag = get_layer_region_flag();
  layer_outline_flag = get_layer_outline_flag();
  layer_advanced_outline_flag = get_layer_advanced_outline_flag();
  layer_curve_gradient_flag = get_layer_curve_gradient_flag();
  layer_plant_flag = get_layer_plant_flag();
}
