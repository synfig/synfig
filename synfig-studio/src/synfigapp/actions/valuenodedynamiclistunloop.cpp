/* === S Y N F I G ========================================================= */
/*!	\file valuenodedynamiclistunloop.cpp
**	\brief Template File
**
**	$Id$
**
**	\legal
**	Copyright (c) 2002-2005 Robert B. Quattlebaum Jr., Adrian Bentley
**	Copyright (c) 2008 Chris Moore
**	Copyright (c) 2012 Carlos López
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

#include "valuenodedynamiclistunloop.h"
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

ACTION_INIT(Action::ValueNodeDynamicListUnLoop);
ACTION_SET_NAME(Action::ValueNodeDynamicListUnLoop,"ValueNodeDynamicListUnLoop");
ACTION_SET_LOCAL_NAME(Action::ValueNodeDynamicListUnLoop,N_("Unloop"));
ACTION_SET_TASK(Action::ValueNodeDynamicListUnLoop,"unloop");
ACTION_SET_CATEGORY(Action::ValueNodeDynamicListUnLoop,Action::CATEGORY_VALUEDESC|Action::CATEGORY_VALUENODE);
ACTION_SET_PRIORITY(Action::ValueNodeDynamicListUnLoop,0);
ACTION_SET_VERSION(Action::ValueNodeDynamicListUnLoop,"0.0");
ACTION_SET_CVS_ID(Action::ValueNodeDynamicListUnLoop,"$Id$");

/* === G L O B A L S ======================================================= */

/* === P R O C E D U R E S ================================================= */

/* === M E T H O D S ======================================================= */

Action::ValueNodeDynamicListUnLoop::ValueNodeDynamicListUnLoop():
	old_loop_value()
{ }

Action::ParamVocab
Action::ValueNodeDynamicListUnLoop::get_param_vocab()
{
	ParamVocab ret(Action::CanvasSpecific::get_param_vocab());
	ret.push_back(ParamDesc("value_node",Param::TYPE_VALUENODE)
		.set_local_name(_("ValueNode"))
	);
	return ret;
}

bool
Action::ValueNodeDynamicListUnLoop::is_candidate(const ParamList &x)
{
	if (!candidate_check(get_param_vocab(),x))
		return false;
	ValueNode::Handle value_node;
	ValueDesc value_desc(x.find("value_desc")->second.get_value_desc());
	if(value_desc.parent_is_value_node())
	{
		value_node = value_desc.get_parent_value_node();
		// let's check if the parent is a composite (if user clicked on tangent duck)
		if(ValueNode_Composite::Handle::cast_dynamic(value_node))
		{
			ValueNode_Composite::Handle compo(ValueNode_Composite::Handle::cast_dynamic(value_node));
			ValueNode_BLine::Handle bline=NULL;
			std::set<Node*>::iterator iter;
			// now check if the grand parent is a dynamic list 'bline' type
			for(iter=compo->parent_set.begin();iter!=compo->parent_set.end();++iter)
				{
					bline=ValueNode_BLine::Handle::cast_dynamic(*iter);
					if(bline)
						break;
				}
			if(bline)
				value_node=bline;
		}
	}
	else
		value_node = x.find("value_node")->second.get_value_node();
	// We need a dynamic list.
	return (ValueNode_DynamicList::Handle::cast_dynamic(value_node) &&
			// We need the list to be looped.
			ValueNode_DynamicList::Handle::cast_dynamic(value_node)->get_loop());
}

bool
Action::ValueNodeDynamicListUnLoop::set_param(const synfig::String& name, const Action::Param &param)
{
	if(!value_node && name=="value_desc" && param.get_type()==Param::TYPE_VALUEDESC)
	{
		ValueDesc value_desc(param.get_value_desc());
		if(!value_desc.parent_is_value_node())
			return false;
		// Let's check if it is a dynamic list
		value_node=ValueNode_DynamicList::Handle::cast_dynamic(value_desc.get_parent_value_node());
		if (!value_node)
		{
			// we didn't found a dynamic list, let's check whether the parent is a composite
			if(ValueNode_Composite::Handle::cast_dynamic(value_desc.get_parent_value_node()))
			{
				ValueNode_Composite::Handle compo(ValueNode_Composite::Handle::cast_dynamic(value_desc.get_parent_value_node()));
				ValueNode_BLine::Handle bline=NULL;
				std::set<Node*>::iterator iter;
				// now check if the grand parent is a 'bline' type
				for(iter=compo->parent_set.begin();iter!=compo->parent_set.end();++iter)
					{
						bline=ValueNode_BLine::Handle::cast_dynamic(*iter);
						if(bline)
							break;
					}
				if(bline)
					value_node=bline;
			}
		}
		if(!value_node)
			return false;
		return true;
	}
	if(!value_node && name=="value_node" && param.get_type()==Param::TYPE_VALUENODE)
	{
		value_node=ValueNode_DynamicList::Handle::cast_dynamic(param.get_value_node());
		if(!value_node)
			return false;
		return true;
	}
	return Action::CanvasSpecific::set_param(name,param);
}

bool
Action::ValueNodeDynamicListUnLoop::is_ready()const
{
	if(!value_node)
		return false;
	return Action::CanvasSpecific::is_ready();
}

void
Action::ValueNodeDynamicListUnLoop::perform()
{
	old_loop_value=value_node->get_loop();
	if(old_loop_value==false)
	{
		set_dirty(false);
		return;
	}
	set_dirty(true);
	value_node->set_loop(false);
	value_node->changed();
}

void
Action::ValueNodeDynamicListUnLoop::undo()
{
	if(old_loop_value==value_node->get_loop())
	{
		set_dirty(false);
		return;
	}
	set_dirty(true);
	value_node->set_loop(old_loop_value);
	value_node->changed();
}
