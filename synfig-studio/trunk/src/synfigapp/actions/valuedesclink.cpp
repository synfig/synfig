/* === S Y N F I G ========================================================= */
/*!	\file valuedesclink.cpp
**	\brief Template File
**
**	$Id: valuedesclink.cpp,v 1.1.1.1 2005/01/07 03:34:37 darco Exp $
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

#include "valuedesclink.h"

#include <synfigapp/canvasinterface.h>
#include <synfig/valuenode_const.h>

#endif

using namespace std;
using namespace etl;
using namespace synfig;
using namespace synfigapp;
using namespace Action;

/* === M A C R O S ========================================================= */

ACTION_INIT(Action::ValueDescLink);
ACTION_SET_NAME(Action::ValueDescLink,"value_desc_link");
ACTION_SET_LOCAL_NAME(Action::ValueDescLink,"Link");
ACTION_SET_TASK(Action::ValueDescLink,"connect");
ACTION_SET_CATEGORY(Action::ValueDescLink,Action::CATEGORY_VALUEDESC);
ACTION_SET_PRIORITY(Action::ValueDescLink,0);
ACTION_SET_VERSION(Action::ValueDescLink,"0.0");
ACTION_SET_CVS_ID(Action::ValueDescLink,"$Id: valuedesclink.cpp,v 1.1.1.1 2005/01/07 03:34:37 darco Exp $");

/* === G L O B A L S ======================================================= */

/* === P R O C E D U R E S ================================================= */

/* === M E T H O D S ======================================================= */

Action::ValueDescLink::ValueDescLink()
{
	poison=false;
}

Action::ParamVocab
Action::ValueDescLink::get_param_vocab()
{
	ParamVocab ret(Action::CanvasSpecific::get_param_vocab());
	
	ret.push_back(ParamDesc("value_desc",Param::TYPE_VALUEDESC)
		.set_local_name(_("ValueDesc to link"))
		.set_requires_multiple()
	);
	
	return ret;
}

bool
Action::ValueDescLink::is_candidate(const ParamList &x)
{
	return candidate_check(get_param_vocab(),x);
}

bool
Action::ValueDescLink::set_param(const synfig::String& name, const Action::Param &param)
{
	if(name=="time" && param.get_type()==Param::TYPE_TIME)
	{
		time=param.get_time();
		return true;
	}
	
	if(name=="value_desc" && param.get_type()==Param::TYPE_VALUEDESC)
	{
		ValueDesc value_desc(param.get_value_desc());
				
		if(value_desc.is_value_node() && value_desc.get_value_node()->is_exported())
		{
			if(link_value_node==value_desc.get_value_node())
				return true;
			
			if(link_value_node && link_value_node->is_exported())
			{
				poison=true;
				return false;
			}
						
			link_value_node=value_desc.get_value_node();
		}
		else if(value_desc.is_value_node())
		{
			if(!link_value_node)
			{
				link_value_node=value_desc.get_value_node();
			}

			// Use the one that is referenced more
			else if(link_value_node->rcount()<value_desc.get_value_node()->rcount())
			{
				link_value_node=value_desc.get_value_node();
			}

			// If the current link value node is a constant and
			// this one isn't, then give preference to the exotic
			else if(ValueNode_Const::Handle::cast_dynamic(link_value_node) && !ValueNode_Const::Handle::cast_dynamic(value_desc.get_value_node()))
			{
				link_value_node=value_desc.get_value_node();
			}

			// If both are animated, and this one has more waypoints,
			// then use the one with more waypoints
			else if(
					ValueNode_Animated::Handle::cast_dynamic(link_value_node)
				&&	ValueNode_Animated::Handle::cast_dynamic(value_desc.get_value_node())
				&& (
					ValueNode_Animated::Handle::cast_dynamic(link_value_node)->waypoint_list().size()
				<	ValueNode_Animated::Handle::cast_dynamic(value_desc.get_value_node())->waypoint_list().size()
				)
			)
			{
				link_value_node=value_desc.get_value_node();
			}
			
			/*
			// Use the one that was most recently changed
			else if(link_value_node->get_time_last_changed()<value_desc.get_value_node()->get_time_last_changed())
			{
				link_value_node=value_desc.get_value_node();
			}
			*/
		}

		
		if(value_desc_list.size() && value_desc.get_value_type()!=value_desc_list.front().get_value_type())
		{
			// Everything must be of the same type
			poison=true;
			return false;
		}
		value_desc_list.push_back(value_desc);

		return true;
	}

	return Action::CanvasSpecific::set_param(name,param);
}

bool
Action::ValueDescLink::is_ready()const
{
	if(poison || value_desc_list.size()<=1)
		return false;
	return Action::CanvasSpecific::is_ready();
}

void
Action::ValueDescLink::prepare()
{
	if(poison || value_desc_list.empty())
		throw Error(Error::TYPE_NOTREADY);
		
	clear();

	if(!link_value_node)
	{
		ValueDesc& value_desc(value_desc_list.front());
		
		link_value_node=ValueNode_Const::create(value_desc.get_value(time));
		
		Action::Handle action(Action::create("value_desc_connect"));
		
		action->set_param("canvas",get_canvas());
		action->set_param("canvas_interface",get_canvas_interface());
		action->set_param("src",link_value_node);
		action->set_param("dest",value_desc);
	
		assert(action->is_ready());		
		if(!action->is_ready())
			throw Error(Error::TYPE_NOTREADY);
	
		add_action_front(action);		
	}

	/*
	if(!link_value_node->is_exported())
	{
		Action::Handle action(Action::create("value_node_add"));
		
		action->set_param("canvas",get_canvas());
		action->set_param("canvas_interface",get_canvas_interface());
		action->set_param("new",link_value_node);
		action->set_param("name",strprintf(_("Unnamed%08d"),synfig::UniqueID().get_uid()));
	
		assert(action->is_ready());		
		if(!action->is_ready())
			throw Error(Error::TYPE_NOTREADY);
	
		add_action_front(action);
	}
	*/
	
	std::list<ValueDesc>::iterator iter;
	for(iter=value_desc_list.begin();iter!=value_desc_list.end();++iter)
	{
		ValueDesc& value_desc(*iter);
		
		if(value_desc.is_value_node() && value_desc.get_value_node()==link_value_node)
			continue;

		Action::Handle action(Action::create("value_desc_connect"));
		
		action->set_param("canvas",get_canvas());
		action->set_param("canvas_interface",get_canvas_interface());
		action->set_param("src",link_value_node);
		action->set_param("dest",value_desc);
	
		assert(action->is_ready());		
		if(!action->is_ready())
			throw Error(Error::TYPE_NOTREADY);
	
		add_action_front(action);		
	}
}
