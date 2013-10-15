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

#include <gtkmm/dialog.h>
#include <gtkmm/entry.h>

#include <synfig/valuenode_dynamiclist.h>
#include <synfigapp/action_system.h>
#include <synfig/valuenode_bline.h>

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
#include <gtkmm/optionmenu.h>
#include "duck.h"
#include "widgets/widget_enum.h"
#include <synfigapp/main.h>

#include "general.h"

#endif

/* === U S I N G =========================================================== */

using namespace std;
using namespace etl;
using namespace synfig;
using namespace studio;

/* === M A C R O S ========================================================= */

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

	//Toolbox display
	Gtk::Table options_table;

	Gtk::Entry		entry_id; //what to name the layer

	Gtk::Adjustment	adj_feather;
	Gtk::Adjustment	adj_number_of_points;
	Gtk::Adjustment	adj_inner_tangent;
	Gtk::Adjustment	adj_outer_tangent;
	Gtk::Adjustment	adj_inner_width;
	Gtk::Adjustment	adj_outer_width;
	Gtk::Adjustment	adj_radius_ratio;
	Gtk::Adjustment	adj_angle_offset;
	Gtk::SpinButton	spin_feather;
	Gtk::SpinButton	spin_number_of_points;
	Gtk::SpinButton	spin_inner_tangent;
	Gtk::SpinButton	spin_outer_tangent;
	Gtk::SpinButton	spin_inner_width;
	Gtk::SpinButton	spin_outer_width;
	Gtk::SpinButton	spin_radius_ratio;
	Gtk::SpinButton	spin_angle_offset;

	Gtk::CheckButton checkbutton_invert;
	Gtk::CheckButton checkbutton_regular_polygon;
	Gtk::CheckButton checkbutton_layer_star;
	Gtk::CheckButton checkbutton_layer_region;
	Gtk::CheckButton checkbutton_layer_outline;
	Gtk::CheckButton checkbutton_layer_advanced_outline;
	Gtk::CheckButton checkbutton_layer_curve_gradient;
	Gtk::CheckButton checkbutton_layer_plant;
	Gtk::CheckButton checkbutton_layer_link_origins;
	Gtk::CheckButton checkbutton_layer_origins_at_center;

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

	synfig::String get_id()const { return entry_id.get_text(); }
	void set_id(const synfig::String& x) { return entry_id.set_text(x); }

	Real get_feather()const { return adj_feather.get_value(); }
	void set_feather(Real f) { adj_feather.set_value(f); }

	Real get_number_of_points()const { return adj_number_of_points.get_value(); }
	void set_number_of_points(Real f) { adj_number_of_points.set_value(f); }

	Real get_inner_tangent()const { return adj_inner_tangent.get_value(); }
	void set_inner_tangent(Real f) { adj_inner_tangent.set_value(f); }

	Real get_outer_tangent()const { return adj_outer_tangent.get_value(); }
	void set_outer_tangent(Real f) { adj_outer_tangent.set_value(f); }

	Real get_inner_width()const { return adj_inner_width.get_value(); }
	void set_inner_width(Real f) { adj_inner_width.set_value(f); }

	Real get_outer_width()const { return adj_outer_width.get_value(); }
	void set_outer_width(Real f) { adj_outer_width.set_value(f); }

	Real get_radius_ratio()const { return adj_radius_ratio.get_value(); }
	void set_radius_ratio(Real f) { adj_radius_ratio.set_value(f); }

	Real get_angle_offset()const { return adj_angle_offset.get_value(); }
	void set_angle_offset(Real f) { adj_angle_offset.set_value(f); }

	bool get_invert()const { return checkbutton_invert.get_active(); }
	void set_invert(bool i) { checkbutton_invert.set_active(i); }

	bool get_regular_polygon()const { return checkbutton_regular_polygon.get_active(); }
	void set_regular_polygon(bool i) { checkbutton_regular_polygon.set_active(i); }

	bool get_layer_star_flag()const { return checkbutton_layer_star.get_active(); }
	void set_layer_star_flag(bool x) { return checkbutton_layer_star.set_active(x); }

	bool get_layer_region_flag()const { return checkbutton_layer_region.get_active(); }
	void set_layer_region_flag(bool x) { return checkbutton_layer_region.set_active(x); }

	bool get_layer_outline_flag()const { return checkbutton_layer_outline.get_active(); }
	void set_layer_outline_flag(bool x) { return checkbutton_layer_outline.set_active(x); }

	bool get_layer_advanced_outline_flag()const { return checkbutton_layer_advanced_outline.get_active(); }
	void set_layer_advanced_outline_flag(bool x) { return checkbutton_layer_advanced_outline.set_active(x); }

	bool get_layer_curve_gradient_flag()const { return checkbutton_layer_curve_gradient.get_active(); }
	void set_layer_curve_gradient_flag(bool x) { return checkbutton_layer_curve_gradient.set_active(x); }

	bool get_layer_plant_flag()const { return checkbutton_layer_plant.get_active(); }
	void set_layer_plant_flag(bool x) { return checkbutton_layer_plant.set_active(x); }

	bool get_layer_link_origins_flag()const { return checkbutton_layer_link_origins.get_active(); }
	void set_layer_link_origins_flag(bool x) { return checkbutton_layer_link_origins.set_active(x); }

	bool get_layer_origins_at_center_flag()const { return checkbutton_layer_origins_at_center.get_active(); }
	void set_layer_origins_at_center_flag(bool x) { return checkbutton_layer_origins_at_center.set_active(x); }

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

		if(settings.get_value("star.feather",value))
			set_feather(atof(value.c_str()));
		else
			set_feather(0);

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

		if(settings.get_value("star.angle_offset",value))
			set_angle_offset(atof(value.c_str()));
		else
			set_angle_offset(0);

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
		settings.set_value("star.feather",strprintf("%f",(float)get_feather()));
		settings.set_value("star.number_of_points",strprintf("%d",(int)(get_number_of_points() + 0.5)));
		settings.set_value("star.inner_tangent",strprintf("%f",(float)get_inner_tangent()));
		settings.set_value("star.outer_tangent",strprintf("%f",(float)get_outer_tangent()));
		settings.set_value("star.inner_width",strprintf("%f",(float)get_inner_width()));
		settings.set_value("star.outer_width",strprintf("%f",(float)get_outer_width()));
		settings.set_value("star.radius_ratio",strprintf("%f",(float)get_radius_ratio()));
		settings.set_value("star.angle_offset",strprintf("%f",(float)get_angle_offset()));
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
	entry_id(),		//   value lower upper  step page
	adj_feather(			0,    0,    1, 0.01, 0.1),
	adj_number_of_points(	0,    2,  120, 1   , 1  ),
	adj_inner_tangent(		0,  -10,   10, 0.01, 0.1),
	adj_outer_tangent(		0,  -10,   10, 0.01, 0.1),
	adj_inner_width(		0,  -10,   10, 0.01, 0.1),
	adj_outer_width(		0,  -10,   10, 0.01, 0.1),
	adj_radius_ratio(		0,  -10,   10, 0.01, 0.1),
	adj_angle_offset(		0, -360,  360, 0.1 , 1  ),
	spin_feather(adj_feather,0.1,3),
	spin_number_of_points(adj_number_of_points,1,0),
	spin_inner_tangent(adj_inner_tangent,1,2),
	spin_outer_tangent(adj_outer_tangent,1,2),
	spin_inner_width(adj_inner_width,1,2),
	spin_outer_width(adj_outer_width,1,2),
	spin_radius_ratio(adj_radius_ratio,1,2),
	spin_angle_offset(adj_angle_offset,1,1),
	checkbutton_invert(_("Invert")),
	checkbutton_regular_polygon(_("Regular Polygon")),
	checkbutton_layer_star(_("Create Star Layer")),
	checkbutton_layer_region(_("Create Region")),
	checkbutton_layer_outline(_("Create Outline")),
	checkbutton_layer_advanced_outline(_("Create Advanced Outline")),
	checkbutton_layer_curve_gradient(_("Create Curve Gradient")),
	checkbutton_layer_plant(_("Create Plant")),
	checkbutton_layer_link_origins(_("Link Origins")),
	checkbutton_layer_origins_at_center(_("Spline Origins at Center"))
{
	egress_on_selection_change=true;

	load_settings();

	// Set up the tool options dialog
	options_table.attach(*manage(new Gtk::Label(_("Star Tool"))),			0, 2,  0,  1, Gtk::EXPAND|Gtk::FILL, Gtk::EXPAND|Gtk::FILL, 0, 0);
	options_table.attach(entry_id,											0, 2,  1,  2, Gtk::EXPAND|Gtk::FILL, Gtk::EXPAND|Gtk::FILL, 0, 0);
	options_table.attach(checkbutton_layer_star,							0, 2,  2,  3, Gtk::EXPAND|Gtk::FILL, Gtk::EXPAND|Gtk::FILL, 0, 0);
	options_table.attach(checkbutton_layer_outline,							0, 2,  3,  4, Gtk::EXPAND|Gtk::FILL, Gtk::EXPAND|Gtk::FILL, 0, 0);
	options_table.attach(checkbutton_layer_advanced_outline,				0, 2,  4,  5, Gtk::EXPAND|Gtk::FILL, Gtk::EXPAND|Gtk::FILL, 0, 0);
	options_table.attach(checkbutton_layer_region,							0, 2,  5,  6, Gtk::EXPAND|Gtk::FILL, Gtk::EXPAND|Gtk::FILL, 0, 0);
	options_table.attach(checkbutton_layer_plant,							0, 2,  6,  7, Gtk::EXPAND|Gtk::FILL, Gtk::EXPAND|Gtk::FILL, 0, 0);
	options_table.attach(checkbutton_layer_curve_gradient,					0, 2,  7,  8, Gtk::EXPAND|Gtk::FILL, Gtk::EXPAND|Gtk::FILL, 0, 0);
	options_table.attach(checkbutton_layer_link_origins,					0, 2,  8,  9, Gtk::EXPAND|Gtk::FILL, Gtk::EXPAND|Gtk::FILL, 0, 0);
	options_table.attach(checkbutton_layer_origins_at_center,				0, 2,  9, 10, Gtk::EXPAND|Gtk::FILL, Gtk::EXPAND|Gtk::FILL, 0, 0);
	options_table.attach(checkbutton_invert,								0, 2, 10, 11, Gtk::EXPAND|Gtk::FILL, Gtk::EXPAND|Gtk::FILL, 0, 0);
	options_table.attach(checkbutton_regular_polygon,						0, 2, 11, 12, Gtk::EXPAND|Gtk::FILL, Gtk::EXPAND|Gtk::FILL, 0, 0);

	options_table.attach(*manage(new Gtk::Label(_("Feather:"))),			0, 1, 12, 13, Gtk::EXPAND|Gtk::FILL, Gtk::EXPAND|Gtk::FILL, 0, 0);
	options_table.attach(spin_feather,										1, 2, 12, 13, Gtk::EXPAND|Gtk::FILL, Gtk::EXPAND|Gtk::FILL, 0, 0);

	options_table.attach(*manage(new Gtk::Label(_("Number of Points:"))),	0, 1, 13, 14, Gtk::EXPAND|Gtk::FILL, Gtk::EXPAND|Gtk::FILL, 0, 0);
	options_table.attach(spin_number_of_points,								1, 2, 13, 14, Gtk::EXPAND|Gtk::FILL, Gtk::EXPAND|Gtk::FILL, 0, 0);

	options_table.attach(*manage(new Gtk::Label(_("Inner Tangent:"))),		0, 1, 14, 15, Gtk::EXPAND|Gtk::FILL, Gtk::EXPAND|Gtk::FILL, 0, 0);
	options_table.attach(spin_inner_tangent,								1, 2, 14, 15, Gtk::EXPAND|Gtk::FILL, Gtk::EXPAND|Gtk::FILL, 0, 0);

	options_table.attach(*manage(new Gtk::Label(_("Outer Tangent:"))),		0, 1, 15, 16, Gtk::EXPAND|Gtk::FILL, Gtk::EXPAND|Gtk::FILL, 0, 0);
	options_table.attach(spin_outer_tangent,								1, 2, 16, 17, Gtk::EXPAND|Gtk::FILL, Gtk::EXPAND|Gtk::FILL, 0, 0);

	options_table.attach(*manage(new Gtk::Label(_("Inner Width:"))),		0, 1, 16, 17, Gtk::EXPAND|Gtk::FILL, Gtk::EXPAND|Gtk::FILL, 0, 0);
	options_table.attach(spin_inner_width,									1, 2, 16, 17, Gtk::EXPAND|Gtk::FILL, Gtk::EXPAND|Gtk::FILL, 0, 0);

	options_table.attach(*manage(new Gtk::Label(_("Outer Width:"))),		0, 1, 17, 18, Gtk::EXPAND|Gtk::FILL, Gtk::EXPAND|Gtk::FILL, 0, 0);
	options_table.attach(spin_outer_width,									1, 2, 17, 18, Gtk::EXPAND|Gtk::FILL, Gtk::EXPAND|Gtk::FILL, 0, 0);

	options_table.attach(*manage(new Gtk::Label(_("Radius Ratio:"))),		0, 1, 18, 19, Gtk::EXPAND|Gtk::FILL, Gtk::EXPAND|Gtk::FILL, 0, 0);
	options_table.attach(spin_radius_ratio,									1, 2, 18, 19, Gtk::EXPAND|Gtk::FILL, Gtk::EXPAND|Gtk::FILL, 0, 0);

	options_table.attach(*manage(new Gtk::Label(_("Angle Offset:"))),		0, 1, 19, 20, Gtk::EXPAND|Gtk::FILL, Gtk::EXPAND|Gtk::FILL, 0, 0);
	options_table.attach(spin_angle_offset,									1, 2, 19, 20, Gtk::EXPAND|Gtk::FILL, Gtk::EXPAND|Gtk::FILL, 0, 0);

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
		layer=get_canvas_interface()->add_layer_to("star",canvas,depth);
		if (!layer)
		{
			get_canvas_view()->get_ui_interface()->error(_("Unable to create layer"));
			group.cancel();
			return;
		}
		layer_selection.push_back(layer);

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

		layer->set_param("feather",get_feather());
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

		Layer::Handle layer(get_canvas_interface()->add_layer_to("curve_gradient",canvas,depth));
		if (!layer)
		{
			get_canvas_view()->get_ui_interface()->error(_("Unable to create layer"));
			group.cancel();
			return;
		}
		layer_selection.push_back(layer);
		layer->set_description(get_id()+_(" Gradient"));
		get_canvas_interface()->signal_layer_new_description()(layer,layer->get_description());

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

		Layer::Handle layer(get_canvas_interface()->add_layer_to("plant",canvas,depth));
		if (!layer)
		{
			get_canvas_view()->get_ui_interface()->error(_("Unable to create layer"));
			group.cancel();
			return;
		}
		layer_selection.push_back(layer);
		layer->set_description(get_id()+_(" Plant"));
		get_canvas_interface()->signal_layer_new_description()(layer,layer->get_description());

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

		Layer::Handle layer(get_canvas_interface()->add_layer_to("region",canvas,depth));
		if (!layer)
		{
			get_canvas_view()->get_ui_interface()->error(_("Unable to create layer"));
			group.cancel();
			return;
		}
		layer_selection.push_back(layer);
		layer->set_description(get_id()+_(" Region"));
		get_canvas_interface()->signal_layer_new_description()(layer,layer->get_description());

		layer->set_param("feather",get_feather());
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

		Layer::Handle layer(get_canvas_interface()->add_layer_to("outline",canvas,depth));
		if (!layer)
		{
			get_canvas_view()->get_ui_interface()->error(_("Unable to create layer"));
			group.cancel();
			return;
		}
		layer_selection.push_back(layer);
		layer->set_description(get_id()+_(" Outline"));
		get_canvas_interface()->signal_layer_new_description()(layer,layer->get_description());

		layer->set_param("feather",get_feather());
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

		Layer::Handle layer(get_canvas_interface()->add_layer_to("advanced_outline",canvas,depth));
		if (!layer)
		{
			get_canvas_view()->get_ui_interface()->error(_("Unable to create layer"));
			group.cancel();
			return;
		}
		layer_selection.push_back(layer);
		layer->set_description(get_id()+_(" Advanced Outline"));
		get_canvas_interface()->signal_layer_new_description()(layer,layer->get_description());

		layer->set_param("feather",get_feather());
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
