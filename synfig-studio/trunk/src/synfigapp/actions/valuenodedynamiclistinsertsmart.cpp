/* === S Y N F I G ========================================================= */
/*!	\file valuenodedynamiclistinsertsmart.cpp
**	\brief Template File
**
**	$Id: valuenodedynamiclistinsertsmart.cpp,v 1.3 2005/01/17 05:20:08 darco Exp $
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

#include "valuenodedynamiclistinsertsmart.h"
#include "valuenodedynamiclistinsert.h"
#include <synfigapp/canvasinterface.h>

#endif

using namespace std;
using namespace etl;
using namespace synfig;
using namespace synfigapp;
using namespace Action;

/* === M A C R O S ========================================================= */

ACTION_INIT(Action::ValueNodeDynamicListInsertSmart);
ACTION_SET_NAME(Action::ValueNodeDynamicListInsertSmart,"value_node_dynamic_list_insert_smart");
ACTION_SET_LOCAL_NAME(Action::ValueNodeDynamicListInsertSmart,"Insert Item (Smart)");
ACTION_SET_TASK(Action::ValueNodeDynamicListInsertSmart,"insert");
ACTION_SET_CATEGORY(Action::ValueNodeDynamicListInsertSmart,Action::CATEGORY_VALUEDESC|Action::CATEGORY_VALUENODE);
ACTION_SET_PRIORITY(Action::ValueNodeDynamicListInsertSmart,-20);
ACTION_SET_VERSION(Action::ValueNodeDynamicListInsertSmart,"0.0");
ACTION_SET_CVS_ID(Action::ValueNodeDynamicListInsertSmart,"$Id: valuenodedynamiclistinsertsmart.cpp,v 1.3 2005/01/17 05:20:08 darco Exp $");

/* === G L O B A L S ======================================================= */

/* === P R O C E D U R E S ================================================= */

/* === M E T H O D S ======================================================= */

Action::ValueNodeDynamicListInsertSmart::ValueNodeDynamicListInsertSmart()
{
	index=0;
	time=0;
	origin=0.5f;
	set_dirty(true);
}

Action::ParamVocab
Action::ValueNodeDynamicListInsertSmart::get_param_vocab()
{
	ParamVocab ret(Action::CanvasSpecific::get_param_vocab());
	
	ret.push_back(ParamDesc("value_desc",Param::TYPE_VALUEDESC)
		.set_local_name(_("ValueDesc"))
	);
	ret.push_back(ParamDesc("time",Param::TYPE_TIME)
		.set_local_name(_("Time"))
		.set_optional()
	);
	ret.push_back(ParamDesc("origin",Param::TYPE_REAL)
		.set_local_name(_("Origin"))
		.set_optional()
	);

	return ret;
}

bool
Action::ValueNodeDynamicListInsertSmart::is_canidate(const ParamList &x)
{
	if(canidate_check(get_param_vocab(),x))
	{
		ValueDesc value_desc(x.find("value_desc")->second.get_value_desc());
		if(!value_desc.parent_is_value_node() || !ValueNode_DynamicList::Handle::cast_dynamic(value_desc.get_parent_value_node()))
			return false;

		return true;
	}
	return false;
}

bool
Action::ValueNodeDynamicListInsertSmart::set_param(const synfig::String& name, const Action::Param &param)
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
	if(name=="time" && param.get_type()==Param::TYPE_TIME)
	{
		time=param.get_time();
		
		return true;
	}
	if(name=="origin" && param.get_type()==Param::TYPE_REAL)
	{
		origin=param.get_real();
		
		return true;
	}

	return Action::CanvasSpecific::set_param(name,param);
}

bool
Action::ValueNodeDynamicListInsertSmart::is_ready()const
{
	if(!value_node)
		return false;
	return Action::CanvasSpecific::is_ready();
}

