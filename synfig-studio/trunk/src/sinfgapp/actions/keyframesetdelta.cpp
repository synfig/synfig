/* === S I N F G =========================================================== */
/*!	\file keyframeset.cpp
**	\brief Template File
**
**	$Id: keyframesetdelta.cpp,v 1.1.1.1 2005/01/07 03:34:37 darco Exp $
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

#include "keyframesetdelta.h"
#include <sinfgapp/canvasinterface.h>
#include <sinfg/valuenode_dynamiclist.h>
#include <sinfg/valuenode_animated.h>
#include "activepointsetsmart.h"
#include "waypointsetsmart.h"

#endif

using namespace std;
using namespace etl;
using namespace sinfg;
using namespace sinfgapp;
using namespace Action;

/* === M A C R O S ========================================================= */

ACTION_INIT(Action::KeyframeSetDelta);
ACTION_SET_NAME(Action::KeyframeSetDelta,"keyframe_set_delta");
ACTION_SET_LOCAL_NAME(Action::KeyframeSetDelta,"Set Keyframe Delta");
ACTION_SET_TASK(Action::KeyframeSetDelta,"set");
ACTION_SET_CATEGORY(Action::KeyframeSetDelta,Action::CATEGORY_KEYFRAME|Action::CATEGORY_HIDDEN);
ACTION_SET_PRIORITY(Action::KeyframeSetDelta,0);
ACTION_SET_VERSION(Action::KeyframeSetDelta,"0.0");
ACTION_SET_CVS_ID(Action::KeyframeSetDelta,"$Id: keyframesetdelta.cpp,v 1.1.1.1 2005/01/07 03:34:37 darco Exp $");

/* === G L O B A L S ======================================================= */

/* === P R O C E D U R E S ================================================= */

/* === M E T H O D S ======================================================= */

Action::KeyframeSetDelta::KeyframeSetDelta():
	delta(0)
{
	keyframe.set_time(Time::end());
	set_dirty(false);
}

Action::ParamVocab
Action::KeyframeSetDelta::get_param_vocab()
{
	ParamVocab ret(Action::CanvasSpecific::get_param_vocab());
	
	ret.push_back(ParamDesc("keyframe",Param::TYPE_KEYFRAME)
		.set_local_name(_("Keyframe"))
	);
	ret.push_back(ParamDesc("delta",Param::TYPE_KEYFRAME)
		.set_local_name(_("Delta"))
	);

	return ret;
}

bool
Action::KeyframeSetDelta::is_canidate(const ParamList &x)
{
	return canidate_check(get_param_vocab(),x);
}

bool
Action::KeyframeSetDelta::set_param(const sinfg::String& name, const Action::Param &param)
{
	if(name=="keyframe" && param.get_type()==Param::TYPE_KEYFRAME)
	{
		keyframe=param.get_keyframe();
		return true;
	}
	if(name=="delta" && param.get_type()==Param::TYPE_TIME)
	{
		delta=param.get_time();
		return true;
	}

	return Action::CanvasSpecific::set_param(name,param);
}

bool
Action::KeyframeSetDelta::is_ready()const
{
	if(keyframe.get_time()==Time::end())
		return false;
	return Action::CanvasSpecific::is_ready();
}

void
Action::KeyframeSetDelta::prepare()
{
	clear();
	value_desc_list.clear();
	get_canvas_interface()->find_important_value_descs(value_desc_list);

	
	Time time(get_canvas()->keyframe_list().find(keyframe)->get_time());
	
	std::vector<sinfgapp::ValueDesc>::iterator iter;
		DEBUGPOINT();
	for(iter=value_desc_list.begin();iter!=value_desc_list.end();++iter)
	{
		ValueDesc& value_desc(*iter);
		ValueNode_Animated::Handle value_node(
			ValueNode_Animated::Handle::cast_dynamic(value_desc.get_value_node())
		);
		
		if(!value_node)
			continue;
		
		try{
			value_node->find(time);
			// if we got to this point, then we know that
			// a waypoint already exists here and we don't
			// need to add a new one.
			continue;
		}catch(...)
		{
			// Make sure there is something previous
			try{
				value_node->find_prev(time);
			}catch(...)
			{
				continue;
			}
		}
		Action::Handle action(Action::create("waypoint_set_smart"));
		
		action->set_param("canvas",get_canvas());
		action->set_param("canvas_interface",get_canvas_interface());
		action->set_param("value_node",ValueNode::Handle::cast_static(value_node));

		action->set_param("time",time);

		assert(action->is_ready());
		if(!action->is_ready())
			throw Error(Error::TYPE_NOTREADY);
	
		add_action(action);						
	}
}

void
Action::KeyframeSetDelta::perform()
{
	if(!delta)
		return;
	Action::Super::perform();
	
//	Time location(keyframe.get_time());
	Time location(get_canvas()->keyframe_list().find(keyframe)->get_time());	
	Time delta(delta);
	
	get_canvas()->keyframe_list().insert_time(location,delta);

	std::vector<sinfgapp::ValueDesc>::iterator iter;
	for(iter=value_desc_list.begin();iter!=value_desc_list.end();++iter)
	{
		ValueDesc& value_desc(*iter);
		if(!value_desc.is_value_node())
			continue;
		ValueNode_Animated::Handle animated(
			ValueNode_Animated::Handle::cast_dynamic(value_desc.get_value_node())
		);
		if(animated)
		{
			animated->insert_time(location,delta);
			continue;
		}
		ValueNode_DynamicList::Handle dyn_list(
			ValueNode_DynamicList::Handle::cast_dynamic(value_desc.get_value_node())
		);
		if(dyn_list)
		{
			dyn_list->insert_time(location,delta);
			continue;
		}
	}

	// Signal that something has changed
	if(get_canvas_interface())
	{
		get_canvas_interface()->signal_keyframe_changed()(keyframe);
	}
	else sinfg::warning("CanvasInterface not set on action");
}

void
Action::KeyframeSetDelta::undo()
{
	if(!delta)
		return;
	
//	Time location(keyframe.get_time());
	Time location(get_canvas()->keyframe_list().find(keyframe)->get_time());	
	Time delta(-delta);
	
	get_canvas()->keyframe_list().insert_time(location,delta);

	std::vector<sinfgapp::ValueDesc>::iterator iter;
	for(iter=value_desc_list.begin();iter!=value_desc_list.end();++iter)
	{
		ValueDesc& value_desc(*iter);
		if(!value_desc.is_value_node())
			continue;
		ValueNode_Animated::Handle animated(
			ValueNode_Animated::Handle::cast_dynamic(value_desc.get_value_node())
		);
		if(animated)
		{
			animated->insert_time(location,delta);
			continue;
		}
		ValueNode_DynamicList::Handle dyn_list(
			ValueNode_DynamicList::Handle::cast_dynamic(value_desc.get_value_node())
		);
		if(dyn_list)
		{
			dyn_list->insert_time(location,delta);
			continue;
		}
	}

	Action::Super::undo();

	// Signal that something has changed
	if(get_canvas_interface())
	{
		get_canvas_interface()->signal_keyframe_changed()(keyframe);
	}
	else sinfg::warning("CanvasInterface not set on action");
}
