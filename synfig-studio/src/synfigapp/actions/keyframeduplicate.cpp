/* === S Y N F I G ========================================================= */
/*!	\file keyframeduplicate.cpp
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

#include "keyframeduplicate.h"
#include <synfigapp/canvasinterface.h>
#include <synfig/valuenodes/valuenode_dynamiclist.h>
#include <synfig/valuenodes/valuenode_animated.h>
#include "activepointsetsmart.h"
#include "waypointsetsmart.h"

#include <synfigapp/localization.h>

#endif

using namespace etl;
using namespace synfig;
using namespace synfigapp;
using namespace Action;

/* === M A C R O S ========================================================= */

ACTION_INIT(Action::KeyframeDuplicate);
ACTION_SET_NAME(Action::KeyframeDuplicate,"KeyframeDuplicate");
ACTION_SET_LOCAL_NAME(Action::KeyframeDuplicate,N_("Duplicate Keyframe"));
ACTION_SET_TASK(Action::KeyframeDuplicate,"duplicate");
ACTION_SET_CATEGORY(Action::KeyframeDuplicate,Action::CATEGORY_KEYFRAME);
ACTION_SET_PRIORITY(Action::KeyframeDuplicate,0);
ACTION_SET_VERSION(Action::KeyframeDuplicate,"0.0");

/* === G L O B A L S ======================================================= */

/* === P R O C E D U R E S ================================================= */

/* === M E T H O D S ======================================================= */

Action::KeyframeDuplicate::KeyframeDuplicate()
{
	new_keyframe.set_time(Time::begin()-1);
	keyframe.set_time(Time::begin()-1);
	set_dirty(true);
}

Action::ParamVocab
Action::KeyframeDuplicate::get_param_vocab()
{
	ParamVocab ret(Action::CanvasSpecific::get_param_vocab());

	ret.push_back(ParamDesc("keyframe",Param::TYPE_KEYFRAME)
		.set_local_name(_("Keyframe"))
		.set_desc(_("Keyframe to be duplicated"))
	);

	ret.push_back(ParamDesc("time",Param::TYPE_TIME)
		.set_local_name(_("Time"))
	);

	return ret;
}

bool
Action::KeyframeDuplicate::is_candidate(const ParamList &x)
{
	return candidate_check(get_param_vocab(),x);
}

bool
Action::KeyframeDuplicate::set_param(const synfig::String& name, const Action::Param &param)
{
	if(name=="keyframe" && param.get_type()==Param::TYPE_KEYFRAME)
	{
		keyframe=param.get_keyframe();
		new_keyframe.set_description(keyframe.get_description()+_(" (Duplicate)"));

		//! TODO add and use keyframe::operator=

		//! Copy the kf's Waypoint::model is exist
		if(keyframe.has_model())
		    new_keyframe.apply_model(keyframe.get_waypoint_model());
		//! Copy the active status
		new_keyframe.set_active(keyframe.active());

		return true;
	}
	if(name=="time" && param.get_type()==Param::TYPE_TIME)
	{
		new_keyframe.set_time(param.get_time());

		return true;
	}

	return Action::CanvasSpecific::set_param(name,param);
}

bool
Action::KeyframeDuplicate::is_ready()const
{
	if(keyframe.get_time()==(Time::begin()-1) || new_keyframe.get_time()==(Time::begin()-1))
		return false;
	return Action::CanvasSpecific::is_ready();
}

