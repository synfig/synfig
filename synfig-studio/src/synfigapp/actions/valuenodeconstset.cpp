/* === S Y N F I G ========================================================= */
/*!	\file valuenodeconstset.cpp
**	\brief Template File
**
**	$Id$
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

#include <synfig/general.h>

#include "valuenodeconstset.h"
#include <synfigapp/canvasinterface.h>

#include <synfigapp/localization.h>

#endif

using namespace std;
using namespace etl;
using namespace synfig;
using namespace synfigapp;
using namespace Action;

/* === M A C R O S ========================================================= */

ACTION_INIT(Action::ValueNodeConstSet);
ACTION_SET_NAME(Action::ValueNodeConstSet,"ValueNodeConstSet");
ACTION_SET_LOCAL_NAME(Action::ValueNodeConstSet,N_("Set ValueNode_Const"));
ACTION_SET_TASK(Action::ValueNodeConstSet,"set");
ACTION_SET_CATEGORY(Action::ValueNodeConstSet,Action::CATEGORY_VALUENODE);
ACTION_SET_PRIORITY(Action::ValueNodeConstSet,0);
ACTION_SET_VERSION(Action::ValueNodeConstSet,"0.0");

/* === G L O B A L S ======================================================= */

/* === P R O C E D U R E S ================================================= */

/* === M E T H O D S ======================================================= */

Action::ValueNodeConstSet::ValueNodeConstSet()
{
	set_dirty(true);
}

Action::ParamVocab
Action::ValueNodeConstSet::get_param_vocab()
{
	ParamVocab ret(Action::CanvasSpecific::get_param_vocab());

	ret.push_back(ParamDesc("value_node",Param::TYPE_VALUENODE)
		.set_local_name(_("ValueNode_Const"))
	);

	ret.push_back(ParamDesc("new_value",Param::TYPE_VALUE)
		.set_local_name(_("ValueBase"))
	);

	return ret;
}

bool
Action::ValueNodeConstSet::is_candidate(const ParamList &x)
{
	if(candidate_check(get_param_vocab(),x))
	{
		if(ValueNode_Const::Handle::cast_dynamic(x.find("value_node")->second.get_value_node()))
			return true;
	}
	return false;
}

bool
Action::ValueNodeConstSet::set_param(const synfig::String& name, const Action::Param &param)
{
	if(name=="value_node" && param.get_type()==Param::TYPE_VALUENODE)
	{
		value_node=ValueNode_Const::Handle::cast_dynamic(param.get_value_node());

		return (bool)value_node;
	}

	if(name=="new_value" && param.get_type()==Param::TYPE_VALUE)
	{
		new_value=param.get_value();

		return true;
	}

	return Action::CanvasSpecific::set_param(name,param);
}

bool
Action::ValueNodeConstSet::is_ready()const
{
	if(!value_node || !new_value.is_valid())
		return false;
	return Action::CanvasSpecific::is_ready();
}

void
Action::ValueNodeConstSet::perform()
{
	//set_dirty(true);

	old_value=value_node->get_value();

	// We shouldn't change the parameters properties when change its value
	new_value.copy_properties_of(old_value);

	value_node->set_value(new_value);

	// Signal that a layer has been inserted
	/*if(get_canvas_interface())
	{
		get_canvas_interface()->signal_value_node_changed()(value_node);
	}*/
}

void
Action::ValueNodeConstSet::undo()
{
	//set_dirty(true);

	value_node->set_value(old_value);

	// Signal that a layer has been inserted
	/*if(get_canvas_interface())
	{
		get_canvas_interface()->signal_value_node_changed()(value_node);
	}*/
}
