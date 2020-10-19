/* === S Y N F I G ========================================================= */
/*!	\file valuenodedynamiclistremovesmart.cpp
**	\brief Template File
**
**	$Id$
**
**	\legal
**	Copyright (c) 2002-2005 Robert B. Quattlebaum Jr., Adrian Bentley
**  Copyright (c) 2008 Chris Moore
**  Copyright (c) 2012 Carlos López
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

#include "valuenodedynamiclistremovesmart.h"
#include "valuenodedynamiclistremove.h"
#include <synfigapp/canvasinterface.h>

#include <synfigapp/localization.h>
#include <synfig/valuenodes/valuenode_composite.h>

#endif

using namespace std;
using namespace etl;
using namespace synfig;
using namespace synfigapp;
using namespace Action;

/* === M A C R O S ========================================================= */

ACTION_INIT(Action::ValueNodeDynamicListRemoveSmart);
ACTION_SET_NAME(Action::ValueNodeDynamicListRemoveSmart,"ValueNodeDynamicListRemoveSmart");
ACTION_SET_LOCAL_NAME(Action::ValueNodeDynamicListRemoveSmart,N_("Remove Item (Smart)"));
ACTION_SET_TASK(Action::ValueNodeDynamicListRemoveSmart,"remove");
ACTION_SET_CATEGORY(Action::ValueNodeDynamicListRemoveSmart,Action::CATEGORY_VALUEDESC|Action::CATEGORY_VALUENODE);
ACTION_SET_PRIORITY(Action::ValueNodeDynamicListRemoveSmart,-19);
ACTION_SET_VERSION(Action::ValueNodeDynamicListRemoveSmart,"0.0");

/* === G L O B A L S ======================================================= */

/* === P R O C E D U R E S ================================================= */

/* === M E T H O D S ======================================================= */

Action::ValueNodeDynamicListRemoveSmart::ValueNodeDynamicListRemoveSmart()
{
	index=0;
	time=0;
	origin=0.5f;
	set_dirty(true);
}

Action::ParamVocab
Action::ValueNodeDynamicListRemoveSmart::get_param_vocab()
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
Action::ValueNodeDynamicListRemoveSmart::is_candidate(const ParamList &x)
{
	if (!candidate_check(get_param_vocab(),x))
		return false;

	ValueDesc value_desc(x.find("value_desc")->second.get_value_desc());
	bool parent_is_dynamic_list(
			value_desc.parent_is_value_node() &&
			ValueNode_DynamicList::Handle::cast_dynamic(value_desc.get_parent_value_node())
			);
	if(!parent_is_dynamic_list)
	{
		// Let's check if we are selecting a composite child
		if(value_desc.parent_is_value_node())
		{
			ValueNode::Handle compo(ValueNode_Composite::Handle::cast_dynamic(value_desc.get_parent_value_node()));
			if(compo)
			{
				ValueNode_DynamicList::Handle parent_list=NULL;
				std::set<Node*>::iterator iter;
				// now check if the composite parent is a dynamic list type
				for(iter=compo->parent_set.begin();iter!=compo->parent_set.end();++iter)
					{
						parent_list=ValueNode_DynamicList::Handle::cast_dynamic(*iter);
						if(parent_list)
							break;
					}
				if(parent_list)
					parent_is_dynamic_list=true;
			}
		}
	}
	return (parent_is_dynamic_list);
}

bool
Action::ValueNodeDynamicListRemoveSmart::set_param(const synfig::String& name, const Action::Param &param)
{
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
Action::ValueNodeDynamicListRemoveSmart::is_ready()const
{
	if(!value_node)
		return false;
	return Action::CanvasSpecific::is_ready();
}

void
Action::ValueNodeDynamicListRemoveSmart::prepare()
{
	clear();

	// If we are in animate editing mode
	if(get_edit_mode()&MODE_ANIMATE)
	{
		Action::Handle action(Action::create("ActivepointSetOff"));

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
	else
	{
		Action::Handle action(Action::create("ValueNodeDynamicListRemove"));

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