void
Action::KeyframeDuplicate::prepare()
{
	clear();

	const synfig::Time old_time=keyframe.get_time();
	const synfig::Time new_time=new_keyframe.get_time();

	KeyframeList::iterator iter;
	
	//try { get_canvas()->keyframe_list().find(keyframe);}
	//catch(synfig::Exception::NotFound)
	if (!get_canvas()->keyframe_list().find(keyframe, iter)) {
		throw Error(_("Unable to find the given keyframe"));
	}

	if (get_canvas()->keyframe_list().find(new_time, iter)) {
		if (iter != get_canvas()->keyframe_list().end()) {
			throw Error(_("A Keyframe already exists at this point in time"));
		}		
	}

	//try { if(get_canvas()->keyframe_list().find(new_time)!=get_canvas()->keyframe_list().end()) throw Error(_("A Keyframe already exists at this point in time"));}
	//catch(...) { }

	// If the times are different, then we
	// will need to romp through the valuenodes
	// and add actions to update their values.
	if(new_time!=old_time)
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
Action::KeyframeDuplicate::process_value_desc(const synfigapp::ValueDesc& value_desc)
{
	const synfig::Time old_time=keyframe.get_time();
	const synfig::Time new_time=new_keyframe.get_time();

	if(value_desc.is_value_node())
	{
		ValueNode::Handle value_node(value_desc.get_value_node());

		// If we are a dynamic list, then we need to update the ActivePoints
		if(ValueNode_DynamicList::Handle::cast_dynamic(value_node))
		{
			ValueNode_DynamicList::Handle value_node_dynamic(ValueNode_DynamicList::Handle::cast_dynamic(value_node));
			int i;

			for(i=0;i<value_node_dynamic->link_count();i++)
			{
				synfigapp::ValueDesc value_desc(value_node_dynamic,i);
				Activepoint activepoint(value_node_dynamic->list[i].new_activepoint_at_time(old_time));
				activepoint.set_time(new_time);

				Action::Handle action(ActivepointSetSmart::create());

				action->set_param("canvas",get_canvas());
				action->set_param("canvas_interface",get_canvas_interface());
				action->set_param("value_desc",value_desc);
				action->set_param("activepoint",activepoint);

				assert(action->is_ready());
				if(!action->is_ready())
					throw Error(Error::TYPE_NOTREADY);

				add_action_front(action);
			}
		}
		else if(ValueNode_Animated::Handle::cast_dynamic(value_node))
		{
			ValueNode_Animated::Handle value_node_animated(ValueNode_Animated::Handle::cast_dynamic(value_node));
			Waypoint waypoint(value_node_animated->new_waypoint_at_time(old_time));
			waypoint.set_time(new_time);

			Action::Handle action(WaypointSetSmart::create());

			action->set_param("canvas",get_canvas());
			action->set_param("canvas_interface",get_canvas_interface());
			action->set_param("value_node",ValueNode::Handle(value_node_animated));
			action->set_param("waypoint",waypoint);

			assert(action->is_ready());
			if(!action->is_ready())
				throw Error(Error::TYPE_NOTREADY);

			add_action_front(action);
		}
	}
}

void
Action::KeyframeDuplicate::perform()
{
	KeyframeList::iterator iter;
	//try { get_canvas()->keyframe_list().find(new_keyframe.get_time()); throw Error(_("A Keyframe already exists at this point in time"));}
	//catch(synfig::Exception::NotFound) { }
	if (get_canvas()->keyframe_list().find(new_keyframe.get_time(), iter)) {
		throw Error(_("A Keyframe already exists at this point in time"));
	}

	//try { get_canvas()->keyframe_list().find(new_keyframe); throw Error(_("This keyframe is already in the ValueNode"));}
	//catch(synfig::Exception::NotFound) { }
	if (get_canvas()->keyframe_list().find(new_keyframe, iter)) {
		throw Error(_("This keyframe is already in the ValueNode"));
	}

	Action::Super::perform();

	get_canvas()->keyframe_list().add(new_keyframe);

	if(get_canvas_interface())
	{
		get_canvas_interface()->signal_keyframe_added()(new_keyframe);
	}
	else synfig::warning("CanvasInterface not set on action");
}

void
Action::KeyframeDuplicate::undo()
{
	Action::Super::undo();
	
	get_canvas()->keyframe_list().erase(new_keyframe);

	if(get_canvas_interface())
	{
		get_canvas_interface()->signal_keyframe_removed()(new_keyframe);
	}
	else synfig::warning("CanvasInterface not set on action");
}
