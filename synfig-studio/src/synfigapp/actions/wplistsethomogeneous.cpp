/* === S Y N F I G ========================================================= */
/*!	\file wplistsethomogeneous.cpp
**	\brief Template File
**
**	$Id$
**
**	\legal
**	Copyright (c) 2002-2005 Robert B. Quattlebaum Jr., Adrian Bentley
**  Copyright (c) 20011 Carlos López
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

#include "wplistsethomogeneous.h"
#include <synfigapp/canvasinterface.h>

#include <synfigapp/general.h>

#endif

using namespace std;
using namespace etl;
using namespace synfig;
using namespace synfigapp;
using namespace Action;

/* === M A C R O S ========================================================= */

ACTION_INIT(Action::WPListSetHomogeneous);
ACTION_SET_NAME(Action::WPListSetHomogeneous,"WPListSetHomogeneous");
ACTION_SET_LOCAL_NAME(Action::WPListSetHomogeneous,N_("Set Homogeneous"));
ACTION_SET_TASK(Action::WPListSetHomogeneous,"sethomogeneous");
ACTION_SET_CATEGORY(Action::WPListSetHomogeneous,Action::CATEGORY_VALUEDESC|Action::CATEGORY_VALUENODE);
ACTION_SET_PRIORITY(Action::WPListSetHomogeneous,0);
ACTION_SET_VERSION(Action::WPListSetHomogeneous,"0.0");
ACTION_SET_CVS_ID(Action::WPListSetHomogeneous,"$Id$");

/* === G L O B A L S ======================================================= */

/* === P R O C E D U R E S ================================================= */

/* === M E T H O D S ======================================================= */

Action::WPListSetHomogeneous::WPListSetHomogeneous()
{
}

Action::ParamVocab
Action::WPListSetHomogeneous::get_param_vocab()
{
	ParamVocab ret(Action::CanvasSpecific::get_param_vocab());

	ret.push_back(ParamDesc("value_node",Param::TYPE_VALUENODE)
		.set_local_name(_("ValueNode"))
	);

	return ret;
}

bool
Action::WPListSetHomogeneous::is_candidate(const ParamList &x)
{
	if (!candidate_check(get_param_vocab(),x))
		return false;

	ValueNode::Handle value_node;
	ValueDesc value_desc(x.find("value_desc")->second.get_value_desc());

	if(value_desc.parent_is_value_node())
		value_node = value_desc.get_parent_value_node();
	else
		value_node = x.find("value_node")->second.get_value_node();

	// We need a WPlist.
	return (ValueNode_WPList::Handle::cast_dynamic(value_node) &&
			// We need the list to have homogeneous unset.
			!ValueNode_WPList::Handle::cast_dynamic(value_node)->get_homogeneous());
}

bool
Action::WPListSetHomogeneous::set_param(const synfig::String& name, const Action::Param &param)
{
	if(!value_node && name=="value_desc" && param.get_type()==Param::TYPE_VALUEDESC)
	{
		ValueDesc value_desc(param.get_value_desc());

		if(!value_desc.parent_is_value_node())
			return false;

		value_node=ValueNode_WPList::Handle::cast_dynamic(value_desc.get_parent_value_node());

		if (!value_node)
			return false;

		return true;
	}

	if(!value_node && name=="value_node" && param.get_type()==Param::TYPE_VALUENODE)
	{
		value_node=ValueNode_WPList::Handle::cast_dynamic(param.get_value_node());

		if(!value_node)
			return false;

		return true;
	}

	return Action::CanvasSpecific::set_param(name,param);
}

bool
Action::WPListSetHomogeneous::is_ready()const
{
	if(!value_node)
		return false;
	return Action::CanvasSpecific::is_ready();
}

void
Action::WPListSetHomogeneous::perform()
{
	old_homogeneous_value=value_node->get_homogeneous();

	if(old_homogeneous_value==true)
	{
		set_dirty(false);
		return;
	}

	set_dirty(true);
	value_node->set_homogeneous(true);

	value_node->changed();
}

void
Action::WPListSetHomogeneous::undo()
{
	if(old_homogeneous_value==value_node->get_homogeneous())
	{
		set_dirty(false);
		return;
	}

	set_dirty(true);
	value_node->set_homogeneous(old_homogeneous_value);

	value_node->changed();
}
