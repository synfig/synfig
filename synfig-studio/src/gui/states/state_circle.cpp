/* === S Y N F I G ========================================================= */
/*!	\file state_circle.cpp
**	\brief Template File
**
**	$Id$
**
**	\legal
**	Copyright (c) 2002-2005 Robert B. Quattlebaum Jr., Adrian Bentley
**	Copyright (c) 2008 Chris Moore
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

#include "state_circle.h"
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
#include "widgets/widget_distance.h"
#include <synfigapp/main.h>

#include "general.h"

#endif

/* === U S I N G =========================================================== */

using namespace std;
using namespace etl;
using namespace synfig;
using namespace studio;

/* === M A C R O S ========================================================= */
enum CircleFalloff
{
	CIRCLE_SQUARED	=0,
	CIRCLE_INTERPOLATION_LINEAR	=1,
	CIRCLE_COSINE	=2,
	CIRCLE_SIGMOND	=3,
	CIRCLE_SQRT		=4,
	CIRCLE_NUM_FALLOFF
};

/* === G L O B A L S ======================================================= */

StateCircle studio::state_circle;

/* === C L A S S E S & S T R U C T S ======================================= */

class studio::StateCircle_Context : public sigc::trackable
{
	etl::handle<CanvasView> canvas_view_;
	CanvasView::IsWorking is_working;

	Duckmatic::Push duckmatic_push;

	Point point_holder;

	etl::handle<Duck> point2_duck;

	void refresh_ducks();
	void on_opacity_changed();
	void opacity_refresh();
	void on_bline_width_changed();
	void bline_width_refresh();

	bool prev_workarea_layer_status_;

	//Toolbox settings
	synfigapp::Settings& settings;

	//Toolbox display
	Gtk::Table options_table;

	Gtk::Entry		entry_id; //what to name the layer

	Gtk::HScale 	*widget_opacity;
	Widget_Distance *widget_bline_width;

	Widget_Enum		enum_falloff;
	Widget_Enum		enum_blend;

	Gtk::Adjustment	adj_feather;
	Gtk::Adjustment	adj_number_of_bline_points;
	Gtk::Adjustment	adj_bline_point_angle_offset;
	Gtk::SpinButton	spin_feather;
	Gtk::SpinButton	spin_number_of_bline_points;
	Gtk::SpinButton	spin_bline_point_angle_offset;

	Gtk::ToggleButton togglebutton_layer_circle;
	Gtk::ToggleButton togglebutton_layer_region;
	Gtk::ToggleButton togglebutton_layer_outline;
	Gtk::ToggleButton togglebutton_layer_advanced_outline;
	Gtk::ToggleButton togglebutton_layer_curve_gradient;
	Gtk::ToggleButton togglebutton_layer_plant;

	Gtk::CheckButton checkbutton_invert;
	Gtk::CheckButton checkbutton_layer_link_origins;
	Gtk::CheckButton checkbutton_layer_origins_at_center;

public:

	// this only counts the layers which will have their origins linked
	int layers_to_create()const
	{
		return
			(get_layer_circle_flag() && get_layer_origins_at_center_flag()) +
			get_layer_region_flag() +
			get_layer_outline_flag() +
			get_layer_advanced_outline_flag() +
			get_layer_curve_gradient_flag() +
			get_layer_plant_flag();
	}

	synfig::String get_id()const { return entry_id.get_text(); }
	void set_id(const synfig::String& x) { return entry_id.set_text(x); }

	int get_falloff()const { return enum_falloff.get_value(); }
	void set_falloff(int x) { return enum_falloff.set_value(x); }

	int get_blend()const { return enum_blend.get_value(); }
	void set_blend(int x) { return enum_blend.set_value(x); }

	Real get_feather()const { return adj_feather.get_value(); }
	void set_feather(Real f) { adj_feather.set_value(f); }

	Real get_number_of_bline_points()const { return adj_number_of_bline_points.get_value(); }
	void set_number_of_bline_points(Real f) { adj_number_of_bline_points.set_value(f); }

