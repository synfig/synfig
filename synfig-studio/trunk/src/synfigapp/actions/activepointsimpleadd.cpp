/* === S Y N F I G ========================================================= */
/*!	\file activepointsimpleadd.cpp
**	\brief Simple add activepoint File
**
**	$Id: activepointsimpleadd.cpp,v 1.1.1.1 2005/01/07 03:34:37 darco Exp $
**
**	\legal
**	Copyright (c) 2004 Adrian Bentley
**
**	This software and associated documentation
**	are CONFIDENTIAL and PROPRIETARY property of
**	the above-mentioned copyright holder.
**
**	You may not copy, print, publish, or in any
**	other way distribute this software without
**	a prior written agreement with
**	the copyright holder.
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

#include "activepointsimpleadd.h"
#include <synfigapp/canvasinterface.h>

#endif

using namespace std;
using namespace etl;
using namespace synfig;
using namespace synfigapp;
using namespace Action;

/* === M A C R O S ========================================================= */

ACTION_INIT(Action::ActivepointSimpleAdd);
ACTION_SET_NAME(Action::ActivepointSimpleAdd,"waypoint_simpleadd");
ACTION_SET_LOCAL_NAME(Action::ActivepointSimpleAdd,"Simply Add Waypoint");
ACTION_SET_TASK(Action::ActivepointSimpleAdd,"add");
ACTION_SET_CATEGORY(Action::ActivepointSimpleAdd,Action::CATEGORY_WAYPOINT);
ACTION_SET_PRIORITY(Action::ActivepointSimpleAdd,0);
ACTION_SET_VERSION(Action::ActivepointSimpleAdd,"0.0");
ACTION_SET_CVS_ID(Action::ActivepointSimpleAdd,"$Id: activepointsimpleadd.cpp,v 1.1.1.1 2005/01/07 03:34:37 darco Exp $");

/* === G L O B A L S ======================================================= */

/* === P R O C E D U R E S ================================================= */

/* === M E T H O D S ======================================================= */

Action::ActivepointSimpleAdd::ActivepointSimpleAdd()
{
	set_dirty(true);
	activepoint.set_time(Time::begin()-1);
}

Action::ParamVocab
Action::ActivepointSimpleAdd::get_param_vocab()
{
	ParamVocab ret(Action::CanvasSpecific::get_param_vocab());
	
	ret.push_back(ParamDesc("value_desc",Param::TYPE_VALUEDESC)
		.set_local_name(_("Destination ValueNode (Animated)"))
	);

	ret.push_back(ParamDesc("activepoint",Param::TYPE_ACTIVEPOINT)
		.set_local_name(_("Activepoint"))
		.set_desc(_("Activepoint to be added"))
	);

	return ret;
}

bool
Action::ActivepointSimpleAdd::is_canidate(const ParamList &x)
{
	if(canidate_check(get_param_vocab(),x))
	{
		ValueDesc value_desc(x.find("value_desc")->second.get_value_desc());
		if(!value_desc.parent_is_value_node() || !ValueNode_DynamicList::Handle::cast_dynamic(value_desc.get_parent_value_node()))
			return false;

		return true;
	}
	return canidate_check(get_param_vocab(),x);
}

bool
Action::ActivepointSimpleAdd::set_param(const synfig::String& name, const Action::Param &param)
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
		
		return true;
	}
	if(name=="activepoint" && param.get_type()==Param::TYPE_ACTIVEPOINT)
	{
		activepoint = param.get_activepoint();
		
		return true;
	}

	return Action::CanvasSpecific::set_param(name,param);
}

bool
Action::ActivepointSimpleAdd::is_ready()const
{
	if(!value_node && activepoint.get_time() != (Time::begin()-1))
		return false;
	return Action::CanvasSpecific::is_ready();
}

void
Action::ActivepointSimpleAdd::perform()
{	
	//remove any pretenders that lie at our destination
	ValueNode_DynamicList::ListEntry::findresult iter = value_node->list[index]
															.find_time(activepoint.get_time());
	
	time_overwrite = false;
	if(iter.second)
	{
		overwritten_ap = *iter.first;
		time_overwrite = true;
		value_node->list[index].erase(overwritten_ap);
	}
	
	//add the value node in since it's safe
	value_node->list[index].add(activepoint);
	
	//sort them...
	value_node->list[index].timing_info.sort();
	
	// Signal that a valuenode has been changed
	value_node->changed();
}

void
Action::ActivepointSimpleAdd::undo()
{
	//remove our old version...
	ValueNode_DynamicList::ListEntry::findresult iter = value_node->list[index].find_uid(activepoint);
	
	if(!iter.second)
	{
		throw Error(_("The activepoint to remove no longer exists"));
	}
	
	//remove the offending value
	value_node->list[index].erase(*iter.first); //could also just use waypoint
	
	if(time_overwrite)
	{
		value_node->list[index].add(overwritten_ap);				
	}
	
	//sort them...
	value_node->list[index].timing_info.sort();
	
	// Signal that a valuenode has been changed
	value_node->changed();
}
