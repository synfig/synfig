/* === S Y N F I G ========================================================= */
/*!	\file state_gradient.cpp
**	\brief Template File
**
**	$Id$
**
**	\legal
**	Copyright (c) 2002-2005 Robert B. Quattlebaum Jr., Adrian Bentley
**  Copyright (c) 2008 Chris Moore
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

#include "state_gradient.h"
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
#define LAYER_CREATION(button, fun, stockid, tooltip)	\
	{ \
		Gtk::Image *icon = manage(new Gtk::Image(Gtk::StockID(stockid), \
			Gtk::ICON_SIZE_SMALL_TOOLBAR)); \
		button.add(*icon); \
	} \
	button.set_relief(Gtk::RELIEF_NONE); \
	button.set_tooltip_text(tooltip); \
	button.signal_toggled().connect(sigc::mem_fun(*this, \
		&studio::StateGradient_Context::fun))
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

StateGradient studio::state_gradient;

/* === C L A S S E S & S T R U C T S ======================================= */

class studio::StateGradient_Context : public sigc::trackable
{
	etl::handle<CanvasView> canvas_view_;
	CanvasView::IsWorking is_working;

	Duckmatic::Push duckmatic_push;

	synfigapp::Settings& settings;

	Point point_holder;

	etl::handle<Duck> point2_duck;

	void refresh_ducks();

	bool prev_workarea_layer_status_;

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
	Gtk::ToggleButton layer_circle_togglebutton;
	Gtk::ToggleButton layer_linear_gradient_togglebutton;
	Gtk::ToggleButton layer_radial_gradient_togglebutton;
	Gtk::ToggleButton layer_conical_gradient_togglebutton;
	Gtk::ToggleButton layer_spiral_gradient_togglebutton;
	Gtk::HBox layer_types_box;

	// blend method
	Gtk::Label blend_label;
	Gtk::HBox blend_box;
	Widget_Enum blend_enum;

	// opacity
	Gtk::Label opacity_label;
	Gtk::HScale opacity_hscl;


public:
	synfig::String get_id()const { return id_entry.get_text(); }
	void set_id(const synfig::String& x) { return id_entry.set_text(x); }

	int get_blend()const { return blend_enum.get_value(); }
	void set_blend(int x) { return blend_enum.set_value(x); }

	Real get_opacity()const { return opacity_hscl.get_value(); }
	void set_opacity(Real x) { opacity_hscl.set_value(x); }

	Smach::event_result event_stop_handler(const Smach::event& x);

	Smach::event_result event_refresh_handler(const Smach::event& x);

	Smach::event_result event_mouse_click_handler(const Smach::event& x);
	Smach::event_result event_refresh_tool_options(const Smach::event& x);

	bool get_layer_linear_gradient_flag()const
	{
		return layer_linear_gradient_togglebutton.get_active();
	}
	void set_layer_linear_gradient_flag(bool x)
	{
		return layer_linear_gradient_togglebutton.set_active(x);
	}

	bool get_layer_radial_gradient_flag()const
	{
		return layer_radial_gradient_togglebutton.get_active();
	}
	void set_layer_radial_gradient_flag(bool x)
	{
		return layer_radial_gradient_togglebutton.set_active(x);
	}

	bool get_layer_conical_gradient_flag()const
	{
		return layer_conical_gradient_togglebutton.get_active();
	}
	void set_layer_conical_gradient_flag(bool x)
	{
		return layer_conical_gradient_togglebutton.set_active(x);
	}

	bool get_layer_spiral_gradient_flag()const
	{
		return layer_spiral_gradient_togglebutton.get_active();
	}
	void set_layer_spiral_gradient_flag(bool x)
	{
		return layer_spiral_gradient_togglebutton.set_active(x);
	}

  bool layer_linear_gradient_flag;
  bool layer_radial_gradient_flag;
  bool layer_conical_gradient_flag;
  bool layer_spiral_gradient_flag;

	void refresh_tool_options();

	StateGradient_Context(CanvasView* canvas_view);

	~StateGradient_Context();

	const etl::handle<CanvasView>& get_canvas_view()const{return canvas_view_;}
	etl::handle<synfigapp::CanvasInterface> get_canvas_interface()const{return canvas_view_->canvas_interface();}
	synfig::Canvas::Handle get_canvas()const{return canvas_view_->get_canvas();}
	WorkArea * get_work_area()const{return canvas_view_->get_work_area();}

