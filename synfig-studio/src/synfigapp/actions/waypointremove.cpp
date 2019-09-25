/* === S Y N F I G ========================================================= */
/*!	\file waypointremove.cpp
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

#include "waypointremove.h"
#include <synfigapp/canvasinterface.h>

#include <synfigapp/localization.h>

#endif

using namespace std;
using namespace etl;
using namespace synfig;
using namespace synfigapp;
using namespace Action;

/* === M A C R O S ========================================================= */

ACTION_INIT(Action::WaypointRemove);
ACTION_SET_NAME(Action::WaypointRemove,"WaypointRemove");
ACTION_SET_LOCAL_NAME(Action::WaypointRemove,N_("Remove Waypoint"));
ACTION_SET_TASK(Action::WaypointRemove,"remove");
ACTION_SET_CATEGORY(Action::WaypointRemove,Action::CATEGORY_WAYPOINT);
ACTION_SET_PRIORITY(Action::WaypointRemove,0);
ACTION_SET_VERSION(Action::WaypointRemove,"0.0");
ACTION_SET_CVS_ID(Action::WaypointRemove,"$Id$");

/* === G L O B A L S ======================================================= */

/* === P R O C E D U R E S ================================================= */

/* === M E T H O D S ======================================================= */

Action::WaypointRemove::WaypointRemove()
{
	waypoint.set_time(Time::begin()-1);
	set_dirty(true);
}

Action::ParamVocab
Action::WaypointRemove::get_param_vocab()
{
	ParamVocab ret(Action::CanvasSpecific::get_param_vocab());

	ret.push_back(ParamDesc("value_node",Param::TYPE_VALUENODE)
		.set_local_name(_("ValueNode (Animated)"))
	);

	ret.push_back(ParamDesc("waypoint",Param::TYPE_WAYPOINT)
		.set_local_name(_("Waypoint"))
		.set_desc(_("Waypoint to be Removed"))
	);

	return ret;
}

bool
Action::WaypointRemove::is_candidate(const ParamList &x)
{
	return candidate_check(get_param_vocab(),x);
}

bool
Action::WaypointRemove::set_param(const synfig::String& name, const Action::Param &param)
{
	if(name=="value_node" && param.get_type()==Param::TYPE_VALUENODE)
	{
		value_node=ValueNode_Animated::Handle::cast_dynamic(param.get_value_node());

		return static_cast<bool>(value_node);
	}
	if(name=="waypoint" && param.get_type()==Param::TYPE_WAYPOINT)
	{
		waypoint=param.get_waypoint();

		return true;
	}

	return Action::CanvasSpecific::set_param(name,param);
}

bool
Action::WaypointRemove::is_ready()const
{
	if(!value_node || waypoint.get_time()==(Time::begin()-1))
		return false;
	return Action::CanvasSpecific::is_ready();
}

void
Action::WaypointRemove::perform()
{
	WaypointList::iterator iter(value_node->find(waypoint));

	if((UniqueID)*iter!=(UniqueID)waypoint)
		throw Error(_("UniqueID mismatch, iter=%d, waypoint=%d"),iter->get_uid(),waypoint.get_uid());

	if(iter->get_time()!=waypoint.get_time())
		throw Error(_("Time mismatch iter=%s, waypoint=%s"),iter->get_time().get_string().c_str(),waypoint.get_time().get_string().c_str());

	waypoint=*iter;

	value_node->erase(waypoint);

	// In this case, we need to convert this to a
	// constant value node
	if(value_node->waypoint_list().size()==0)
	{
		if(!value_node_ref)
		{
			value_node_ref=waypoint.get_value_node();
			if(!value_node_ref)
				throw Error(_("Unable to create ValueNode_Reference"));
		}

		// fix 2256600 (and 2321845) : deleting the last waypoint of an exported valuenode unexported it
		// if the waypoint's value isn't exported, set its id to be the id of the parent node
		if (value_node_ref->get_id() == "" && value_node->get_id() != "")
		{
			const String id(value_node->get_id());
			Canvas::LooseHandle canvas(value_node->get_parent_canvas());
			canvas->remove_value_node(value_node, false);
			canvas->add_value_node(value_node_ref, id);
		}

		value_node->replace(value_node_ref);
		value_node->editable_waypoint_list().clear();

		if(get_canvas_interface())
		{
			get_canvas_interface()->signal_value_node_replaced()(value_node,value_node_ref);
		}
	}

	value_node->changed();
}

void
Action::WaypointRemove::undo()
{
	if(value_node_ref)
	{
		if(value_node->waypoint_list().size()!=0)
			throw Error(_("This animated value node should be empty, but for some reason it isn't. This is a bug. (1)"));

		if (value_node->get_id() == "" && value_node_ref->get_id() != "")
		{
			const String id(value_node_ref->get_id());
			Canvas::LooseHandle canvas(value_node_ref->get_parent_canvas());
			canvas->remove_value_node(value_node_ref, false);
			canvas->add_value_node(value_node, id);
		}

		value_node_ref->replace(value_node);

		waypoint.set_value_node(value_node_ref);

		if(get_canvas_interface())
			get_canvas_interface()->signal_value_node_replaced()(value_node_ref,value_node);

		if(value_node->waypoint_list().size()!=0)
			throw Error(_("This animated value node should be empty, but for some reason it isn't. This is a bug. (2)"));
	}

	if(value_node->waypoint_list().size()!=0)
	{
		try { value_node->find(waypoint.get_time()); throw Error(_("A Waypoint already exists at this point in time"));}
		catch (const synfig::Exception::NotFound&) { }

		try { if(value_node->find(waypoint)!=value_node->waypoint_list().end()) throw Error(_("This waypoint is already in the ValueNode"));}
		catch (const synfig::Exception::NotFound&) { }
	}

	value_node->add(waypoint);

/*_if(get_canvas_interface())
	{
		get_canvas_interface()->signal_value_node_changed()(value_node);
	}
	else synfig::warning("CanvasInterface not set on action");*/
}
