/* === S Y N F I G ========================================================= */
/*!	\file state_draw.cpp
**	\brief Template File
**
**	$Id$
**
**	\legal
**	Copyright (c) 2002-2005 Robert B. Quattlebaum Jr., Adrian Bentley
**	Copyright (c) 2007 Chris Moore
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

#include "state_draw.h"
#include "state_stroke.h"
#include "canvasview.h"
#include "workarea.h"
#include "app.h"
#include <synfig/valuenode_bline.h>
#include <synfig/valuenode_composite.h>
#include <ETL/hermite>
#include <ETL/calculus>
#include <utility>
#include "event_mouse.h"
#include "event_layerclick.h"
#include "toolbox.h"

#include <synfigapp/blineconvert.h>
#include <synfigapp/main.h>

#include <ETL/gaussian>
#include "dialog_tooloptions.h"

#include <gtkmm/table.h>
#include <gtkmm/label.h>
#include <gtkmm/button.h>
#include <gtkmm/checkbutton.h>
#include <gtkmm/scale.h>
#include <sigc++/connection.h>

#endif

/* === U S I N G =========================================================== */

using namespace std;
using namespace etl;
using namespace synfig;
using namespace studio;

/* === M A C R O S ========================================================= */

/* === G L O B A L S ======================================================= */

StateDraw studio::state_draw;

/* === C L A S S E S & S T R U C T S ======================================= */

class studio::StateDraw_Context : public sigc::trackable
{
	typedef etl::smart_ptr<std::list<synfig::Point> > StrokeData;
	typedef etl::smart_ptr<std::list<synfig::Real> > WidthData;

	typedef list< pair<StrokeData,WidthData> > StrokeQueue;

	StrokeQueue stroke_queue;


	etl::handle<CanvasView> canvas_view_;
	CanvasView::IsWorking is_working;

	bool prev_table_status;
	bool loop_;
	bool prev_workarea_layer_status_;

	int nested;
	SigC::Connection process_queue_connection;

	ValueNode_BLine::Handle last_stroke;
	synfig::String last_stroke_id;

	Gtk::Menu menu;

	//Duckmatic::Push duckmatic_push;

	std::list< etl::smart_ptr<std::list<synfig::Point> > > stroke_list;

	void refresh_ducks();

	Duckmatic::Type old_duckmask;

	void fill_last_stroke();

	Smach::event_result new_bline(std::list<synfig::BLinePoint> bline,bool loop_bline_flag,float radius);

	Smach::event_result new_region(std::list<synfig::BLinePoint> bline,synfig::Real radius);

	Smach::event_result extend_bline_from_begin(ValueNode_BLine::Handle value_node,std::list<synfig::BLinePoint> bline,bool complete_loop);
	Smach::event_result extend_bline_from_end(ValueNode_BLine::Handle value_node,std::list<synfig::BLinePoint> bline,bool complete_loop);
	void reverse_bline(std::list<synfig::BLinePoint> &bline);

	synfigapp::Settings& settings;

	Gtk::Table options_table;
	Gtk::Entry entry_id;
	Gtk::CheckButton checkbutton_pressure_width;
	Gtk::CheckButton checkbutton_round_ends;
	Gtk::CheckButton checkbutton_auto_loop;	  // whether to loop new strokes which start and end in the same place
	Gtk::CheckButton checkbutton_auto_extend; // whether to extend existing lines
	Gtk::CheckButton checkbutton_auto_link;	  // whether to link new ducks to existing ducks
	Gtk::CheckButton checkbutton_region;	  // whether to create regions
	Gtk::CheckButton checkbutton_outline;	  // whether to create outlines
	Gtk::CheckButton checkbutton_auto_export;
	Gtk::Button button_fill_last_stroke;

	//pressure spinner and such
	Gtk::Adjustment	 adj_min_pressure;
	Gtk::SpinButton  spin_min_pressure;
	Gtk::CheckButton check_min_pressure;

	Gtk::Adjustment	 adj_feather;
	Gtk::SpinButton  spin_feather;

	Gtk::Adjustment	 adj_globalthres;
	Gtk::SpinButton  spin_globalthres;

	Gtk::Adjustment	 adj_localthres;
	Gtk::CheckButton check_localerror;
	void UpdateErrorBox();	//switches the stuff if need be :)

	//Added by Adrian - data drive HOOOOO
	synfigapp::BLineConverter blineconv;

public:
	synfig::String get_id()const { return entry_id.get_text(); }
	void set_id(const synfig::String& x) { return entry_id.set_text(x); }

	bool get_pressure_width_flag()const { return checkbutton_pressure_width.get_active(); }
	void set_pressure_width_flag(bool x) { return checkbutton_pressure_width.set_active(x); }

	bool get_auto_loop_flag()const { return checkbutton_auto_loop.get_active(); }
	void set_auto_loop_flag(bool x) { return checkbutton_auto_loop.set_active(x); }

	bool get_auto_extend_flag()const { return checkbutton_auto_extend.get_active(); }
	void set_auto_extend_flag(bool x) { return checkbutton_auto_extend.set_active(x); }

	bool get_auto_link_flag()const { return checkbutton_auto_link.get_active(); }
	void set_auto_link_flag(bool x) { return checkbutton_auto_link.set_active(x); }

	bool get_region_flag()const { return checkbutton_region.get_active(); }
	void set_region_flag(bool x) { return checkbutton_region.set_active(x); }

	bool get_outline_flag()const { return checkbutton_outline.get_active(); }
	void set_outline_flag(bool x) { return checkbutton_outline.set_active(x); }

	bool get_auto_export_flag()const { return checkbutton_auto_export.get_active(); }
	void set_auto_export_flag(bool x) { return checkbutton_auto_export.set_active(x); }

	Real get_min_pressure() const { return adj_min_pressure.get_value(); }
	void set_min_pressure(Real x) { return adj_min_pressure.set_value(x); }

	Real get_feather() const { return adj_feather.get_value(); }
	void set_feather(Real x) { return adj_feather.set_value(x); }

	Real get_gthres() const { return adj_globalthres.get_value(); }
	void set_gthres(Real x) { return adj_globalthres.set_value(x); }

	Real get_lthres() const { return adj_localthres.get_value(); }
	void set_lthres(Real x) { return adj_localthres.set_value(x); }

	bool get_local_error_flag() const { return check_localerror.get_active(); }
	void set_local_error_flag(bool x) { check_localerror.set_active(x); }

	bool get_min_pressure_flag()const { return check_min_pressure.get_active(); }
	void set_min_pressure_flag(bool x) { check_min_pressure.set_active(x); }

	void load_settings();
	void save_settings();
	void increment_id();

	Smach::event_result event_stop_handler(const Smach::event& x);

	Smach::event_result event_refresh_handler(const Smach::event& x);

	Smach::event_result event_mouse_down_handler(const Smach::event& x);

	Smach::event_result event_stroke(const Smach::event& x);
	Smach::event_result event_refresh_tool_options(const Smach::event& x);
	void refresh_tool_options();