	//void on_user_click(synfig::Point point);
	void load_settings();
	void save_settings();
	void reset();
	void increment_id();

	void make_gradient(const Point& p1, const Point& p2);
	bool egress_on_selection_change;
	Smach::event_result event_layer_selection_changed_handler(const Smach::event& /*x*/)
	{
		if(egress_on_selection_change)
			throw &state_normal; //throw Smach::egress_exception();
		return Smach::RESULT_OK;
	}

	void toggle_layer_linear_gradient();
	void toggle_layer_radial_gradient();
	void toggle_layer_conical_gradient();
	void toggle_layer_spiral_gradient();


};	// END of class StateGradient_Context

/* === M E T H O D S ======================================================= */

StateGradient::StateGradient():
	Smach::state<StateGradient_Context>("gradient")
{
	insert(event_def(EVENT_LAYER_SELECTION_CHANGED,&StateGradient_Context::event_layer_selection_changed_handler));
	insert(event_def(EVENT_STOP,&StateGradient_Context::event_stop_handler));
	insert(event_def(EVENT_TABLES_SHOW,&StateGradient_Context::event_stop_handler));
	insert(event_def(EVENT_REFRESH,&StateGradient_Context::event_refresh_handler));
	insert(event_def(EVENT_REFRESH_DUCKS,&StateGradient_Context::event_refresh_handler));
	insert(event_def(EVENT_WORKAREA_MOUSE_BUTTON_DOWN,&StateGradient_Context::event_mouse_click_handler));
	insert(event_def(EVENT_WORKAREA_MOUSE_BUTTON_DRAG,&StateGradient_Context::event_mouse_click_handler));
	insert(event_def(EVENT_WORKAREA_MOUSE_BUTTON_UP,&StateGradient_Context::event_mouse_click_handler));
	insert(event_def(EVENT_REFRESH_TOOL_OPTIONS,&StateGradient_Context::event_refresh_tool_options));
}

StateGradient::~StateGradient()
{
}

void* StateGradient::enter_state(studio::CanvasView* machine_context) const
{
	return new StateGradient_Context(machine_context);
}

void
StateGradient_Context::load_settings()
{
	try
	{
		synfig::ChangeLocale change_locale(LC_NUMERIC, "C");
		String value;

		if(settings.get_value("gradient.id",value))
			set_id(value);
		else
			set_id("Gradient");

		if(settings.get_value("gradient.layer_linear_gradient",value) && value=="0")
			set_layer_linear_gradient_flag(false);
		else
			set_layer_linear_gradient_flag(true);

		if(settings.get_value("gradient.layer_radial_gradient",value) && value=="0")
			set_layer_radial_gradient_flag(false);
		else
			set_layer_radial_gradient_flag(true);

		if(settings.get_value("gradient.layer_conical_gradient",value) && value=="0")
			set_layer_conical_gradient_flag(false);
		else
			set_layer_conical_gradient_flag(true);

		if(settings.get_value("gradient.layer_spiral_gradient",value) && value=="0")
			set_layer_spiral_gradient_flag(false);
		else
			set_layer_spiral_gradient_flag(true);

		if(settings.get_value("gradient.blend",value))
			set_blend(atoi(value.c_str()));
		else
			set_blend(Color::BLEND_COMPOSITE);

		if(settings.get_value("gradient.opacity",value))
			set_opacity(atof(value.c_str()));
		else
			set_opacity(1);

	  // determine layer flags
		layer_linear_gradient_flag = get_layer_linear_gradient_flag();
	  layer_radial_gradient_flag = get_layer_radial_gradient_flag();
	  layer_conical_gradient_flag = get_layer_conical_gradient_flag();
	  layer_spiral_gradient_flag = get_layer_spiral_gradient_flag();
	}
	catch(...)
	{
		synfig::warning("State Gradient: Caught exception when attempting to load settings.");
	}
}

