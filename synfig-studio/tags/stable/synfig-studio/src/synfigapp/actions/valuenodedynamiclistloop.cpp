/* === S Y N F I G ========================================================= */
/*!	\file valuenodedynamiclistloop.cpp
**	\brief Template File
**
**	$Id: valuenodedynamiclistloop.cpp,v 1.1.1.1 2005/01/07 03:34:37 darco Exp $
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

#include "valuenodedynamiclistloop.h"
#include <synfigapp/canvasinterface.h>

#endif

using namespace std;
using namespace etl;
using namespace synfig;
using namespace synfigapp;
using namespace Action;

/* === M A C R O S ========================================================= */

ACTION_INIT(Action::ValueNodeDynamicListLoop);
ACTION_SET_NAME(Action::ValueNodeDynamicListLoop,"value_node_dynamic_list_loop");
ACTION_SET_LOCAL_NAME(Action::ValueNodeDynamicListLoop,"Loop");
ACTION_SET_TASK(Action::ValueNodeDynamicListLoop,"loop");
ACTION_SET_CATEGORY(Action::ValueNodeDynamicListLoop,Action::CATEGORY_VALUEDESC|Action::CATEGORY_VALUENODE);
ACTION_SET_PRIORITY(Action::ValueNodeDynamicListLoop,0);
ACTION_SET_VERSION(Action::ValueNodeDynamicListLoop,"0.0");
ACTION_SET_CVS_ID(Action::ValueNodeDynamicListLoop,"$Id: valuenodedynamiclistloop.cpp,v 1.1.1.1 2005/01/07 03:34:37 darco Exp $");

/* === G L O B A L S ======================================================= */

/* === P R O C E D U R E S ================================================= */

/* === M E T H O D S ======================================================= */

Action::ValueNodeDynamicListLoop::ValueNodeDynamicListLoop()
{
}

Action::ParamVocab
Action::ValueNodeDynamicListLoop::get_param_vocab()
{
	ParamVocab ret(Action::CanvasSpecific::get_param_vocab());
	
	ret.push_back(ParamDesc("value_node",Param::TYPE_VALUENODE)
		.set_local_name(_("ValueNode"))
	);

	return ret;
}

bool
Action::ValueNodeDynamicListLoop::is_canidate(const ParamList &x)
{
	if(canidate_check(get_param_vocab(),x))
	{
		ValueNode::Handle value_node(x.find("value_node")->second.get_value_node());
		if(!ValueNode_DynamicList::Handle::cast_dynamic(value_node))
			return false;
		if(ValueNode_DynamicList::Handle::cast_dynamic(value_node)->get_loop()==true)
			return false;
		return true;
	}
	return false;
}

bool
Action::ValueNodeDynamicListLoop::set_param(const synfig::String& name, const Action::Param &param)
{
	if(name=="value_node" && param.get_type()==Param::TYPE_VALUENODE)
	{		
		value_node=ValueNode_DynamicList::Handle::cast_dynamic(param.get_value_node());
		
		if(!value_node)
			return false;

		return true;
	}

	return Action::CanvasSpecific::set_param(name,param);
}

bool
Action::ValueNodeDynamicListLoop::is_ready()const
{
	if(!value_node)
		return false;
	return Action::CanvasSpecific::is_ready();
}

void
Action::ValueNodeDynamicListLoop::perform()
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
Action::ValueNodeDynamicListLoop::undo()
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
