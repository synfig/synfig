/* === S Y N F I G ========================================================= */
/*!	\file valuedescdisconnect.cpp
**	\brief Template File
**
**	$Id$
**
**	\legal
**	Copyright (c) 2002-2005 Robert B. Quattlebaum Jr., Adrian Bentley
**	Copyright (c) 2008 Chris Moore
**	Copyright (c) 2017 caryoscelus
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

#include "layerparamdisconnect.h"
#include "waypointdisconnect.h"
#include "valuenodelinkdisconnect.h"
#include "valuenodereplace.h"

#include "valuedescdisconnect.h"
#include "valuedescconvert.h"
#include <synfigapp/canvasinterface.h>
#include <synfig/valuenodes/valuenode_const.h>
#include <synfig/valuenodes/valuenode_duplicate.h>
#include <synfig/valuenodes/valuenode_bone.h>

#include <synfigapp/localization.h>

#endif

using namespace std;
using namespace etl;
using namespace synfig;
using namespace synfigapp;
using namespace Action;

/* === M A C R O S ========================================================= */

ACTION_INIT_NO_GET_LOCAL_NAME(Action::ValueDescDisconnect);
ACTION_SET_NAME(Action::ValueDescDisconnect,"ValueDescDisconnect");
ACTION_SET_LOCAL_NAME(Action::ValueDescDisconnect,N_("Disconnect"));
ACTION_SET_TASK(Action::ValueDescDisconnect,"disconnect");
ACTION_SET_CATEGORY(Action::ValueDescDisconnect,Action::CATEGORY_VALUEDESC);
ACTION_SET_PRIORITY(Action::ValueDescDisconnect,-100);
ACTION_SET_VERSION(Action::ValueDescDisconnect,"0.0");

/* === G L O B A L S ======================================================= */

/* === P R O C E D U R E S ================================================= */

/* === M E T H O D S ======================================================= */

Action::ValueDescDisconnect::ValueDescDisconnect():
	time(0)
{
}

synfig::String
Action::ValueDescDisconnect::get_local_name()const
{
  // TRANSLATORS: This is used in the History dialog when a ValueNode is disconnected.
  return strprintf(_("Disconnect %s"),
                   value_desc
                   ? value_desc.get_description().c_str()
                   : _("ValueDesc"));
}

Action::ParamVocab
Action::ValueDescDisconnect::get_param_vocab()
{
	ParamVocab ret(Action::CanvasSpecific::get_param_vocab());

	ret.push_back(ParamDesc("value_desc",Param::TYPE_VALUEDESC)
		.set_local_name(_("ValueDesc"))
	);

	ret.push_back(ParamDesc("time",Param::TYPE_TIME)
		.set_local_name(_("Time"))
		.set_optional()
	);

	return ret;
}

bool
Action::ValueDescDisconnect::is_candidate(const ParamList &x)
{
	if (!candidate_check(get_param_vocab(),x))
		return false;

	ValueDesc value_desc(x.find("value_desc")->second.get_value_desc());

	// don't allow Bone ValueNodes to be disconnected
	if(getenv("SYNFIG_DISALLOW_BONE_DISCONNECT") &&
	   value_desc.is_value_node() &&
	   ValueNode_Bone::Handle::cast_dynamic(value_desc.get_value_node()))
		return false;
	// don't allow the Index parameter of the Duplicate layer to be disconnected
	if(value_desc.parent_is_layer() && value_desc.get_layer()->get_name() == "duplicate" && value_desc.get_param_name() == "index")
		return false;
	if(!value_desc.parent_is_canvas() && value_desc.is_value_node() && value_desc.get_value_node()->rcount()>1)
		return true;
	if(value_desc.is_const())
		return false;
	if(value_desc.is_value_node() && ValueNode_Const::Handle::cast_dynamic(value_desc.get_value_node()))
		return false;
	// don't allow Duplicate ValueNodes in the Children dialog to be disconnected
	if(value_desc.is_value_node() &&
	   ValueNode_Duplicate::Handle::cast_dynamic(value_desc.get_value_node()) &&
	   !value_desc.parent_is_layer() &&
	   !value_desc.parent_is_value_node())
		return false;
	return true;
}

bool
Action::ValueDescDisconnect::set_param(const synfig::String& name, const Action::Param &param)
{
	if(name=="value_desc" && param.get_type()==Param::TYPE_VALUEDESC)
	{
		value_desc=param.get_value_desc();

		return true;
	}

	if(name=="time" && param.get_type()==Param::TYPE_TIME)
	{
		time=param.get_time();

		return true;
	}

	return Action::CanvasSpecific::set_param(name,param);
}

bool
Action::ValueDescDisconnect::is_ready()const
{
	if(!value_desc)
		return false;
	return Action::CanvasSpecific::is_ready();
}

void
Action::ValueDescDisconnect::prepare()
{
	clear();

	if(value_desc.get_value_type() == type_transformation)
	{
		Action::Handle action(ValueDescConvert::create());

		action->set_param("canvas",get_canvas());
		action->set_param("canvas_interface",get_canvas_interface());
		action->set_param("value_desc",value_desc);
		action->set_param("type","composite");
		action->set_param("time",time);

		assert(action->is_ready());
		if(!action->is_ready())
			throw Error(Error::TYPE_NOTREADY);

		add_action_front(action);
		return;
	}
	else
	if(value_desc.parent_is_canvas())
	{
		ValueNode::Handle src_value_node;
		src_value_node=ValueNode_Const::create((*value_desc.get_value_node())(time));

		Action::Handle action(ValueNodeReplace::create());

		action->set_param("canvas",get_canvas());
		action->set_param("canvas_interface",get_canvas_interface());
		action->set_param("src",src_value_node);
		action->set_param("dest",value_desc.get_value_node());

		assert(action->is_ready());
		if(!action->is_ready())
			throw Error(Error::TYPE_NOTREADY);

		add_action_front(action);
		return;
	}
	else
	if(value_desc.parent_is_waypoint())
	{
		Action::Handle action(WaypointDisconnect::create());

		action->set_param("canvas",get_canvas());
		action->set_param("canvas_interface",get_canvas_interface());
		action->set_param("parent_value_node",value_desc.get_parent_value_node());
		action->set_param("waypoint_time",value_desc.get_waypoint_time());
		action->set_param("time",time);

		assert(action->is_ready());
		if(!action->is_ready())
			throw Error(Error::TYPE_NOTREADY);

		add_action_front(action);
		return;
	}
	else
	if(value_desc.parent_is_linkable_value_node())
	{
		Action::Handle action(ValueNodeLinkDisconnect::create());

		action->set_param("canvas",get_canvas());
		action->set_param("canvas_interface",get_canvas_interface());
		action->set_param("parent_value_node",value_desc.get_parent_value_node());
		action->set_param("index",value_desc.get_index());
		action->set_param("time",time);

		assert(action->is_ready());
		if(!action->is_ready())
			throw Error(Error::TYPE_NOTREADY);

		add_action_front(action);
		return;
	}
	else
	if(value_desc.parent_is_layer())
	{
		Action::Handle action(LayerParamDisconnect::create());

		action->set_param("canvas",get_canvas());
		action->set_param("canvas_interface",get_canvas_interface());
		action->set_param("layer",value_desc.get_layer());
		action->set_param("param",value_desc.get_param_name());
		action->set_param("time",time);

		assert(action->is_ready());
		if(!action->is_ready())
			throw Error(Error::TYPE_NOTREADY);

		add_action_front(action);
		return;
	}

	throw Error(_("ValueDesc is not recognized or supported."));
}
