/* === S Y N F I G ========================================================= */
/*!	\file blinepointtangentmerge.cpp
**	\brief Template File
**
**	$Id$
**
**	\legal
**	Copyright (c) 2002-2005 Robert B. Quattlebaum Jr., Adrian Bentley
**	Copyright (c) 2007 Chris Moore
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

#include "blinepointtangentmerge.h"
#include "valuedescset.h"

#include "activepointset.h"
#include "activepointadd.h"

#include "valuedescconnect.h"
#include <synfigapp/canvasinterface.h>

#include <synfigapp/general.h>

#endif

using namespace std;
using namespace etl;
using namespace synfig;
using namespace synfigapp;
using namespace Action;

/* === M A C R O S ========================================================= */

ACTION_INIT_NO_GET_LOCAL_NAME(Action::BLinePointTangentMerge);
ACTION_SET_NAME(Action::BLinePointTangentMerge,"bline_point_tangent_merge");
ACTION_SET_LOCAL_NAME(Action::BLinePointTangentMerge,N_("Merge Tangents"));
ACTION_SET_TASK(Action::BLinePointTangentMerge,"merge");
ACTION_SET_CATEGORY(Action::BLinePointTangentMerge,Action::CATEGORY_VALUENODE);
ACTION_SET_PRIORITY(Action::BLinePointTangentMerge,0);
ACTION_SET_VERSION(Action::BLinePointTangentMerge,"0.0");
ACTION_SET_CVS_ID(Action::BLinePointTangentMerge,"$Id$");

/* === G L O B A L S ======================================================= */

/* === P R O C E D U R E S ================================================= */

/* === M E T H O D S ======================================================= */

Action::BLinePointTangentMerge::BLinePointTangentMerge()
{
	time=(Time::begin()-1);
	set_dirty(true);
}

synfig::String
Action::BLinePointTangentMerge::get_local_name()const
{
	return strprintf(_("Merge Tangents of '%s'"), ((ValueNode::Handle)(value_node))->get_description().c_str());
}

Action::ParamVocab
Action::BLinePointTangentMerge::get_param_vocab()
{
	ParamVocab ret(Action::CanvasSpecific::get_param_vocab());

	ret.push_back(ParamDesc("value_node",Param::TYPE_VALUENODE)
		.set_local_name(_("ValueNode of BLinePoint"))
	);

	ret.push_back(ParamDesc("time",Param::TYPE_TIME)
		.set_local_name(_("Time"))
	);

	return ret;
}

bool
Action::BLinePointTangentMerge::is_candidate(const ParamList &x)
{
	if(candidate_check(get_param_vocab(),x))
	{
		ValueNode_Composite::Handle value_node;
		value_node=ValueNode_Composite::Handle::cast_dynamic(x.find("value_node")->second.get_value_node());
		if(!value_node || value_node->get_type()!=ValueBase::TYPE_BLINEPOINT)
			return false;
		synfig::Time time(x.find("time")->second.get_time());
		if((*value_node->get_link("split"))(time).get(bool())==false)
			return false;
		return true;
	}
	return false;
}

bool
Action::BLinePointTangentMerge::set_param(const synfig::String& name, const Action::Param &param)
{
	if(name=="value_node" && param.get_type()==Param::TYPE_VALUENODE)
	{
		value_node=value_node.cast_dynamic(param.get_value_node());

		return (bool)(value_node);
	}
	if(name=="time" && param.get_type()==Param::TYPE_TIME)
	{
		time=param.get_time();

		return true;
	}

	return Action::CanvasSpecific::set_param(name,param);
}

bool
Action::BLinePointTangentMerge::is_ready()const
{
	if(!value_node)
		synfig::error("Missing or bad value_node");

	if(time==(Time::begin()-1))
		synfig::error("Missing time");

	if(!value_node || time==(Time::begin()-1))
		return false;
	return Action::CanvasSpecific::is_ready();
}

void
Action::BLinePointTangentMerge::prepare()
{
	clear();

	Action::Handle action;

	action=Action::create("value_desc_set");
	if(!action)
		throw Error(_("Couldn't find action \"value_desc_set\""));

	action->set_param("canvas",get_canvas());
	action->set_param("canvas_interface",get_canvas_interface());
	action->set_param("value_desc",ValueDesc(value_node,3));
	action->set_param("time",time);
	action->set_param("new_value",synfig::ValueBase(false));

	assert(action->is_ready());
	if(!action->is_ready())
		throw Error(Error::TYPE_NOTREADY);

	add_action(action);
}
