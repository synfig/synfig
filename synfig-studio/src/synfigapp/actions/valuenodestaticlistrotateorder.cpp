/* === S Y N F I G ========================================================= */
/*!	\file valuenodestaticlistrotateorder.cpp
**	\brief Template File
**
**	$Id$
**
**	\legal
**	Copyright (c) 2002-2005 Robert B. Quattlebaum Jr., Adrian Bentley
**  Copyright (c) 2008 Chris Moore
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

#include "valuenodestaticlistrotateorder.h"
#include <synfigapp/canvasinterface.h>

#include <synfigapp/localization.h>

#endif

using namespace std;
using namespace etl;
using namespace synfig;
using namespace synfigapp;
using namespace Action;

/* === M A C R O S ========================================================= */

ACTION_INIT(Action::ValueNodeStaticListRotateOrder);
ACTION_SET_NAME(Action::ValueNodeStaticListRotateOrder,"ValueNodeStaticListRotateOrder");
ACTION_SET_LOCAL_NAME(Action::ValueNodeStaticListRotateOrder,N_("Rotate Order"));
ACTION_SET_TASK(Action::ValueNodeStaticListRotateOrder,"rotate");
ACTION_SET_CATEGORY(Action::ValueNodeStaticListRotateOrder,Action::CATEGORY_VALUEDESC|Action::CATEGORY_VALUENODE);
ACTION_SET_PRIORITY(Action::ValueNodeStaticListRotateOrder,0);
ACTION_SET_VERSION(Action::ValueNodeStaticListRotateOrder,"0.0");

/* === G L O B A L S ======================================================= */

/* === P R O C E D U R E S ================================================= */

/* === M E T H O D S ======================================================= */

Action::ValueNodeStaticListRotateOrder::ValueNodeStaticListRotateOrder()
{
	index=0;
	set_dirty(true);
}

Action::ParamVocab
Action::ValueNodeStaticListRotateOrder::get_param_vocab()
{
	ParamVocab ret(Action::CanvasSpecific::get_param_vocab());

	ret.push_back(ParamDesc("value_desc",Param::TYPE_VALUEDESC)
		.set_local_name(_("ValueDesc"))
	);

	return ret;
}

bool
Action::ValueNodeStaticListRotateOrder::is_candidate(const ParamList &x)
{
	if (!candidate_check(get_param_vocab(),x))
		return false;

	ValueDesc value_desc(x.find("value_desc")->second.get_value_desc());

	ValueNode_StaticList::Handle static_list;
	return (value_desc.parent_is_value_node() &&
			// We need a static list
			(static_list = ValueNode_StaticList::Handle::cast_dynamic(value_desc.get_parent_value_node())) &&
			// We need the list not to be a list of bones
			(getenv("SYNFIG_ALLOW_ROTATE_ORDER_FOR_BONES") || static_list->get_contained_type() != type_bone_object));
}

bool
Action::ValueNodeStaticListRotateOrder::set_param(const synfig::String& name, const Action::Param &param)
{
	if(name=="value_desc" && param.get_type()==Param::TYPE_VALUEDESC)
	{
		ValueDesc value_desc(param.get_value_desc());

		if(!value_desc.parent_is_value_node())
			return false;

		value_node=ValueNode_StaticList::Handle::cast_dynamic(value_desc.get_parent_value_node());

		if(!value_node)
			return false;

		index=value_desc.get_index();

		return true;
	}

	return Action::CanvasSpecific::set_param(name,param);
}

bool
Action::ValueNodeStaticListRotateOrder::is_ready()const
{
	if(!value_node)
		return false;
	return Action::CanvasSpecific::is_ready();
}

void
Action::ValueNodeStaticListRotateOrder::prepare()
{
	clear();

	for(int i(0);i<(value_node->link_count()-index)%value_node->link_count();++i)
	{
		ValueDesc value_desc(value_node,value_node->link_count()-1-i);
		ValueNode::Handle child(value_desc.get_value_node());

		{
			Action::Handle action(Action::create("ValueNodeStaticListRemove"));

			action->set_param("canvas",get_canvas());
			action->set_param("canvas_interface",get_canvas_interface());
			action->set_param("value_desc",ValueDesc(value_node,value_node->link_count()-1));

			assert(action->is_ready());
			if(!action->is_ready())
				throw Error(Error::TYPE_NOTREADY);

			add_action(action);
		}

		{
			Action::Handle action(Action::create("ValueNodeStaticListInsert"));

			action->set_param("canvas",get_canvas());
			action->set_param("canvas_interface",get_canvas_interface());
			action->set_param("item",child);
			action->set_param("value_desc",ValueDesc(value_node,0));

			assert(action->is_ready());
			if(!action->is_ready())
				throw Error(Error::TYPE_NOTREADY);

			add_action(action);
		}
	}
}