void
Action::ValueNodeDynamicListInsertSmart::prepare()
{	
	//clear();
	// HACK
	if(!first_time())
		return;
	
	// If we are in animate editing mode
	if(get_edit_mode()&MODE_ANIMATE)
	{
		int index(ValueNodeDynamicListInsertSmart::index);

		// In this case we need to first determine if there is
		// a currently disabled item in the list that we can
		// turn on. If not, then we need to go ahead and create one.
		synfig::info("ValueNodeDynamicListInsertSmart: index=%d",index);
		synfig::info("ValueNodeDynamicListInsertSmart: value_node->list.size()=%d",value_node->list.size());
		if(value_node->list.size()<=index && index>0)
			synfig::info("ValueNodeDynamicListInsertSmart: value_node->list[index-1].status_at_time(time)=%d",value_node->list[index-1].status_at_time(time));
		
		if(value_node->list.size()>=index && index>0 && !value_node->list[index-1].status_at_time(time))
		{
			// Ok, we do not have to create a new
			// entry in the dynamic list after all.			
			// However, we do need to set the
			// position and tangent of this point.
			ValueNode_DynamicList::ListEntry list_entry(value_node->create_list_entry(index,time,origin));
			ValueBase value((*list_entry.value_node)(time));
			index--;
			
			ValueDesc item_value_desc(value_node,index);

			Action::Handle action(Action::create("value_desc_set"));
	
			if(!action)
				throw Error(_("Unable to find action value_desc_set (bug)"));
			
			action->set_param("edit_mode",get_edit_mode());
			action->set_param("canvas",get_canvas());
			action->set_param("canvas_interface",get_canvas_interface());
			action->set_param("time",time);
			action->set_param("new_value",value);
			action->set_param("value_desc",ValueDesc(value_node,index));
			
			if(!action->is_ready())
				throw Error(Error::TYPE_NOTREADY);
	
			add_action(action);			
		}
		else
		{
			// Ok, not a big deal, we just need to
			// add a new item
			Action::Handle action(Action::create("value_node_dynamic_list_insert"));
	
			if(!action)
				throw Error(_("Unable to find action (bug)"));
			
			action->set_param("canvas",get_canvas());
			action->set_param("canvas_interface",get_canvas_interface());
			action->set_param("time",time);
			action->set_param("origin",origin);
			action->set_param("value_desc",ValueDesc(value_node,index));
			
			if(!action->is_ready())
				throw Error(Error::TYPE_NOTREADY);
	
			add_action(action);
			
			action=Action::create("activepoint_set_off");
	
			if(!action)
				throw Error(_("Unable to find action \"activepoint_set_off\""));
			
			action->set_param("edit_mode",MODE_ANIMATE);
			action->set_param("canvas",get_canvas());
			action->set_param("canvas_interface",get_canvas_interface());
			action->set_param("time",Time::begin());
			action->set_param("origin",origin);
			action->set_param("value_desc",ValueDesc(value_node,index));
			
			if(!action->is_ready())
				throw Error(Error::TYPE_NOTREADY);
	
			add_action(action);
		}
			
		// Now we set the activepoint up and then we'll be done
		Action::Handle action(Action::create("activepoint_set_on"));

		if(!action)
			throw Error(_("Unable to find action \"activepoint_set_on\""));
		
		action->set_param("edit_mode",get_edit_mode());
		action->set_param("canvas",get_canvas());
		action->set_param("canvas_interface",get_canvas_interface());
		action->set_param("time",time);
		action->set_param("origin",origin);
		action->set_param("value_desc",ValueDesc(value_node,index));
		
		if(!action->is_ready())
			throw Error(Error::TYPE_NOTREADY);

		add_action(action);
	}
	else
	{
		Action::Handle action(Action::create("value_node_dynamic_list_insert"));

		if(!action)
			throw Error(_("Unable to find action (bug)"));
		
		action->set_param("canvas",get_canvas());
		action->set_param("canvas_interface",get_canvas_interface());
		action->set_param("time",time);
		action->set_param("origin",origin);
		action->set_param("value_desc",ValueDesc(value_node,index));
		
		if(!action->is_ready())
			throw Error(Error::TYPE_NOTREADY);

		add_action(action);	
	}
}
