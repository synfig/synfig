/* === S Y N F I G ========================================================= */
/*!	\file state_bone.cpp
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

#include "state_bone.h"
#include <synfig/general.h>

#include <synfig/valuenodes/valuenode_dynamiclist.h>

#include "state_normal.h"
#include "canvasview.h"
#include "workarea.h"
#include "app.h"
#include <synfig/valuenodes/valuenode_bline.h>
#include <ETL/hermite>
#include <ETL/calculus>
#include <utility>
#include "event_mouse.h"
#include "event_keyboard.h"
#include "event_layerclick.h"
#include "docks/dock_toolbox.h"
#include "docks/dialog_tooloptions.h"
#include "widgets/widget_enum.h"
#include "widgets/widget_distance.h"
#include <synfig/transform.h>
#include <synfigapp/main.h>

#include <gui/localization.h>

#include <gtkmm/separatormenuitem.h>
#include <gtkmm/imagemenuitem.h>
#endif

/* === U S I N G =========================================================== */

using namespace std;
using namespace etl;
using namespace studio;
using namespace synfig;

/* === M A C R O S ========================================================= */

// indentation for options layout
#ifndef SPACING
#define SPACING(name, px) \
	Gtk::Alignment *name = Gtk::manage(new Gtk::Alignment()); \
	name->set_size_request(px)
#endif

#define GAP	(3)
#define INDENTATION (6)

/* === G L O B A L S ======================================================= */

StateBone studio::state_bone;

/* === C L A S S E S & S T R U C T S ======================================= */

class studio::StateBone_Context : public sigc::trackable
{
	etl::handle<CanvasView> canvas_view_;
	CanvasView::IsWorking is_working;

	bool prev_table_status;
	bool prev_workarea_layer_status_;

	int depth;
	Canvas::Handle canvas;

	Gtk::Menu menu;

	Duckmatic::Push duckmatic_push;

	std::list<synfig::ValueNode_Const::Handle> bone_list;

	// Toolbox settings
	synfigapp::Settings& settings;

	// holder of optons
	Gtk::Table options_table;

	// title
	Gtk::Label title_label;

	// layer name:
	Gtk::Label id_label;
	Gtk::HBox id_box;
	Gtk::Entry id_entry;

	// Default bone width
	Gtk::Label bone_width_label;
	Widget_Distance bone_width_dist;

public:

	synfig::String get_id() const { return id_entry.get_text();}
	void set_id(const synfig::String& x){ return id_entry.set_text(x);}

	Real get_bone_width() const {
		return bone_width_dist.get_value().get(
				Distance::SYSTEM_UNITS,
				get_canvas_view()->get_canvas()->rend_desc()
		);
	}
	void set_bone_width(Distance x){return bone_width_dist.set_value(x);}

	Smach::event_result event_stop_handler(const Smach::event& x);

	Smach::event_result event_refresh_handler(const Smach::event& x);

	Smach::event_result event_key_press_handler(const Smach::event& x);
	Smach::event_result event_key_release_handler(const Smach::event& x);
	Smach::event_result event_mouse_click_handler(const Smach::event& x);
	Smach::event_result event_mouse_doubleclick_handler(const Smach::event& x);
	Smach::event_result event_mouse_release_handler(const Smach::event& x);
	Smach::event_result event_mouse_motion_handler(const Smach::event& x);
	Smach::event_result event_refresh_tool_options(const Smach::event& x);

	Smach::event_result event_hijack(const Smach::event& /*x*/){return Smach::RESULT_ACCEPT; }

	void refresh_tool_options();

	StateBone_Context(CanvasView* canvas_view);

	~StateBone_Context();

	const etl::handle<CanvasView>& get_canvas_view() const {return canvas_view_;}
	etl::handle<synfigapp::CanvasInterface> get_canvas_interface() const {return canvas_view_->canvas_interface();}
	synfig::Canvas::Handle get_canvas() const {return canvas_view_->get_canvas();}
	WorkArea * get_work_area() const {return canvas_view_->get_work_area();}
	const synfig::TransformStack& get_transform_stack() const {return get_work_area()->get_curr_transform_stack();}

	void load_settings();
	void save_settings();
	void reset();
	void increment_id();

	bool run();
	bool run_();

	bool egress_on_selection_change;
	Smach::event_result event_layer_selection_changed_handler(const Smach::event& /*x*/)
	{
		if(egress_on_selection_change)
			throw &state_normal;
		return Smach::RESULT_OK;
	}

}; // END of class StateBone_Context

/* === M E T H O D S ======================================================= */