	Smach::event_result process_stroke(StrokeData stroke_data, WidthData width_data, bool region_flag=false);

	bool process_queue();


	StateDraw_Context(CanvasView* canvas_view);

	~StateDraw_Context();

	const etl::handle<CanvasView>& get_canvas_view()const{return canvas_view_;}
	etl::handle<synfigapp::CanvasInterface> get_canvas_interface()const{return canvas_view_->canvas_interface();}
	synfig::Time get_time()const { return get_canvas_interface()->get_time(); }
	synfig::Canvas::Handle get_canvas()const{return canvas_view_->get_canvas();}
	WorkArea * get_work_area()const{return canvas_view_->get_work_area();}

	//void on_user_click(synfig::Point point);

//	bool run();
};	// END of class StateDraw_Context


/* === M E T H O D S ======================================================= */

StateDraw::StateDraw():
	Smach::state<StateDraw_Context>("draw")
{
	insert(event_def(EVENT_STOP,&StateDraw_Context::event_stop_handler));
	insert(event_def(EVENT_REFRESH,&StateDraw_Context::event_refresh_handler));
	insert(event_def(EVENT_WORKAREA_MOUSE_BUTTON_DOWN,&StateDraw_Context::event_mouse_down_handler));
	insert(event_def(EVENT_WORKAREA_STROKE,&StateDraw_Context::event_stroke));
	insert(event_def(EVENT_REFRESH_TOOL_OPTIONS,&StateDraw_Context::event_refresh_tool_options));
}

StateDraw::~StateDraw()
{
}


void
StateDraw_Context::load_settings()
{
	String value;

	if(settings.get_value("draw.id",value))
		set_id(value);
	else
		set_id("NewDrawing");

	if(settings.get_value("draw.pressure_width",value) && value=="0")
		set_pressure_width_flag(false);
	else
		set_pressure_width_flag(true);

	if(settings.get_value("draw.auto_loop",value) && value=="0")
		set_auto_loop_flag(false);
	else
		set_auto_loop_flag(true);

	if(settings.get_value("draw.auto_extend",value) && value=="0")
		set_auto_extend_flag(false);
	else
		set_auto_extend_flag(true);

	if(settings.get_value("draw.auto_link",value) && value=="0")
		set_auto_link_flag(false);
	else
		set_auto_link_flag(true);

	if(settings.get_value("draw.region",value) && value=="0")
		set_region_flag(false);
	else
		set_region_flag(true);

	if(settings.get_value("draw.outline",value) && value=="0")
		set_outline_flag(false);
	else
		set_outline_flag(true);

	if(settings.get_value("draw.auto_export",value) && value=="1")
		set_auto_export_flag(true);
	else
		set_auto_export_flag(false);

	if(settings.get_value("draw.min_pressure_on",value) && value=="0")
		set_min_pressure_flag(false);
	else
		set_min_pressure_flag(true);

	if(settings.get_value("draw.min_pressure",value))
	{
		Real n = atof(value.c_str());
		set_min_pressure(n);
	}else
		set_min_pressure(0);

	if(settings.get_value("draw.feather",value))
	{
		Real n = atof(value.c_str());
		set_feather(n);
	}else
		set_feather(0);

	if(settings.get_value("draw.gthreshold",value))
	{
		Real n = atof(value.c_str());
		set_gthres(n);
	}

	if(settings.get_value("draw.lthreshold",value))
	{
		Real n = atof(value.c_str());
		set_lthres(n);
	}

	if(settings.get_value("draw.localize",value) && value == "1")
		set_local_error_flag(true);
	else
		set_local_error_flag(false);
}

void
StateDraw_Context::save_settings()
{
	settings.set_value("draw.id",get_id().c_str());
	settings.set_value("draw.pressure_width",get_pressure_width_flag()?"1":"0");
	settings.set_value("draw.auto_loop",get_auto_loop_flag()?"1":"0");
	settings.set_value("draw.auto_extend",get_auto_extend_flag()?"1":"0");
	settings.set_value("draw.auto_link",get_auto_link_flag()?"1":"0");
	settings.set_value("draw.region",get_region_flag()?"1":"0");
	settings.set_value("draw.outline",get_outline_flag()?"1":"0");
	settings.set_value("draw.auto_export",get_auto_export_flag()?"1":"0");
	settings.set_value("draw.min_pressure",strprintf("%f",get_min_pressure()));
	settings.set_value("draw.feather",strprintf("%f",get_feather()));
	settings.set_value("draw.min_pressure_on",get_min_pressure_flag()?"1":"0");
	settings.set_value("draw.gthreshold",strprintf("%f",get_gthres()));
	settings.set_value("draw.lthreshold",strprintf("%f",get_lthres()));
	settings.set_value("draw.localize",get_local_error_flag()?"1":"0");
}

