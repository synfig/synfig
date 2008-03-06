/* === S Y N F I G ========================================================= */
/*!	\file valuedescblinelink.cpp
**	\brief Template File
**
**	$Id$
**
**	\legal
**	Copyright (c) 2002-2005 Robert B. Quattlebaum Jr., Adrian Bentley
**	Copyright (c) 2007, 2008 Chris Moore
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

#include "valuenodelinkconnect.h"
#include "valuedescblinelink.h"

#include <synfigapp/canvasinterface.h>
#include <synfig/valuenode_const.h>
#include <synfig/valuenode_composite.h>
#include <synfig/valuenode_blinecalcvertex.h>

#include <synfigapp/general.h>

#endif

using namespace std;
using namespace etl;
using namespace synfig;
using namespace synfigapp;
using namespace Action;

/* === M A C R O S ========================================================= */

ACTION_INIT(Action::ValueDescBLineLink);
ACTION_SET_NAME(Action::ValueDescBLineLink,"value_desc_bline_link");
ACTION_SET_LOCAL_NAME(Action::ValueDescBLineLink,N_("Link to BLine"));
ACTION_SET_TASK(Action::ValueDescBLineLink,"connect");
ACTION_SET_CATEGORY(Action::ValueDescBLineLink,Action::CATEGORY_BEZIER);
ACTION_SET_PRIORITY(Action::ValueDescBLineLink,0);
ACTION_SET_VERSION(Action::ValueDescBLineLink,"0.0");
ACTION_SET_CVS_ID(Action::ValueDescBLineLink,"$Id$");

/* === G L O B A L S ======================================================= */

/* === P R O C E D U R E S ================================================= */

/* === M E T H O D S ======================================================= */

Action::ValueDescBLineLink::ValueDescBLineLink()
{
}

Action::ParamVocab
Action::ValueDescBLineLink::get_param_vocab()
{
	ParamVocab ret(Action::CanvasSpecific::get_param_vocab());

	ret.push_back(ParamDesc("selected_value_desc",Param::TYPE_VALUEDESC)
		.set_local_name(_("ValueDesc to link"))
		.set_supports_multiple()
	);
	ret.push_back(ParamDesc("value_desc",Param::TYPE_VALUEDESC)
		.set_local_name(_("ValueDesc on BLine to link to"))
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
Action::ValueDescBLineLink::is_candidate(const ParamList &x)
{
	ParamList::const_iterator i;

	ValueDesc value_desc(x.find("value_desc")->second.get_value_desc());

	if (!candidate_check(get_param_vocab(),x))
		return false;

	return (value_desc.parent_is_value_node() &&
			// We need a dynamic list.
			ValueNode_DynamicList::Handle::cast_dynamic(value_desc.get_parent_value_node()));
}

bool
Action::ValueDescBLineLink::set_param(const synfig::String& name, const Action::Param &param)
{
	if(name=="time" && param.get_type()==Param::TYPE_TIME)
	{
		time=param.get_time();
		return true;
	}

	if(name=="value_desc" && param.get_type()==Param::TYPE_VALUEDESC)
	{
		value_desc = param.get_value_desc();
		index=value_desc.get_index();
		return true;
	}

	if(name=="selected_value_desc" && param.get_type()==Param::TYPE_VALUEDESC)
	{
		ValueDesc value_desc(param.get_value_desc());
		value_desc_list.push_back(value_desc);
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
Action::ValueDescBLineLink::is_ready()const
{
	if(value_desc_list.size()<1)
		return false;
	if(!value_desc)
		return false;
	return Action::CanvasSpecific::is_ready();
}

void
Action::ValueDescBLineLink::prepare()
{
	if(value_desc_list.empty())
		throw Error(Error::TYPE_NOTREADY);

	clear();

	ValueNode_DynamicList::Handle bline_value_node(ValueNode_DynamicList::Handle::cast_dynamic(value_desc.get_parent_value_node()));
	bool loop(bline_value_node->get_loop());
	int loop_adjust(loop ? 0 : -1);
	const std::vector<ValueBase> bline((*bline_value_node)(time));
	int size = bline.size();
	Real amount = (index + origin + loop_adjust) / (size + loop_adjust);

	std::list<ValueDesc>::iterator iter;
	for(iter=value_desc_list.begin();iter!=value_desc_list.end();++iter)
	{
		ValueDesc& value_desc(*iter);
		if (value_desc.parent_is_value_node())
		{
			ValueNode::Handle value_node(value_desc.get_value_node());
			if (value_desc.get_value_type()==ValueBase::TYPE_BLINEPOINT &&
				value_desc.is_value_node() &&
				ValueNode_Composite::Handle::cast_dynamic(value_desc.get_value_node()))
			{
				LinkableValueNode::Handle bline_vertex(ValueNode_BLineCalcVertex::create(ValueBase::TYPE_VECTOR));
				bline_vertex->set_link("bline", bline_value_node);
				bline_vertex->set_link("loop", ValueNode_Const::create(loop));
				bline_vertex->set_link("amount", ValueNode_Const::create(amount));

				Action::Handle action(ValueNodeLinkConnect::create());

				action->set_param("canvas",get_canvas());
				action->set_param("canvas_interface",get_canvas_interface());
				action->set_param("parent_value_node",value_node);
				action->set_param("value_node",ValueNode::Handle(bline_vertex));
				action->set_param("index",0); // index for 'vertex' in 'composite'

				assert(action->is_ready());
				if(!action->is_ready())
					throw Error(Error::TYPE_NOTREADY);

				add_action_front(action);
			}
		}
	}
}