StateBone::StateBone() :
	Smach::state<StateBone_Context>("bone")
{
	insert(event_def(EVENT_LAYER_SELECTION_CHANGED,		&StateBone_Context::event_layer_selection_changed_handler));
	insert(event_def(EVENT_STOP,						&StateBone_Context::event_stop_handler));
	insert(event_def(EVENT_REFRESH,						&StateBone_Context::event_refresh_handler));
	insert(event_def(EVENT_REFRESH_DUCKS,				&StateBone_Context::event_hijack));
	insert(event_def(EVENT_WORKAREA_KEY_DOWN,			&StateBone_Context::event_key_press_handler));
	insert(event_def(EVENT_WORKAREA_KEY_UP,				&StateBone_Context::event_key_release_handler));
	insert(event_def(EVENT_WORKAREA_MOUSE_BUTTON_DOWN,	&StateBone_Context::event_mouse_click_handler));
	insert(event_def(EVENT_WORKAREA_MOUSE_2BUTTON_DOWN,	&StateBone_Context::event_mouse_doubleclick_handler));
	insert(event_def(EVENT_WORKAREA_MOUSE_BUTTON_UP,	&StateBone_Context::event_mouse_release_handler));
	insert(event_def(EVENT_WORKAREA_MOUSE_MOTION,		&StateBone_Context::event_mouse_motion_handler));
	insert(event_def(EVENT_WORKAREA_MOUSE_BUTTON_DRAG,	&StateBone_Context::event_mouse_motion_handler));
	insert(event_def(EVENT_REFRESH_TOOL_OPTIONS,		&StateBone_Context::event_refresh_tool_options));
}

StateBone::~StateBone()
{
}

void* StateBone::enter_state(studio::CanvasView *machine_context) const
{
	return new StateBone_Context(machine_context);
}

void
StateBone_Context::load_settings()
{
	try
	{
		synfig::ChangeLocale change_locale(LC_NUMERIC,"C");
		string value;

		if(settings.get_value("bone.id",value))
			set_id(value);
		else
			set_id(_("NewSkeleton"));

		if(settings.get_value("bone.bone_width",value) && value!="")
			set_bone_width(Distance(atof(value.c_str()),App::distance_system));
		else
			set_bone_width(Distance(1,App::distance_system)); // default width
	}
	catch(...)
	{
		synfig::warning("State Bone: Caught exception when attempting to load settings.");
	}
}

void
StateBone_Context::save_settings()
{
	try
	{
		synfig::ChangeLocale change_locale(LC_NUMERIC,"C");

		settings.set_value("bone.id",get_id().c_str());
		settings.set_value("bone.bone_width",bone_width_dist.get_value().get_string());
	}
	catch (...)
	{
		synfig::warning("State Bone: Caught exception when attempting to save settings.");
	}
}

void
StateBone_Context::reset()
{
	bone_list.clear();
}

