/* === S Y N F I G ========================================================= */
/*!	\file state_bline.cpp
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

#include "state_bline.h"
#include "canvasview.h"
#include "workarea.h"
#include "app.h"
#include <synfig/valuenode_bline.h>
#include <ETL/hermite>
#include <ETL/calculus>
#include <utility>
#include "event_mouse.h"
#include "event_layerclick.h"
#include "toolbox.h"
#include "dialog_tooloptions.h"
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

	bool on_vertex_change(const synfig::Point &point, synfig::ValueNode_Const::Handle value_node);
	bool on_tangent1_change(const synfig::Point &point, synfig::ValueNode_Const::Handle value_node);
	bool on_tangent2_change(const synfig::Point &point, synfig::ValueNode_Const::Handle value_node);


	void popup_handle_menu(synfig::ValueNode_Const::Handle value_node);
	void popup_vertex_menu(synfig::ValueNode_Const::Handle value_node);
	void popup_bezier_menu(float location, synfig::ValueNode_Const::Handle value_node);

	void bline_detach_handle(synfig::ValueNode_Const::Handle value_node);
	void bline_attach_handle(synfig::ValueNode_Const::Handle value_node);
	void bline_delete_vertex(synfig::ValueNode_Const::Handle value_node);
	void bline_insert_vertex(synfig::ValueNode_Const::Handle value_node,float origin=0.5);
	void loop_bline();
	void unloop_bline();

	void refresh_ducks(bool x=true);

	Gtk::Table options_table;
	Gtk::Entry entry_id;
	Gtk::CheckButton checkbutton_layer_region;
	Gtk::CheckButton checkbutton_layer_bline;
	Gtk::CheckButton checkbutton_layer_curve_gradient;
	Gtk::CheckButton checkbutton_layer_link_offsets;
	Gtk::CheckButton checkbutton_auto_export;
	Gtk::Button button_make;
	Gtk::Button button_clear;
	Gtk::Adjustment	 adj_feather;
	Gtk::SpinButton  spin_feather;



public:

	int layers_to_create()const
	{
		return
			get_layer_region_flag()+
			get_layer_bline_flag()+
			get_layer_curve_gradient_flag();
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

	bool get_layer_bline_flag()const { return checkbutton_layer_bline.get_active(); }
	void set_layer_bline_flag(bool x) { return checkbutton_layer_bline.set_active(x); }

	bool get_layer_curve_gradient_flag()const { return checkbutton_layer_curve_gradient.get_active(); }
	void set_layer_curve_gradient_flag(bool x) { return checkbutton_layer_curve_gradient.set_active(x); }

	bool get_layer_link_offsets_flag()const { return checkbutton_layer_link_offsets.get_active(); }
	void set_layer_link_offsets_flag(bool x) { return checkbutton_layer_link_offsets.set_active(x); }

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
	const synfig::TransformStack& get_transform_stack()const { return canvas_view_->get_curr_transform_stack(); }

	void load_settings();
	void save_settings();
	void reset();
	void increment_id();
	//void on_user_click(synfig::Point point);

	bool run_();
	bool run();

	bool egress_on_selection_change;
	Smach::event_result event_layer_selection_changed_handler(const Smach::event& /*x*/)
	{
		if(egress_on_selection_change)
			throw Smach::egress_exception();
		return Smach::RESULT_OK;
	}

};	// END of class StateBLine_Context


/* === M E T H O D S ======================================================= */

