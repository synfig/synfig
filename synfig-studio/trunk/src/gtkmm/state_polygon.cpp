/* === S Y N F I G ========================================================= */
/*!	\file state_polygon.cpp
**	\brief Template File
**
**	$Id$
**
**	\legal
**	Copyright (c) 2002-2005 Robert B. Quattlebaum Jr., Adrian Bentley
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

#include "state_polygon.h"
#include "canvasview.h"
#include "workarea.h"
#include "app.h"

#include <synfigapp/action.h>
#include "event_mouse.h"
#include "event_layerclick.h"
#include "toolbox.h"
#include "dialog_tooloptions.h"
#include <synfigapp/main.h>

#endif

/* === U S I N G =========================================================== */

using namespace std;
using namespace etl;
using namespace synfig;
using namespace studio;

/* === M A C R O S ========================================================= */

/* === G L O B A L S ======================================================= */

StatePolygon studio::state_polygon;

/* === C L A S S E S & S T R U C T S ======================================= */

class studio::StatePolygon_Context : public sigc::trackable
{
	etl::handle<CanvasView> canvas_view_;
	CanvasView::IsWorking is_working;

	bool prev_table_status;
	bool prev_workarea_layer_status_;

	Gtk::Menu menu;

	Duckmatic::Push duckmatic_push;

	std::list<synfig::Point> polygon_point_list;
	synfigapp::Settings& settings;


	bool on_polygon_duck_change(const synfig::Point &point, std::list<synfig::Point>::iterator iter);


	void popup_handle_menu(synfigapp::ValueDesc value_desc);


	void refresh_ducks();

	Gtk::Table options_table;
	Gtk::Entry entry_id;
	Gtk::Button button_make;

public:
	synfig::String get_id()const { return entry_id.get_text(); }
	void set_id(const synfig::String& x) { return entry_id.set_text(x); }

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
			throw Smach::egress_exception();
		return Smach::RESULT_OK;
	}

	void run();
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
	String value;

	if(settings.get_value("polygon.id",value))
		set_id(value);
	else
		set_id("Polygon");
}

