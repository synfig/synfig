/* === S Y N F I G ========================================================= */
/*!	\file activepointsetoff.cpp
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

#include "activepointsetoff.h"
#include "activepointsetsmart.h"
#include "valuenodelinkconnect.h"
#include "valuenodereplace.h"

#include "activepointset.h"
#include "activepointadd.h"

#include "valuedescconnect.h"

#include <synfigapp/canvasinterface.h>
#include <synfigapp/localization.h>

#endif

using namespace std;
using namespace etl;
using namespace synfig;
using namespace synfigapp;
using namespace Action;

/* === M A C R O S ========================================================= */

ACTION_INIT(Action::ActivepointSetOff);
ACTION_SET_NAME(Action::ActivepointSetOff,"ActivepointSetOff");
ACTION_SET_LOCAL_NAME(Action::ActivepointSetOff,N_("Mark Activepoint as \"Off\""));
ACTION_SET_TASK(Action::ActivepointSetOff,"set_off");
ACTION_SET_CATEGORY(Action::ActivepointSetOff,Action::CATEGORY_ACTIVEPOINT|Action::CATEGORY_VALUEDESC);
ACTION_SET_PRIORITY(Action::ActivepointSetOff,-9);
ACTION_SET_VERSION(Action::ActivepointSetOff,"0.0");

/* === G L O B A L S ======================================================= */

/* === P R O C E D U R E S ================================================= */

/* === M E T H O D S ======================================================= */

Action::ActivepointSetOff::ActivepointSetOff():
	index()
{
	activepoint.set_time(Time::begin()-1);
	time_set=false;
	set_dirty(true);
}

Action::ParamVocab
Action::ActivepointSetOff::get_param_vocab()
{
	ParamVocab ret(Action::CanvasSpecific::get_param_vocab());

	ret.push_back(ParamDesc("value_desc",Param::TYPE_VALUEDESC)
		.set_local_name(_("ValueDesc"))
	);

	ret.push_back(ParamDesc("activepoint",Param::TYPE_ACTIVEPOINT)
		.set_local_name(_("Activepoint"))
		.set_optional()
	);

	ret.push_back(ParamDesc("time",Param::TYPE_TIME)
		.set_local_name(_("Time"))
		.set_optional()
	);

	return ret;
}

bool
Action::ActivepointSetOff::is_candidate(const ParamList &x)
{
	if (!candidate_check(get_param_vocab(),x))
		return false;

	ValueDesc value_desc(x.find("value_desc")->second.get_value_desc());

	if (!(value_desc.parent_is_value_node() &&
		  // We need a dynamic list.
		  ValueNode_DynamicList::Handle::cast_dynamic(value_desc.get_parent_value_node())))
		return false;

	Canvas::Handle canvas(x.find("canvas")->second.get_canvas());

	// We are only a candidate if this canvas is animated.
	return (canvas->rend_desc().get_time_start() != canvas->rend_desc().get_time_end() &&
			// We need either an activepoint or a time.
			(x.count("activepoint") || x.count("time")));
}

bool
Action::ActivepointSetOff::set_param(const synfig::String& name, const Action::Param &param)
{
	if(name=="value_desc" && param.get_type()==Param::TYPE_VALUEDESC)
	{
		value_desc=param.get_value_desc();

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
Action::ActivepointSetOff::is_ready()const
{
	if(!value_node)
		synfig::error("Missing value_node");

	if(activepoint.get_time()==(Time::begin()-1))
		synfig::error("Missing activepoint");

	if(!value_node || activepoint.get_time()==(Time::begin()-1))
		return false;
	return Action::CanvasSpecific::is_ready();
}

// This function is called if a time is specified, but not
// a activepoint. In this case, we need to calculate the value
// of the activepoint
void
Action::ActivepointSetOff::calc_activepoint()
{
	const Time time(activepoint.get_time());

	try { activepoint=*value_node->list[index].find(time); }
	catch(...)
	{
		activepoint.set_time(time);
		activepoint.set_state(value_node->list[index].status_at_time(time));
		activepoint.set_priority(0);
	}
}

void
Action::ActivepointSetOff::prepare()
{
	clear();

	// Turn the activepoint off
	activepoint.set_state(false);

	Action::Handle action(ActivepointSetSmart::create());

	action->set_param("edit_mode",get_edit_mode());
	action->set_param("canvas",get_canvas());
	action->set_param("canvas_interface",get_canvas_interface());
	action->set_param("value_desc",value_desc);
	action->set_param("activepoint",activepoint);

	assert(action->is_ready());
	if(!action->is_ready())
		throw Error(Error::TYPE_NOTREADY);

	add_action_front(action);
}
