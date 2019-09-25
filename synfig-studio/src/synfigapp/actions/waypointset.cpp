/* === S Y N F I G ========================================================= */
/*!	\file waypointset.cpp
**	\brief Template File
**
**	$Id$
**
**	\legal
**	Copyright (c) 2002-2005 Robert B. Quattlebaum Jr., Adrian Bentley
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

#include "waypointset.h"
#include <synfigapp/canvasinterface.h>

#include <synfigapp/localization.h>

#endif

using namespace std;
using namespace etl;
using namespace synfig;
using namespace synfigapp;
using namespace Action;

/* === M A C R O S ========================================================= */

ACTION_INIT(Action::WaypointSet);
ACTION_SET_NAME(Action::WaypointSet,"WaypointSet");
ACTION_SET_LOCAL_NAME(Action::WaypointSet,N_("Set Waypoint"));
ACTION_SET_TASK(Action::WaypointSet,"set");
ACTION_SET_CATEGORY(Action::WaypointSet,Action::CATEGORY_WAYPOINT);
ACTION_SET_PRIORITY(Action::WaypointSet,0);
ACTION_SET_VERSION(Action::WaypointSet,"0.0");
ACTION_SET_CVS_ID(Action::WaypointSet,"$Id$");

/* === G L O B A L S ======================================================= */

/* === P R O C E D U R E S ================================================= */

/* === M E T H O D S ======================================================= */

Action::WaypointSet::WaypointSet()
{
	set_dirty(true);
}

Action::ParamVocab
Action::WaypointSet::get_param_vocab()
{
	ParamVocab ret(Action::CanvasSpecific::get_param_vocab());

	ret.push_back(ParamDesc("value_node",Param::TYPE_VALUENODE)
		.set_local_name(_("Destination ValueNode (Animated)"))
	);

	ret.push_back(ParamDesc("waypoint",Param::TYPE_WAYPOINT)
		.set_local_name(_("Waypoint"))
		.set_desc(_("Waypoint to be changed"))
		.set_supports_multiple()
	);

	return ret;
}

bool
Action::WaypointSet::is_candidate(const ParamList &x)
{
	return candidate_check(get_param_vocab(),x);
}

bool
Action::WaypointSet::set_param(const synfig::String& name, const Action::Param &param)
{
	if(name=="value_node" && param.get_type()==Param::TYPE_VALUENODE)
	{
		value_node=ValueNode_Animated::Handle::cast_dynamic(param.get_value_node());

		return static_cast<bool>(value_node);
	}
	if(name=="waypoint" && param.get_type()==Param::TYPE_WAYPOINT)
	{
		//NOTE: at the moment there is no error checking for multiple sets!!!
		waypoints.push_back(param.get_waypoint());

		return true;
	}

	return Action::CanvasSpecific::set_param(name,param);
}

bool
Action::WaypointSet::is_ready()const
{
	if(!value_node || waypoints.empty())
		return false;
	return Action::CanvasSpecific::is_ready();
}

void
Action::WaypointSet::perform()
{
	WaypointList::iterator iter;

#if 1
	vector<WaypointList::iterator>	iters;
	vector<Waypoint>::iterator i = waypoints.begin(), end = waypoints.end();

	for(; i != end; ++i)
	{
		try { iters.push_back(value_node->find(*i)); }
		catch (const synfig::Exception::NotFound&)
		{
			throw Error(_("Unable to find waypoint"));
		}
	}

	//check to see which valuenodes are going to override because of the time...
	ValueNode_Animated::findresult timeiter;

	for(i = waypoints.begin(); i != end; ++i)
	{
		timeiter = value_node->find_time(i->get_time());

		bool candelete = timeiter.second;

		//we only want to track overwrites (not waypoints that are also being modified)
		if(candelete)
		{
			for(vector<WaypointList::iterator>::iterator ii = iters.begin(); ii != iters.end(); ++ii)
			{
				if(timeiter.first == *ii)
				{
					candelete = false;
					break;
				}
			}
		}

		//if we can still delete it after checking, record it, and then remove them all later
		if(candelete)
		{
			Waypoint w = *timeiter.first;
			overwritten_waypoints.push_back(w);
		}
	}

	//overwrite all the valuenodes we're supposed to set
	{
		i = waypoints.begin();
		for(vector<WaypointList::iterator>::iterator ii = iters.begin(); ii != iters.end() && i != end; ++ii, ++i)
		{
			old_waypoints.push_back(**ii);
			**ii = *i; //set the point to the corresponding point in the normal waypoint list
		}
	}

	//remove all the points we're supposed to be overwriting
	{
		vector<Waypoint>::iterator 	oi = overwritten_waypoints.begin(),
									oend = overwritten_waypoints.end();
		for(; oi != oend; ++oi)
		{
			// possible earlier waypoints removed and added again, but for now
			// we do not need to delete them, because we move them in rescaling function
			// and that function cannot find waypoint to move
			//value_node->erase(*oi);
		}
	}

#else
	try { iter=value_node->find(waypoint); }
	catch(synfig::Exception::NotFound)
	{
		throw Error(_("Unable to find waypoint"));
	}

	//find the value at the old time before we replace it
	ValueNode_Animated::findresult timeiter;
	timeiter = value_node->find_time(waypoint.get_time());

	//we only want to track overwrites (not inplace modifications)
	if(timeiter.second && waypoint.get_uid() == timeiter.first->get_uid())
	{
		timeiter.second = false;
	}

	//copy and overwrite
	old_waypoint=*iter;
	*iter=waypoint;

	//if we've found a unique one then we need to erase it, but store it first
	if(timeiter.second)
	{
		time_overwrite = true;
		overwritten_wp = *timeiter.first;

		value_node->erase(overwritten_wp);
	}
#endif

	// Signal that a valuenode has been changed
	value_node->changed();
}

void
Action::WaypointSet::undo()
{
	WaypointList::iterator iter;

#if 1
	vector<Waypoint>::iterator i = old_waypoints.begin(), end = old_waypoints.end();

	for(; i != end; ++i)
	{
		try { iter = value_node->find(*i); }
		catch (const synfig::Exception::NotFound&)
		{
			throw Error(_("Unable to find waypoint"));
		}

		//overwrite with old one
		*iter = *i;
	}

	//add back in all the points that we removed before...
	{
		vector<Waypoint>::iterator 	oi = overwritten_waypoints.begin(),
									oend = overwritten_waypoints.end();
		for(; oi != oend; ++oi)
		{
			value_node->add(*oi);
		}
	}

#else
	try { iter=value_node->find(old_waypoint); }
	catch(synfig::Exception::NotFound)
	{
		throw Error(_("Unable to find waypoint"));
	}

	*iter=old_waypoint;

	if(time_overwrite)
	{
		value_node->add(overwritten_wp);
	}
#endif

	// Signal that a valuenode has been changed
	value_node->changed();
}