void
StatePolygon_Context::save_settings()
{
	settings.set_value("polygon.id",get_id().c_str());
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
		for(digits=0;(int)id.size()-1>=digits && id[id.size()-1-digits]<='9' && id[id.size()-1-digits]>='0';digits++)while(false);

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
	entry_id(),
	button_make(_("Make"))
{
	egress_on_selection_change=true;
	load_settings();

	// Set up the tool options dialog
	//options_table.attach(*manage(new Gtk::Label(_("Polygon Tool"))), 0, 2, 0, 1, Gtk::EXPAND|Gtk::FILL, Gtk::EXPAND|Gtk::FILL, 0, 0);
	options_table.attach(entry_id, 0, 2, 1, 2, Gtk::EXPAND|Gtk::FILL, Gtk::EXPAND|Gtk::FILL, 0, 0);
	//options_table.attach(button_make, 0, 2, 4, 5, Gtk::EXPAND|Gtk::FILL, Gtk::EXPAND|Gtk::FILL, 0, 0);
	button_make.signal_pressed().connect(sigc::mem_fun(*this,&StatePolygon_Context::run));
	options_table.show_all();
	refresh_tool_options();
	App::dialog_tool_options->present();


	// Turn off layer clicking
	get_work_area()->set_allow_layer_clicks(false);

	// clear out the ducks
	get_work_area()->clear_ducks();

	// Refresh the work area
	get_work_area()->queue_draw();

	get_canvas_view()->work_area->set_cursor(Gdk::CROSSHAIR);

	// Hide the tables if they are showing
	prev_table_status=get_canvas_view()->tables_are_visible();
	if(prev_table_status)get_canvas_view()->hide_tables();

	// Disable the time bar
	get_canvas_view()->set_sensitive_timebar(false);

	// Connect a signal
	//get_work_area()->signal_user_click().connect(sigc::mem_fun(*this,&studio::StatePolygon_Context::on_user_click));

	App::toolbox->refresh();
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

	get_canvas_view()->work_area->reset_cursor();

	// Enable the time bar
	get_canvas_view()->set_sensitive_timebar(true);

	// Bring back the tables if they were out before
	if(prev_table_status)get_canvas_view()->show_tables();

	// Refresh the work area
	get_work_area()->queue_draw();

	App::toolbox->refresh();
}

Smach::event_result
StatePolygon_Context::event_stop_handler(const Smach::event& /*x*/)
{
	synfig::info("STATE RotoPolygon: Received Stop Event");
	//throw Smach::egress_exception();
	reset();
	return Smach::RESULT_ACCEPT;

}

Smach::event_result
StatePolygon_Context::event_refresh_handler(const Smach::event& /*x*/)
{
	synfig::info("STATE RotoPolygon: Received Refresh Event");
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

		{
			synfigapp::Action::PassiveGrouper group(get_canvas_interface()->get_instance().get(),_("New Polygon"));
			synfigapp::PushMode push_mode(get_canvas_interface(),synfigapp::MODE_NORMAL);

			Layer::Handle layer(get_canvas_interface()->add_layer_to("polygon",canvas,depth));
			layer->set_description(get_id());
			get_canvas_interface()->signal_layer_new_description()(layer,layer->get_description());

			layer->disconnect_dynamic_param("vector_list");
			if(!layer->set_param("vector_list",polygon_point_list))
			{
				group.cancel();
				get_canvas_view()->get_ui_interface()->error("Unable to set layer parameter");
				return;
			}

			{
				synfigapp::Action::Handle action(synfigapp::Action::create("value_desc_convert"));
				synfigapp::ValueDesc value_desc(layer,"vector_list");
				action->set_param("canvas",get_canvas());
				action->set_param("canvas_interface",get_canvas_interface());
				action->set_param("value_desc",value_desc);
				action->set_param("type","dynamic_list");
				if(!get_canvas_interface()->get_instance()->perform_action(action))
				{
					group.cancel();
					get_canvas_view()->get_ui_interface()->error("Unable to execute action \"value_desc_convert\"");
					return;
				}
			}
			egress_on_selection_change=false;
			get_canvas_interface()->get_selection_manager()->clear_selected_layers();
			get_canvas_interface()->get_selection_manager()->set_selected_layer(layer);
			egress_on_selection_change=true;
			//get_canvas_interface()->signal_dirty_preview()();
		}
/*
		else
		{
			ValueNode::Handle value_node=(ValueNode_Const::create(polygon_point_list));
			std::string valuenode_name="Poly";
			while(studio::App::dialog_entry("New Polygon", "Please enter the new ID for this value_node",valuenode_name))
				if(get_canvas_interface()->add_value_node(value_node,valuenode_name))
					return true;
		}
*/
	reset();
	increment_id();
}

Smach::event_result
StatePolygon_Context::event_mouse_click_handler(const Smach::event& x)
{
	synfig::info("STATE ROTOPOLYGON: Received mouse button down Event");
	const EventMouse& event(*reinterpret_cast<const EventMouse*>(&x));
	switch(event.button)
	{
	case BUTTON_LEFT:
		polygon_point_list.push_back(get_work_area()->snap_point_to_grid(event.pos));
		refresh_ducks();
		return Smach::RESULT_ACCEPT;

	case BUTTON_RIGHT: // Intercept the right-button click to short-circuit the pop-up menu
		if (!getenv("SYNFIG_ENABLE_POPUP_MENU_IN_ALL_TOOLS"))
			return Smach::RESULT_ACCEPT;

	default:
		return Smach::RESULT_OK;
	}
}


void
StatePolygon_Context::refresh_ducks()
{
	get_work_area()->clear_ducks();

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
StatePolygon_Context::on_polygon_duck_change(const synfig::Point &point, std::list<synfig::Point>::iterator iter)
{
	*iter=point;
	return true;
}