StateBLine::StateBLine():
	Smach::state<StateBLine_Context>("bline")
{
	insert(event_def(EVENT_LAYER_SELECTION_CHANGED,&StateBLine_Context::event_layer_selection_changed_handler));
	insert(event_def(EVENT_STOP,&StateBLine_Context::event_stop_handler));
	insert(event_def(EVENT_REFRESH,&StateBLine_Context::event_refresh_handler));
	insert(event_def(EVENT_REFRESH_DUCKS,&StateBLine_Context::event_hijack));
	insert(event_def(EVENT_WORKAREA_MOUSE_BUTTON_DOWN,&StateBLine_Context::event_mouse_click_handler));
	insert(event_def(EVENT_WORKAREA_MOUSE_BUTTON_UP,&StateBLine_Context::event_mouse_release_handler));
	insert(event_def(EVENT_WORKAREA_MOUSE_MOTION,&StateBLine_Context::event_mouse_motion_handler));
	insert(event_def(EVENT_WORKAREA_MOUSE_BUTTON_DRAG,&StateBLine_Context::event_mouse_motion_handler));
	insert(event_def(EVENT_REFRESH_TOOL_OPTIONS,&StateBLine_Context::event_refresh_tool_options));
}

StateBLine::~StateBLine()
{
}

void
StateBLine_Context::load_settings()
{
	String value;

	if(settings.get_value("bline.layer_region",value) && value=="0")
		set_layer_region_flag(false);
	else
		set_layer_region_flag(true);

	if(settings.get_value("bline.layer_bline",value) && value=="0")
		set_layer_bline_flag(false);
	else
		set_layer_bline_flag(true);

	if(settings.get_value("bline.layer_curve_gradient",value) && value=="1")
		set_layer_curve_gradient_flag(true);
	else
		set_layer_curve_gradient_flag(false);

	if(settings.get_value("bline.layer_link_offsets",value) && value=="0")
		set_layer_link_offsets_flag(false);
	else
		set_layer_link_offsets_flag(true);

	if(settings.get_value("bline.auto_export",value) && value=="1")
		set_auto_export_flag(true);
	else
		set_auto_export_flag(false);

	if(settings.get_value("bline.id",value))
		set_id(value);
	else
		set_id("NewBLine");

	if(settings.get_value("bline.feather",value))
	{
		Real n = atof(value.c_str());
		set_feather(n);
	}

	sanity_check();
}

