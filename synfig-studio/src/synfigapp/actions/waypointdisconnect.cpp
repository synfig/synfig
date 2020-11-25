/* === S Y N F I G ========================================================= */
/*!	\file waypointdisconnect.cpp
**	\brief Template File
**
**	$Id$
**
**	\legal
**	Copyright (c) 2002-2005 Robert B. Quattlebaum Jr., Adrian Bentley
**	Copyright (c) 2008 Chris Moore
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

#include "waypointdisconnect.h"
#include <synfigapp/canvasinterface.h>
#include <synfig/valuenodes/valuenode_const.h>

#include <synfigapp/localization.h>

#endif

using namespace std;
using namespace etl;
using namespace synfig;
using namespace synfigapp;
using namespace Action;

/* === M A C R O S ========================================================= */

ACTION_INIT(Action::WaypointDisconnect);
ACTION_SET_NAME(Action::WaypointDisconnect,"WaypointDisconnect");
ACTION_SET_LOCAL_NAME(Action::WaypointDisconnect,N_("Disconnect Waypoint"));
ACTION_SET_TASK(Action::WaypointDisconnect,"disconnect");
ACTION_SET_CATEGORY(Action::WaypointDisconnect,Action::CATEGORY_WAYPOINT);
ACTION_SET_PRIORITY(Action::WaypointDisconnect,0);
ACTION_SET_VERSION(Action::WaypointDisconnect,"0.0");

/* === G L O B A L S ======================================================= */

/* === P R O C E D U R E S ================================================= */

/* === M E T H O D S ======================================================= */

Action::WaypointDisconnect::WaypointDisconnect():
	waypoint_time_set(false),
	time_set(false)
{
}

Action::ParamVocab
Action::WaypointDisconnect::get_param_vocab()
{
	ParamVocab ret(Action::CanvasSpecific::get_param_vocab());

	ret.push_back(ParamDesc("parent_value_node",Param::TYPE_VALUENODE)
		.set_local_name(_("Parent ValueNode"))
	);

	ret.push_back(ParamDesc("waypoint_time",Param::TYPE_TIME)
		.set_local_name(_("Waypoint Time"))
	);

	ret.push_back(ParamDesc("time",Param::TYPE_TIME)
		.set_local_name(_("Time"))
		.set_optional()
	);

	return ret;
}

bool
Action::WaypointDisconnect::is_candidate(const ParamList &x)
{
	return candidate_check(get_param_vocab(),x);
}

bool
Action::WaypointDisconnect::set_param(const synfig::String& name, const Action::Param &param)
{
	if(name=="parent_value_node" && param.get_type()==Param::TYPE_VALUENODE)
	{
		parent_value_node=ValueNode_Animated::Handle::cast_dynamic(param.get_value_node());

		return static_cast<bool>(parent_value_node);
	}

	if(name=="waypoint_time" && param.get_type()==Param::TYPE_TIME)
	{
		waypoint_time=param.get_time();
		waypoint_time_set=true;

		return true;
	}

	if(name=="time" && param.get_type()==Param::TYPE_TIME)
	{
		time=param.get_time();
		time_set=true;

		return true;
	}

	return Action::CanvasSpecific::set_param(name,param);
}

bool
Action::WaypointDisconnect::is_ready()const
{
	if(!parent_value_node || !waypoint_time_set)
		return false;
	return Action::CanvasSpecific::is_ready();
}

void
Action::WaypointDisconnect::perform()
{
	ValueNode_Animated::WaypointList::iterator iter(parent_value_node->find(waypoint_time));

	old_value_node=iter->get_value_node();

	iter->set_value_node(ValueNode_Const::create((*old_value_node)(time_set ? time : waypoint_time)));

	/*
	if(get_canvas()->get_time()!=time)
		set_dirty(true);
	else
		set_dirty(false);

	if(get_canvas_interface())
	{
		get_canvas_interface()->signal_value_node_changed()(parent_value_node);
	}
	*/
}

void
Action::WaypointDisconnect::undo()
{
	ValueNode_Animated::WaypointList::iterator iter(parent_value_node->find(waypoint_time));

	iter->set_value_node(old_value_node);

	/*if(get_canvas()->get_time()!=time)
		set_dirty(true);
	else
		set_dirty(false);

	if(get_canvas_interface())
	{
		get_canvas_interface()->signal_value_node_changed()(parent_value_node);
	}*/
}
