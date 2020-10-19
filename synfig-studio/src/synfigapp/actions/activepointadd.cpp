/* === S Y N F I G ========================================================= */
/*!	\file activepointadd.cpp
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

#include "activepointadd.h"
#include <synfigapp/canvasinterface.h>

#include <synfigapp/localization.h>

#endif

using namespace std;
using namespace etl;
using namespace synfig;
using namespace synfigapp;
using namespace Action;

/* === M A C R O S ========================================================= */

ACTION_INIT(Action::ActivepointAdd);
ACTION_SET_NAME(Action::ActivepointAdd,"ActivepointAdd");
ACTION_SET_LOCAL_NAME(Action::ActivepointAdd,N_("Add Activepoint"));
ACTION_SET_TASK(Action::ActivepointAdd,"add");
ACTION_SET_CATEGORY(Action::ActivepointAdd,Action::CATEGORY_ACTIVEPOINT);
ACTION_SET_PRIORITY(Action::ActivepointAdd,0);
ACTION_SET_VERSION(Action::ActivepointAdd,"0.0");

/* === G L O B A L S ======================================================= */

/* === P R O C E D U R E S ================================================= */

/* === M E T H O D S ======================================================= */

Action::ActivepointAdd::ActivepointAdd():
	index(),
	time_set(false)
{
	activepoint.set_time(Time::begin()-1);
	set_dirty(true);
}

Action::ParamVocab
Action::ActivepointAdd::get_param_vocab()
{
	ParamVocab ret(Action::CanvasSpecific::get_param_vocab());

	ret.push_back(ParamDesc("value_desc",Param::TYPE_VALUEDESC)
		.set_local_name(_("ValueDesc"))
	);

	ret.push_back(ParamDesc("activepoint",Param::TYPE_ACTIVEPOINT)
		.set_local_name(_("New Activepoint"))
		.set_desc(_("Activepoint to be added"))
		.set_optional()
	);

	ret.push_back(ParamDesc("time",Param::TYPE_TIME)
		.set_local_name(_("Time"))
		.set_desc(_("Time where activepoint is to be added"))
		.set_optional()
	);

	return ret;
}

bool
Action::ActivepointAdd::is_candidate(const ParamList &x)
{
	if (!candidate_check(get_param_vocab(),x))
		return false;

	ValueDesc value_desc(x.find("value_desc")->second.get_value_desc());

	return (value_desc.parent_is_value_node() &&
			// We need a dynamic list.
			ValueNode_DynamicList::Handle::cast_dynamic(value_desc.get_parent_value_node()) &&
			// We need either an activepoint or a time.
			(x.count("activepoint") || x.count("time")));
}

bool
Action::ActivepointAdd::set_param(const synfig::String& name, const Action::Param &param)
{
	if(name=="value_desc" && param.get_type()==Param::TYPE_VALUEDESC)
	{
		ValueDesc value_desc(param.get_value_desc());

		if(!value_desc.parent_is_value_node())
			return false;

		value_node=ValueNode_DynamicList::Handle::cast_dynamic(value_desc.get_parent_value_node());

		if(!value_node)
			return false;

		index=value_desc.get_index();

		if(time_set)
			calc_activepoint();

		return true;
	}
	if(name=="activepoint" && param.get_type()==Param::TYPE_ACTIVEPOINT && !time_set)
	{
		activepoint=param.get_activepoint();

		return true;
	}
	if(name=="time" && param.get_type()==Param::TYPE_TIME && activepoint.get_time()==Time::begin()-1)
	{
		activepoint.set_time(param.get_time());
		time_set=true;

		if(value_node)
			calc_activepoint();

		return true;
	}

	return Action::CanvasSpecific::set_param(name,param);
}

bool
Action::ActivepointAdd::is_ready()const
{
	if(!value_node || activepoint.get_time()==(Time::begin()-1))
		return false;
	return Action::CanvasSpecific::is_ready();
}

// This function is called if a time is specified, but not
// a activepoint. In this case, we need to calculate the value
// of the activepoint
void
Action::ActivepointAdd::calc_activepoint()
{
	const Time time(activepoint.get_time());
	activepoint.set_state(value_node->list[index].status_at_time(time));
	activepoint.set_priority(0);

	// In this case, nothing is really changing, so there will be
	// no need to redraw the window
	set_dirty(false);
}

void
Action::ActivepointAdd::perform()
{
	try { value_node->list[index].find(activepoint.get_time()); throw Error(_("A Activepoint already exists at this point in time"));}
	catch (const synfig::Exception::NotFound&) { }

	try { if(value_node->list[index].find(activepoint)!=value_node->list[index].timing_info.end()) throw Error(_("This activepoint is already in the ValueNode"));}
	catch (const synfig::Exception::NotFound&) { }

	value_node->list[index].add(activepoint);
	value_node->changed();

	/*if(get_canvas_interface())
	{
		get_canvas_interface()->signal_value_node_changed()(value_node);
	}
	else synfig::warning("CanvasInterface not set on action");
	*/
}

void
Action::ActivepointAdd::undo()
{
	value_node->list[index].erase(activepoint);
	value_node->changed();
	/*
	// Signal that a layer has been inserted
	if(get_canvas_interface())
	{
		get_canvas_interface()->signal_value_node_changed()(value_node);
	}
	else synfig::warning("CanvasInterface not set on action");
	*/
}
