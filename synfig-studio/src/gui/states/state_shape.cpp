/* === S Y N F I G ========================================================= */
/*!	\file state_shape.cpp
**	\brief Generic shape tool state
**
**	$Id$
**
**	\legal
**	Copyright (c) 2002-2005 Robert B. Quattlebaum Jr., Adrian Bentley
**	Copyright (c) 2008 Chris Moore
**	Copyright (c) 2010 Carlos López
**	Copyright (c) 2016 caryoscelus
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

#include "state_shape.h"
#include "docks/dock_toolbox.h"

#endif

/* === U S I N G =========================================================== */

using namespace std;
using namespace etl;
using namespace synfig;
using namespace studio;

/* === M A C R O S ========================================================= */

/* === G L O B A L S ======================================================= */

/* === M E T H O D S ======================================================= */

void
StateShape_Context::increment_id()
{
	String id(get_id());
	int number=1;
	int digits=0;

	if(id.empty())
		id=get_name();

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

void
StateShape_Context::refresh_tool_options()
{
	App::dialog_tool_options->clear();
	App::dialog_tool_options->set_widget(options_table);
	App::dialog_tool_options->set_local_name(get_local_name());
	App::dialog_tool_options->set_name(get_name_lower());
}

Smach::event_result
StateShape_Context::event_refresh_tool_options(const Smach::event& /*x*/)
{
	refresh_tool_options();
	return Smach::RESULT_ACCEPT;
}

Smach::event_result
StateShape_Context::event_refresh_handler(const Smach::event& /*x*/)
{
	refresh_ducks();
	return Smach::RESULT_ACCEPT;
}

Smach::event_result
StateShape_Context::event_stop_handler(const Smach::event& /*x*/)
{
	throw &state_normal;
	return Smach::RESULT_OK;
}

void
StateShape_Context::load_settings()
{
	try
	{
		do_load_settings();
	}
	catch(...)
	{
		synfig::warning("State "+get_name()+": Caught exception when attempting to load settings.");
	}
}

void
StateShape_Context::save_settings()
{
	try
	{
		do_save_settings();
	}
	catch(...)
	{
		synfig::warning("State "+get_name()+": Caught exception when attempting to save settings.");
	}
}

void
StateShape_Context::do_load_settings()
{
	synfig::ChangeLocale change_locale(LC_NUMERIC, "C");
	String value;

	if(settings.get_value(get_name_lower()+".id",value))
		set_id(value);
	else
		set_id(get_name());

	if(settings.get_value(get_name_lower()+".blend",value) && value != "")
		set_blend(atoi(value.c_str()));
	else
		set_blend(0);//(int)Color::BLEND_COMPOSITE); //0 should be blend composites value

	if(settings.get_value(get_name_lower()+".opacity",value))
		set_opacity(atof(value.c_str()));
	else
		set_opacity(1);

	if(settings.get_value(get_name_lower()+".bline_width",value) && value != "")
		set_bline_width(Distance(atof(value.c_str()), App::distance_system));
	else
		set_bline_width(Distance(1, App::distance_system)); // default width

	if(settings.get_value(get_name_lower()+".feather",value))
		set_feather_size(Distance(atof(value.c_str()), App::distance_system));
	else
		set_feather_size(Distance(0, App::distance_system)); // default feather

	if(settings.get_value(get_name_lower()+".invert",value) && value != "0")
		set_invert(true);
	else
		set_invert(false);

	if(settings.get_value(get_name_lower()+".layer_shape",value) && value=="0")
		set_layer_shape_flag(false);
	else
		set_layer_shape_flag(true);

	if(settings.get_value(get_name_lower()+".layer_region",value) && value=="1")
		set_layer_region_flag(true);
	else
		set_layer_region_flag(false);

	if(settings.get_value(get_name_lower()+".layer_outline",value) && value=="1")
		set_layer_outline_flag(true);
	else
		set_layer_outline_flag(false);

	if(settings.get_value(get_name_lower()+".layer_advanced_outline",value) && value=="1")
		set_layer_advanced_outline_flag(true);
	else
		set_layer_advanced_outline_flag(false);

	if(settings.get_value(get_name_lower()+".layer_curve_gradient",value) && value=="1")
		set_layer_curve_gradient_flag(true);
	else
		set_layer_curve_gradient_flag(false);

	if(settings.get_value(get_name_lower()+".layer_plant",value) && value=="1")
		set_layer_plant_flag(true);
	else
		set_layer_plant_flag(false);

	if(settings.get_value(get_name_lower()+".layer_link_origins",value) && value=="0")
		set_layer_link_origins_flag(false);
	else
		set_layer_link_origins_flag(true);

	// determine layer flags
	layer_shape_flag = get_layer_shape_flag();
	layer_region_flag = get_layer_region_flag();
	layer_outline_flag = get_layer_outline_flag();
	layer_advanced_outline_flag = get_layer_outline_flag();
	layer_curve_gradient_flag = get_layer_curve_gradient_flag();
	layer_plant_flag = get_layer_plant_flag();
}

void
StateShape_Context::do_save_settings()
{
	synfig::ChangeLocale change_locale(LC_NUMERIC, "C");
	settings.set_value(get_name_lower()+".id",get_id().c_str());
	settings.set_value(get_name_lower()+".blend",strprintf("%d",get_blend()));
	settings.set_value(get_name_lower()+".opacity",strprintf("%f",(float)get_opacity()));
	settings.set_value(get_name_lower()+".bline_width", bline_width_dist.get_value().get_string());
	settings.set_value(get_name_lower()+".feather", feather_dist.get_value().get_string());
	settings.set_value(get_name_lower()+".invert",get_invert()?"1":"0");
	settings.set_value(get_name_lower()+".layer_shape",get_layer_shape_flag()?"1":"0");
	settings.set_value(get_name_lower()+".layer_outline",get_layer_outline_flag()?"1":"0");
	settings.set_value(get_name_lower()+".layer_advanced_outline",get_layer_advanced_outline_flag()?"1":"0");
	settings.set_value(get_name_lower()+".layer_region",get_layer_region_flag()?"1":"0");
	settings.set_value(get_name_lower()+".layer_curve_gradient",get_layer_curve_gradient_flag()?"1":"0");
	settings.set_value(get_name_lower()+".layer_plant",get_layer_plant_flag()?"1":"0");
	settings.set_value(get_name_lower()+".layer_link_origins",get_layer_link_origins_flag()?"1":"0");
}

StateShape_Context::StateShape_Context(CanvasView* canvas_view) :
	State_Context(canvas_view),
	is_working(*canvas_view),
	prev_workarea_layer_status_(get_work_area()->get_allow_layer_clicks()),
	duckmatic_push(get_work_area()),
	settings(synfigapp::Main::get_selected_input_device()->settings()),
	opacity_hscl(0.0f, 1.0125f, 0.0125f)
{
}

StateShape_Context::~StateShape_Context()
{
}

void
StateShape_Context::leave()
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
