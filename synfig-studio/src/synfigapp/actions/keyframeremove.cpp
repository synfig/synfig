/* === S Y N F I G ========================================================= */
/*!	\file keyframeremove.cpp
**	\brief Template File
**
**	$Id$
**
**	\legal
**	Copyright (c) 2002-2005 Robert B. Quattlebaum Jr., Adrian Bentley
**	Copyright (c) 2012-2013 Konstantin Dmitriev
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

#include "keyframeremove.h"
#include <synfigapp/canvasinterface.h>
#include <synfig/valuenodes/valuenode_dynamiclist.h>
#include <synfig/valuenodes/valuenode_animated.h>
#include "activepointremove.h"
#include "waypointremove.h"

#include <synfigapp/localization.h>

#endif

using namespace etl;
using namespace synfig;
using namespace synfigapp;
using namespace Action;

/* === M A C R O S ========================================================= */

ACTION_INIT(Action::KeyframeRemove);
ACTION_SET_NAME(Action::KeyframeRemove,"KeyframeRemove");
ACTION_SET_LOCAL_NAME(Action::KeyframeRemove,N_("Remove Keyframe"));
ACTION_SET_TASK(Action::KeyframeRemove,"remove");
ACTION_SET_CATEGORY(Action::KeyframeRemove,Action::CATEGORY_KEYFRAME);
ACTION_SET_PRIORITY(Action::KeyframeRemove,0);
ACTION_SET_VERSION(Action::KeyframeRemove,"0.0");

/* === G L O B A L S ======================================================= */

/* === P R O C E D U R E S ================================================= */

/* === M E T H O D S ======================================================= */

Action::KeyframeRemove::KeyframeRemove()
{
	keyframe.set_time(Time::begin()-1);
	set_dirty(true);
}

Action::ParamVocab
Action::KeyframeRemove::get_param_vocab()
{
	ParamVocab ret(Action::CanvasSpecific::get_param_vocab());

	ret.push_back(ParamDesc("keyframe",Param::TYPE_KEYFRAME)
		.set_local_name(_("Keyframe"))
		.set_desc(_("Keyframe to be removed"))
	);

	return ret;
}

bool
Action::KeyframeRemove::is_candidate(const ParamList &x)
{
	return candidate_check(get_param_vocab(),x);
}

bool
Action::KeyframeRemove::set_param(const synfig::String& name, const Action::Param &param)
{
	if(name=="keyframe" && param.get_type()==Param::TYPE_KEYFRAME)
	{
		keyframe=param.get_keyframe();
		// For some reason the state of the keyframe is not always passed correctly
		// Make sure to get it right:
		KeyframeList::iterator iter;
		//keyframe.set_active(get_canvas()->keyframe_list().find(keyframe)->active());
		if (get_canvas()->keyframe_list().find(keyframe, iter)) {
			keyframe.set_active(iter->active());
		}
		return true;
	}

	return Action::CanvasSpecific::set_param(name,param);
}

bool
Action::KeyframeRemove::is_ready()const
{
	if(keyframe.get_time()==(Time::begin()-1))
		return false;
	return Action::CanvasSpecific::is_ready();
}

void
Action::KeyframeRemove::prepare()
{
	clear();

	KeyframeList::iterator iter;
	//try { get_canvas()->keyframe_list().find(keyframe);}
	//catch(synfig::Exception::NotFound)
	if (!get_canvas()->keyframe_list().find(keyframe, iter)) {
		throw Error(_("Unable to find the given keyframe"));
	}


	if (keyframe.active()){
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
Action::KeyframeRemove::process_value_desc(const synfigapp::ValueDesc& value_desc)
{
	const synfig::Time time(keyframe.get_time());

	if(value_desc.is_value_node())
	{
		ValueNode::Handle value_node(value_desc.get_value_node());

		// If we are a dynamic list, then we need to update the ActivePoints
		if(ValueNode_DynamicList::Handle::cast_dynamic(value_node))
		{
			ValueNode_DynamicList::Handle value_node_dynamic(ValueNode_DynamicList::Handle::cast_dynamic(value_node));
			int i;
			for(i=0;i<value_node_dynamic->link_count();i++)
			try
			{
				Activepoint activepoint;
				activepoint=*value_node_dynamic->list[i].find(time);

				synfigapp::ValueDesc value_desc(value_node_dynamic,i);

				Action::Handle action(ActivepointRemove::create());

				action->set_param("canvas",get_canvas());
				action->set_param("canvas_interface",get_canvas_interface());
				action->set_param("value_desc",value_desc);
				action->set_param("activepoint",activepoint);

				assert(action->is_ready());
				if(!action->is_ready())
					throw Error(Error::TYPE_NOTREADY);

				add_action_front(action);
			}
			catch(...)
			{
			}
		}
		else if(ValueNode_Animated::Handle::cast_dynamic(value_node))
		try
		{
			ValueNode_Animated::Handle value_node_animated(ValueNode_Animated::Handle::cast_dynamic(value_node));
			Waypoint waypoint;
			waypoint=*value_node_animated->find(time);
			assert(waypoint.get_time()==time);

			Action::Handle action(WaypointRemove::create());

			action->set_param("canvas",get_canvas());
			action->set_param("canvas_interface",get_canvas_interface());
			action->set_param("value_node",ValueNode::Handle(value_node_animated));
			action->set_param("waypoint",waypoint);

			assert(action->is_ready());
			if(!action->is_ready())
				throw Error(Error::TYPE_NOTREADY);

			add_action_front(action);
		}
		catch(...)
		{
		}
	}
}


void
Action::KeyframeRemove::perform()
{
	Action::Super::perform();
	
	get_canvas()->keyframe_list().erase(keyframe);
	
	if(get_canvas_interface())
	{
		get_canvas_interface()->signal_keyframe_removed()(keyframe);
	}
	else synfig::warning("CanvasInterface not set on action");
}

void
Action::KeyframeRemove::undo()
{
	KeyframeList::iterator iter;

	//try { get_canvas()->keyframe_list().find(keyframe.get_time()); throw Error(_("A Keyframe already exists at this point in time"));}
	//catch(synfig::Exception::NotFound) { }
	if (get_canvas()->keyframe_list().find(keyframe.get_time(), iter)) {
		throw Error(_("A Keyframe already exists at this point in time"));
	}

	//try { get_canvas()->keyframe_list().find(keyframe); throw Error(_("This keyframe is already in the ValueNode"));}
	//catch(synfig::Exception::NotFound) { }
	if (get_canvas()->keyframe_list().find(keyframe, iter)) {
		throw Error(_("This keyframe is already in the ValueNode"));
	}

	Action::Super::undo();

	get_canvas()->keyframe_list().add(keyframe);

	if(get_canvas_interface())
	{
		get_canvas_interface()->signal_keyframe_added()(keyframe);
	}
	else synfig::warning("CanvasInterface not set on action");
}
