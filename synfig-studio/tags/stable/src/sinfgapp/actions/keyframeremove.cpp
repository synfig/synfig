/* === S I N F G =========================================================== */
/*!	\file keyframeremove.cpp
**	\brief Template File
**
**	$Id: keyframeremove.cpp,v 1.1.1.1 2005/01/07 03:34:37 darco Exp $
**
**	\legal
**	Copyright (c) 2002 Robert B. Quattlebaum Jr.
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

#include "keyframeremove.h"
#include <sinfgapp/canvasinterface.h>
#include <sinfg/valuenode_dynamiclist.h>
#include <sinfg/valuenode_animated.h>
#include "activepointremove.h"
#include "waypointremove.h"

#endif

using namespace std;
using namespace etl;
using namespace sinfg;
using namespace sinfgapp;
using namespace Action;

/* === M A C R O S ========================================================= */

ACTION_INIT(Action::KeyframeRemove);
ACTION_SET_NAME(Action::KeyframeRemove,"keyframe_remove");
ACTION_SET_LOCAL_NAME(Action::KeyframeRemove,"Remove Keyframe");
ACTION_SET_TASK(Action::KeyframeRemove,"remove");
ACTION_SET_CATEGORY(Action::KeyframeRemove,Action::CATEGORY_KEYFRAME);
ACTION_SET_PRIORITY(Action::KeyframeRemove,0);
ACTION_SET_VERSION(Action::KeyframeRemove,"0.0");
ACTION_SET_CVS_ID(Action::KeyframeRemove,"$Id: keyframeremove.cpp,v 1.1.1.1 2005/01/07 03:34:37 darco Exp $");

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
Action::KeyframeRemove::is_canidate(const ParamList &x)
{
	return canidate_check(get_param_vocab(),x);
}

bool
Action::KeyframeRemove::set_param(const sinfg::String& name, const Action::Param &param)
{
	if(name=="keyframe" && param.get_type()==Param::TYPE_KEYFRAME)
	{
		keyframe=param.get_keyframe();
		
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

	try { get_canvas()->keyframe_list().find(keyframe);}
	catch(sinfg::Exception::NotFound)
	{
		throw Error(_("Unable to find the given keyframe"));
	}	

	
	{
		std::vector<sinfgapp::ValueDesc> value_desc_list;
		get_canvas_interface()->find_important_value_descs(value_desc_list);
		while(!value_desc_list.empty())
		{
			process_value_desc(value_desc_list.back());
			value_desc_list.pop_back();
		}
	}
}

void
Action::KeyframeRemove::process_value_desc(const sinfgapp::ValueDesc& value_desc)
{	
	const sinfg::Time time(keyframe.get_time());

	if(value_desc.is_value_node())
	{
		ValueNode::Handle value_node(value_desc.get_value_node());
	
		// If we are a dynamic list, then we need to update the ActivePoints
		if(ValueNode_DynamicList::Handle::cast_dynamic(value_node))
		{
			ValueNode_DynamicList::Handle value_node(ValueNode_DynamicList::Handle::cast_dynamic(value_node));
			int i;
			for(i=0;i<value_node->link_count();i++)
			try
			{
				Activepoint activepoint;
				activepoint=*value_node->list[i].find(time);

				sinfgapp::ValueDesc value_desc(value_node,i);

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
			ValueNode_Animated::Handle value_node(ValueNode_Animated::Handle::cast_dynamic(value_node));
			Waypoint waypoint;
			waypoint=*value_node->find(time);
			assert(waypoint.get_time()==time);
			
			Action::Handle action(WaypointRemove::create());
			
			action->set_param("canvas",get_canvas());
			action->set_param("canvas_interface",get_canvas_interface());
			action->set_param("value_node",ValueNode::Handle(value_node));
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
	
	if(get_canvas_interface())
	{
		get_canvas_interface()->signal_keyframe_removed()(keyframe);
	}
	else sinfg::warning("CanvasInterface not set on action");

	get_canvas()->keyframe_list().erase(keyframe);	
}

void
Action::KeyframeRemove::undo()
{
	try { get_canvas()->keyframe_list().find(keyframe.get_time()); throw Error(_("A Keyframe already exists at this point in time"));}
	catch(sinfg::Exception::NotFound) { }	

	try { get_canvas()->keyframe_list().find(keyframe); throw Error(_("This keyframe is already in the ValueNode"));}
	catch(sinfg::Exception::NotFound) { }	

	Action::Super::undo();
	
	get_canvas()->keyframe_list().add(keyframe);
	
	if(get_canvas_interface())
	{
		get_canvas_interface()->signal_keyframe_added()(keyframe);
	}
	else sinfg::warning("CanvasInterface not set on action");
}
