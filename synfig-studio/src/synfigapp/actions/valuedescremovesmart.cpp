/* === S Y N F I G ========================================================= */
/*!	\file ValueDescRemoveSmart.cpp
**	\brief Implementation of multiple value descriptions remove action
**
**	$Id$
**
**	\legal
**	Copyright (c) 2002-2005 Robert B. Quattlebaum Jr., Adrian Bentley
**	Copyright (c) 2011, 2012 Carlos López
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

#include "valuedescremovesmart.h"
#include <synfigapp/canvasinterface.h>

#include <synfigapp/localization.h>
#include <synfig/valuenodes/valuenode_composite.h>

#endif

using namespace etl;
using namespace synfig;
using namespace synfigapp;
using namespace Action;

/* === M A C R O S ========================================================= */

ACTION_INIT(Action::ValueDescRemoveSmart);
ACTION_SET_NAME(Action::ValueDescRemoveSmart,"ValueDescRemoveSmart");
ACTION_SET_LOCAL_NAME(Action::ValueDescRemoveSmart,N_("Remove Multiple Items (Smart)"));
ACTION_SET_TASK(Action::ValueDescRemoveSmart,"remove");
ACTION_SET_CATEGORY(Action::ValueDescRemoveSmart,Action::CATEGORY_VALUEDESC);
ACTION_SET_PRIORITY(Action::ValueDescRemoveSmart,-19);
ACTION_SET_VERSION(Action::ValueDescRemoveSmart,"0.0");

/* === G L O B A L S ======================================================= */

/* === P R O C E D U R E S ================================================= */

/* === M E T H O D S ======================================================= */

Action::ValueDescRemoveSmart::ValueDescRemoveSmart()
{
	time=0;
	origin=0.5f;
	set_dirty(true);
	value_nodes.clear();
}

Action::ParamVocab
Action::ValueDescRemoveSmart::get_param_vocab()
{
	ParamVocab ret(Action::CanvasSpecific::get_param_vocab());
	ret.push_back(ParamDesc("value_desc",Param::TYPE_VALUEDESC)
		.set_local_name(_("ValueDesc"))
		.set_requires_multiple()
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
Action::ValueDescRemoveSmart::is_candidate(const ParamList &x)
{
	if (!candidate_check(get_param_vocab(),x))
		return false;
	// Check all the value_desc parameters to be Dynamic List parent.
	ParamList::const_iterator it;
	for(it=x.equal_range("value_desc").first; it!=x.equal_range("value_desc").second; ++it)
	{
		ValueDesc value_desc(it->second.get_value_desc());
		// if any of the value descs is not Dynamic List parent then ...
		if(!(value_desc.parent_is_value_node() &&
			ValueNode_DynamicList::Handle::cast_dynamic(value_desc.get_parent_value_node())))
		{
			// ... Let's check if we are selecting a composite child...
			if(value_desc.parent_is_value_node())
			{
				ValueNode::Handle compo(ValueNode_Composite::Handle::cast_dynamic(value_desc.get_parent_value_node()));
				if(compo)
				{
					ValueNode_DynamicList::Handle parent_list=NULL;
					std::set<Node*>::iterator iter;
					// and if the composite parent is a dynamic list type
					for(iter=compo->parent_set.begin();iter!=compo->parent_set.end();++iter)
						{
							parent_list=ValueNode_DynamicList::Handle::cast_dynamic(*iter);
							if(parent_list)
								break;
						}
					if(parent_list)
						continue;
				}
			}
			return false;
		}
	}
	return true;
}

bool
Action::ValueDescRemoveSmart::set_param(const synfig::String& name, const Action::Param &param)
{
	ValueNode_DynamicList::Handle value_node;
	if(name=="value_desc" && param.get_type()==Param::TYPE_VALUEDESC)
	{
		ValueDesc value_desc(param.get_value_desc());
		if(!value_desc.parent_is_value_node())
			return false;
		value_node=ValueNode_DynamicList::Handle::cast_dynamic(value_desc.get_parent_value_node());
		if(!value_node)
		{
			ValueNode::Handle compo(ValueNode_Composite::Handle::cast_dynamic(value_desc.get_parent_value_node()));
			if(compo)
			{
				ValueNode_DynamicList::Handle parent_list=NULL;
				std::set<Node*>::iterator iter;
				// now check if the composite's parent is a dynamic list type
				for(iter=compo->parent_set.begin();iter!=compo->parent_set.end();++iter)
					{
						parent_list=ValueNode_DynamicList::Handle::cast_dynamic(*iter);
						if(parent_list)
						{
							value_node=parent_list;
							// Now we need to find the index of this composite item
							// on the dynamic list
							int i;
							for(i=0;i<value_node->link_count();i++)
								if(compo->get_guid()==value_node->get_link(i)->get_guid())
									break;
							if(i<value_node->link_count())
								value_desc=synfigapp::ValueDesc(value_node, i);
							else
								return false;
							break;
						}
					}
				if(!value_node)
					return false;
			}
			else
				return false;
			if(!value_node)
				return false;
		}
		ValueNodes::iterator it;
		// Try to find the current parent value node in our map
		it=value_nodes.find(value_node);
		if(it==value_nodes.end())
		{
			// Not found, then create a new one
			value_nodes[value_node].push_back(value_desc.get_index());
		}
		else
		{
			// found, then insert the index of the value desc.
			// Maybe the index is already inserted.
			// Later is ignored if that happen
			it->second.push_back(value_desc.get_index());
		}
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
Action::ValueDescRemoveSmart::is_ready()const
{
	// Only ready if there is any parent value desc in our map
	if(!value_nodes.size())
		return false;
	return Action::CanvasSpecific::is_ready();
}

void
Action::ValueDescRemoveSmart::prepare()
{
	ValueNodes::iterator it;
	clear();
	for(it=value_nodes.begin();it!=value_nodes.end();++it)
	{
		synfig::ValueNode_DynamicList::Handle value_node(it->first);
		std::list<int> l(it->second.begin(), it->second.end());
		std::list<int>::iterator i;
		// sor the indexes to perform the actions from higher to lower index
		l.sort();
		int prev_index=-1;
		for(i=l.begin();i!=l.end();++i)
		{
			int index(*i);
			// This prevents duplicated index
			if(index==prev_index)
				continue;
			prev_index=index;
			Action::Handle action;
			// If we are in animate editing mode
			if(get_edit_mode()&MODE_ANIMATE)
				action=Action::create("ActivepointSetOff");
			else
				action=Action::create("ValueNodeDynamicListRemove");
			if(!action)
				throw Error(_("Unable to find action (bug)"));
			action->set_param("canvas",get_canvas());
			action->set_param("canvas_interface",get_canvas_interface());
			action->set_param("time",time);
			action->set_param("origin",origin);
			action->set_param("value_desc",ValueDesc(value_node,index));
			if(!action->is_ready())
				throw Error(Error::TYPE_NOTREADY);
			add_action_front(action);
		}
	}
}
