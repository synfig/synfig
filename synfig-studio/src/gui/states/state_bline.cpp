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

#include <gtkmm/dialog.h>
#include <gtkmm/entry.h>

#include <synfig/valuenode_dynamiclist.h>

#include "state_bline.h"
#include "state_normal.h"
#include "canvasview.h"
#include "workarea.h"
#include "app.h"
#include <synfig/valuenode_bline.h>
#include <ETL/hermite>
#include <ETL/calculus>
#include <utility>
#include "event_mouse.h"
#include "event_layerclick.h"
#include "docks/dock_toolbox.h"
#include "docks/dialog_tooloptions.h"
#include <gtkmm/spinbutton.h>
#include <synfig/transform.h>
#include <synfigapp/main.h>

#include "general.h"

#endif

/* === U S I N G =========================================================== */

using namespace std;
using namespace etl;
using namespace synfig;
using namespace studio;

/* === M A C R O S ========================================================= */

// if defined, show the first duck as green while drawing
#define DISTINGUISH_FIRST_DUCK

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
	synfigapp::Settings& settings;

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

	Gtk::Table options_table;
	Gtk::Entry entry_id;
	Gtk::CheckButton checkbutton_layer_region;
	Gtk::CheckButton checkbutton_layer_outline;
	Gtk::CheckButton checkbutton_layer_advanced_outline;
	Gtk::CheckButton checkbutton_layer_curve_gradient;
	Gtk::CheckButton checkbutton_layer_plant;
	Gtk::CheckButton checkbutton_layer_link_origins;
	Gtk::CheckButton checkbutton_auto_export;
	Gtk::Button button_make;
	Gtk::Button button_clear;
	Gtk::Adjustment	 adj_feather;
	Gtk::SpinButton  spin_feather;



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

	bool get_auto_export_flag()const { return checkbutton_auto_export.get_active(); }
	void set_auto_export_flag(bool x) { return checkbutton_auto_export.set_active(x); }

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
	synfig::String get_id()const { return entry_id.get_text(); }
	void set_id(const synfig::String& x) { return entry_id.set_text(x); }

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

		if(settings.get_value("bline.id",value))
			set_id(value);
		else
			set_id(_("NewSpline"));

		if(settings.get_value("bline.feather",value))
		{
			Real n = atof(value.c_str());
			set_feather(n);
		}
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
		settings.set_value("bline.layer_outline",get_layer_outline_flag()?"1":"0");
		settings.set_value("bline.layer_advanced_outline",get_layer_advanced_outline_flag()?"1":"0");
		settings.set_value("bline.layer_region",get_layer_region_flag()?"1":"0");
		settings.set_value("bline.layer_curve_gradient",get_layer_curve_gradient_flag()?"1":"0");
		settings.set_value("bline.layer_plant",get_layer_plant_flag()?"1":"0");
		settings.set_value("bline.layer_link_origins",get_layer_link_origins_flag()?"1":"0");
		settings.set_value("bline.auto_export",get_auto_export_flag()?"1":"0");
		settings.set_value("bline.id",get_id().c_str());
		settings.set_value("bline.feather",strprintf("%f",get_feather()));
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
	entry_id(),
	checkbutton_layer_region(_("Create Region")),
	checkbutton_layer_outline(_("Create Outline")),
	checkbutton_layer_advanced_outline(_("Create Advanced Outline")),
	checkbutton_layer_curve_gradient(_("Create Curve Gradient")),
	checkbutton_layer_plant(_("Create Plant")),
	checkbutton_layer_link_origins(_("Link Origins")),
	checkbutton_auto_export(_("Auto Export")),
	button_make(_("Make")),
	button_clear(_("Clear")),
	adj_feather(0,0,10000,0.01,0.1),
	spin_feather(adj_feather,0.01,4)
{
	egress_on_selection_change=true;
	load_settings();

	// Set up the tool options dialog
	options_table.attach(*manage(new Gtk::Label(_("Spline Tool"))),	0, 2,  0,  1, Gtk::EXPAND|Gtk::FILL, Gtk::EXPAND|Gtk::FILL, 0, 0);
	options_table.attach(entry_id,									0, 2,  1,  2, Gtk::EXPAND|Gtk::FILL, Gtk::EXPAND|Gtk::FILL, 0, 0);
	options_table.attach(checkbutton_layer_outline,					0, 2,  2,  3, Gtk::EXPAND|Gtk::FILL, Gtk::EXPAND|Gtk::FILL, 0, 0);
	options_table.attach(checkbutton_layer_advanced_outline,		0, 2,  3,  4, Gtk::EXPAND|Gtk::FILL, Gtk::EXPAND|Gtk::FILL, 0, 0);
	options_table.attach(checkbutton_layer_region,					0, 2,  4,  5, Gtk::EXPAND|Gtk::FILL, Gtk::EXPAND|Gtk::FILL, 0, 0);
	options_table.attach(checkbutton_layer_plant,					0, 2,  5,  6, Gtk::EXPAND|Gtk::FILL, Gtk::EXPAND|Gtk::FILL, 0, 0);
	options_table.attach(checkbutton_layer_curve_gradient,			0, 2,  6,  7, Gtk::EXPAND|Gtk::FILL, Gtk::EXPAND|Gtk::FILL, 0, 0);
	options_table.attach(checkbutton_layer_link_origins,			0, 2,  7,  8, Gtk::EXPAND|Gtk::FILL, Gtk::EXPAND|Gtk::FILL, 0, 0);
	options_table.attach(checkbutton_auto_export,					0, 2,  8,  9, Gtk::EXPAND|Gtk::FILL, Gtk::EXPAND|Gtk::FILL, 0, 0);
	options_table.attach(*manage(new Gtk::Label(_("Feather"))), 	0, 1, 10, 11, Gtk::EXPAND|Gtk::FILL, Gtk::EXPAND|Gtk::FILL, 0, 0);
	options_table.attach(spin_feather,								1, 2, 10, 11, Gtk::EXPAND|Gtk::FILL, Gtk::EXPAND|Gtk::FILL, 0, 0);
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

			Layer::Handle layer(get_canvas_interface()->add_layer_to("curve_gradient",canvas,depth));
			if (!layer)
			{
				group.cancel();
				throw String(_("Unable to create layer"));
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

			Layer::Handle layer(get_canvas_interface()->add_layer_to("plant",canvas,depth));
			if (!layer)
			{
				group.cancel();
				throw String(_("Unable to create layer"));
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

			Layer::Handle layer(get_canvas_interface()->add_layer_to("region",canvas,depth));
			if (!layer)
			{
				group.cancel();
				throw String(_("Unable to create layer"));
			}
			layer_selection.push_back(layer);
			layer->set_description(get_id()+_(" Region"));
			get_canvas_interface()->signal_layer_new_description()(layer,layer->get_description());

			if(get_feather())
			{
				layer->set_param("feather",get_feather());
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

			Layer::Handle layer(get_canvas_interface()->add_layer_to("outline",canvas,depth));
			if (!layer)
			{
				group.cancel();
				throw String(_("Unable to create layer"));
			}
			layer_selection.push_back(layer);
			layer->set_description(get_id()+_(" Outline"));
			get_canvas_interface()->signal_layer_new_description()(layer,layer->get_description());
			if(get_feather())
			{
				layer->set_param("feather",get_feather());
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

			Layer::Handle layer(get_canvas_interface()->add_layer_to("advanced_outline",canvas,depth));
			if (!layer)
			{
				group.cancel();
				throw String(_("Unable to create layer"));
			}
			layer_selection.push_back(layer);
			layer->set_description(get_id()+_(" Advanced Outline"));
			get_canvas_interface()->signal_layer_new_description()(layer,layer->get_description());
			if(get_feather())
			{
				layer->set_param("feather",get_feather());
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
	menu.items().clear();
	if(loop_)
	{
		menu.items().push_back(Gtk::Menu_Helpers::MenuElem(_("Unloop Spline"),
				sigc::mem_fun(*this,&studio::StateBLine_Context::unloop_bline)
		));
	} else {
		menu.items().push_back(Gtk::Menu_Helpers::MenuElem(_("Loop Spline"),
				sigc::mem_fun(*this,&studio::StateBLine_Context::loop_bline)
		));
	}
	menu.items().push_back(Gtk::Menu_Helpers::SeparatorElem());
	menu.items().push_back(Gtk::Menu_Helpers::MenuElem(_("Delete Vertex"),
		sigc::bind(
			sigc::mem_fun(*this,&studio::StateBLine_Context::bline_delete_vertex),
			value_node
		)
	));
	menu.items().push_back(Gtk::Menu_Helpers::SeparatorElem());
	BLinePoint bline_point(value_node->get_value().get(BLinePoint()));

	#define STATE_BLINE_ADD_MENU_ITEM(title, split_angle, split_radius)	\
		menu.items().push_back(Gtk::Menu_Helpers::MenuElem(_(title),	\
			sigc::bind(													\
				sigc::mem_fun(*this,&studio::StateBLine_Context::bline_set_split_handle), \
				value_node, split_angle, split_radius					\
			)															\
		))

	bool split_angle = bline_point.get_split_tangent_angle();
	bool split_radius = bline_point.get_split_tangent_radius();

	if (split_angle)
		STATE_BLINE_ADD_MENU_ITEM("Merge Tangents's Angle", false, split_radius);
	else
		STATE_BLINE_ADD_MENU_ITEM("Split Tangents's Angle", true, split_radius);

	if (split_radius)
		STATE_BLINE_ADD_MENU_ITEM("Merge Tangents's Radius", split_angle, false);
	else
		STATE_BLINE_ADD_MENU_ITEM("Split Tangents's Radius", split_angle, true);

	if (split_angle || split_radius)
		STATE_BLINE_ADD_MENU_ITEM("Merge Tangents", false, false);
	if (!split_angle && !split_radius)
		STATE_BLINE_ADD_MENU_ITEM("Split Tangents", true, true);

	#undef STATE_BLINE_ADD_MENU_ITEM

	menu.popup(0,0);
}

void
StateBLine_Context::popup_bezier_menu(float location, synfig::ValueNode_Const::Handle value_node)
{
	menu.items().clear();
	menu.items().push_back(Gtk::Menu_Helpers::MenuElem(_("Insert Vertex"),
		sigc::bind(
			sigc::bind(
				sigc::mem_fun(*this,&studio::StateBLine_Context::bline_insert_vertex),
				location
			),
			value_node
		)
	));
	menu.items().push_back(Gtk::Menu_Helpers::SeparatorElem());
	if(loop_)
	{
		menu.items().push_back(Gtk::Menu_Helpers::MenuElem(_("Unloop Spline"),
				sigc::mem_fun(*this,&studio::StateBLine_Context::unloop_bline)
		));
	} else {
		menu.items().push_back(Gtk::Menu_Helpers::MenuElem(_("Loop Spline"),
				sigc::mem_fun(*this,&studio::StateBLine_Context::loop_bline)
		));
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
	menu.items().clear();

	BLinePoint bline_point(value_node->get_value().get(BLinePoint()));

	#define STATE_BLINE_ADD_MENU_ITEM(title, split_angle, split_radius)	\
		menu.items().push_back(Gtk::Menu_Helpers::MenuElem(_(title),	\
			sigc::bind(													\
				sigc::mem_fun(*this,&studio::StateBLine_Context::bline_set_split_handle), \
				value_node, split_angle, split_radius					\
			)															\
		))

	bool split_angle = bline_point.get_split_tangent_angle();
	bool split_radius = bline_point.get_split_tangent_radius();

	if (split_angle)
		STATE_BLINE_ADD_MENU_ITEM("Merge Tangents's Angle", false, split_radius);
	else
		STATE_BLINE_ADD_MENU_ITEM("Split Tangents's Angle", true, split_radius);

	if (split_radius)
		STATE_BLINE_ADD_MENU_ITEM("Merge Tangents's Radius", split_angle, false);
	else
		STATE_BLINE_ADD_MENU_ITEM("Split Tangents's Radius", split_angle, true);

	if (split_angle || split_radius)
		STATE_BLINE_ADD_MENU_ITEM("Merge Tangents", false, false);
	if (!split_angle && !split_radius)
		STATE_BLINE_ADD_MENU_ITEM("Split Tangents", true, true);

	#undef STATE_BLINE_ADD_MENU_ITEM

		menu.items().push_back(Gtk::Menu_Helpers::SeparatorElem());
	if(loop_)
	{
		menu.items().push_back(Gtk::Menu_Helpers::MenuElem(_("Unloop Spline"),
				sigc::mem_fun(*this,&studio::StateBLine_Context::unloop_bline)
		));
	} else {
		menu.items().push_back(Gtk::Menu_Helpers::MenuElem(_("Loop Spline"),
				sigc::mem_fun(*this,&studio::StateBLine_Context::loop_bline)
		));
	}
	menu.items().push_back(Gtk::Menu_Helpers::SeparatorElem());
	menu.items().push_back(Gtk::Menu_Helpers::MenuElem(_("Delete Vertex"),
		sigc::bind(
			sigc::mem_fun(*this,&studio::StateBLine_Context::bline_delete_vertex),
			value_node
		)
	));
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