	Real get_bline_point_angle_offset()const { return adj_bline_point_angle_offset.get_value(); }
	void set_bline_point_angle_offset(Real f) { adj_bline_point_angle_offset.set_value(f); }

	bool get_invert()const { return checkbutton_invert.get_active(); }
	void set_invert(bool i) { checkbutton_invert.set_active(i); }

	bool get_layer_circle_flag()const { return togglebutton_layer_circle.get_active(); }
	void set_layer_circle_flag(bool x) { return togglebutton_layer_circle.set_active(x); }

	bool get_layer_region_flag()const { return togglebutton_layer_region.get_active(); }
	void set_layer_region_flag(bool x) { return togglebutton_layer_region.set_active(x); }

	bool get_layer_outline_flag()const { return togglebutton_layer_outline.get_active(); }
	void set_layer_outline_flag(bool x) { return togglebutton_layer_outline.set_active(x); }

	bool get_layer_advanced_outline_flag()const { return togglebutton_layer_advanced_outline.get_active(); }
	void set_layer_advanced_outline_flag(bool x) { return togglebutton_layer_advanced_outline.set_active(x); }

	bool get_layer_curve_gradient_flag()const { return togglebutton_layer_curve_gradient.get_active(); }
	void set_layer_curve_gradient_flag(bool x) { return togglebutton_layer_curve_gradient.set_active(x); }

	bool get_layer_plant_flag()const { return togglebutton_layer_plant.get_active(); }
	void set_layer_plant_flag(bool x) { return togglebutton_layer_plant.set_active(x); }

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
	StateCircle_Context(CanvasView* canvas_view);
	~StateCircle_Context();

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

	void make_circle(const Point& p1, const Point& p2);

};	// END of class StateCircle_Context

/* === M E T H O D S ======================================================= */

StateCircle::StateCircle():
	Smach::state<StateCircle_Context>("circle")
{
	insert(event_def(EVENT_LAYER_SELECTION_CHANGED,&StateCircle_Context::event_layer_selection_changed_handler));
	insert(event_def(EVENT_STOP,&StateCircle_Context::event_stop_handler));
	insert(event_def(EVENT_REFRESH,&StateCircle_Context::event_refresh_handler));
	insert(event_def(EVENT_REFRESH_DUCKS,&StateCircle_Context::event_refresh_handler));
	insert(event_def(EVENT_WORKAREA_MOUSE_BUTTON_DOWN,&StateCircle_Context::event_mouse_click_handler));
	insert(event_def(EVENT_WORKAREA_MOUSE_BUTTON_DRAG,&StateCircle_Context::event_mouse_click_handler));
	insert(event_def(EVENT_WORKAREA_MOUSE_BUTTON_UP,&StateCircle_Context::event_mouse_click_handler));
	insert(event_def(EVENT_REFRESH_TOOL_OPTIONS,&StateCircle_Context::event_refresh_tool_options));
}

StateCircle::~StateCircle()
{
}

