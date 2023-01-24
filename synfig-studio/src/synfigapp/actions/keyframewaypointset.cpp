/* === S Y N F I G ========================================================= */
/*!	\file keyframewaypointset.cpp
**	\brief Template File
**
**	\legal
**	Copyright (c) 2002-2005 Robert B. Quattlebaum Jr., Adrian Bentley
**
**	This file is part of Synfig.
**
**	Synfig is free software: you can redistribute it and/or modify
**	it under the terms of the GNU General Public License as published by
**	the Free Software Foundation, either version 2 of the License, or
**	(at your option) any later version.
**
**	Synfig is distributed in the hope that it will be useful,
**	but WITHOUT ANY WARRANTY; without even the implied warranty of
**	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
**	GNU General Public License for more details.
**
**	You should have received a copy of the GNU General Public License
**	along with Synfig.  If not, see <https://www.gnu.org/licenses/>.
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

#include "keyframewaypointset.h"
#include <synfigapp/canvasinterface.h>
#include <synfig/valuenodes/valuenode_dynamiclist.h>
#include <synfig/valuenodes/valuenode_animated.h>
#include "activepointsetsmart.h"
#include "waypointsetsmart.h"

#include <synfigapp/localization.h>

#endif

using namespace synfig;
using namespace synfigapp;
using namespace Action;

/* === M A C R O S ========================================================= */

ACTION_INIT(Action::KeyframeWaypointSet);
ACTION_SET_NAME(Action::KeyframeWaypointSet,"KeyframeWaypointSet");
ACTION_SET_LOCAL_NAME(Action::KeyframeWaypointSet,N_("Set Waypoints at Keyframe"));
ACTION_SET_TASK(Action::KeyframeWaypointSet,"set");
ACTION_SET_CATEGORY(Action::KeyframeWaypointSet,Action::CATEGORY_KEYFRAME);
ACTION_SET_PRIORITY(Action::KeyframeWaypointSet,0);
ACTION_SET_VERSION(Action::KeyframeWaypointSet,"0.0");

/* === G L O B A L S ======================================================= */

/* === P R O C E D U R E S ================================================= */

/* === M E T H O D S ======================================================= */

Action::KeyframeWaypointSet::KeyframeWaypointSet()
{
	keyframe.set_time(Time::begin()-1);
	set_dirty(false);
}

Action::ParamVocab
Action::KeyframeWaypointSet::get_param_vocab()
{
	ParamVocab ret(Action::CanvasSpecific::get_param_vocab());

	ret.push_back(ParamDesc("keyframe",Param::TYPE_KEYFRAME)
		.set_local_name(_("Keyframe"))
	);

	ret.push_back(ParamDesc("model",Param::TYPE_WAYPOINTMODEL)
		.set_local_name(_("Waypoint Model"))
	);

	return ret;
}

bool
Action::KeyframeWaypointSet::is_candidate(const ParamList &x)
{
	return candidate_check(get_param_vocab(),x);
}

bool
Action::KeyframeWaypointSet::set_param(const synfig::String& name, const Action::Param &param)
{
	if(name=="keyframe" && param.get_type()==Param::TYPE_KEYFRAME)
	{
		keyframe=param.get_keyframe();

		return true;
	}
	if(name=="model" && param.get_type()==Param::TYPE_WAYPOINTMODEL)
	{
		waypoint_model=param.get_waypoint_model();

		return true;
	}

	return Action::CanvasSpecific::set_param(name,param);
}

bool
Action::KeyframeWaypointSet::is_ready()const
{
	if(keyframe.get_time()==(Time::begin()-1) || waypoint_model.is_trivial())
		return false;
	return Action::CanvasSpecific::is_ready();
}

void
Action::KeyframeWaypointSet::prepare()
{
	clear();

	KeyframeList::iterator iter;
	//try { get_canvas()->keyframe_list().find(keyframe);}
	//catch(synfig::Exception::NotFound)
	if (!get_canvas()->keyframe_list().find(keyframe, iter)) {
		throw Error(_("Unable to find the given keyframe"));
	}

	{
		std::vector<synfigapp::ValueDesc> value_desc_list;
		get_canvas_interface()->find_important_value_descs(value_desc_list);
		while(!value_desc_list.empty())
		{
			process_value_desc(value_desc_list.back());
			value_desc_list.pop_back();
		}
	}
}

void
Action::KeyframeWaypointSet::process_value_desc(const synfigapp::ValueDesc& value_desc)
{
	if(value_desc.is_value_node())
	{
		ValueNode_Animated::Handle value_node(ValueNode_Animated::Handle::cast_dynamic(value_desc.get_value_node()));

		if(value_node)
		{
			Action::Handle action(WaypointSetSmart::create());

			action->set_param("canvas",get_canvas());
			action->set_param("canvas_interface",get_canvas_interface());
			action->set_param("value_node",ValueNode::Handle(value_node));

			Waypoint waypoint;
			try
			{
				waypoint=*value_node->find(keyframe.get_time());
			}
			catch(...)
			{
				waypoint.set_time(keyframe.get_time());
				waypoint.set_value((*value_node)(keyframe.get_time()));
			}

			keyframe.apply_model(waypoint_model);
			//*get_canvas()->keyframe_list().find(keyframe)=keyframe;
			KeyframeList::iterator iter;
			if (get_canvas()->keyframe_list().find(keyframe, iter)) *iter = keyframe;

			waypoint.apply_model(waypoint_model);

			action->set_param("waypoint",waypoint);

			assert(action->is_ready());
			if(!action->is_ready())
				throw Error(Error::TYPE_NOTREADY);

			add_action_front(action);
		}
	}
}
