/* === S Y N F I G ========================================================= */
/*!	\file valuenodestaticlistloop.cpp
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

#include "valuenodestaticlistloop.h"
#include <synfigapp/canvasinterface.h>

#include <synfigapp/localization.h>

#endif

using namespace etl;
using namespace synfig;
using namespace synfigapp;
using namespace Action;

/* === M A C R O S ========================================================= */

ACTION_INIT(Action::ValueNodeStaticListLoop);
ACTION_SET_NAME(Action::ValueNodeStaticListLoop,"ValueNodeStaticListLoop");
ACTION_SET_LOCAL_NAME(Action::ValueNodeStaticListLoop,N_("Loop"));
ACTION_SET_TASK(Action::ValueNodeStaticListLoop,"loop");
ACTION_SET_CATEGORY(Action::ValueNodeStaticListLoop,Action::CATEGORY_VALUEDESC|Action::CATEGORY_VALUENODE);
ACTION_SET_PRIORITY(Action::ValueNodeStaticListLoop,0);
ACTION_SET_VERSION(Action::ValueNodeStaticListLoop,"0.0");

/* === G L O B A L S ======================================================= */

/* === P R O C E D U R E S ================================================= */

/* === M E T H O D S ======================================================= */

Action::ValueNodeStaticListLoop::ValueNodeStaticListLoop():
	old_loop_value()
{ }

Action::ParamVocab
Action::ValueNodeStaticListLoop::get_param_vocab()
{
	ParamVocab ret(Action::CanvasSpecific::get_param_vocab());

	ret.push_back(ParamDesc("value_node",Param::TYPE_VALUENODE)
		.set_local_name(_("ValueNode"))
	);

	return ret;
}

bool
Action::ValueNodeStaticListLoop::is_candidate(const ParamList &x)
{
	if (!candidate_check(get_param_vocab(),x))
		return false;

	ValueNode::Handle value_node;
	ValueDesc value_desc(x.find("value_desc")->second.get_value_desc());

	if(value_desc.parent_is_value_node())
		value_node = value_desc.get_parent_value_node();
	else
		value_node = x.find("value_node")->second.get_value_node();

	ValueNode_StaticList::Handle static_list;
	// We need a static list
	return ((static_list = ValueNode_StaticList::Handle::cast_dynamic(value_node)) &&
			// We need the list not to be a list of bones
			static_list->get_contained_type() != type_bone_object &&
			// We need the list not to be looped
			!static_list->get_loop());
}

bool
Action::ValueNodeStaticListLoop::set_param(const synfig::String& name, const Action::Param &param)
{
	if(!value_node && name=="value_desc" && param.get_type()==Param::TYPE_VALUEDESC)
	{
		ValueDesc value_desc(param.get_value_desc());

		if(!value_desc.parent_is_value_node())
			return false;

		value_node=ValueNode_StaticList::Handle::cast_dynamic(value_desc.get_parent_value_node());

		if (!value_node)
			return false;

		return true;
	}

	if(!value_node && name=="value_node" && param.get_type()==Param::TYPE_VALUENODE)
	{
		value_node=ValueNode_StaticList::Handle::cast_dynamic(param.get_value_node());

		if(!value_node)
			return false;

		return true;
	}

	return Action::CanvasSpecific::set_param(name,param);
}

bool
Action::ValueNodeStaticListLoop::is_ready()const
{
	if(!value_node)
		return false;
	return Action::CanvasSpecific::is_ready();
}

void
Action::ValueNodeStaticListLoop::perform()
{
	old_loop_value=value_node->get_loop();

	if(old_loop_value==true)
	{
		set_dirty(false);
		return;
	}
	set_dirty(true);
	value_node->set_loop(true);

	value_node->changed();
/*_if(get_canvas_interface())
	{
		get_canvas_interface()->signal_value_node_changed()(value_node);
	}
	else synfig::warning("CanvasInterface not set on action");*/
}

void
Action::ValueNodeStaticListLoop::undo()
{
	if(old_loop_value==value_node->get_loop())
	{
		set_dirty(false);
		return;
	}
	set_dirty(true);
	value_node->set_loop(old_loop_value);

	value_node->changed();
/*_if(get_canvas_interface())
	{
		get_canvas_interface()->signal_value_node_changed()(value_node);
	}
	else synfig::warning("CanvasInterface not set on action");*/
}
