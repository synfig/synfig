/* === S Y N F I G ========================================================= */
/*!	\file rotoscope_bline.cpp
**	\brief Template File
**
**	$Id: state_draw.cpp,v 1.1.1.1 2005/01/07 03:34:36 darco Exp $
**
**	\legal
**	Copyright (c) 2002 Robert B. Quattlebaum Jr.
**
**	This software and associated documentation
**	are CONFIDENTIAL and PROPRIETARY property of
**	the above-mentioned copyright holder.
**
**	You may not copy, print, publish, or in any
**	other way distribute this software without
**	a prior written agreement with
**	the copyright holder.
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
	
	Gtk::Menu menu;

	//Duckmatic::Push duckmatic_push;
	
	std::list< etl::smart_ptr<std::list<synfig::Point> > > stroke_list;

	void refresh_ducks();
	
	Duckmatic::Type old_duckmask;

	void fill_last_stroke();
	
	Smach::event_result new_bline(std::list<synfig::BLinePoint> bline,bool loop_bline_flag,float radius);

	Smach::event_result new_region(std::list<synfig::BLinePoint> bline,synfig::Real radius);

	Smach::event_result extend_bline_from_begin(ValueNode_BLine::Handle value_node,std::list<synfig::BLinePoint> bline);
	Smach::event_result extend_bline_from_end(ValueNode_BLine::Handle value_node,std::list<synfig::BLinePoint> bline);
	void reverse_bline(std::list<synfig::BLinePoint> &bline);

	synfigapp::Settings& settings;

	Gtk::Table options_table;
	Gtk::CheckButton checkbutton_pressure_width;
	Gtk::CheckButton checkbutton_round_ends;
	Gtk::CheckButton checkbutton_auto_loop;
	Gtk::CheckButton checkbutton_auto_connect;
	Gtk::CheckButton checkbutton_region_only;
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
	bool get_pressure_width_flag()const { return checkbutton_pressure_width.get_active(); }
	void set_pressure_width_flag(bool x) { return checkbutton_pressure_width.set_active(x); }

	bool get_auto_loop_flag()const { return checkbutton_auto_loop.get_active(); }
	void set_auto_loop_flag(bool x) { return checkbutton_auto_loop.set_active(x); }

	bool get_auto_connect_flag()const { return checkbutton_auto_connect.get_active(); }
	void set_auto_connect_flag(bool x) { return checkbutton_auto_connect.set_active(x); }

	bool get_region_only_flag()const { return checkbutton_region_only.get_active(); }
	void set_region_only_flag(bool x) { return checkbutton_region_only.set_active(x); }
	
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

	if(settings.get_value("draw.pressure_width",value) && value=="0")
		set_pressure_width_flag(false);
	else
		set_pressure_width_flag(true);

	if(settings.get_value("draw.auto_loop",value) && value=="0")
		set_auto_loop_flag(false);
	else
		set_auto_loop_flag(true);

	if(settings.get_value("draw.auto_connect",value) && value=="0")
		set_auto_connect_flag(false);
	else
		set_auto_connect_flag(true);

	if(settings.get_value("draw.region_only",value) && value=="1")
		set_region_only_flag(true);
	else
		set_region_only_flag(false);
	
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
	settings.set_value("draw.pressure_width",get_pressure_width_flag()?"1":"0");
	settings.set_value("draw.auto_loop",get_auto_loop_flag()?"1":"0");
	settings.set_value("draw.auto_connect",get_auto_connect_flag()?"1":"0");
	settings.set_value("draw.region_only",get_region_only_flag()?"1":"0");
	settings.set_value("draw.min_pressure",strprintf("%f",get_min_pressure()));
	settings.set_value("draw.feather",strprintf("%f",get_feather()));
	settings.set_value("draw.min_pressure_on",get_min_pressure_flag()?"1":"0");
	settings.set_value("draw.gthreshold",strprintf("%f",get_gthres()));
	settings.set_value("draw.lthreshold",strprintf("%f",get_lthres()));	
	settings.set_value("draw.localize",get_local_error_flag()?"1":"0");
}

StateDraw_Context::StateDraw_Context(CanvasView* canvas_view):
	canvas_view_(canvas_view),
	is_working(*canvas_view),
	loop_(false),
	prev_workarea_layer_status_(get_work_area()->allow_layer_clicks),
	settings(synfigapp::Main::get_selected_input_device()->settings()),
	checkbutton_pressure_width(_("Pressure Width")),
	checkbutton_auto_loop(_("Auto Loop")),
	checkbutton_auto_connect(_("Auto Connect")),
	checkbutton_region_only(_("Create Region Only")),
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
	options_table.attach(checkbutton_pressure_width, 0, 2, 1, 2, Gtk::EXPAND|Gtk::FILL, Gtk::EXPAND|Gtk::FILL, 0, 0);
	options_table.attach(checkbutton_auto_loop, 0, 2, 2, 3, Gtk::EXPAND|Gtk::FILL, Gtk::EXPAND|Gtk::FILL, 0, 0);	
	options_table.attach(checkbutton_auto_connect, 0, 2, 3, 4, Gtk::EXPAND|Gtk::FILL, Gtk::EXPAND|Gtk::FILL, 0, 0);	
	options_table.attach(checkbutton_region_only, 0, 2, 4, 5, Gtk::EXPAND|Gtk::FILL, Gtk::EXPAND|Gtk::FILL, 0, 0);	
	
	options_table.attach(check_min_pressure, 0, 2, 5, 6, Gtk::EXPAND|Gtk::FILL, Gtk::EXPAND|Gtk::FILL, 0, 0);
	options_table.attach(spin_min_pressure, 0, 2, 6, 7, Gtk::EXPAND|Gtk::FILL, Gtk::EXPAND|Gtk::FILL, 0, 0);

	options_table.attach(*manage(new Gtk::Label(_("Feather"))), 0, 1, 7, 8, Gtk::EXPAND|Gtk::FILL, Gtk::EXPAND|Gtk::FILL, 0, 0);	
	options_table.attach(spin_feather, 1, 2, 7, 8, Gtk::EXPAND|Gtk::FILL, Gtk::EXPAND|Gtk::FILL, 0, 0);
	
	options_table.attach(check_localerror, 0, 2, 8, 9, Gtk::EXPAND|Gtk::FILL, Gtk::EXPAND|Gtk::FILL, 0, 0);	
	options_table.attach(*manage(new Gtk::Label(_("Smooth"))), 0, 1, 9, 10, Gtk::EXPAND|Gtk::FILL, Gtk::EXPAND|Gtk::FILL, 0, 0);	
	options_table.attach(spin_globalthres, 1, 2, 9, 10, Gtk::EXPAND|Gtk::FILL, Gtk::EXPAND|Gtk::FILL, 0, 0);

	//options_table.attach(button_fill_last_stroke, 0, 2, 10, 11, Gtk::EXPAND|Gtk::FILL, Gtk::EXPAND|Gtk::FILL, 0, 0);	
	
	button_fill_last_stroke.signal_pressed().connect(sigc::mem_fun(*this,&StateDraw_Context::fill_last_stroke));
	check_localerror.signal_toggled().connect(sigc::mem_fun(*this,&StateDraw_Context::UpdateErrorBox));
	
	options_table.show_all();
	refresh_tool_options();
	//App::dialog_tool_options->set_widget(options_table);
	App::dialog_tool_options->present();
	
	
	old_duckmask=get_work_area()->get_type_mask();
	get_work_area()->set_type_mask(Duck::TYPE_ALL-Duck::TYPE_TANGENT-Duck::TYPE_WIDTH);
	
	// Turn off layer clicking
	get_work_area()->allow_layer_clicks=false;

	// Turn off duck clicking
	get_work_area()->allow_duck_clicks=false;
	
	// clear out the ducks
	//get_work_area()->clear_ducks();
	
	// Refresh the work area
	//get_work_area()->queue_draw();
	
	// Hide the tables if they are showing
	prev_table_status=get_canvas_view()->tables_are_visible();
	//if(prev_table_status)get_canvas_view()->hide_tables();
		
	// Hide the time bar
	get_canvas_view()->hide_timebar();
	
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
StateDraw_Context::event_refresh_tool_options(const Smach::event& x)
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
	get_work_area()->allow_layer_clicks=prev_workarea_layer_status_;

	// Restore duck clicking
	get_work_area()->allow_duck_clicks=true;

	// Show the time bar
	if(get_canvas_view()->get_canvas()->rend_desc().get_time_start()!=get_canvas_view()->get_canvas()->rend_desc().get_time_end())
		get_canvas_view()->show_timebar();

	// Bring back the tables if they were out before
	if(prev_table_status)get_canvas_view()->show_tables();
			
	// Refresh the work area
	get_work_area()->queue_draw();

	App::toolbox->refresh();
}

Smach::event_result
StateDraw_Context::event_stop_handler(const Smach::event& x)
{
	throw Smach::egress_exception();
}

Smach::event_result
StateDraw_Context::event_refresh_handler(const Smach::event& x)
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
	
	case BUTTON_RIGHT: // Intercept the right-button click to short-circut the pop-up menu
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
	
	// If the start and end points are similar, then make then the same point
	if(get_auto_loop_flag())
	if(bline.size()>2&&(bline.front().get_vertex()-bline.back().get_vertex()).mag()<=radius)
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
			Real width(bline.front().get_width()+width);
			width=width<=1?width:1;
			bline.front().set_width(width);
		}
	}

	// If the bline only has once blinepoint, then there is nothing to do.
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

	//ValueNode_BLine::Handle value_node(ValueNode_BLine::create(synfig::ValueBase(bline,loop_bline_flag)));
	ValueNode_BLine::Handle value_node;

	{
		std::list<synfig::BLinePoint> trans_bline;
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
			
			bline_point.set_vertex(new_vertex);
			
			trans_bline.push_back(bline_point);
		}
		value_node=ValueNode_BLine::create(synfig::ValueBase(trans_bline,loop_bline_flag));
	}

	// Find any ducks at the start or end that we might attach to
	// (If we aren't a loop)
	if(!loop_bline_flag && get_auto_connect_flag())
	{
		
		etl::handle<Duck> start_duck(get_work_area()->find_duck(bline.front().get_vertex(),radius,Duck::TYPE_VERTEX));
		etl::handle<Duck> finish_duck(get_work_area()->find_duck(bline.back().get_vertex(),radius,Duck::TYPE_VERTEX));
		
		if(start_duck)do
		{
			synfigapp::ValueDesc value_desc(start_duck->get_value_desc());
			if(!value_desc)
			{
				break;
			}

			ValueNode_BLine::Handle value_node_bline;

			if(value_desc.parent_is_value_node())
			{
				value_node_bline=ValueNode_BLine::Handle::cast_dynamic(value_desc.get_parent_value_node());
			}
			if(value_node_bline)
			{
				if(value_desc.get_index()==0)
				{
					// SPECIAL CASE -- EXTENSION
					// We need to reverse the BLine first.
					bline.pop_front();
					reverse_bline(bline);
					return extend_bline_from_begin(value_node_bline,bline);					
				}
				if(value_desc.get_index()==value_node_bline->link_count()-1)
				{
					// SPECIAL CASE -- EXTENSION
					bline.pop_front();
					return extend_bline_from_end(value_node_bline,bline);
				}
			}

			switch(value_desc.get_value_type())
			{
			case synfig::ValueBase::TYPE_BLINEPOINT:
				//get_canvas_interface()->auto_export(value_desc);
				//value_node->list.front().value_node=value_desc.get_value_node();
				
				value_desc=synfigapp::ValueDesc(LinkableValueNode::Handle::cast_dynamic(value_desc.get_value_node()),0);
				//break;
			case synfig::ValueBase::TYPE_VECTOR:
				get_canvas_interface()->auto_export(value_desc);
				LinkableValueNode::Handle::cast_dynamic(value_node->list.front().value_node)->set_link(0,value_desc.get_value_node());
				break;
			default:
				break;
			}
		}while(0);
		
		if(finish_duck)do
		{
			synfigapp::ValueDesc value_desc(finish_duck->get_value_desc());
			if(!value_desc)
			{
				break;
			}

			ValueNode_BLine::Handle value_node_bline;

			if(value_desc.parent_is_value_node())
				value_node_bline=ValueNode_BLine::Handle::cast_dynamic(value_desc.get_parent_value_node());
			if(value_node_bline)
			{
				if(value_desc.get_index()==0)
				{
					// SPECIAL CASE -- EXTENSION
					bline.pop_back();
					return extend_bline_from_begin(value_node_bline,bline);					
				}
				if(value_desc.get_index()==value_node_bline->link_count()-1)
				{
					// SPECIAL CASE -- EXTENSION
					// We need to reverse the BLine first.
					bline.pop_back();
					reverse_bline(bline);
					return extend_bline_from_end(value_node_bline,bline);
				}
			}

			switch(value_desc.get_value_type())
			{
			case synfig::ValueBase::TYPE_BLINEPOINT:
				//get_canvas_interface()->auto_export(value_desc);
				//value_node->list.back().value_node=value_desc.get_value_node();

				value_desc=synfigapp::ValueDesc(LinkableValueNode::Handle::cast_dynamic(value_desc.get_value_node()),0);
				//break;
			case synfig::ValueBase::TYPE_VECTOR:
				get_canvas_interface()->auto_export(value_desc);
				LinkableValueNode::Handle::cast_dynamic(value_node->list.back().value_node)->set_link(0,value_desc.get_value_node());
				break;
			default:
				break;
			}
			
		}while(0);
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
		
		if(get_region_only_flag())
			layer=get_canvas_interface()->add_layer_to("region",canvas,depth);
		else
			layer=get_canvas_interface()->add_layer_to("outline",canvas,depth);
			
		if(get_feather())
		{
			layer->set_param("feather",get_feather());
			get_canvas_interface()->signal_layer_param_changed()(layer,"feather");
		}
		assert(layer);
		//layer->set_description(strprintf("Stroke %d",number));
		//get_canvas_interface()->signal_layer_new_description()(layer,layer->get_description());
		
		
		
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
			//refresh_ducks();
			return Smach::RESULT_ERROR;
		}
		get_canvas_view()->get_selection_manager()->set_selected_layer(layer);
		//refresh_ducks();
	}
	
	last_stroke=value_node;
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
			if(value_prev.get_value_node()==value_desc.get_value_node()
				|| value_desc.get_value_node()==value_next.get_value_node())
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
			
			if(value_desc.parent_is_value_node() && value_next.parent_is_value_node())
			if(value_desc.get_parent_value_node()==value_next.get_parent_value_node() && (next!=vertex_list.end()))
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
			if(value_desc.parent_is_value_node() && value_next.parent_is_value_node())
			if(value_desc.get_parent_value_node()!=value_next.get_parent_value_node() &&
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
StateDraw_Context::extend_bline_from_begin(ValueNode_BLine::Handle value_node,std::list<synfig::BLinePoint> bline)
{
	// Create the action group
	synfigapp::Action::PassiveGrouper group(get_canvas_interface()->get_instance().get(),_("Extend BLine"));
	
	std::list<synfig::BLinePoint>::reverse_iterator iter;
	iter=bline.rbegin();
	for(;!(iter==bline.rend());++iter)
	{
		//iter->reverse();
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
StateDraw_Context::extend_bline_from_end(ValueNode_BLine::Handle value_node,std::list<synfig::BLinePoint> bline)
{
	// Create the action group
	synfigapp::Action::PassiveGrouper group(get_canvas_interface()->get_instance().get(),_("Extend BLine"));

	std::list<synfig::BLinePoint>::iterator iter;
	iter=bline.begin();
	for(;iter!=bline.end();++iter)
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

	synfigapp::Action::Handle action(synfigapp::Action::create("layer_param_connect"));
	
	assert(action);
	
	action->set_param("canvas",get_canvas());			
	action->set_param("canvas_interface",get_canvas_interface());			
	action->set_param("layer",layer);			
	if(!action->set_param("param",String("segment_list")))
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
