/* === S Y N F I G ========================================================= */
/*!	\file waypointadd.cpp
**	\brief Template File
**
**	$Id$
**
**	\legal
**	Copyright (c) 2002-2005 Robert B. Quattlebaum Jr., Adrian Bentley
**  Copyright (c) 2008 Chris Moore
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

#include "waypointadd.h"
#include <synfigapp/canvasinterface.h>
#include <synfigapp/main.h>

#include <synfigapp/localization.h>

#endif

using namespace std;
using namespace etl;
using namespace synfig;
using namespace synfigapp;
using namespace Action;

/* === M A C R O S ========================================================= */

ACTION_INIT(Action::WaypointAdd);
ACTION_SET_NAME(Action::WaypointAdd,"WaypointAdd");
ACTION_SET_LOCAL_NAME(Action::WaypointAdd,N_("Add Waypoint"));
ACTION_SET_TASK(Action::WaypointAdd,"add");
ACTION_SET_CATEGORY(Action::WaypointAdd,Action::CATEGORY_WAYPOINT);
ACTION_SET_PRIORITY(Action::WaypointAdd,0);
ACTION_SET_VERSION(Action::WaypointAdd,"0.0");

/* === G L O B A L S ======================================================= */

/* === P R O C E D U R E S ================================================= */

/* === M E T H O D S ======================================================= */

Action::WaypointAdd::WaypointAdd()
{
	waypoint.set_time(Time::begin()-1);
	time_set=false;
	set_dirty(true);
}

Action::ParamVocab
Action::WaypointAdd::get_param_vocab()
{
	ParamVocab ret(Action::CanvasSpecific::get_param_vocab());

	ret.push_back(ParamDesc("value_node",Param::TYPE_VALUENODE)
		.set_local_name(_("Destination ValueNode (Animated)"))
	);

	ret.push_back(ParamDesc("waypoint",Param::TYPE_WAYPOINT)
		.set_local_name(_("New Waypoint"))
		.set_desc(_("Waypoint to be added"))
		.set_optional()
	);

	ret.push_back(ParamDesc("time",Param::TYPE_TIME)
		.set_local_name(_("Time"))
		.set_desc(_("Time where waypoint is to be added"))
		.set_optional()
	);

	return ret;
}

bool
Action::WaypointAdd::is_candidate(const ParamList &x)
{
	return (candidate_check(get_param_vocab(),x) &&
			// We need an animated valuenode.
			ValueNode_Animated::Handle::cast_dynamic(x.find("value_node")->second.get_value_node()) &&
			// We need either a waypoint or a time.
			(x.count("waypoint") || x.count("time")));
}

bool
Action::WaypointAdd::set_param(const synfig::String& name, const Action::Param &param)
{
	if(name=="value_node" && param.get_type()==Param::TYPE_VALUENODE)
	{
		value_node=ValueNode_Animated::Handle::cast_dynamic(param.get_value_node());
		if(time_set)
			calc_waypoint();

		return static_cast<bool>(value_node);
	}
	if(name=="waypoint" && param.get_type()==Param::TYPE_WAYPOINT && !time_set)
	{
		waypoint=param.get_waypoint();

		return true;
	}
	if(name=="time" && param.get_type()==Param::TYPE_TIME && waypoint.get_time()==Time::begin()-1)
	{
		waypoint.set_time(param.get_time());
		time_set=true;

		if(value_node)
			calc_waypoint();

		return true;
	}

	return Action::CanvasSpecific::set_param(name,param);
}

bool
Action::WaypointAdd::is_ready()const
{
	if(!value_node || waypoint.get_time()==(Time::begin()-1))
		return false;
	return Action::CanvasSpecific::is_ready();
}

// This function is called if a time is specified, but not
// a waypoint. In this case, we need to calculate the value
// of the waypoint
void
Action::WaypointAdd::calc_waypoint()
{
	Time time=waypoint.get_time();
	Waypoint original(waypoint);
	waypoint=value_node->new_waypoint_at_time(time);
	waypoint.mimic(original);
	Interpolation interp=value_node->get_interpolation();
	waypoint.set_before(interp==INTERPOLATION_UNDEFINED?synfigapp::Main::get_interpolation():interp);
	waypoint.set_after(interp==INTERPOLATION_UNDEFINED?synfigapp::Main::get_interpolation():interp);

/*
	ValueNode_Animated::WaypointList &waypoint_list(value_node->waypoint_list());
	ValueNode_Animated::WaypointList::iterator iter;

	if(waypoint_list.empty())
	{
		waypoint.set_value((*value_node)(time));
		return;
	}

	ValueNode_Animated::WaypointList::iterator closest=waypoint_list.begin();

	for(iter=waypoint_list.begin();iter!=waypoint_list.end();++iter)
	{
		const Real dist(abs(iter->get_time()-time));
		if(dist<abs(closest->get_time()-time))
			closest=iter;
	}
	if(!closest->is_static())
		waypoint.set_value_node(closest->get_value_node());
	else
		waypoint.set_value((*value_node)(time));
	*/
}

void
Action::WaypointAdd::perform()
{
	try { value_node->find(waypoint.get_time()); throw Error(_("A Waypoint already exists at this point in time (%s)"),waypoint.get_time().get_string().c_str());}
	catch (const synfig::Exception::NotFound&) { }

	try { if(value_node->find(waypoint)!=value_node->waypoint_list().end()) throw Error(_("This waypoint is already in the ValueNode"));}
	catch (const synfig::Exception::NotFound&) { }

	value_node->add(waypoint);

	value_node->changed();
/*_if(get_canvas_interface())
	{
		get_canvas_interface()->signal_value_node_changed()(value_node);
	}
	else synfig::warning("CanvasInterface not set on action");*/
}

void
Action::WaypointAdd::undo()
{
	value_node->erase(waypoint);

	value_node->changed();
/*_if(get_canvas_interface())
	{
		get_canvas_interface()->signal_value_node_changed()(value_node);
	}
	else synfig::warning("CanvasInterface not set on action");*/
}