void
StateGradient_Context::save_settings()
{
	try
	{
		synfig::ChangeLocale change_locale(LC_NUMERIC, "C");
		settings.set_value("gradient.id",get_id().c_str());
		settings.set_value("gradient.layer_linear_gradient",get_layer_linear_gradient_flag()?"1":"0");
		settings.set_value("gradient.layer_radial_gradient",get_layer_radial_gradient_flag()?"1":"0");
		settings.set_value("gradient.layer_conical_gradient",get_layer_conical_gradient_flag()?"1":"0");
		settings.set_value("gradient.layer_spiral_gradient",get_layer_spiral_gradient_flag()?"1":"0");
		settings.set_value("gradient.blend",strprintf("%d",get_blend()));
		settings.set_value("gradient.opacity",strprintf("%f",(float)get_opacity()));
	}
	catch(...)
	{
		synfig::warning("State Gradient: Caught exception when attempting to save settings.");
	}
}

void
StateGradient_Context::reset()
{
	refresh_ducks();
}

void
StateGradient_Context::increment_id()
{
	String id(get_id());
	int number=1;
	int digits=0;

	if(id.empty())
		id="Gradient";

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

StateGradient_Context::StateGradient_Context(CanvasView* canvas_view):
	canvas_view_(canvas_view),
	is_working(*canvas_view),
	duckmatic_push(get_work_area()),
	settings(synfigapp::Main::get_selected_input_device()->settings()),
	prev_workarea_layer_status_(get_work_area()->get_allow_layer_clicks()),
	opacity_hscl(0.0f, 1.0125f, 0.0125f)

{
	egress_on_selection_change=true;

	// Set up the tool options dialog

	// title
	title_label.set_label(_("Gradient Creation"));
	Pango::AttrList list;
	Pango::AttrInt attr = Pango::Attribute::create_attr_weight(Pango::WEIGHT_BOLD);
	list.insert(attr);
	title_label.set_attributes(list);
	title_label.set_alignment(Gtk::ALIGN_START, Gtk::ALIGN_CENTER);

	// layer name
	id_label.set_label(_("Name:"));
	SPACING(name_gap, GAP);
	id_box.pack_start(id_label, Gtk::PACK_SHRINK);
	id_box.pack_start(*name_gap, Gtk::PACK_SHRINK);
	id_box.pack_start(id_entry, Gtk::PACK_EXPAND_WIDGET);

	// layer (gradient) creation label
	layer_types_label.set_label(_("Layer Type:"));
	layer_types_label.set_alignment(Gtk::ALIGN_START, Gtk::ALIGN_CENTER);
	// layer creation buttons
	LAYER_CREATION(layer_linear_gradient_togglebutton, toggle_layer_linear_gradient,
		("synfig-layer_gradient_linear"), _("Create a linear gradient"));
	LAYER_CREATION(layer_radial_gradient_togglebutton, toggle_layer_radial_gradient,
		("synfig-layer_gradient_radial"), _("Create a radial gradient"));
	LAYER_CREATION(layer_conical_gradient_togglebutton, toggle_layer_conical_gradient,
		("synfig-layer_gradient_conical"), _("Create a conical gradient"));
	LAYER_CREATION(layer_spiral_gradient_togglebutton, toggle_layer_spiral_gradient,
		("synfig-layer_gradient_spiral"), _("Create a spiral gradient"));

	SPACING(layer_types_indent, INDENTATION);
	layer_types_box.pack_start(*layer_types_indent, Gtk::PACK_SHRINK);
	layer_types_box.pack_start(layer_linear_gradient_togglebutton, Gtk::PACK_SHRINK);
	layer_types_box.pack_start(layer_radial_gradient_togglebutton, Gtk::PACK_SHRINK);
	layer_types_box.pack_start(layer_conical_gradient_togglebutton, Gtk::PACK_SHRINK);
	layer_types_box.pack_start(layer_spiral_gradient_togglebutton, Gtk::PACK_SHRINK);

	// blend method label
	blend_label.set_label(_("Blend Method:"));
	blend_label.set_alignment(Gtk::ALIGN_START, Gtk::ALIGN_CENTER);
	SPACING(blend_gap, GAP);
	blend_box.pack_start(blend_label, Gtk::PACK_SHRINK);
	blend_box.pack_start(*blend_gap, Gtk::PACK_SHRINK);
	// blend method
	blend_enum.set_param_desc(ParamDesc(Color::BLEND_COMPOSITE,"blend_method")
		.set_local_name(_("Blend Method"))
		.set_description(_("Defines the blend method to be used for gradients")));

	// opacity label
	opacity_label.set_label(_("Opacity:"));
	opacity_label.set_alignment(Gtk::ALIGN_START, Gtk::ALIGN_CENTER);
	// opacity
	opacity_hscl.set_digits(2);
	opacity_hscl.set_value_pos(Gtk::POS_LEFT);
	opacity_hscl.set_tooltip_text(_("Opacity"));

	// options table
	// 0, title
	options_table.attach(title_label,
		0, 2,  0,  1, Gtk::FILL, Gtk::FILL, 0, 0
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

	// fine-tune options layout
	options_table.set_border_width(GAP*2); // border width
	options_table.set_row_spacings(GAP); // row gap
	options_table.set_row_spacing(0, GAP*2); // the gap between first and second row.
	options_table.set_row_spacing(2, 1); // row gap between label and icon of layer type
	//options_table.set_row_spacing(6, 0); // the final row using border width of table
	options_table.set_margin_bottom(0);
	options_table.show_all();

	load_settings();
	refresh_tool_options();
	App::dialog_tool_options->present();


	// Turn off layer clicking
	get_work_area()->set_allow_layer_clicks(false);

	get_work_area()->set_cursor(Gdk::CROSSHAIR);

	// clear out the ducks
	get_work_area()->clear_ducks();

	// Refresh the work area
	get_work_area()->queue_draw();

	get_work_area()->refresh_cursor();

	// Hide the tables if they are showing
	get_canvas_view()->hide_tables();

	// Disable the time bar
	//get_canvas_view()->set_sensitive_timebar(false);

	// Connect a signal
	//get_work_area()->signal_user_click().connect(sigc::mem_fun(*this,&studio::StateGradient_Context::on_user_click));

	App::dock_toolbox->refresh();
}

void
StateGradient_Context::refresh_tool_options()
{
	App::dialog_tool_options->clear();
	App::dialog_tool_options->set_widget(options_table);
	App::dialog_tool_options->set_local_name(_("Gradient Tool"));
	App::dialog_tool_options->set_icon("tool_gradient_icon");
}

Smach::event_result
StateGradient_Context::event_refresh_tool_options(const Smach::event& /*x*/)
{
	refresh_tool_options();
	return Smach::RESULT_ACCEPT;
}

StateGradient_Context::~StateGradient_Context()
{
	save_settings();

	// Restore layer clicking
//	get_work_area()->set_allow_layer_clicks(prev_workarea_layer_status_);
	get_work_area()->set_allow_layer_clicks(true);
	get_work_area()->reset_cursor();

	App::dialog_tool_options->clear();

	// Enable the time bar
	//get_canvas_view()->set_sensitive_timebar(true);

	// Bring back the tables if they were out before
	//if(prev_table_status)get_canvas_view()->show_tables();

	// Refresh the work area
	get_work_area()->queue_draw();

	get_canvas_view()->queue_rebuild_ducks();

	//get_canvas_view()->show_tables();

	get_work_area()->refresh_cursor();

	App::dock_toolbox->refresh();
}

Smach::event_result
StateGradient_Context::event_stop_handler(const Smach::event& /*x*/)
{
	//throw Smach::egress_exception();
	throw &state_normal;
	return Smach::RESULT_OK;
}

Smach::event_result
StateGradient_Context::event_refresh_handler(const Smach::event& /*x*/)
{
	refresh_ducks();
	return Smach::RESULT_ACCEPT;
}

void
StateGradient_Context::make_gradient(const Point& _p1, const Point& _p2)
{
	synfigapp::Action::PassiveGrouper group(get_canvas_interface()->get_instance().get(),_("New Gradient"));
	synfigapp::PushMode push_mode(get_canvas_interface(),synfigapp::MODE_NORMAL);

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
	const synfig::TransformStack& transform(get_work_area()->get_curr_transform_stack());
	const Point p1(transform.unperform(_p1));
	const Point p2(transform.unperform(_p2));

	if (get_layer_linear_gradient_flag())
	{
		egress_on_selection_change=false;
		layer=get_canvas_interface()->add_layer_to("linear_gradient",canvas,depth);
		egress_on_selection_change=true;
		if (!layer)
		{
			get_canvas_view()->get_ui_interface()->error(_("Unable to create layer"));
			group.cancel();
			return;
		}
		layer->set_param("p1",p1);
		get_canvas_interface()->signal_layer_param_changed()(layer,"p1");
		layer->set_param("p2",p2);
		get_canvas_interface()->signal_layer_param_changed()(layer,"p2");
	}

	else if (get_layer_radial_gradient_flag())
	{
		egress_on_selection_change=false;
		layer=get_canvas_interface()->add_layer_to("radial_gradient",canvas,depth);
		egress_on_selection_change=true;
		if (!layer)
		{
			get_canvas_view()->get_ui_interface()->error(_("Unable to create layer"));
			group.cancel();
			return;
		}
		layer->set_param("center",p1);
		get_canvas_interface()->signal_layer_param_changed()(layer,"center");
		layer->set_param("radius",(p2-p1).mag());
		get_canvas_interface()->signal_layer_param_changed()(layer,"radius");
	}

	else if (get_layer_conical_gradient_flag())
	{
		egress_on_selection_change=false;
		layer=get_canvas_interface()->add_layer_to("conical_gradient",canvas,depth);
		egress_on_selection_change=true;
		if (!layer)
		{
			get_canvas_view()->get_ui_interface()->error(_("Unable to create layer"));
			group.cancel();
			return;
		}
		layer->set_param("center",p1);
		get_canvas_interface()->signal_layer_param_changed()(layer,"center");
		{
			Vector diff(p2-p1);
			layer->set_param("angle",Angle::tan(diff[1],diff[0]));
			get_canvas_interface()->signal_layer_param_changed()(layer,"angle");
		}
	}

	else if (get_layer_spiral_gradient_flag())
	{
		egress_on_selection_change=false;
		layer=get_canvas_interface()->add_layer_to("spiral_gradient",canvas,depth);
		egress_on_selection_change=true;
		if (!layer)
		{
			get_canvas_view()->get_ui_interface()->error(_("Unable to create layer"));
			group.cancel();
			return;
		}
		layer->set_param("center",p1);
		get_canvas_interface()->signal_layer_param_changed()(layer,"center");
		layer->set_param("radius",(p2-p1).mag());
		get_canvas_interface()->signal_layer_param_changed()(layer,"radius");
		{
			Vector diff(p2-p1);
			layer->set_param("angle",Angle::tan(diff[1],diff[0]));
			get_canvas_interface()->signal_layer_param_changed()(layer,"angle");
		}
	}
	else return;

	layer->set_param("blend_method", get_blend());
	get_canvas_interface()->signal_layer_param_changed()(layer, "blend_method");

	layer->set_param("amount", get_opacity());
	get_canvas_interface()->signal_layer_param_changed()(layer, "amount");

	layer->set_description(get_id());
	get_canvas_interface()->signal_layer_new_description()(layer,layer->get_description());

	egress_on_selection_change=false;
	synfigapp::SelectionManager::LayerList layer_selection;
	if (!getenv("SYNFIG_TOOLS_CLEAR_SELECTION"))
		layer_selection = get_canvas_view()->get_selection_manager()->get_selected_layers();
	get_canvas_interface()->get_selection_manager()->clear_selected_layers();
	layer_selection.push_back(layer);
	get_canvas_interface()->get_selection_manager()->set_selected_layers(layer_selection);
	egress_on_selection_change=true;

	reset();
	increment_id();
}

Smach::event_result
StateGradient_Context::event_mouse_click_handler(const Smach::event& x)
{
	const EventMouse& event(*reinterpret_cast<const EventMouse*>(&x));

	if(event.key==EVENT_WORKAREA_MOUSE_BUTTON_DOWN && event.button==BUTTON_LEFT)
	{
		point_holder=get_work_area()->snap_point_to_grid(event.pos);
		etl::handle<Duck> duck=new Duck();
		duck->set_point(point_holder);
		duck->set_name("p1");
		duck->set_type(Duck::TYPE_POSITION);
		get_work_area()->add_duck(duck);

		point2_duck=new Duck();
		point2_duck->set_point(point_holder);
		point2_duck->set_name("p2");
		point2_duck->set_type(Duck::TYPE_POSITION);
		get_work_area()->add_duck(point2_duck);

		handle<Duckmatic::Bezier> bezier(new Duckmatic::Bezier());
		bezier->p1=bezier->c1=duck;
		bezier->p2=bezier->c2=point2_duck;
		get_work_area()->add_bezier(bezier);

		return Smach::RESULT_ACCEPT;
	}

	if(event.key==EVENT_WORKAREA_MOUSE_BUTTON_DRAG && event.button==BUTTON_LEFT)
	{
		if (!point2_duck) return Smach::RESULT_OK;
		point2_duck->set_point(get_work_area()->snap_point_to_grid(event.pos));
		get_work_area()->queue_draw();
		return Smach::RESULT_ACCEPT;
	}

	if(event.key==EVENT_WORKAREA_MOUSE_BUTTON_UP && event.button==BUTTON_LEFT)
	{
		make_gradient(point_holder, get_work_area()->snap_point_to_grid(event.pos));
		get_work_area()->clear_ducks();
		return Smach::RESULT_ACCEPT;
	}

	return Smach::RESULT_OK;
}


void
StateGradient_Context::refresh_ducks()
{
	get_work_area()->clear_ducks();
	get_work_area()->queue_draw();
}


void
StateGradient_Context::toggle_layer_linear_gradient()
{
	// enable linear gradient layer and disable the others
	if(!layer_linear_gradient_flag)
	{
		set_layer_linear_gradient_flag(true);
		set_layer_radial_gradient_flag(false);
		set_layer_conical_gradient_flag(false);
		set_layer_spiral_gradient_flag(false);

		// update flags
		layer_linear_gradient_flag = true;
		layer_radial_gradient_flag = false;
		layer_conical_gradient_flag = false;
		layer_spiral_gradient_flag = false;
	}

	// don't allow to disable the enabled layer
	if(get_layer_linear_gradient_flag() +
		get_layer_radial_gradient_flag() +
		get_layer_conical_gradient_flag() +
		get_layer_spiral_gradient_flag() == 0
			) set_layer_linear_gradient_flag(true);
}


void
StateGradient_Context::toggle_layer_radial_gradient()
{
	// enable radial gradient layer and disable the others
	if(!layer_radial_gradient_flag)
	{
		set_layer_linear_gradient_flag(false);
		set_layer_radial_gradient_flag(true);
		set_layer_spiral_gradient_flag(false);
		set_layer_conical_gradient_flag(false);

		// update flags
		layer_linear_gradient_flag = false;
		layer_radial_gradient_flag = true;
		layer_conical_gradient_flag = false;
		layer_spiral_gradient_flag = false;
	}

	// don't allow to disable the enabled layer
	if(get_layer_linear_gradient_flag() +
		get_layer_radial_gradient_flag() +
		get_layer_conical_gradient_flag() +
		get_layer_spiral_gradient_flag() == 0
			) set_layer_radial_gradient_flag(true);
}


void
StateGradient_Context::toggle_layer_conical_gradient()
{
	// enable conical gradient layer and disable the others
	if(!layer_conical_gradient_flag)
	{
		set_layer_linear_gradient_flag(false);
		set_layer_radial_gradient_flag(false);
		set_layer_conical_gradient_flag(true);
		set_layer_spiral_gradient_flag(false);

		// update flags
		layer_linear_gradient_flag = false;
		layer_radial_gradient_flag = false;
		layer_conical_gradient_flag = true;
		layer_spiral_gradient_flag = false;
	}

	// don't allow to disable the enabled layer
	if(get_layer_linear_gradient_flag() +
		get_layer_radial_gradient_flag() +
		get_layer_conical_gradient_flag() +
		get_layer_spiral_gradient_flag() == 0
			) set_layer_conical_gradient_flag(true);
}


void
StateGradient_Context::toggle_layer_spiral_gradient()
{
	// enable spiral gradient layer and disable the others
	if(!layer_spiral_gradient_flag)
	{
		set_layer_linear_gradient_flag(false);
		set_layer_radial_gradient_flag(false);
		set_layer_conical_gradient_flag(false);
		set_layer_spiral_gradient_flag(true);

		// update flags
		layer_linear_gradient_flag = false;
		layer_radial_gradient_flag = false;
		layer_conical_gradient_flag = false;
		layer_spiral_gradient_flag = true;
	}

	// don't allow to disable the enabled layer
	if(get_layer_linear_gradient_flag() +
		get_layer_radial_gradient_flag() +
		get_layer_conical_gradient_flag() +
		get_layer_spiral_gradient_flag() == 0
			) set_layer_spiral_gradient_flag(true);
}