void
StateCircle_Context::load_settings()
{
	try
	{
		synfig::ChangeLocale change_locale(LC_NUMERIC, "C");
		String value;

		//parse the arguments yargh!
		if(settings.get_value("circle.id",value))
			set_id(value);
		else
			set_id("Circle");

		if(settings.get_value("circle.fallofftype",value) && value != "")
			set_falloff(atoi(value.c_str()));
		else
			set_falloff(2);

		if(settings.get_value("circle.blend",value) && value != "")
			set_blend(atoi(value.c_str()));
		else
			set_blend(0);//(int)Color::BLEND_COMPOSITE); //0 should be blend composites value

		if(settings.get_value("circle.feather",value))
			set_feather(atof(value.c_str()));
		else
			set_feather(0);

		if(settings.get_value("circle.number_of_bline_points",value))
			set_number_of_bline_points(atof(value.c_str()));
		else
			set_number_of_bline_points(4);

		if(settings.get_value("circle.bline_point_angle_offset",value))
			set_bline_point_angle_offset(atof(value.c_str()));
		else
			set_bline_point_angle_offset(0);

		if(settings.get_value("circle.invert",value) && value != "0")
			set_invert(true);
		else
			set_invert(false);

		if(settings.get_value("circle.layer_circle",value) && value=="0")
			set_layer_circle_flag(false);
		else
			set_layer_circle_flag(true);

		if(settings.get_value("circle.layer_region",value) && value=="1")
			set_layer_region_flag(true);
		else
			set_layer_region_flag(false);

		if(settings.get_value("circle.layer_outline",value) && value=="1")
			set_layer_outline_flag(true);
		else
			set_layer_outline_flag(false);

		if(settings.get_value("circle.layer_advanced_outline",value) && value=="1")
			set_layer_advanced_outline_flag(true);
		else
			set_layer_advanced_outline_flag(false);

		if(settings.get_value("circle.layer_curve_gradient",value) && value=="1")
			set_layer_curve_gradient_flag(true);
		else
			set_layer_curve_gradient_flag(false);

		if(settings.get_value("circle.layer_plant",value) && value=="1")
			set_layer_plant_flag(true);
		else
			set_layer_plant_flag(false);

		if(settings.get_value("circle.layer_link_origins",value) && value=="0")
			set_layer_link_origins_flag(false);
		else
			set_layer_link_origins_flag(true);

		if(settings.get_value("circle.layer_origins_at_center",value) && value=="0")
			set_layer_origins_at_center_flag(false);
		else
			set_layer_origins_at_center_flag(true);
	}
	catch(...)
	{
		synfig::warning("State Circle: Caught exception when attempting to load settings.");
	}
}

void
StateCircle_Context::save_settings()
{
	try
	{
		synfig::ChangeLocale change_locale(LC_NUMERIC, "C");
		settings.set_value("circle.id",get_id());
		settings.set_value("circle.fallofftype",strprintf("%d",get_falloff()));
		settings.set_value("circle.blend",strprintf("%d",get_blend()));
		settings.set_value("circle.feather",strprintf("%f",(float)get_feather()));
		settings.set_value("circle.number_of_bline_points",strprintf("%d",(int)(get_number_of_bline_points() + 0.5)));
		settings.set_value("circle.bline_point_angle_offset",strprintf("%f",(float)get_bline_point_angle_offset()));
		settings.set_value("circle.invert",get_invert()?"1":"0");
		settings.set_value("circle.layer_circle",get_layer_circle_flag()?"1":"0");
		settings.set_value("circle.layer_outline",get_layer_outline_flag()?"1":"0");
		settings.set_value("circle.layer_advanced_outline",get_layer_advanced_outline_flag()?"1":"0");
		settings.set_value("circle.layer_region",get_layer_region_flag()?"1":"0");
		settings.set_value("circle.layer_curve_gradient",get_layer_curve_gradient_flag()?"1":"0");
		settings.set_value("circle.layer_plant",get_layer_plant_flag()?"1":"0");
		settings.set_value("circle.layer_link_origins",get_layer_link_origins_flag()?"1":"0");
		settings.set_value("circle.layer_origins_at_center",get_layer_origins_at_center_flag()?"1":"0");
	}
	catch(...)
	{
		synfig::warning("State Circle: Caught exception when attempting to save settings.");
	}
}

void
StateCircle_Context::reset()
{
	refresh_ducks();
}