void
StateBLine_Context::save_settings()
{
	sanity_check();
	settings.set_value("bline.layer_bline",get_layer_bline_flag()?"1":"0");
	settings.set_value("bline.layer_region",get_layer_region_flag()?"1":"0");
	settings.set_value("bline.layer_curve_gradient",get_layer_curve_gradient_flag()?"1":"0");
	settings.set_value("bline.layer_link_offsets",get_layer_link_offsets_flag()?"1":"0");
	settings.set_value("bline.auto_export",get_auto_export_flag()?"1":"0");
	settings.set_value("bline.id",get_id().c_str());
	settings.set_value("bline.feather",strprintf("%f",get_feather()));
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
		id="NewBLine";

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


StateBLine_Context::StateBLine_Context(CanvasView* canvas_view):
	canvas_view_(canvas_view),
	is_working(*canvas_view),
	loop_(false),
	prev_workarea_layer_status_(get_work_area()->get_allow_layer_clicks()),
	duckmatic_push(get_work_area()),
	settings(synfigapp::Main::get_selected_input_device()->settings()),
	entry_id(),
	checkbutton_layer_region(_("Fill")),
	checkbutton_layer_bline(_("Outline")),
	checkbutton_layer_curve_gradient(_("Gradient")),
	checkbutton_layer_link_offsets(_("Link Offsets")),
	checkbutton_auto_export(_("Auto Export")),
	button_make(_("Make")),
	button_clear(_("Clear")),
	adj_feather(0,0,10000,0.01,0.1),
	spin_feather(adj_feather,0.01,4)
{
	depth=-1;
	egress_on_selection_change=true;
	load_settings();

	// Set up the tool options dialog
	//options_table.attach(*manage(new Gtk::Label(_("BLine Tool"))), 0, 2, 0, 1, Gtk::EXPAND|Gtk::FILL, Gtk::EXPAND|Gtk::FILL, 0, 0);
	options_table.attach(entry_id,								0, 2,  1,  2, Gtk::EXPAND|Gtk::FILL, Gtk::EXPAND|Gtk::FILL, 0, 0);
	options_table.attach(checkbutton_layer_region,				0, 2,  2,  3, Gtk::EXPAND|Gtk::FILL, Gtk::EXPAND|Gtk::FILL, 0, 0);
	options_table.attach(checkbutton_layer_bline,				0, 2,  3,  4, Gtk::EXPAND|Gtk::FILL, Gtk::EXPAND|Gtk::FILL, 0, 0);
	options_table.attach(checkbutton_layer_curve_gradient,		0, 2,  4,  5, Gtk::EXPAND|Gtk::FILL, Gtk::EXPAND|Gtk::FILL, 0, 0);
	options_table.attach(checkbutton_layer_link_offsets,		0, 2,  5,  6, Gtk::EXPAND|Gtk::FILL, Gtk::EXPAND|Gtk::FILL, 0, 0);
	options_table.attach(checkbutton_auto_export,				0, 2,  6,  7, Gtk::EXPAND|Gtk::FILL, Gtk::EXPAND|Gtk::FILL, 0, 0);
	options_table.attach(*manage(new Gtk::Label(_("Feather"))), 0, 1, 10, 11, Gtk::EXPAND|Gtk::FILL, Gtk::EXPAND|Gtk::FILL, 0, 0);
	options_table.attach(spin_feather,							1, 2, 10, 11, Gtk::EXPAND|Gtk::FILL, Gtk::EXPAND|Gtk::FILL, 0, 0);
	//options_table.attach(button_make, 0, 2, 5, 6, Gtk::EXPAND|Gtk::FILL, Gtk::EXPAND|Gtk::FILL, 0, 0);
	//button_make.signal_pressed().connect(sigc::mem_fun(*this,&StateBLine_Context::run));
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

	// Connect a signal
	//get_work_area()->signal_user_click().connect(sigc::mem_fun(*this,&studio::StateBLine_Context::on_user_click));
	get_canvas_view()->work_area->set_cursor(Gdk::CROSSHAIR);

	App::toolbox->refresh();
}

void
StateBLine_Context::refresh_tool_options()
{
	App::dialog_tool_options->clear();
	App::dialog_tool_options->set_widget(options_table);
	App::dialog_tool_options->set_local_name(_("BLine Tool"));
	App::dialog_tool_options->set_name("bline");

	App::dialog_tool_options->add_button(
		Gtk::StockID("gtk-execute"),
		_("Make BLine and/or Region")
	)->signal_clicked().connect(
		sigc::hide_return(sigc::mem_fun(
			*this,
			&StateBLine_Context::run
		))
	);

	App::dialog_tool_options->add_button(
		Gtk::StockID("gtk-clear"),
		_("Clear current BLine")
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

	get_canvas_view()->work_area->reset_cursor();

	// Restore layer clicking
	get_work_area()->set_allow_layer_clicks(prev_workarea_layer_status_);

	// Enable the time bar
	get_canvas_view()->set_sensitive_timebar(true);

	// Bring back the tables if they were out before
	if(prev_table_status)get_canvas_view()->show_tables();

//	get_canvas_view()->get_smach().process_event(EVENT_REFRESH_DUCKS);

	// Refresh the work area
	get_work_area()->queue_draw();

	App::toolbox->refresh();
}

Smach::event_result
StateBLine_Context::event_stop_handler(const Smach::event& /*x*/)
{
	synfig::info("STATE RotoBLine: Received Stop Event");
//	run();
	reset();
//	throw Smach::egress_exception();
//	get_canvas_view()->get_smach().pop_state();
	return Smach::RESULT_ACCEPT;
}

Smach::event_result
StateBLine_Context::event_refresh_handler(const Smach::event& /*x*/)
{
	synfig::info("STATE RotoBLine: Received Refresh Event");
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
		//get_canvas_view()->get_ui_interface()->error(_("You need at least two (2) points to create a BLine"));
		return false;
	}

	do
	{

		// Create the action group
		synfigapp::Action::PassiveGrouper group(get_canvas_interface()->get_instance().get(),_("New BLine"));

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

		ValueNode_Const::Handle value_node_offset(ValueNode_Const::create(Vector()));
		assert(value_node_offset);

		// Set the looping flag
		value_node_bline->set_loop(loop_);

		// Add the BLine to the canvas
		if(get_auto_export_flag() && !get_canvas_interface()->add_value_node(value_node_bline,get_id()))
		{
			//get_canvas_view()->get_ui_interface()->error(_("Unable to add value node"));
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

		synfigapp::SelectionManager::LayerList layer_selection;

		// If we were asked to create a region layer, go ahead and do so
		if(get_layer_region_flag())
		{
			synfigapp::PushMode push_mode(get_canvas_interface(),synfigapp::MODE_NORMAL);

			Layer::Handle layer(get_canvas_interface()->add_layer_to("region",canvas,depth));
			assert(layer);
			layer_selection.push_back(layer);
			layer->set_description(get_id()+_(" Region"));
			get_canvas_interface()->signal_layer_new_description()(layer,layer->get_description());

			if(get_feather())
			{
				layer->set_param("feather",get_feather());
				get_canvas_interface()->signal_layer_param_changed()(layer,"feather");
			}

			if(get_layer_bline_flag())
				layer->set_param("color",synfigapp::Main::get_background_color());

			// I don't know if it's safe to reuse the same layer_param_connect action, so I'm
			// using 2 separate ones.
			{
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
					//get_canvas_view()->get_ui_interface()->error(_("Unable to create Region layer"));
					group.cancel();
					throw String(_("Unable to create Region layer"));
					return false;
				}
			}

			if (get_layer_link_offsets_flag())
			{
				synfigapp::Action::Handle action(synfigapp::Action::create("layer_param_connect"));
				assert(action);

				action->set_param("canvas",get_canvas());
				action->set_param("canvas_interface",get_canvas_interface());
				action->set_param("layer",layer);
				if(!action->set_param("param",String("offset")))
					synfig::error("LayerParamConnect didn't like \"param\"");
				if(!action->set_param("value_node",ValueNode::Handle(value_node_offset)))
					synfig::error("LayerParamConnect didn't like \"value_node\"");

				if(!get_canvas_interface()->get_instance()->perform_action(action))
				{
					//get_canvas_view()->get_ui_interface()->error(_("Unable to create Region layer"));
					group.cancel();
					throw String(_("Unable to create Region layer"));
					return false;
				}
			}
		}

		// If we were asked to create a BLine layer, go ahead and do so
		if(get_layer_bline_flag())
		{
			synfigapp::PushMode push_mode(get_canvas_interface(),synfigapp::MODE_NORMAL);

			Layer::Handle layer(get_canvas_interface()->add_layer_to("outline",canvas,depth));
			assert(layer);
			layer_selection.push_back(layer);
			layer->set_description(get_id()+_(" Outline"));
			get_canvas_interface()->signal_layer_new_description()(layer,layer->get_description());
			if(get_feather())
			{
				layer->set_param("feather",get_feather());
				get_canvas_interface()->signal_layer_param_changed()(layer,"feather");
			}

			{
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
					//get_canvas_view()->get_ui_interface()->error(_("Unable to create BLine layer"));
					group.cancel();
					throw String(_("Unable to create Outline layer"));
					return false;
				}
			}

			if (get_layer_link_offsets_flag())
			{
				synfigapp::Action::Handle action(synfigapp::Action::create("layer_param_connect"));
				assert(action);

				action->set_param("canvas",get_canvas());
				action->set_param("canvas_interface",get_canvas_interface());
				action->set_param("layer",layer);
				if(!action->set_param("param",String("offset")))
					synfig::error("LayerParamConnect didn't like \"param\"");
				if(!action->set_param("value_node",ValueNode::Handle(value_node_offset)))
					synfig::error("LayerParamConnect didn't like \"value_node\"");

				if(!get_canvas_interface()->get_instance()->perform_action(action))
				{
					//get_canvas_view()->get_ui_interface()->error(_("Unable to create BLine layer"));
					group.cancel();
					throw String(_("Unable to create Outline layer"));
					return false;
				}
			}

			/*if(get_layer_region_flag() && !get_auto_export_flag())
			{
				get_canvas_interface()->auto_export(synfigapp::ValueDesc(layer,"bline"));
			}*/
		}



		// If we were asked to create a CurveGradient layer, go ahead and do so
		if(get_layer_curve_gradient_flag())
		{
			synfigapp::PushMode push_mode(get_canvas_interface(),synfigapp::MODE_NORMAL);

			Layer::Handle layer(get_canvas_interface()->add_layer_to("curve_gradient",canvas,depth));
			assert(layer);
			layer_selection.push_back(layer);
			layer->set_description(get_id()+_(" Gradient"));
			get_canvas_interface()->signal_layer_new_description()(layer,layer->get_description());

			{
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
					//get_canvas_view()->get_ui_interface()->error(_("Unable to create BLine layer"));
					group.cancel();
					throw String(_("Unable to create Gradient layer"));
					return false;
				}
			}

			if (get_layer_link_offsets_flag())
			{
				synfigapp::Action::Handle action(synfigapp::Action::create("layer_param_connect"));
				assert(action);

				action->set_param("canvas",get_canvas());
				action->set_param("canvas_interface",get_canvas_interface());
				action->set_param("layer",layer);
				if(!action->set_param("param",String("offset")))
					synfig::error("LayerParamConnect didn't like \"param\"");
				if(!action->set_param("value_node",ValueNode::Handle(value_node_offset)))
					synfig::error("LayerParamConnect didn't like \"value_node\"");

				if(!get_canvas_interface()->get_instance()->perform_action(action))
				{
					//get_canvas_view()->get_ui_interface()->error(_("Unable to create BLine layer"));
					group.cancel();
					throw String(_("Unable to create Gradient layer"));
					return false;
				}
			}

			/*if(get_layer_region_flag() && !get_auto_export_flag())
			{
				get_canvas_interface()->auto_export(synfigapp::ValueDesc(layer,"bline"));
			}*/
		}

		egress_on_selection_change=false;
		get_canvas_interface()->get_selection_manager()->clear_selected_layers();
		get_canvas_interface()->get_selection_manager()->set_selected_layers(layer_selection);
		egress_on_selection_change=true;

		//if(finish_bline_dialog.get_region_flag() || finish_bline_dialog.get_bline_flag())
		//	get_canvas_interface()->signal_dirty_preview()();

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
		//synfig::info("Moved Duck");
		Point p(get_work_area()->snap_point_to_grid(event.pos));
		curr_duck->set_trans_point(p);
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
		//synfig::info("Released current duck");
		curr_duck->signal_edited()(curr_duck->get_point());
		if(next_duck)
		{
			//synfig::info("grabbing next duck");
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
	synfig::info("STATE BLINE: Received mouse button down Event");
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
			//bline_point.set_width(synfigapp::Main::get_bline_width());
			bline_point.set_width(1.0f);
			bline_point.set_origin(0.5f);
			bline_point.set_split_tangent_flag(false);
			bline_point.set_tangent1(Vector(0,0));

			// set the tangent
			/*
			if(bline_point_list.empty())
			{
				bline_point.set_tangent1(Vector(1,1));
			}
			else
			{
				const Vector t(event.pos-bline_point_list.back()->get_value().get(BLinePoint()).get_vertex());
				bline_point.set_tangent1(t);
			}

			if(bline_point_list.size()>1)
			{
				std::list<synfig::ValueNode_Const::Handle>::iterator iter;
				iter=bline_point_list.end();
				iter--;iter--;
				BLinePoint prev(bline_point_list.back()->get_value().get(BLinePoint()));
				prev.set_tangent1(event.pos-(*iter)->get_value().get(BLinePoint()).get_vertex());
				bline_point_list.back()->set_value(prev);
			};
			*/

			bline_point_list.push_back(ValueNode_Const::create(bline_point));

			refresh_ducks();
			return Smach::RESULT_ACCEPT;
		}

	case BUTTON_RIGHT: // Intercept the right-button click to short-circuit the pop-up menu
		if (!getenv("SYNFIG_ENABLE_POPUP_MENU_IN_ALL_TOOLS"))
			return Smach::RESULT_ACCEPT;

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
	handle<WorkArea::Duck> duck,tduck;
	BLinePoint bline_point;

	for(iter=bline_point_list.begin();iter!=bline_point_list.end();++iter)
	{
		ValueNode_Const::Handle value_node(*iter);
		bline_point=(value_node->get_value().get(BLinePoint()));
		assert(value_node);


		// First add the duck associated with this vertex
		duck=new WorkArea::Duck(bline_point.get_vertex());
		duck->set_editable(true);
		duck->set_type(Duck::TYPE_VERTEX);
		duck->set_name(strprintf("%x-vertex",value_node.get()));
		duck->signal_edited().connect(
			sigc::bind(sigc::mem_fun(*this,&studio::StateBLine_Context::on_vertex_change),value_node)
		);
		duck->signal_user_click(2).connect(
			sigc::bind(sigc::mem_fun(*this,&studio::StateBLine_Context::popup_vertex_menu),value_node)
		);
		duck->set_guid(value_node->get_guid()^GUID::hasher(0));

		get_work_area()->add_duck(duck);

		// Add the tangent1 duck
		tduck=new WorkArea::Duck(bline_point.get_tangent1());
		tduck->set_editable(true);
		tduck->set_name(strprintf("%x-tangent1",value_node.get()));
		tduck->set_origin(duck);
		tduck->set_scalar(-0.33333333333333333);
		tduck->set_tangent(true);
		tduck->set_guid(value_node->get_guid()^GUID::hasher(3));
		tduck->signal_edited().connect(
			sigc::bind(sigc::mem_fun(*this,&studio::StateBLine_Context::on_tangent1_change),value_node)
		);
		tduck->signal_user_click(2).connect(
			sigc::bind(sigc::mem_fun(*this,&studio::StateBLine_Context::popup_handle_menu),value_node)
		);

		// See if we need to add that duck to the previous bezier
		if(bezier)
		{
			get_work_area()->add_duck(tduck);
			bezier->p2=duck;
			bezier->c2=tduck;

			bezier->signal_user_click(2).connect(
				sigc::bind(
					sigc::mem_fun(
						*this,
						&studio::StateBLine_Context::popup_bezier_menu
					),
					value_node
				)
			);

			//get_work_area()->add_duck(bezier->c1);
			//get_work_area()->add_duck(bezier->c2);
			get_work_area()->add_bezier(bezier);

			bezier=0;
		}

		// Now we see if we need to create a bezier
		list<ValueNode_Const::Handle>::iterator next(iter);
		next++;

		// If our next iterator is the end, then we don't need
		// to add a bezier.
		//if(next==bline_point_list.end() && !loop_)
		//	continue;

		bezier=new WorkArea::Bezier();

		// Add the tangent2 duck
		tduck=new WorkArea::Duck(bline_point.get_tangent2());
		tduck->set_editable(true);
		tduck->set_origin(duck);
		tduck->set_scalar(0.33333333333333333);
		tduck->set_tangent(true);
		if(bline_point.get_split_tangent_flag())
		{
			tduck->set_name(strprintf("%x-tangent2",value_node.get()));
			tduck->signal_edited().connect(
				sigc::bind(sigc::mem_fun(*this,&studio::StateBLine_Context::on_tangent2_change),value_node)
			);
		}
		else
		{
			tduck->set_name(strprintf("%x-tangent1",value_node.get()));
			tduck->signal_edited().connect(
				sigc::bind(sigc::mem_fun(*this,&studio::StateBLine_Context::on_tangent1_change),value_node)
			);
		}
		tduck->set_guid(value_node->get_guid()^GUID::hasher(4));
		tduck->signal_user_click(2).connect(
			sigc::bind(sigc::mem_fun(*this,&studio::StateBLine_Context::popup_handle_menu),value_node)
		);

		// Setup the next bezier
		bezier->p1=duck;
		bezier->c1=tduck;

		get_work_area()->add_duck(tduck);
		curr_duck=tduck;
	}

	// Add the loop, if requested
	if(bezier && loop_)
	{
		curr_duck=0;
		BLinePoint bline_point(bline_point_list.front()->get_value().get(BLinePoint()));

		duck=new WorkArea::Duck(bline_point.get_vertex());
		duck->set_editable(true);
		duck->set_name(strprintf("%x-vertex",bline_point_list.front().get()));
		duck->signal_edited().connect(
			sigc::bind(sigc::mem_fun(*this,&studio::StateBLine_Context::on_vertex_change),bline_point_list.front())
		);
		duck->signal_user_click(2).connect(
			sigc::bind(sigc::mem_fun(*this,&studio::StateBLine_Context::popup_vertex_menu),bline_point_list.front())
		);
		get_work_area()->add_duck(duck);

		// Add the tangent1 duck
		tduck=new WorkArea::Duck(bline_point.get_tangent1());
		tduck->set_editable(true);
		tduck->set_name(strprintf("%x-tangent1",bline_point_list.front().get()));
		tduck->set_origin(duck);
		tduck->set_scalar(-0.33333333333333333);
		tduck->set_tangent(true);
		tduck->signal_edited().connect(
			sigc::bind(sigc::mem_fun(*this,&studio::StateBLine_Context::on_tangent1_change),bline_point_list.front())
		);
		tduck->signal_user_click(2).connect(
			sigc::bind(sigc::mem_fun(*this,&studio::StateBLine_Context::popup_handle_menu),bline_point_list.front())
		);
		get_work_area()->add_duck(tduck);

		bezier->p2=duck;
		bezier->c2=tduck;

		bezier->signal_user_click(2).connect(
			sigc::bind(
				sigc::mem_fun(
					*this,
					&studio::StateBLine_Context::popup_bezier_menu
				),
				bline_point_list.front()
			)
		);

		//get_work_area()->add_duck(bezier->c1);
		get_work_area()->add_bezier(bezier);
	}
	if(bezier && !loop_)
	{
		duck=new WorkArea::Duck(bline_point.get_vertex());
		duck->set_editable(false);
		duck->set_name("temp");

		// Add the tangent1 duck
		tduck=new WorkArea::Duck(Vector(0,0));
		tduck->set_editable(false);
		tduck->set_name("ttemp");
		tduck->set_origin(duck);
		tduck->set_scalar(-0.33333333333333333);

		tduck->set_tangent(true);
		bezier->p2=duck;
		bezier->c2=tduck;

		get_work_area()->add_duck(bezier->p2);
		//get_work_area()->add_duck(bezier->c2);
		get_work_area()->add_bezier(bezier);

		duck->set_guid(GUID());
		tduck->set_guid(GUID());

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
StateBLine_Context::on_vertex_change(const synfig::Point &point, synfig::ValueNode_Const::Handle value_node)
{
	BLinePoint bline_point(value_node->get_value().get(BLinePoint()));
	bline_point.set_vertex(point);
	value_node->set_value(bline_point);
	//refresh_ducks();
	return true;
}

bool
StateBLine_Context::on_tangent1_change(const synfig::Point &point, synfig::ValueNode_Const::Handle value_node)
{
	BLinePoint bline_point(value_node->get_value().get(BLinePoint()));
	bline_point.set_tangent1(point);
	value_node->set_value(bline_point);
	//refresh_ducks();
	return true;
}

bool
StateBLine_Context::on_tangent2_change(const synfig::Point &point, synfig::ValueNode_Const::Handle value_node)
{
	BLinePoint bline_point(value_node->get_value().get(BLinePoint()));
	bline_point.set_tangent2(point);
	value_node->set_value(bline_point);
	//refresh_ducks();
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
		menu.items().push_back(Gtk::Menu_Helpers::MenuElem(_("Unloop BLine"),
				sigc::mem_fun(*this,&studio::StateBLine_Context::unloop_bline)
		));
	} else {
		menu.items().push_back(Gtk::Menu_Helpers::MenuElem(_("Loop BLine"),
				sigc::mem_fun(*this,&studio::StateBLine_Context::loop_bline)
		));
	}

	menu.items().push_back(Gtk::Menu_Helpers::MenuElem(_("Delete Vertex"),
		sigc::bind(
			sigc::mem_fun(*this,&studio::StateBLine_Context::bline_delete_vertex),
			value_node
		)
	));

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

	menu.popup(0,0);
}

void
StateBLine_Context::bline_insert_vertex(synfig::ValueNode_Const::Handle value_node, float origin)
{
	list<ValueNode_Const::Handle>::iterator iter;

	for(iter=bline_point_list.begin();iter!=bline_point_list.end();++iter)
		if(*iter==value_node)
		{
			list<ValueNode_Const::Handle>::iterator prev(iter);
			--prev;

			BLinePoint bline_point;

			BLinePoint next_bline_point((*iter)->get_value().get(BLinePoint()));
			BLinePoint prev_bline_point;

			if(iter!=bline_point_list.begin())
			{
				prev_bline_point=(*prev)->get_value().get(BLinePoint());
			}
			else
			{
				prev_bline_point.set_vertex(Point(0,0));
				prev_bline_point.set_width(next_bline_point.get_width());
				prev_bline_point.set_origin(0.5);
				prev_bline_point.set_split_tangent_flag(false);
			}

			etl::hermite<Vector> curve(prev_bline_point.get_vertex(),next_bline_point.get_vertex(),prev_bline_point.get_tangent2(),next_bline_point.get_tangent1());
			etl::derivative< etl::hermite<Vector> > deriv(curve);

			bline_point.set_vertex(curve(origin));
			bline_point.set_width((next_bline_point.get_width()-prev_bline_point.get_width())*origin+prev_bline_point.get_width());
#ifdef ETL_FIXED_DERIVATIVE
			bline_point.set_tangent1(deriv(origin)*std::min(1.0f-origin,origin));
#else
			bline_point.set_tangent1(-deriv(origin)*std::min(1.0f-origin,origin));
#endif
			bline_point.set_tangent2(bline_point.get_tangent1());
			bline_point.set_split_tangent_flag(false);
			bline_point.set_origin(origin);

/*
			bline_point.set_vertex((next_bline_point.get_vertex()+prev_bline_point.get_vertex())*0.5);
			bline_point.set_width((next_bline_point.get_width()+prev_bline_point.get_width())*0.5);
			bline_point.set_origin(origin);
			bline_point.set_split_tangent_flag(false);
			bline_point.set_tangent1((next_bline_point.get_vertex()-prev_bline_point.get_vertex())*0.5);
*/

			bline_point_list.insert(iter,ValueNode_Const::create(bline_point));
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

	if(bline_point.get_split_tangent_flag())
		menu.items().push_back(Gtk::Menu_Helpers::MenuElem(_("Merge Tangents"),
			sigc::bind(
				sigc::mem_fun(*this,&studio::StateBLine_Context::bline_attach_handle),
				value_node
			)
		));
	else
		menu.items().push_back(Gtk::Menu_Helpers::MenuElem(_("Split Tangents"),
			sigc::bind(
				sigc::mem_fun(*this,&studio::StateBLine_Context::bline_detach_handle),
				value_node
			)
		));

	menu.popup(0,0);
}

void
StateBLine_Context::bline_detach_handle(synfig::ValueNode_Const::Handle value_node)
{
	BLinePoint bline_point(value_node->get_value().get(BLinePoint()));
	bline_point.set_split_tangent_flag(true);
	bline_point.set_tangent2(bline_point.get_tangent1());
	value_node->set_value(bline_point);
	refresh_ducks(false);
}

void
StateBLine_Context::bline_attach_handle(synfig::ValueNode_Const::Handle value_node)
{
	BLinePoint bline_point(value_node->get_value().get(BLinePoint()));
	bline_point.set_tangent1((bline_point.get_tangent1()+bline_point.get_tangent2())*0.5);
	bline_point.set_split_tangent_flag(false);
	value_node->set_value(bline_point);
	refresh_ducks(false);
}