void
StateDraw_Context::increment_id()
{
	String id(get_id());
	int number=1;
	int digits=0;

	if(id.empty())
		id="Drawing";

	// If there is a number
	// already at the end of the
	// id, then remove it.
	if(id[id.size()-1]<='9' && id[id.size()-1]>='0')
	{
		// figure out how many digits it is
		for(digits=0;(int)id.size()-1>=digits && id[id.size()-1-digits]<='9' && id[id.size()-1-digits]>='0';digits++)while(false);

		String str_number;
		str_number=String(id,id.size()-digits,id.size());
		id=String(id,0,id.size()-digits);
		synfig::info("---------------- \"%s\"",str_number.c_str());

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

StateDraw_Context::StateDraw_Context(CanvasView* canvas_view):
	canvas_view_(canvas_view),
	is_working(*canvas_view),
	loop_(false),
	prev_workarea_layer_status_(get_work_area()->get_allow_layer_clicks()),
	settings(synfigapp::Main::get_selected_input_device()->settings()),
	entry_id(),
	checkbutton_pressure_width(_("Pressure Width")),
	checkbutton_auto_loop(_("Auto Loop")),
	checkbutton_auto_extend(_("Auto Extend")),
	checkbutton_auto_link(_("Auto Link")),
	checkbutton_region(_("Create Region")),
	checkbutton_outline(_("Create Outline")),
	checkbutton_auto_export(_("Auto Export")),
	button_fill_last_stroke(_("Fill Last Stroke")),
	adj_min_pressure(0,0,1,0.01,0.1),
	spin_min_pressure(adj_min_pressure,0.1,3),
	check_min_pressure(_("Min Pressure")),
	adj_feather(0,0,10000,0.01,0.1),
	spin_feather(adj_feather,0.01,4),
	adj_globalthres(.70f,0.01,10000,0.01,0.1),
	spin_globalthres(adj_globalthres,0.01,3),
	adj_localthres(20,1,100000,0.1,1),
	check_localerror(_("LocalError"))

{
	synfig::info("STATE SKETCH: entering state");

	nested=0;
	load_settings();

	UpdateErrorBox();

	//options_table.attach(*manage(new Gtk::Label(_("Draw Tool"))), 0, 2, 0, 1, Gtk::EXPAND|Gtk::FILL, Gtk::EXPAND|Gtk::FILL, 0, 0);
	options_table.attach(entry_id,								0, 2,  1,  2, Gtk::EXPAND|Gtk::FILL, Gtk::EXPAND|Gtk::FILL, 0, 0);
	options_table.attach(checkbutton_region,					0, 2,  2,  3, Gtk::EXPAND|Gtk::FILL, Gtk::EXPAND|Gtk::FILL, 0, 0);
	options_table.attach(checkbutton_outline,					0, 2,  3,  4, Gtk::EXPAND|Gtk::FILL, Gtk::EXPAND|Gtk::FILL, 0, 0);
	options_table.attach(checkbutton_auto_loop,					0, 2,  4,  5, Gtk::EXPAND|Gtk::FILL, Gtk::EXPAND|Gtk::FILL, 0, 0);
	options_table.attach(checkbutton_auto_extend,				0, 2,  5,  6, Gtk::EXPAND|Gtk::FILL, Gtk::EXPAND|Gtk::FILL, 0, 0);
	options_table.attach(checkbutton_auto_link,					0, 2,  6,  7, Gtk::EXPAND|Gtk::FILL, Gtk::EXPAND|Gtk::FILL, 0, 0);
	options_table.attach(checkbutton_auto_export,				0, 2,  7,  8, Gtk::EXPAND|Gtk::FILL, Gtk::EXPAND|Gtk::FILL, 0, 0);
	options_table.attach(checkbutton_pressure_width,			0, 2,  8,  9, Gtk::EXPAND|Gtk::FILL, Gtk::EXPAND|Gtk::FILL, 0, 0);
	options_table.attach(check_localerror,						0, 2,  9, 10, Gtk::EXPAND|Gtk::FILL, Gtk::EXPAND|Gtk::FILL, 0, 0);

	options_table.attach(check_min_pressure,					0, 1, 10, 11, Gtk::EXPAND|Gtk::FILL, Gtk::EXPAND|Gtk::FILL, 0, 0);
	options_table.attach(spin_min_pressure,						1, 2, 10, 11, Gtk::EXPAND|Gtk::FILL, Gtk::EXPAND|Gtk::FILL, 0, 0);

	options_table.attach(*manage(new Gtk::Label(_("Smooth"))),	0, 1, 11, 12, Gtk::EXPAND|Gtk::FILL, Gtk::EXPAND|Gtk::FILL, 0, 0);
	options_table.attach(spin_globalthres,						1, 2, 11, 12, Gtk::EXPAND|Gtk::FILL, Gtk::EXPAND|Gtk::FILL, 0, 0);

	options_table.attach(*manage(new Gtk::Label(_("Feather"))), 0, 1, 12, 13, Gtk::EXPAND|Gtk::FILL, Gtk::EXPAND|Gtk::FILL, 0, 0);
	options_table.attach(spin_feather,							1, 2, 12, 13, Gtk::EXPAND|Gtk::FILL, Gtk::EXPAND|Gtk::FILL, 0, 0);

	//options_table.attach(button_fill_last_stroke, 0, 2, 13, 14, Gtk::EXPAND|Gtk::FILL, Gtk::EXPAND|Gtk::FILL, 0, 0);

	button_fill_last_stroke.signal_pressed().connect(sigc::mem_fun(*this,&StateDraw_Context::fill_last_stroke));
	check_localerror.signal_toggled().connect(sigc::mem_fun(*this,&StateDraw_Context::UpdateErrorBox));

	options_table.show_all();
	refresh_tool_options();
	//App::dialog_tool_options->set_widget(options_table);
	App::dialog_tool_options->present();


	old_duckmask=get_work_area()->get_type_mask();
	get_work_area()->set_type_mask(Duck::TYPE_ALL-Duck::TYPE_TANGENT-Duck::TYPE_WIDTH);

	// Turn off layer clicking
	get_work_area()->set_allow_layer_clicks(false);

	// Turn off duck clicking
	get_work_area()->set_allow_duck_clicks(false);

	// clear out the ducks
	//get_work_area()->clear_ducks();

	// Refresh the work area
	//get_work_area()->queue_draw();

	// Hide the tables if they are showing
	prev_table_status=get_canvas_view()->tables_are_visible();
	//if(prev_table_status)get_canvas_view()->hide_tables();

	// Disable the time bar
	get_canvas_view()->set_sensitive_timebar(false);

	// Connect a signal
	//get_work_area()->signal_user_click().connect(sigc::mem_fun(*this,&studio::StateDraw_Context::on_user_click));

	get_canvas_view()->work_area->set_cursor(Gdk::PENCIL);

	App::toolbox->refresh();

	refresh_ducks();
}


void StateDraw_Context::UpdateErrorBox()
{
	if(get_local_error_flag())
	{
		spin_globalthres.set_adjustment(adj_localthres);
		spin_globalthres.set_value(adj_localthres.get_value());
		spin_globalthres.set_increments(0.1,1);
	}else
	{
		spin_globalthres.set_adjustment(adj_globalthres);
		spin_globalthres.set_value(adj_globalthres.get_value());
		spin_globalthres.set_increments(0.01,.1);
	}

	spin_globalthres.update();
}

void
StateDraw_Context::refresh_tool_options()
{
	App::dialog_tool_options->clear();
	App::dialog_tool_options->set_widget(options_table);
	App::dialog_tool_options->set_local_name(_("Draw Tool"));
	App::dialog_tool_options->set_name("draw");

	App::dialog_tool_options->add_button(
		Gtk::StockID("synfig-fill"),
		_("Fill Last Stroke")
	)->signal_clicked().connect(
		sigc::mem_fun(
			*this,
			&StateDraw_Context::fill_last_stroke
		)
	);

}

Smach::event_result
StateDraw_Context::event_refresh_tool_options(const Smach::event& /*x*/)
{
	refresh_tool_options();
	return Smach::RESULT_ACCEPT;
}

StateDraw_Context::~StateDraw_Context()
{
	save_settings();

	App::dialog_tool_options->clear();

	get_work_area()->set_type_mask(old_duckmask);

	get_canvas_view()->work_area->reset_cursor();

	// Restore layer clicking
	get_work_area()->set_allow_layer_clicks(prev_workarea_layer_status_);

	// Restore duck clicking
	get_work_area()->set_allow_duck_clicks(true);

	// Enable the time bar
	get_canvas_view()->set_sensitive_timebar(true);

	// Bring back the tables if they were out before
	if(prev_table_status)get_canvas_view()->show_tables();

	// Refresh the work area
	get_work_area()->queue_draw();

	App::toolbox->refresh();
}

Smach::event_result
StateDraw_Context::event_stop_handler(const Smach::event& /*x*/)
{
	throw Smach::egress_exception();
}

Smach::event_result
StateDraw_Context::event_refresh_handler(const Smach::event& /*x*/)
{
	refresh_ducks();
	return Smach::RESULT_ACCEPT;
}

Smach::event_result
StateDraw_Context::event_mouse_down_handler(const Smach::event& x)
{
	const EventMouse& event(*reinterpret_cast<const EventMouse*>(&x));
	switch(event.button)
	{
	case BUTTON_LEFT:
		{
			// Enter the stroke state to get the stroke
			get_canvas_view()->get_smach().push_state(&state_stroke);
			return Smach::RESULT_ACCEPT;
		}

	case BUTTON_RIGHT: // Intercept the right-button click to short-circuit the pop-up menu
		return Smach::RESULT_ACCEPT;

	default:
		return Smach::RESULT_OK;
	}
}

#define SIMILAR_TANGENT_THRESHOLD	(0.2)

struct debugclass
{
	synfig::String x;
	debugclass(const synfig::String &x):x(x)
	{
//		synfig::warning(">>>>>>>>>>>>>>>>>>> "+x);
	}
	~debugclass()
	{
//		synfig::warning("<<<<<<<<<<<<<<<<<<< "+x);
	}
};

struct DepthCounter
{
	int &i;
	DepthCounter(int &i):i(i) { i++; }
	~DepthCounter() { i--; }
};

Smach::event_result
StateDraw_Context::event_stroke(const Smach::event& x)
{
//	debugclass debugger("StateDraw_Context::event_stroke(const Smach::event& x)");

	const EventStroke& event(*reinterpret_cast<const EventStroke*>(&x));

	assert(event.stroke_data);

	get_work_area()->add_stroke(event.stroke_data,synfigapp::Main::get_foreground_color());

	if(nested==0)
	{
		DirtyTrap dirty_trap(get_work_area());
		Smach::event_result result;
		result=process_stroke(event.stroke_data,event.width_data,event.modifier&Gdk::CONTROL_MASK || event.modifier&Gdk::BUTTON2_MASK);
		process_queue();
		return result;
	}

	stroke_queue.push_back(pair<StrokeData,WidthData>(event.stroke_data,event.width_data));

	return Smach::RESULT_ACCEPT;
}

bool
StateDraw_Context::process_queue()
{
//	debugclass debugger("StateDraw_Context::process_queue()");
	if(nested)
		return true;
	DepthCounter depth_counter(nested);
	while(!stroke_queue.empty())
	{
		pair<StrokeData,WidthData> front(stroke_queue.front());
		process_stroke(front.first,front.second);
		stroke_queue.pop_front();
	}
	return false;
}

Smach::event_result
StateDraw_Context::process_stroke(StrokeData stroke_data, WidthData width_data, bool region_flag)
{
//	debugclass debugger("StateDraw_Context::process_stroke");
	DepthCounter depth_counter(nested);

	const float radius(synfigapp::Main::get_bline_width().units(get_canvas()->rend_desc())+(abs(get_work_area()->get_pw())+abs(get_work_area()->get_ph()))*5);


	// If we aren't using pressure width,
	// then set all the width to 1
	if(!get_pressure_width_flag())
	{
		std::list<synfig::Real>::iterator iter;
		for(iter=width_data->begin();iter!=width_data->end();++iter)
		{
			*iter=1.0;
		}
	}

	//get_work_area()->add_stroke(event.stroke_data,synfigapp::Main::get_foreground_color());
	//stroke_list.push_back(event.stroke_data);
	//refresh_ducks();

	std::list<synfig::BLinePoint> bline;
	bool loop_bline_flag(false);

	//Changed by Adrian - use resident class :)
	//synfigapp::convert_stroke_to_bline(bline, *event.stroke_data,*event.width_data, synfigapp::Main::get_bline_width());
	blineconv.width = synfigapp::Main::get_bline_width().units(get_canvas()->rend_desc());

	if(get_local_error_flag())
	{
		float pw = get_work_area()->get_pw();
		float ph = get_work_area()->get_ph();

		blineconv.pixelwidth = sqrt(pw*pw+ph*ph);
		blineconv.smoothness = get_lthres();
	}else
	{
		blineconv.pixelwidth = 1;
		blineconv.smoothness = get_gthres();
	}

	blineconv(bline,*stroke_data,*width_data);

	//Postprocess to require minimum pressure
	if(get_min_pressure_flag())
	{
		synfigapp::BLineConverter::EnforceMinWidth(bline,get_min_pressure());
	}

	// If the start and end points are similar, then make them the same point
	if(get_auto_loop_flag() &&
	   bline.size()>2&&(bline.front().get_vertex()-bline.back().get_vertex()).mag()<=radius)
	{
		loop_bline_flag=true;
		Vector tangent;
		Real width(0);

		while(bline.size()>2&&(bline.front().get_vertex()-bline.back().get_vertex()).mag()<=radius)
		{
			tangent=bline.back().get_tangent1();
			width=bline.back().get_width();
			bline.pop_back();
		}

		if(abs(bline.front().get_tangent1().norm()*tangent.norm().perp())>SIMILAR_TANGENT_THRESHOLD)
		{
			// If the tangents are not similar, then
			// split the tangents
			bline.front().set_split_tangent_flag(true);
			bline.front().set_tangent1(tangent);
		}
		else
		{
			// If the tangents are similar, then set the tangent
			// to the average of the two
			bline.front().set_tangent((tangent+bline.front().get_tangent1())*0.5f);
		}

		// Add the widths of the two points
		{
			Real tmp_width(bline.front().get_width()+width);
			tmp_width=tmp_width<=1?tmp_width:1;
			bline.front().set_width(tmp_width);
		}
	}

	// If the bline only has one blinepoint, then there is nothing to do.
	if(bline.size()<=1)
		return Smach::RESULT_OK;

	if(region_flag)
		return new_region(bline,radius);

	return new_bline(bline,loop_bline_flag,radius);
}

Smach::event_result
StateDraw_Context::new_bline(std::list<synfig::BLinePoint> bline,bool loop_bline_flag,float radius)
{
	// Create the action group
	synfigapp::Action::PassiveGrouper group(get_canvas_interface()->get_instance().get(),_("Sketch BLine"));

	bool shift_offset = false;
	Vector shift_offset_vector;
	bool join_start_no_extend=false,join_finish_no_extend=false;
	synfigapp::ValueDesc start_duck_value_desc,finish_duck_value_desc;
	bool extend_start=false,extend_finish=false,complete_loop=false;
	bool extend_start_join_same=false,extend_start_join_different=false;
	bool extend_finish_join_same=false,extend_finish_join_different=false;
	int start_duck_index = 0,finish_duck_index = 0; // initialized to keep the compiler happy; shouldn't be needed though
	ValueNode_BLine::Handle start_duck_value_node_bline=NULL,finish_duck_value_node_bline=NULL;

	// Find any ducks at the start or end that we might attach to
	// (this used to only run if we didn't just draw a loop - ie. !loop_bline_flag
	// but having loops auto-connect can be useful as well)
	if(get_auto_extend_flag() || get_auto_link_flag())
	{
		etl::handle<Duck> start_duck(get_work_area()->find_duck(bline.front().get_vertex(),radius,Duck::TYPE_VERTEX));
		etl::handle<Duck> finish_duck(get_work_area()->find_duck(bline.back().get_vertex(),radius,Duck::TYPE_VERTEX));

		// check whether the start of the new line extends an
		// existing line.  this is only the case if the new
		// line isn't a self-contained loop, and if the new
		// line starts at one of the ends of an existing line
		if(start_duck)do
		{
			if(!(start_duck_value_desc=start_duck->get_value_desc()))break;
			if(loop_bline_flag)break; // loops don't extend anything
			if(!start_duck_value_desc.parent_is_value_node())break;
			start_duck_index=start_duck_value_desc.get_index(); // which point on the line did we start drawing at
			start_duck_value_node_bline=ValueNode_BLine::Handle::cast_dynamic(start_duck_value_desc.get_parent_value_node());
			if(!get_auto_extend_flag())break;

			// don't extend looped blines
			if(start_duck_value_node_bline&&!start_duck_value_node_bline->get_loop()&&
			   // did we start drawing at either end of the line?
			   (start_duck_index==0||start_duck_index==start_duck_value_node_bline->link_count()-1))
			{
				extend_start=true;
				shift_offset=true;
				shift_offset_vector=start_duck->get_origin();
			}
		}while(0);

		// check whether the end of the new line extends an
		// existing line.  this is only the case if the new
		// line isn't a self-contained loop, and if the new
		// line ends at one of the ends of an existing line
		if(finish_duck)do
		{
			if(!(finish_duck_value_desc=finish_duck->get_value_desc()))break;
			if(loop_bline_flag)break;
			if(!finish_duck_value_desc.parent_is_value_node())break;
			finish_duck_index=finish_duck_value_desc.get_index();
			finish_duck_value_node_bline=ValueNode_BLine::Handle::cast_dynamic(finish_duck_value_desc.get_parent_value_node());
			if(!get_auto_extend_flag())break;

			// don't extend looped blines
			if(finish_duck_value_node_bline&&!finish_duck_value_node_bline->get_loop()&&
			   (finish_duck_index==0||finish_duck_index==finish_duck_value_node_bline->link_count()-1))
				if(extend_start)
				{
					// we've started and finished drawing at the end of a bline.  we can't
					// extend both blines, so unless we started and finished at the 2 ends
					// of the same bline, only extend the one we started on
					if(start_duck_value_node_bline==finish_duck_value_node_bline)
						complete_loop=extend_finish=true;
				}else{
					extend_finish=true;
					shift_offset=true;
					shift_offset_vector=finish_duck->get_origin();
				}
		}while(0);

		// if the new line's start didn't extend an existing line,
		// check whether it needs to be linked to an existing duck
		if(!extend_start&&get_auto_link_flag()&&start_duck&&start_duck_value_desc)
			switch(start_duck_value_desc.get_value_type())
			{
			case synfig::ValueBase::TYPE_BLINEPOINT:
				start_duck_value_desc=synfigapp::ValueDesc(LinkableValueNode::Handle::cast_dynamic(start_duck_value_desc.get_value_node()),0);
				// fall through
			case synfig::ValueBase::TYPE_VECTOR:
				if (shift_offset && shift_offset_vector != start_duck->get_origin())
					break;
				shift_offset = true;
				shift_offset_vector = start_duck->get_origin();
				get_canvas_interface()->auto_export(start_duck_value_desc);
				if (extend_finish)
					if(start_duck_value_node_bline&&start_duck_value_node_bline==finish_duck_value_node_bline)
						extend_finish_join_same=true;
					else
						extend_finish_join_different=true;
				else
					join_start_no_extend=true;
				// fall through
			default:break;
			}

		// if the new line's end didn't extend an existing line,
		// check whether it needs to be linked to an existing duck
		if(!extend_finish&&get_auto_link_flag()&&finish_duck&&finish_duck_value_desc)
			switch(finish_duck_value_desc.get_value_type())
			{
			case synfig::ValueBase::TYPE_BLINEPOINT:
				finish_duck_value_desc=synfigapp::ValueDesc(LinkableValueNode::Handle::cast_dynamic(finish_duck_value_desc.get_value_node()),0);
				// fall through
			case synfig::ValueBase::TYPE_VECTOR:
				if (shift_offset && shift_offset_vector != finish_duck->get_origin())
					break;
				shift_offset = true;
				shift_offset_vector = finish_duck->get_origin();
				get_canvas_interface()->auto_export(finish_duck_value_desc);
				if(extend_start)
					if(finish_duck_value_node_bline&&start_duck_value_node_bline==finish_duck_value_node_bline)
						extend_start_join_same=true;
					else
						extend_start_join_different=true;
				else
					join_finish_no_extend=true;
				// fall through
			default:break;
			}
	}

	ValueNode_BLine::Handle value_node;
	std::list<synfig::BLinePoint> trans_bline;

	{
		std::list<synfig::BLinePoint>::iterator iter;
		const synfig::TransformStack& transform(get_canvas_view()->get_curr_transform_stack());

		for(iter=bline.begin();iter!=bline.end();++iter)
		{
			BLinePoint bline_point(*iter);
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

			if (shift_offset)
				new_vertex=new_vertex-shift_offset_vector;

			bline_point.set_vertex(new_vertex);

			trans_bline.push_back(bline_point);
		}
		value_node=ValueNode_BLine::create(synfig::ValueBase(trans_bline,loop_bline_flag));
	}

	Smach::event_result result;
	synfig::ValueNode_DynamicList::ListEntry source;

	// the new line's start extends an existing line
	if(extend_start)
	{
		int target_offset = 0;
		if(complete_loop)trans_bline.pop_back();
		trans_bline.pop_front();
		if(start_duck_index==0)
		{	// We need to reverse the BLine first.
			reverse_bline(trans_bline);
			result=extend_bline_from_begin(start_duck_value_node_bline,trans_bline,complete_loop);
			source=start_duck_value_node_bline->list.front();
			target_offset=trans_bline.size();
		}
		else
		{
			result=extend_bline_from_end(start_duck_value_node_bline,trans_bline,complete_loop);
			source=start_duck_value_node_bline->list.back();
		}

		if(extend_start_join_different)
			LinkableValueNode::Handle::cast_dynamic(source.value_node)->
				set_link(0,finish_duck_value_desc.get_value_node());
		else if(extend_start_join_same)
			LinkableValueNode::Handle::cast_dynamic(source.value_node)->
				set_link(0,synfigapp::ValueDesc(LinkableValueNode::Handle::cast_dynamic(start_duck_value_node_bline->
													list[target_offset+finish_duck_index].value_node),0).get_value_node());
		return result;
	}

	// the new line's end extends an existing line
	if(extend_finish)
	{
		int target_offset = 0;
		trans_bline.pop_back();
		if(finish_duck_index==0)
		{
			result=extend_bline_from_begin(finish_duck_value_node_bline,trans_bline,false);
			source=finish_duck_value_node_bline->list.front();
			target_offset=trans_bline.size();
		}
		else
		{	// We need to reverse the BLine first.
			reverse_bline(trans_bline);
			result=extend_bline_from_end(finish_duck_value_node_bline,trans_bline,false);
			source=finish_duck_value_node_bline->list.back();
		}

		if(extend_finish_join_different)
			LinkableValueNode::Handle::cast_dynamic(source.value_node)->
				set_link(0,start_duck_value_desc.get_value_node());
		else if(extend_finish_join_same)
			LinkableValueNode::Handle::cast_dynamic(source.value_node)->
				set_link(0,synfigapp::ValueDesc(LinkableValueNode::Handle::cast_dynamic(finish_duck_value_node_bline->
													list[target_offset+start_duck_index].value_node),0).get_value_node());
		return result;
	}

	if (join_start_no_extend)
		LinkableValueNode::Handle::cast_dynamic(value_node->list.front().value_node)->
		  set_link(0,start_duck_value_desc.get_value_node());

	if (join_finish_no_extend)
		LinkableValueNode::Handle::cast_dynamic(value_node->list.back().value_node)->
		  set_link(0,finish_duck_value_desc.get_value_node());

	if(get_auto_export_flag()) {
		printf("this is where we would export the new line\n");
		if (!get_canvas_interface()->add_value_node(value_node,get_id()))
		{
			/* it's no big deal, is it?  let's keep the shape anyway */
			// get_canvas_view()->get_ui_interface()->error(_("Unable to add value node"));
			// group.cancel();
			// increment_id();
			// return Smach::RESULT_ERROR;
		}
	}

	// Create the layer
	{
		Layer::Handle layer;
		Canvas::Handle canvas(get_canvas_view()->get_canvas());
		int depth(0);

		// we are temporarily using the layer to hold something
		layer=get_canvas_view()->get_selection_manager()->get_selected_layer();
		if(layer)
		{
			depth=layer->get_depth();
			canvas=layer->get_canvas();
		}

		//int number(synfig::UniqueID().get_uid());

		synfigapp::PushMode push_mode(get_canvas_interface(),synfigapp::MODE_NORMAL);

		// if they're both defined, we'll add the region later
		if(get_outline_flag())
		{
			layer=get_canvas_interface()->add_layer_to("outline",canvas,depth);
			layer->set_description(get_id()+_(" Outline"));
		}
		else
		{
			layer=get_canvas_interface()->add_layer_to("region",canvas,depth);
			layer->set_description(get_id()+_(" Region"));
		}

		if(get_feather())
		{
			layer->set_param("feather",get_feather());
			get_canvas_interface()->signal_layer_param_changed()(layer,"feather");
		}
		assert(layer);
		//layer->set_description(strprintf("Stroke %d",number));
		//get_canvas_interface()->signal_layer_new_description()(layer,layer->get_description());

		if (shift_offset)
			get_canvas_interface()->
			  change_value(synfigapp::ValueDesc(layer,"offset"),shift_offset_vector);

		synfigapp::Action::Handle action(synfigapp::Action::create("layer_param_connect"));

		assert(action);

		action->set_param("canvas",get_canvas());
		action->set_param("canvas_interface",get_canvas_interface());
		action->set_param("layer",layer);
		if(!action->set_param("param",String("bline")))
			synfig::error("LayerParamConnect didn't like \"param\"");
		if(!action->set_param("value_node",ValueNode::Handle(value_node)))
			synfig::error("LayerParamConnect didn't like \"value_node\"");

		if(!get_canvas_interface()->get_instance()->perform_action(action))
		{
			get_canvas_view()->get_ui_interface()->error(_("Unable to create layer"));
			group.cancel();
			increment_id();
			//refresh_ducks();
			return Smach::RESULT_ERROR;
		}
		get_canvas_view()->get_selection_manager()->set_selected_layer(layer);
		//refresh_ducks();
	}

	last_stroke=value_node;
	last_stroke_id=get_id();

	if(get_outline_flag() && get_region_flag())
		fill_last_stroke();

	increment_id();
	return Smach::RESULT_ACCEPT;
}

Smach::event_result
StateDraw_Context::new_region(std::list<synfig::BLinePoint> bline, synfig::Real radius)
{
	// Create the action group
	synfigapp::Action::PassiveGrouper group(get_canvas_interface()->get_instance().get(),_("Define Region"));

	std::list<synfigapp::ValueDesc> vertex_list;

	// First we need to come up with a rough list of
	// BLinePoints that we are going to be using to
	// define our region.
	{
		std::list<synfig::BLinePoint>::iterator iter;
		for(iter=bline.begin();iter!=bline.end();++iter)
		{
			etl::handle<Duck> duck(get_work_area()->find_duck(iter->get_vertex(),0,Duck::TYPE_VERTEX));

			if(!duck)
			{
				synfig::info(__FILE__":%d: Nothing to enclose!",__LINE__);
				return Smach::RESULT_OK;
			}


			assert(duck->get_type()==Duck::TYPE_VERTEX);

			synfigapp::ValueDesc value_desc(duck->get_value_desc());

			if(!value_desc)
			{
				synfig::info(__FILE__":%d: Got a hit, but no ValueDesc on this duck",__LINE__);
				continue;
			}

			switch(value_desc.get_value_type())
			{
			case synfig::ValueBase::TYPE_BLINEPOINT:
				//if(vertex_list.empty() || value_desc!=vertex_list.back())
				vertex_list.push_back(value_desc);
				assert(vertex_list.back().is_valid());

				break;
			default:
				break;
			}
		}
	}

	if(vertex_list.size()<=2)
	{
		synfig::info(__FILE__":%d: Vertex list too small to make region.",__LINE__);
		return Smach::RESULT_OK;
	}

	assert(vertex_list.back().is_valid());

	// Remove any duplicates
	{
	}

	// Now we need to clean the list of vertices up
	// a bit. This includes inserting missing vertices
	// and removing extraneous ones.
	// We can do this in multiple passes.
	int i=0;
	for(bool done=false;!done && i<30;i++)
	{
		// Set done to "true" for now. If
		// any updates are performed, we will
		// change it back to false.
		done=true;

		std::list<synfigapp::ValueDesc>::iterator prev,iter,next;
		prev=vertex_list.end();prev--;	// Set prev to the last ValueDesc
		next=vertex_list.begin();
		iter=next++; // Set iter to the first value desc, and next to the second

		for(;iter!=vertex_list.end();prev=iter,iter=next++)
		{
			synfigapp::ValueDesc value_prev(*prev);
			synfigapp::ValueDesc value_desc(*iter);
			synfigapp::ValueDesc value_next((next==vertex_list.end())?vertex_list.front():*next);

			assert(value_desc.is_valid());
			assert(value_next.is_valid());
			assert(value_prev.is_valid());

			//synfig::info("-------");
			//synfig::info(__FILE__":%d: value_prev 0x%08X:%d",__LINE__,value_prev.get_parent_value_node().get(),value_prev.get_index());
			//synfig::info(__FILE__":%d: value_desc 0x%08X:%d",__LINE__,value_desc.get_parent_value_node().get(),value_desc.get_index());
			//synfig::info(__FILE__":%d: value_next 0x%08X:%d",__LINE__,value_next.get_parent_value_node().get(),value_next.get_index());

			/*
			if(value_prev.parent_is_value_node() && value_desc.parent_is_value_node() && value_next.parent_is_value_node())
			{
				// Remove random extraneous vertices
				if(value_prev.get_parent_value_node()==value_next.get_parent_value_node() &&
					value_prev.get_parent_value_node()!=value_desc.get_parent_value_node())
				{
					DEBUGPOINT();
					vertex_list.erase(iter);
					done=false;
					break;
				}
			}
			*/

			// Remove duplicate vertices
			if(value_prev.get_value_node()==value_desc.get_value_node() ||
			   value_desc.get_value_node()==value_next.get_value_node())
			{
				DEBUGPOINT();
				vertex_list.erase(iter);
				done=false;
				break;
			}
			if(value_prev.get_value_node()==value_next.get_value_node())
			{
				DEBUGPOINT();
				vertex_list.erase(prev);
				done=false;
				break;
			}

			if(value_desc.parent_is_value_node() &&
			   value_next.parent_is_value_node() &&
			   value_desc.get_parent_value_node() == value_next.get_parent_value_node() &&
			   next != vertex_list.end())
			{
				// Fill in missing vertices
				if(value_desc.get_index()<value_next.get_index()-1)
				{
					DEBUGPOINT();
					vertex_list.insert(next,synfigapp::ValueDesc(value_desc.get_parent_value_node(),value_desc.get_index()+1));
					done=false;
					break;
				}
				if(value_next.get_index()<value_desc.get_index()-1)
				{
					DEBUGPOINT();
					vertex_list.insert(next,synfigapp::ValueDesc(value_desc.get_parent_value_node(),value_next.get_index()+1));
					done=false;
					break;
				}
			}

			// Ensure that connections
			// between blines are properly
			// connected
			if(value_desc.parent_is_value_node() && value_next.parent_is_value_node() &&
			   value_desc.get_parent_value_node()!=value_next.get_parent_value_node() &&
			   value_desc.get_value_node()!=value_next.get_value_node())
			{
				BLinePoint vertex(value_desc.get_value(get_time()).get(BLinePoint()));
				BLinePoint vertex_next(value_next.get_value(get_time()).get(BLinePoint()));

				//synfig::info("--------");
				//synfig::info(__FILE__":%d: vertex: [%f, %f]",__LINE__,vertex.get_vertex()[0],vertex.get_vertex()[1]);
				//synfig::info(__FILE__":%d: vertex_next: [%f, %f]",__LINE__,vertex_next.get_vertex()[0],vertex_next.get_vertex()[1]);

				if((vertex.get_vertex()-vertex_next.get_vertex()).mag_squared()<radius*radius)
				{
					DEBUGPOINT();
					ValueNode_Composite::Handle value_node;
					ValueNode_Composite::Handle value_node_next;
					value_node=ValueNode_Composite::Handle::cast_dynamic(value_desc.get_value_node().clone());
					value_node_next=ValueNode_Composite::Handle::cast_dynamic(value_next.get_value_node().clone());
					if(!value_node || !value_node_next)
					{
						synfig::info(__FILE__":%d: Unable to properly connect blines.",__LINE__);
						continue;
					}
					DEBUGPOINT();
					value_node->set_link(5,value_node_next->get_link(5));
					value_node->set_link(3,ValueNode_Const::create(true));

					get_canvas_interface()->auto_export(value_node);
					assert(value_node->is_exported());
					*iter=synfigapp::ValueDesc(get_canvas(),value_node->get_id());
					vertex_list.erase(next);
					done=false;
					break;
				}
				else
				{
					DEBUGPOINT();
					bool positive_trend(value_desc.get_index()>value_prev.get_index());

					if(!positive_trend && value_desc.get_index()>0)
					{
						DEBUGPOINT();
						vertex_list.insert(next,synfigapp::ValueDesc(value_desc.get_parent_value_node(),value_desc.get_index()-1));
						done=false;
						break;
					}
					if(positive_trend && value_desc.get_index()<LinkableValueNode::Handle::cast_static(value_desc.get_value_node())->link_count()-1)
					{
						DEBUGPOINT();
						vertex_list.insert(next,synfigapp::ValueDesc(value_desc.get_parent_value_node(),value_desc.get_index()+1));
						done=false;
						break;
					}
				}

			}
		}
	}

	if(vertex_list.size()<=2)
	{
		synfig::info(__FILE__":%d: Vertex list too small to make region.",__LINE__);
		return Smach::RESULT_OK;
	}

	ValueNode_BLine::Handle value_node_bline;

	// Now we need to test for the trivial case,
	// which is where all of the vertices
	// come from one BLine.
	if(vertex_list.front().parent_is_linkable_value_node())
	{
		bool trivial_case(true);
		ValueNode::Handle trivial_case_value_node;

		trivial_case_value_node=vertex_list.front().get_parent_value_node();

		std::list<synfigapp::ValueDesc>::iterator iter;
		for(iter=vertex_list.begin();iter!=vertex_list.end();++iter)
		{
			if(trivial_case_value_node!=iter->get_parent_value_node())
			{
				trivial_case=false;
				break;
			}
		}
		if(trivial_case)
			value_node_bline=ValueNode_BLine::Handle::cast_dynamic(trivial_case_value_node);
	}

	// If we aren't the trivial case,
	// then go ahead and create the new
	// BLine value node
	if(!value_node_bline)
	{
		value_node_bline=ValueNode_BLine::create();

		std::list<synfigapp::ValueDesc>::iterator iter;
		for(iter=vertex_list.begin();iter!=vertex_list.end();++iter)
		{
			// Ensure that the vertex is exported.
			get_canvas_interface()->auto_export(*iter);

			value_node_bline->add(iter->get_value_node());
			//value_node_bline->add(ValueNode_BLine::ListEntry(iter->get_value_node()));
		}

		value_node_bline->set_loop(true);
	}

	get_canvas_interface()->auto_export(value_node_bline);

	// Now we create the region layer
	// Create the layer
	{
		Layer::Handle layer;
		Canvas::Handle canvas(get_canvas_view()->get_canvas());
		int depth(0);

		// we are temporarily using the layer to hold something
		layer=get_canvas_view()->get_selection_manager()->get_selected_layer();
		if(layer)
		{
			depth=layer->get_depth();
			canvas=layer->get_canvas();
		}

		synfigapp::PushMode push_mode(get_canvas_interface(),synfigapp::MODE_NORMAL);

		layer=get_canvas_interface()->add_layer_to("region",canvas,depth);
		assert(layer);
		layer->set_param("color",synfigapp::Main::get_background_color());
		if(get_feather())
		{
			layer->set_param("feather",get_feather());
			get_canvas_interface()->signal_layer_param_changed()(layer,"feather");
		}
		get_canvas_interface()->signal_layer_param_changed()(layer,"color");

		synfigapp::Action::Handle action(synfigapp::Action::create("layer_param_connect"));

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
			get_canvas_view()->get_ui_interface()->error(_("Unable to create Region layer"));
			group.cancel();
			return Smach::RESULT_ERROR;
		}
		get_canvas_view()->get_selection_manager()->set_selected_layer(layer);
	}

	return Smach::RESULT_ACCEPT;
}

void
StateDraw_Context::refresh_ducks()
{
	get_canvas_view()->queue_rebuild_ducks();
/*
	get_work_area()->clear_ducks();


	std::list< etl::smart_ptr<std::list<synfig::Point> > >::iterator iter;

	for(iter=stroke_list.begin();iter!=stroke_list.end();++iter)
	{
		get_work_area()->add_stroke(*iter);
	}

	get_work_area()->queue_draw();
*/
}


Smach::event_result
StateDraw_Context::extend_bline_from_begin(ValueNode_BLine::Handle value_node,std::list<synfig::BLinePoint> bline,bool complete_loop)
{
	// Create the action group
	synfigapp::Action::PassiveGrouper group(get_canvas_interface()->get_instance().get(),_("Extend BLine"));

	if (complete_loop)
	{
		synfigapp::Action::Handle action(synfigapp::Action::create("value_node_dynamic_list_loop"));
		assert(action);

		action->set_param("canvas",get_canvas());
		action->set_param("canvas_interface",get_canvas_interface());
		action->set_param("value_node",ValueNode::Handle(value_node));

		if(!get_canvas_interface()->get_instance()->perform_action(action))
		{
			get_canvas_view()->get_ui_interface()->error(_("Unable to set loop for bline"));
			group.cancel();
			return Smach::RESULT_ERROR;
		}
	}

	std::list<synfig::BLinePoint>::reverse_iterator iter;
	for(iter=bline.rbegin();!(iter==bline.rend());++iter)
	{
		ValueNode_Composite::Handle composite(ValueNode_Composite::create(*iter));

		synfigapp::Action::Handle action(synfigapp::Action::create("value_node_dynamic_list_insert"));

		assert(action);
		synfigapp::ValueDesc value_desc(value_node,0);

		action->set_param("canvas",get_canvas());
		action->set_param("canvas_interface",get_canvas_interface());
		action->set_param("value_desc",value_desc);
		if(!action->set_param("item",ValueNode::Handle(composite)))
			synfig::error("ACTION didn't like \"item\"");

		if(!get_canvas_interface()->get_instance()->perform_action(action))
		{
			get_canvas_view()->get_ui_interface()->error(_("Unable to insert item"));
			group.cancel();
			//refresh_ducks();
			return Smach::RESULT_ERROR;
		}
	}
	last_stroke=value_node;
	return Smach::RESULT_ACCEPT;
}

Smach::event_result
StateDraw_Context::extend_bline_from_end(ValueNode_BLine::Handle value_node,std::list<synfig::BLinePoint> bline,bool complete_loop)
{
	// Create the action group
	synfigapp::Action::PassiveGrouper group(get_canvas_interface()->get_instance().get(),_("Extend BLine"));

	if (complete_loop)
	{
		synfigapp::Action::Handle action(synfigapp::Action::create("value_node_dynamic_list_loop"));
		assert(action);

		action->set_param("canvas",get_canvas());
		action->set_param("canvas_interface",get_canvas_interface());
		action->set_param("value_node",ValueNode::Handle(value_node));

		if(!get_canvas_interface()->get_instance()->perform_action(action))
		{
			get_canvas_view()->get_ui_interface()->error(_("Unable to set loop for bline"));
			group.cancel();
			return Smach::RESULT_ERROR;
		}
	}

	std::list<synfig::BLinePoint>::iterator iter;
	for(iter=bline.begin();iter!=bline.end();++iter)
	{
		ValueNode_Composite::Handle composite(ValueNode_Composite::create(*iter));

		synfigapp::Action::Handle action(synfigapp::Action::create("value_node_dynamic_list_insert"));

		assert(action);
		synfigapp::ValueDesc value_desc(value_node,value_node->link_count());

		action->set_param("canvas",get_canvas());
		action->set_param("canvas_interface",get_canvas_interface());
		action->set_param("value_desc",value_desc);
		if(!action->set_param("item",ValueNode::Handle(composite)))
			synfig::error("ACTION didn't like \"item\"");

		if(!get_canvas_interface()->get_instance()->perform_action(action))
		{
			get_canvas_view()->get_ui_interface()->error(_("Unable to insert item"));
			group.cancel();
			//refresh_ducks();
			return Smach::RESULT_ERROR;
		}
	}
	last_stroke=value_node;
	return Smach::RESULT_ACCEPT;
}

void
StateDraw_Context::reverse_bline(std::list<synfig::BLinePoint> &bline)
{
	int i;

	std::list<synfig::BLinePoint>::iterator iter,eiter;
	iter=bline.begin();
	eiter=bline.end();
	eiter--;
	for(i=0;i<(int)bline.size()/2;++iter,--eiter,i++)
	{
		iter_swap(iter,eiter);
		iter->reverse();
		eiter->reverse();
	}
}

void
StateDraw_Context::fill_last_stroke()
{
	if(!last_stroke)
		return;

	synfigapp::Action::PassiveGrouper group(get_canvas_interface()->get_instance().get(),_("Fill Stroke"));

	Layer::Handle layer;

	get_canvas_interface()->auto_export(last_stroke);

	synfigapp::PushMode push_mode(get_canvas_interface(),synfigapp::MODE_NORMAL);

	layer=get_canvas_interface()->add_layer("region");
	assert(layer);
	layer->set_param("color",synfigapp::Main::get_background_color());
	layer->set_description(last_stroke_id + _(" Region"));

	synfigapp::Action::Handle action(synfigapp::Action::create("layer_param_connect"));

	assert(action);

	action->set_param("canvas",get_canvas());
	action->set_param("canvas_interface",get_canvas_interface());
	action->set_param("layer",layer);
	if(!action->set_param("param",String("bline")))
		synfig::error("LayerParamConnect didn't like \"param\"");
	if(!action->set_param("value_node",ValueNode::Handle(last_stroke)))
		synfig::error("LayerParamConnect didn't like \"value_node\"");

	if(!get_canvas_interface()->get_instance()->perform_action(action))
	{
		get_canvas_view()->get_ui_interface()->error(_("Unable to create Region layer"));
		group.cancel();
		return;
	}
	get_canvas_view()->get_selection_manager()->set_selected_layer(layer);
}
