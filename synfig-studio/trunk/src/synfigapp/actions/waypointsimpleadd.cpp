/* === S Y N F I G ========================================================= */
/*!	\file waypointsimpleadd.cpp
**	\brief Simple add waypoint File
**
**	$Id: waypointsimpleadd.cpp,v 1.1.1.1 2005/01/07 03:34:37 darco Exp $
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

#include "waypointsimpleadd.h"
#include <synfigapp/canvasinterface.h>

#endif

using namespace std;
using namespace etl;
using namespace synfig;
using namespace synfigapp;
using namespace Action;

/* === M A C R O S ========================================================= */

ACTION_INIT(Action::WaypointSimpleAdd);
ACTION_SET_NAME(Action::WaypointSimpleAdd,"waypoint_simpleadd");
ACTION_SET_LOCAL_NAME(Action::WaypointSimpleAdd,"Simply Add Waypoint");
ACTION_SET_TASK(Action::WaypointSimpleAdd,"add");
ACTION_SET_CATEGORY(Action::WaypointSimpleAdd,Action::CATEGORY_WAYPOINT);
ACTION_SET_PRIORITY(Action::WaypointSimpleAdd,0);
ACTION_SET_VERSION(Action::WaypointSimpleAdd,"0.0");
ACTION_SET_CVS_ID(Action::WaypointSimpleAdd,"$Id: waypointsimpleadd.cpp,v 1.1.1.1 2005/01/07 03:34:37 darco Exp $");

/* === G L O B A L S ======================================================= */

/* === P R O C E D U R E S ================================================= */

/* === M E T H O D S ======================================================= */

Action::WaypointSimpleAdd::WaypointSimpleAdd()
{
	set_dirty(true);
	waypoint.set_time(Time::begin()-1);
}

Action::ParamVocab
Action::WaypointSimpleAdd::get_param_vocab()
{
	ParamVocab ret(Action::CanvasSpecific::get_param_vocab());
	
	ret.push_back(ParamDesc("value_node",Param::TYPE_VALUENODE)
		.set_local_name(_("Destination ValueNode (Animated)"))
	);

	ret.push_back(ParamDesc("waypoint",Param::TYPE_WAYPOINT)
		.set_local_name(_("Waypoint"))
		.set_desc(_("Waypoint to be added"))
	);

	return ret;
}

bool
Action::WaypointSimpleAdd::is_canidate(const ParamList &x)
{
	return canidate_check(get_param_vocab(),x);
}

bool
Action::WaypointSimpleAdd::set_param(const synfig::String& name, const Action::Param &param)
{
	if(name=="value_node" && param.get_type()==Param::TYPE_VALUENODE)
	{
		value_node=ValueNode_Animated::Handle::cast_dynamic(param.get_value_node());
		
		return static_cast<bool>(value_node);
	}
	if(name=="waypoint" && param.get_type()==Param::TYPE_WAYPOINT)
	{
		waypoint = param.get_waypoint();
		
		return true;
	}

	return Action::CanvasSpecific::set_param(name,param);
}

bool
Action::WaypointSimpleAdd::is_ready()const
{
	if(!value_node && waypoint.get_time() != (Time::begin()-1))
		return false;
	return Action::CanvasSpecific::is_ready();
}

void
Action::WaypointSimpleAdd::perform()
{	
	//remove any pretenders that lie at our destination
	ValueNode_Animated::findresult iter = value_node->find_time(waypoint.get_time());
	
	time_overwrite = false;
	if(iter.second)
	{
		overwritten_wp = *iter.first;
		time_overwrite = true;
	}
	
	//add the value node in since it's safe
	value_node->add(waypoint);
	
	// Signal that a valuenode has been changed
	value_node->changed();
}

void
Action::WaypointSimpleAdd::undo()
{
	//remove our old version...
	ValueNode_Animated::findresult iter = value_node->find_uid(waypoint);
	
	if(!iter.second)
	{
		throw Error(_("The waypoint to remove no longer exists"));
	}
	
	//remove the offending value
	value_node->erase(*iter.first); //could also just use waypoint
	
	if(time_overwrite)
	{
		value_node->add(overwritten_wp);				
	}
	
	// Signal that a valuenode has been changed
	value_node->changed();
}
