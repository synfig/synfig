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

#include <gtkmm/dialog.h>
#include <gtkmm/entry.h>

#include <synfig/valuenode_dynamiclist.h>
#include <synfigapp/action_system.h>
#include <synfig/valuenode_bline.h>

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


	bool on_polygon_duck_change(const studio::Duck &duck, std::list<synfig::Point>::iterator iter);


	void popup_handle_menu(synfigapp::ValueDesc value_desc);


	void refresh_ducks();

	Gtk::Table options_table;
	Gtk::Entry entry_id;
	Gtk::CheckButton checkbutton_invert;
	Gtk::CheckButton checkbutton_layer_polygon;
	Gtk::CheckButton checkbutton_layer_region;
	Gtk::CheckButton checkbutton_layer_outline;
	Gtk::CheckButton checkbutton_layer_advanced_outline;
	Gtk::CheckButton checkbutton_layer_curve_gradient;
	Gtk::CheckButton checkbutton_layer_plant;
	Gtk::CheckButton checkbutton_layer_link_origins;
	Gtk::Button button_make;
	Gtk::Adjustment	 adj_feather;
	Gtk::SpinButton  spin_feather;

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

	synfig::String get_id()const { return entry_id.get_text(); }
	void set_id(const synfig::String& x) { return entry_id.set_text(x); }

	bool get_invert()const { return checkbutton_invert.get_active(); }
	void set_invert(bool i) { checkbutton_invert.set_active(i); }

	bool get_layer_polygon_flag()const { return checkbutton_layer_polygon.get_active(); }
	void set_layer_polygon_flag(bool x) { return checkbutton_layer_polygon.set_active(x); }

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

	Real get_feather() const { return adj_feather.get_value(); }
	void set_feather(Real x) { return adj_feather.set_value(x); }

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

		if(settings.get_value("polygon.feather",value))
		{
			Real n = atof(value.c_str());
			set_feather(n);
		}
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
		settings.set_value("polygon.invert",get_invert()?"1":"0");
		settings.set_value("polygon.layer_polygon",get_layer_polygon_flag()?"1":"0");
		settings.set_value("polygon.layer_outline",get_layer_outline_flag()?"1":"0");
		settings.set_value("polygon.layer_advanced_outline",get_layer_advanced_outline_flag()?"1":"0");
		settings.set_value("polygon.layer_region",get_layer_region_flag()?"1":"0");
		settings.set_value("polygon.layer_curve_gradient",get_layer_curve_gradient_flag()?"1":"0");
		settings.set_value("polygon.layer_plant",get_layer_plant_flag()?"1":"0");
		settings.set_value("polygon.layer_link_origins",get_layer_link_origins_flag()?"1":"0");
		settings.set_value("polygon.feather",strprintf("%f",get_feather()));
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
	entry_id(),
	checkbutton_invert(_("Invert")),
	checkbutton_layer_polygon(_("Create Polygon Layer")),
	checkbutton_layer_region(_("Create Region")),
	checkbutton_layer_outline(_("Create Outline")),
	checkbutton_layer_advanced_outline(_("Create Advanced Outline")),
	checkbutton_layer_curve_gradient(_("Create Curve Gradient")),
	checkbutton_layer_plant(_("Create Plant")),
	checkbutton_layer_link_origins(_("Link Origins")),
	button_make(_("Make")),
	adj_feather(0,0,10000,0.01,0.1),
	spin_feather(adj_feather,0.01,4)
{
	egress_on_selection_change=true;
	load_settings();

	// Set up the tool options dialog
	options_table.attach(*manage(new Gtk::Label(_("Polygon Tool"))),	0, 2, 0,  1, Gtk::EXPAND|Gtk::FILL, Gtk::EXPAND|Gtk::FILL, 0, 0);
	options_table.attach(entry_id,										0, 2, 1,  2, Gtk::EXPAND|Gtk::FILL, Gtk::EXPAND|Gtk::FILL, 0, 0);

	options_table.attach(checkbutton_layer_polygon,						0, 2, 2,  3, Gtk::EXPAND|Gtk::FILL, Gtk::EXPAND|Gtk::FILL, 0, 0);
	options_table.attach(checkbutton_layer_outline,						0, 2, 3,  4, Gtk::EXPAND|Gtk::FILL, Gtk::EXPAND|Gtk::FILL, 0, 0);
	options_table.attach(checkbutton_layer_advanced_outline,			0, 2, 4,  5, Gtk::EXPAND|Gtk::FILL, Gtk::EXPAND|Gtk::FILL, 0, 0);
	options_table.attach(checkbutton_layer_region,						0, 2, 5,  6, Gtk::EXPAND|Gtk::FILL, Gtk::EXPAND|Gtk::FILL, 0, 0);
	options_table.attach(checkbutton_layer_plant,						0, 2, 6,  7, Gtk::EXPAND|Gtk::FILL, Gtk::EXPAND|Gtk::FILL, 0, 0);
	options_table.attach(checkbutton_layer_curve_gradient,				0, 2, 7,  8, Gtk::EXPAND|Gtk::FILL, Gtk::EXPAND|Gtk::FILL, 0, 0);
	options_table.attach(checkbutton_layer_link_origins,				0, 2, 8,  9, Gtk::EXPAND|Gtk::FILL, Gtk::EXPAND|Gtk::FILL, 0, 0);

	//invert flag
	options_table.attach(checkbutton_invert,							0, 2, 9, 10, Gtk::EXPAND|Gtk::FILL, Gtk::EXPAND|Gtk::FILL, 0, 0);

	//feather stuff
	options_table.attach(*manage(new Gtk::Label(_("Feather"))), 		0, 1,10, 11, Gtk::EXPAND|Gtk::FILL, Gtk::EXPAND|Gtk::FILL, 0, 0);
	options_table.attach(spin_feather,									1, 2,10, 11, Gtk::EXPAND|Gtk::FILL, Gtk::EXPAND|Gtk::FILL, 0, 0);

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
		new_list.push_back(*(new BLinePoint));
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
		layer=get_canvas_interface()->add_layer_to("polygon",canvas,depth);
		if (!layer)
		{
			get_canvas_view()->get_ui_interface()->error(_("Unable to create layer"));
			group.cancel();
			return;
		}
		layer_selection.push_back(layer);

		layer->set_param("invert",get_invert());
		get_canvas_interface()->signal_layer_param_changed()(layer,"invert");

		if(get_feather())
		{
			layer->set_param("feather",get_feather());
			get_canvas_interface()->signal_layer_param_changed()(layer,"feather");
		}

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
