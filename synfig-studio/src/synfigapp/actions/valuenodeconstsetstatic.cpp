/* === S Y N F I G ========================================================= */
/*!	\file valuenodeconstsetstatic.cpp
**	\brief Template File
**
**	$Id$
**
**	\legal
**	Copyright (c) 2002-2005 Robert B. Quattlebaum Jr., Adrian Bentley
**	Copyright (c) 2010 Carlos López
**
**	This file is part of Synfig.
**
**	Synfig is free software: you can redistribute it and/or modify
**	it under the terms of the GNU General Public License as published by
**	the Free Software Foundation, either version 2 of the License, or
**	(at your option) any later version.
**
**	Synfig is distributed in the hope that it will be useful,
**	but WITHOUT ANY WARRANTY; without even the implied warranty of
**	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
**	GNU General Public License for more details.
**
**	You should have received a copy of the GNU General Public License
**	along with Synfig.  If not, see <https://www.gnu.org/licenses/>.
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

#include "valuenodeconstsetstatic.h"
#include <synfigapp/canvasinterface.h>

#include <synfigapp/localization.h>

#endif

using namespace synfig;
using namespace synfigapp;
using namespace Action;

/* === M A C R O S ========================================================= */

ACTION_INIT(Action::ValueNodeConstSetStatic);
ACTION_SET_NAME(Action::ValueNodeConstSetStatic,"ValueNodeConstSetStatic");
ACTION_SET_LOCAL_NAME(Action::ValueNodeConstSetStatic,N_("Forbid Animation"));
ACTION_SET_TASK(Action::ValueNodeConstSetStatic,"setstatic");
ACTION_SET_CATEGORY(Action::ValueNodeConstSetStatic,Action::CATEGORY_VALUEDESC|Action::CATEGORY_VALUENODE);
ACTION_SET_PRIORITY(Action::ValueNodeConstSetStatic,0);
ACTION_SET_VERSION(Action::ValueNodeConstSetStatic,"0.0");

/* === G L O B A L S ======================================================= */

/* === P R O C E D U R E S ================================================= */

/* === M E T H O D S ======================================================= */

Action::ValueNodeConstSetStatic::ValueNodeConstSetStatic():
	old_static_value()
{ }

Action::ParamVocab
Action::ValueNodeConstSetStatic::get_param_vocab()
{
	ParamVocab ret(Action::CanvasSpecific::get_param_vocab());

	ret.push_back(ParamDesc("value_node",Param::TYPE_VALUENODE)
		.set_local_name(_("ValueNode"))
	);

	return ret;
}

bool
Action::ValueNodeConstSetStatic::is_candidate(const ParamList &x)
{
	if (!candidate_check(get_param_vocab(),x))
		return false;

	ValueNode::Handle value_node;
	ValueDesc value_desc(x.find("value_desc")->second.get_value_desc());

	if(value_desc.parent_is_value_node_const() || value_desc.parent_is_linkable_value_node())
		value_node = value_desc.get_value_node();
	else
		value_node = x.find("value_node")->second.get_value_node();

	// We need a constant value node or a constant layer param.
	return (
			(ValueNode_Const::Handle::cast_dynamic(value_node) &&
			// We need the constant value node to be not static.
			!ValueNode_Const::Handle::cast_dynamic(value_node)->get_static())
			);
}

bool
Action::ValueNodeConstSetStatic::set_param(const synfig::String& name, const Action::Param &param)
{
	if(!value_node && name=="value_desc" && param.get_type()==Param::TYPE_VALUEDESC)
	{
		ValueDesc value_desc(param.get_value_desc());
		if(!value_desc.parent_is_value_node())
			return false;

		value_node=ValueNode_Const::Handle::cast_dynamic(value_desc.get_value_node());

		if(!value_node)
			return false;

		return true;
	}

	if(!value_node && name=="value_node" && param.get_type()==Param::TYPE_VALUENODE)
	{
		value_node=ValueNode_Const::Handle::cast_dynamic(param.get_value_node());

		if(!value_node)
			return false;

		return true;
	}

	return Action::CanvasSpecific::set_param(name,param);
}

bool
Action::ValueNodeConstSetStatic::is_ready()const
{
	if(!value_node)
		return false;
	return Action::CanvasSpecific::is_ready();
}

void
Action::ValueNodeConstSetStatic::perform()
{
	old_static_value=value_node->get_static();

	if(old_static_value==true)
	{
		set_dirty(false);
		return;
	}
	set_dirty(true);
	value_node->set_static(true);

	value_node->changed();
}

void
Action::ValueNodeConstSetStatic::undo()
{
	if(old_static_value==value_node->get_static())
	{
		set_dirty(false);
		return;
	}

	set_dirty(true);
	value_node->set_static(old_static_value);

	value_node->changed();

}