void
StateCircle_Context::increment_id()
{
	String id(get_id());
	int number=1;
	int digits=0;

	if(id.empty())
		id="Circle";

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

StateCircle_Context::StateCircle_Context(CanvasView* canvas_view):
	canvas_view_(canvas_view),
	is_working(*canvas_view),
	duckmatic_push(get_work_area()),
	prev_workarea_layer_status_(get_work_area()->get_allow_layer_clicks()),
	settings(synfigapp::Main::get_selected_input_device()->settings()),
	entry_id(),				//   value lower upper  step page
	adj_feather(					0,    0,    1, 0.01, 0.1),
	adj_number_of_bline_points(		0,    2,  120, 1   , 1  ),
	adj_bline_point_angle_offset(	0, -360,  360, 0.1 , 1  ),
	spin_feather(adj_feather,0.1,3),
	spin_number_of_bline_points(adj_number_of_bline_points,1,0),
	spin_bline_point_angle_offset(adj_bline_point_angle_offset,1,1),
	togglebutton_layer_circle(),
	togglebutton_layer_region(),
	togglebutton_layer_outline(),
	togglebutton_layer_advanced_outline(),
	togglebutton_layer_curve_gradient(),
	togglebutton_layer_plant(),
	checkbutton_invert(),
	checkbutton_layer_link_origins(),
	checkbutton_layer_origins_at_center()
{
	egress_on_selection_change=true;

	// Set up the tool options dialog

	// labels
	Gtk::Label *title_label = manage(new class Gtk::Label(_("Circle Creation")));
	Pango::AttrList list;
	Pango::AttrInt attr = Pango::Attribute::create_attr_weight(Pango::WEIGHT_BOLD);
	list.insert(attr);
	title_label->set_attributes(list);
	title_label->set_alignment(Gtk::ALIGN_LEFT, Gtk::ALIGN_CENTER);

	Gtk::Label *id_label = manage(new class Gtk::Label(_("Name:")));
	id_label->set_alignment(Gtk::ALIGN_LEFT, Gtk::ALIGN_CENTER);

	Gtk::Label *layer_types_label = manage(new class Gtk::Label(_("Create:")));
	layer_types_label->set_alignment(Gtk::ALIGN_LEFT, Gtk::ALIGN_CENTER);

	Gtk::Label *blend_label = manage(new class Gtk::Label(_("Blend Method:")));
	blend_label->set_alignment(Gtk::ALIGN_LEFT, Gtk::ALIGN_CENTER);

	Gtk::Label *opacity_label = manage(new class Gtk::Label(_("Opacity:")));
	opacity_label->set_alignment(Gtk::ALIGN_LEFT, Gtk::ALIGN_CENTER);

	Gtk::Label *bline_width_label = manage(new class Gtk::Label(_("Brush Size:")));
	bline_width_label->set_alignment(Gtk::ALIGN_LEFT, Gtk::ALIGN_CENTER);

	Gtk::Label *falloff_label = manage(new class Gtk::Label(_("Falloff:")));
	falloff_label->set_alignment(Gtk::ALIGN_LEFT, Gtk::ALIGN_CENTER);

	Gtk::Label *feather_label = manage(new class Gtk::Label(_("Feather:")));
	feather_label->set_alignment(Gtk::ALIGN_LEFT, Gtk::ALIGN_CENTER);

	Gtk::Label *bline_points_label = manage(new class Gtk::Label(_("Spline Points:")));
	bline_points_label->set_alignment(Gtk::ALIGN_LEFT, Gtk::ALIGN_CENTER);

	Gtk::Label *bline_point_angle_offset_label = manage(new class Gtk::Label(_("Point Offset:")));
	bline_point_angle_offset_label->set_alignment(Gtk::ALIGN_LEFT, Gtk::ALIGN_CENTER);

	Gtk::Label *label_invert = manage(new class Gtk::Label("Invert"));
	label_invert->set_alignment(Gtk::ALIGN_LEFT, Gtk::ALIGN_CENTER);

	Gtk::Label *label_link_origins = manage(new class Gtk::Label("Link Origins"));
	label_link_origins->set_alignment(Gtk::ALIGN_LEFT, Gtk::ALIGN_CENTER);

	Gtk::Label *label_origins_at_center = manage(new class Gtk::Label("Spline Origins at Center"));
	label_origins_at_center->set_alignment(Gtk::ALIGN_LEFT, Gtk::ALIGN_CENTER);

	// add icons to layer creation buttons
	{
		Gtk::Image *icon = manage(new Gtk::Image(Gtk::StockID("synfig-layer_geometry_circle"),
			Gtk::ICON_SIZE_SMALL_TOOLBAR));
		togglebutton_layer_circle.add(*icon);
		togglebutton_layer_circle.set_relief(Gtk::RELIEF_NONE);
	}
	{
		Gtk::Image *icon = manage(new Gtk::Image(Gtk::StockID("synfig-layer_geometry_region"),
			Gtk::ICON_SIZE_SMALL_TOOLBAR));
		togglebutton_layer_region.add(*icon);
		togglebutton_layer_region.set_relief(Gtk::RELIEF_NONE);
	}
	{
		Gtk::Image *icon = manage(new Gtk::Image(Gtk::StockID("synfig-layer_geometry_outline"),
			Gtk::ICON_SIZE_SMALL_TOOLBAR));
		togglebutton_layer_outline.add(*icon);
		togglebutton_layer_outline.set_relief(Gtk::RELIEF_NONE);
	}
	{
		Gtk::Image *icon = manage(new Gtk::Image(Gtk::StockID("synfig-layer_geometry_advanced_outline"),
			Gtk::ICON_SIZE_SMALL_TOOLBAR));
		togglebutton_layer_advanced_outline.add(*icon);
		togglebutton_layer_advanced_outline.set_relief(Gtk::RELIEF_NONE);
	}
	{
		Gtk::Image *icon = manage(new Gtk::Image(Gtk::StockID("synfig-layer_other_plant"),
			Gtk::ICON_SIZE_SMALL_TOOLBAR));
		togglebutton_layer_plant.add(*icon);
		togglebutton_layer_plant.set_relief(Gtk::RELIEF_NONE);
	}
	{
		Gtk::Image *icon = manage(new Gtk::Image(Gtk::StockID("synfig-layer_gradient_curve"),
			Gtk::ICON_SIZE_SMALL_TOOLBAR));
		togglebutton_layer_curve_gradient.add(*icon);
		togglebutton_layer_curve_gradient.set_relief(Gtk::RELIEF_NONE);
	}

	// pack all layer creation buttons in one hbox
	Gtk::HBox *layer_types_box = manage(new class Gtk::HBox());

	Gtk::Alignment *space = Gtk::manage(new Gtk::Alignment());
	space->set_size_request(10);

	layer_types_box->pack_start(*space, Gtk::PACK_SHRINK);
	layer_types_box->pack_start(togglebutton_layer_circle, Gtk::PACK_SHRINK);
	layer_types_box->pack_start(togglebutton_layer_region, Gtk::PACK_SHRINK);
	layer_types_box->pack_start(togglebutton_layer_outline, Gtk::PACK_SHRINK);
	layer_types_box->pack_start(togglebutton_layer_advanced_outline, Gtk::PACK_SHRINK);
	layer_types_box->pack_start(togglebutton_layer_plant, Gtk::PACK_SHRINK);
	layer_types_box->pack_start(togglebutton_layer_curve_gradient, Gtk::PACK_SHRINK);

	// pack entry_id and id_label together in one hbox
	Gtk::HBox *id_box = manage(new class Gtk::HBox());

	Gtk::Alignment *space1 = Gtk::manage(new Gtk::Alignment());
	space1->set_size_request(10);

	id_box->pack_start(*id_label, Gtk::PACK_SHRINK);
	id_box->pack_start(*space1, Gtk::PACK_SHRINK);
	id_box->pack_start(entry_id);

	// pack spline point offset and a space in a hbox
	Gtk::HBox *bline_point_angle_offset_box = manage(new class Gtk::HBox());

	Gtk::Alignment *space2 = Gtk::manage(new Gtk::Alignment());
	space2->set_size_request(10);

	bline_point_angle_offset_box->pack_start(*space2, Gtk::PACK_SHRINK);
	bline_point_angle_offset_box->pack_start(*bline_point_angle_offset_label, Gtk::PACK_SHRINK);

	// pack spline point offset and a space in a hbox
	Gtk::HBox *falloff_box = manage(new class Gtk::HBox());

	Gtk::Alignment *space3 = Gtk::manage(new Gtk::Alignment());
	space3->set_size_request(10);

	falloff_box->pack_start(*space3, Gtk::PACK_SHRINK);
	falloff_box->pack_start(*falloff_label, Gtk::PACK_SHRINK);

	// pack checkbuttons and their own labels together
	Gtk::HBox *box_invert = manage(new class Gtk::HBox());
	box_invert->pack_start(*label_invert);
	box_invert->pack_end(checkbutton_invert, Gtk::PACK_SHRINK);

	Gtk::HBox *box_link_origins = manage(new class Gtk::HBox());
	box_link_origins->pack_start(*label_link_origins);
	box_link_origins->pack_end(checkbutton_layer_link_origins, Gtk::PACK_SHRINK);

	Gtk::HBox *box_origins_at_center = manage(new class Gtk::HBox());
	box_origins_at_center->pack_start(*label_origins_at_center);
	box_origins_at_center->pack_end(checkbutton_layer_origins_at_center, Gtk::PACK_SHRINK);

	// widget opacity
	widget_opacity = manage(new Gtk::HScale(0.0f,1.01f,0.01f));
	widget_opacity->set_digits(2);
	widget_opacity->set_value_pos(Gtk::POS_LEFT);
	widget_opacity->signal_value_changed().connect(sigc::mem_fun(*this,&studio::StateCircle_Context::on_opacity_changed));
	widget_opacity->set_tooltip_text(_("Default Opacity"));
	widget_opacity->set_value_pos(Gtk::POS_LEFT);

	// widget bline width
	widget_bline_width = manage(new Widget_Distance());
	bline_width_refresh();
	widget_bline_width->set_digits(2);
	widget_bline_width->set_range(0,10000000);
	widget_bline_width->signal_value_changed().connect(sigc::mem_fun(*this,&studio::StateCircle_Context::on_bline_width_changed));
	widget_bline_width->set_tooltip_text(_("Brush Size"));

	// feather falloff
	enum_falloff.set_param_desc(ParamDesc("falloff")
		.set_local_name(_("Falloff"))
		.set_description(_("Determines the falloff function for the feather"))
		.set_hint("enum")
		.add_enum_value(CIRCLE_INTERPOLATION_LINEAR,"linear",_("Linear"))
		.add_enum_value(CIRCLE_SQUARED,"squared",_("Squared"))
		.add_enum_value(CIRCLE_SQRT,"sqrt",_("Square Root"))
		.add_enum_value(CIRCLE_SIGMOND,"sigmond",_("Sigmond"))
		.add_enum_value(CIRCLE_COSINE,"cosine",_("Cosine")));

	enum_blend.set_param_desc(ParamDesc(Color::BLEND_COMPOSITE,"blend_method")
		.set_local_name(_("Blend Method"))
		.set_description(_("Defines the blend method to be used for circles")));

	load_settings();

	// 0, title
	options_table.attach(*title_label,
		0, 2,  0,  1, Gtk::FILL, Gtk::FILL, 0, 0
		);
	// 1, name
	options_table.attach(*id_box,
		0, 2, 1, 2, Gtk::FILL, Gtk::FILL, 0, 0
		);
	// 2, layer types creation
	options_table.attach(*layer_types_label,
		0, 2, 2, 3, Gtk::FILL, Gtk::FILL, 0, 0
		);
	options_table.attach(*layer_types_box,
		0, 2, 3, 4, Gtk::FILL, Gtk::FILL, 0, 0
		);
	// 3, blend method
	options_table.attach(*blend_label,
		0, 1, 4, 5, Gtk::EXPAND|Gtk::FILL, Gtk::FILL, 0, 0
		);
	options_table.attach(enum_blend,
		1, 2, 4, 5, Gtk::EXPAND|Gtk::FILL, Gtk::FILL, 0, 0
		);
	// 4, opacity
	options_table.attach(*opacity_label,
		0, 1, 5, 6, Gtk::EXPAND|Gtk::FILL, Gtk::FILL, 0, 0
		);
	options_table.attach(*widget_opacity,
		1, 2, 5, 6, Gtk::EXPAND|Gtk::FILL, Gtk::FILL, 0, 0
	);
	// 5, brush size
	options_table.attach(*bline_width_label,
		0, 1, 6, 7, Gtk::EXPAND|Gtk::FILL, Gtk::FILL, 0, 0
		);
	options_table.attach(*widget_bline_width,
		1, 2, 6, 7, Gtk::EXPAND|Gtk::FILL, Gtk::FILL, 0, 0
		);
	// 6, spline points
	options_table.attach(*bline_points_label,
		0, 1, 7, 8, Gtk::EXPAND|Gtk::FILL, Gtk::FILL, 0, 0
		);
	options_table.attach(spin_number_of_bline_points,
		1, 2, 7, 8, Gtk::EXPAND|Gtk::FILL, Gtk::FILL, 0, 0
		);
	// 7, spline points offset
	options_table.attach(*bline_point_angle_offset_box,
		0, 1, 8, 9, Gtk::EXPAND|Gtk::FILL, Gtk::FILL, 0, 0
		);
	options_table.attach(spin_bline_point_angle_offset,
		1, 2, 8, 9, Gtk::EXPAND|Gtk::FILL, Gtk::FILL, 0, 0
		);
	// 7, invert
	options_table.attach(*box_invert,
		0, 2,  9, 10, Gtk::EXPAND|Gtk::FILL, Gtk::FILL, 0, 0
		);
	// 8, feather
	options_table.attach(*feather_label,
		0, 1, 10, 11, Gtk::EXPAND|Gtk::FILL, Gtk::FILL, 0, 0
		);
  options_table.attach(spin_feather,
		1, 2, 10, 11, Gtk::EXPAND|Gtk::FILL, Gtk::FILL, 0, 0
		);
  // 9, falloff
  options_table.attach(*falloff_box,
		0, 1, 11, 12, Gtk::EXPAND|Gtk::FILL, Gtk::FILL, 0, 0
		);
	options_table.attach(enum_falloff,
		1, 2, 11, 12, Gtk::EXPAND|Gtk::FILL, Gtk::FILL, 0, 0
		);
	// 10, link origins
	options_table.attach(*box_link_origins,
		0, 2,  12,  13, Gtk::FILL, Gtk::FILL, 0, 0
		);
	// 11, origins at center
	options_table.attach(*box_origins_at_center,
		0, 2,  13,  14, Gtk::FILL, Gtk::FILL, 0, 0
		);

	// fine-tune options layout
	options_table.set_border_width(6); // border width 6 px
	options_table.set_row_spacings(3); // row gap 3 px
	options_table.set_row_spacing(0, 6); // the gap between first and second row.
	options_table.set_row_spacing(2, 1); // row gap between label and icon of layer type 1 px
	options_table.set_row_spacing(13, 0); // the final row using border width of table

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
	synfigapp::Main::signal_opacity_changed().connect(sigc::mem_fun(*this,&studio::StateCircle_Context::opacity_refresh));

	App::dock_toolbox->refresh();
}

void
StateCircle_Context::refresh_tool_options()
{
	App::dialog_tool_options->clear();
	App::dialog_tool_options->set_widget(options_table);
	App::dialog_tool_options->set_local_name(_("Circle Tool"));
	App::dialog_tool_options->set_name("circle");
	opacity_refresh();
}

Smach::event_result
StateCircle_Context::event_refresh_tool_options(const Smach::event& /*x*/)
{
	refresh_tool_options();
	return Smach::RESULT_ACCEPT;
}

StateCircle_Context::~StateCircle_Context()
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
StateCircle_Context::event_stop_handler(const Smach::event& /*x*/)
{
	throw &state_normal;
	return Smach::RESULT_OK;
}

Smach::event_result
StateCircle_Context::event_refresh_handler(const Smach::event& /*x*/)
{
	refresh_ducks();
	return Smach::RESULT_ACCEPT;
}

void
StateCircle_Context::make_circle(const Point& _p1, const Point& _p2)
{
	synfigapp::Action::PassiveGrouper group(get_canvas_interface()->get_instance().get(),_("New Circle"));
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

	Real radius((p2-p1).mag());
	int points = get_number_of_bline_points();
	Angle::deg offset(get_bline_point_angle_offset());
	Angle::deg angle(360.0/points);
	Real tangent(4 * ((points == 2)
					  ? 1
					  : ((2 * Angle::cos(angle/2).get() - Angle::cos(angle).get() - 1) / Angle::sin(angle).get())));
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
	for (int i = 0; i < points; i++)
	{
		new_list.push_back(*(new BLinePoint));
		new_list[i].set_width(1);
		new_list[i].set_vertex(Point(radius*Angle::cos(angle*i + offset).get() + x,
									 radius*Angle::sin(angle*i + offset).get() + y));
		new_list[i].set_tangent(Point(-radius*tangent*Angle::sin(angle*i + offset).get(),
									   radius*tangent*Angle::cos(angle*i + offset).get()));
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
	//   C I R C L E
	///////////////////////////////////////////////////////////////////////////

	if (get_layer_circle_flag() &&
		get_falloff() >= 0 && get_falloff() < CIRCLE_NUM_FALLOFF)
	{
		layer=get_canvas_interface()->add_layer_to("circle",canvas,depth);
		if (!layer)
		{
			get_canvas_view()->get_ui_interface()->error(_("Unable to create layer"));
			group.cancel();
			return;
		}
		layer_selection.push_back(layer);

		layer->set_param("radius",(p2-p1).mag());
		get_canvas_interface()->signal_layer_param_changed()(layer,"radius");

		layer->set_param("falloff",get_falloff());
		get_canvas_interface()->signal_layer_param_changed()(layer,"falloff");

		layer->set_param("feather",get_feather());
		get_canvas_interface()->signal_layer_param_changed()(layer,"feather");

		layer->set_param("invert",get_invert());
		get_canvas_interface()->signal_layer_param_changed()(layer,"invert");

		layer->set_param("blend_method",get_blend());
		get_canvas_interface()->signal_layer_param_changed()(layer,"blend_method");

		layer->set_description(get_id());
		get_canvas_interface()->signal_layer_new_description()(layer,layer->get_description());

		// only link the circle's origin parameter if the option is selected, we're putting bline
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
				throw String(_("Unable to create Circle layer"));
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

		layer->set_param("blend_method",get_blend());
		get_canvas_interface()->signal_layer_param_changed()(layer,"blend_method");

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
				//get_canvas_view()->get_ui_interface()->error(_("Unable to create Region layer"));
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
				//get_canvas_view()->get_ui_interface()->error(_("Unable to create Region layer"));
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

		layer->set_param("blend_method",get_blend());
		get_canvas_interface()->signal_layer_param_changed()(layer,"blend_method");

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

		layer->set_param("blend_method",get_blend());
		get_canvas_interface()->signal_layer_param_changed()(layer,"blend_method");

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
StateCircle_Context::event_mouse_click_handler(const Smach::event& x)
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

		make_circle(point_holder, point);
		get_work_area()->clear_ducks();
		return Smach::RESULT_ACCEPT;
	}

	return Smach::RESULT_OK;
}


void
StateCircle_Context::refresh_ducks()
{
	get_work_area()->clear_ducks();
	get_work_area()->queue_draw();
}

void
StateCircle_Context::on_opacity_changed()
{
	synfigapp::Main::set_opacity(widget_opacity->get_value());
}

void
StateCircle_Context::opacity_refresh()
{
	widget_opacity->set_value(synfigapp::Main::get_opacity());
}

void
StateCircle_Context::bline_width_refresh()
{
	widget_bline_width->set_value(synfigapp::Main::get_bline_width());
}

void
StateCircle_Context::on_bline_width_changed()
{
	synfigapp::Main::set_bline_width(widget_bline_width->get_value());
}