void
StateBone_Context::increment_id()
{
	String id(get_id());
	int number=1;
	int digits=0;

	if(id.empty())
		id="NewSkeleton";

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

StateBone_Context::StateBone_Context(CanvasView *canvas_view) :
	canvas_view_(canvas_view),
	is_working(*canvas_view),
	prev_table_status(false),
	prev_workarea_layer_status_(get_work_area()->get_allow_layer_clicks()),
	depth(-1),
	duckmatic_push(get_work_area()),
	settings(synfigapp::Main::get_selected_input_device()->settings())
{
	egress_on_selection_change=true;

	/*setting up the tool options menu*/

	// 0, title
	title_label.set_label(_("Skeleton Creation"));
	Pango::AttrList list;
	Pango::AttrInt attr = Pango::Attribute::create_attr_weight(Pango::WEIGHT_BOLD);
	list.insert(attr);
	title_label.set_attributes(list);
	title_label.set_alignment(Gtk::ALIGN_START,Gtk::ALIGN_CENTER);

	// 1, layer name label and entry
	id_label.set_label(_("Name:"));
	id_label.set_alignment(Gtk::ALIGN_START,Gtk::ALIGN_CENTER);
	SPACING(id_gap,GAP);
	id_box.pack_start(id_label, Gtk::PACK_SHRINK);
	id_box.pack_start(*id_gap, Gtk::PACK_SHRINK);

	id_box.pack_start(id_entry);

	// 2, Bone width
	bone_width_label.set_label(_("Default Bone Width:"));
	bone_width_label.set_alignment(Gtk::ALIGN_START, Gtk::ALIGN_CENTER);
	bone_width_label.set_sensitive(false);

	bone_width_dist.set_digits(2);
	bone_width_dist.set_range(0,10000000);
	bone_width_dist.set_sensitive(false);

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
	// 2, default bone width
	options_table.attach(bone_width_label,
						 0, 1, 6, 7, Gtk::EXPAND|Gtk::FILL, Gtk::FILL, 0, 0
	);
	options_table.attach(bone_width_dist,
						 1, 2, 6, 7, Gtk::EXPAND|Gtk::FILL, Gtk::FILL, 0, 0
	);

	// fine-tune options layout
	options_table.set_border_width(GAP*2); // border width
	options_table.set_row_spacings(GAP); // row gap
	options_table.set_row_spacing(0, GAP*2); // the gap between first and second row.
	options_table.set_row_spacing(2, 1); // row gap between label and icon of layer type
	//options_table.set_row_spacing(10, 0); //// the final row using border width of table
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

	// Hide the tables if they are showing
	prev_table_status=get_canvas_view()->tables_are_visible();
	if(prev_table_status)get_canvas_view()->hide_tables();

	// Disable the time bar
	get_canvas_view()->set_sensitive_timebar(false);

	get_work_area()->set_cursor(Gdk::DOT);

	App::dock_toolbox->refresh();
}

void
StateBone_Context::refresh_tool_options()
{
	App::dialog_tool_options->clear();
	App::dialog_tool_options->set_widget(options_table);
	App::dialog_tool_options->set_local_name(_("Skeleton Tool"));
	App::dialog_tool_options->set_name("bone");

	App::dialog_tool_options->add_button(
			Gtk::StockID("gtk-execute"),
			_("Make Skeleton")
	)->signal_clicked().connect(
			sigc::hide_return(sigc::mem_fun(
					*this,
					&StateBone_Context::run
			))
	);

	App::dialog_tool_options->add_button(
			Gtk::StockID("gtk-clear"),
			_("Clear current Skeleton")
	)->signal_clicked().connect(
			sigc::mem_fun(
					*this,
					&StateBone_Context::reset
			)
	);
}

Smach::event_result
StateBone_Context::event_refresh_tool_options(const Smach::event& /*x*/)
{
	refresh_tool_options();
	return Smach::RESULT_ACCEPT;
}

StateBone_Context::~StateBone_Context()
{
	try {
		run();
	} catch (...) {
		synfig::error("Internal error destroying StateBoneContext");
	}

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
StateBone_Context::event_stop_handler(const Smach::event &x)
{
	synfig::info("Skeleton Tool stop handler called");
	reset();
	return Smach::RESULT_ACCEPT;
}

Smach::event_result
StateBone_Context::event_refresh_handler(const Smach::event& x)
{
	synfig::info("Skeleton Tool refresh handler called");
	return Smach::RESULT_ACCEPT;
}

bool
StateBone_Context::run_()
{
	synfig::info("Running the main code");
	reset();
	increment_id();
	return true;
}

bool
StateBone_Context::run()
{
	String err;
	bool success(false);
	for(int i=5;i>0 && !success;i--)try
		{
			success=run_();
		}
		catch (const String& s)
		{
			err=s;
		}
	if(!success && !err.empty())
	{
		get_canvas_view()->get_ui_interface()->error(err);
	}
	return success;
}

Smach::event_result
StateBone_Context::event_key_press_handler(const Smach::event& x)
{
	const EventKeyboard& event(*reinterpret_cast<const EventKeyboard*>(&x));
	switch(event.keyval)
	{
		case GDK_KEY_Return:
		case GDK_KEY_KP_Enter:
			synfig::info("Enter pressed");
			return Smach::RESULT_ACCEPT;
	}
	return Smach::RESULT_REJECT;
}

Smach::event_result
StateBone_Context::event_key_release_handler(const Smach::event& x)
{
	const EventKeyboard& event(*reinterpret_cast<const EventKeyboard*>(&x));
	switch(event.keyval)
	{
		case GDK_KEY_Escape:
			synfig::info("Escape released");
			reset();
			return Smach::RESULT_ACCEPT;
	}
	return Smach::RESULT_REJECT;
}

Smach::event_result
StateBone_Context::event_mouse_motion_handler(const Smach::event& x)
{
	const EventMouse& event(*reinterpret_cast<const EventMouse*>(&x));

	Point p(get_work_area()->snap_point_to_grid(event.pos));
	synfig::info("Mouse motion at "+to_string(p.mag_squared()));
	return Smach::RESULT_ACCEPT;
}

Smach::event_result
StateBone_Context::event_mouse_release_handler(const Smach::event& x)
{
	const EventMouse& event(*reinterpret_cast<const EventMouse*>(&x));

	Point p(get_work_area()->snap_point_to_grid(event.pos));
	synfig::info("Mouse release at "+to_string(p.mag_squared()));
	return Smach::RESULT_ACCEPT;
}

Smach::event_result
StateBone_Context::event_mouse_click_handler(const Smach::event& x)
{
	const EventMouse& event(*reinterpret_cast<const EventMouse*>(&x));
	switch(event.button)
	{
		case BUTTON_LEFT:
		{
			synfig::info("Left Mouse Click!");
			return Smach::RESULT_ACCEPT;
		}

		case BUTTON_RIGHT:
		{
			synfig::info("Right Mouse Click!");
			return Smach::RESULT_ACCEPT;
		}

		default:
			return Smach::RESULT_OK;
	}
}

Smach::event_result
StateBone_Context::event_mouse_doubleclick_handler(const Smach::event& x)
{
	const EventMouse& event(*reinterpret_cast<const EventMouse*>(&x));
	switch(event.button)
	{
		case BUTTON_LEFT:
		{
			synfig::info("Double Click with left button!");
			run();
			return Smach::RESULT_ACCEPT;
		}
		case BUTTON_RIGHT:
		{
			synfig::info("Double Click with right button!");
			return Smach::RESULT_ACCEPT;
		}
		default:
			break;
	}
	return Smach::RESULT_OK;
}
