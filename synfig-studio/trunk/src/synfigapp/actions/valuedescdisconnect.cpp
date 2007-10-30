/* === S Y N F I G ========================================================= */
/*!	\file valuedescdisconnect.cpp
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

#include "layerparamdisconnect.h"
#include "valuenodelinkdisconnect.h"
#include "valuenodereplace.h"

#include "valuedescdisconnect.h"
#include <synfigapp/canvasinterface.h>
#include <synfig/valuenode_const.h>

#endif

using namespace std;
using namespace etl;
using namespace synfig;
using namespace synfigapp;
using namespace Action;

/* === M A C R O S ========================================================= */

ACTION_INIT(Action::ValueDescDisconnect);
ACTION_SET_NAME(Action::ValueDescDisconnect,"value_desc_disconnect");
ACTION_SET_LOCAL_NAME(Action::ValueDescDisconnect,"Disconnect");
ACTION_SET_TASK(Action::ValueDescDisconnect,"disconnect");
ACTION_SET_CATEGORY(Action::ValueDescDisconnect,Action::CATEGORY_VALUEDESC);
ACTION_SET_PRIORITY(Action::ValueDescDisconnect,-100);
ACTION_SET_VERSION(Action::ValueDescDisconnect,"0.0");
ACTION_SET_CVS_ID(Action::ValueDescDisconnect,"$Id$");

/* === G L O B A L S ======================================================= */

/* === P R O C E D U R E S ================================================= */

/* === M E T H O D S ======================================================= */

Action::ValueDescDisconnect::ValueDescDisconnect():
	time(0)
{
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
	if(candidate_check(get_param_vocab(),x))
	{
		ValueDesc value_desc(x.find("value_desc")->second.get_value_desc());
		if(!value_desc.parent_is_canvas() && value_desc.is_value_node() && value_desc.get_value_node()->rcount()>1)
			return true;
		if(value_desc.is_const())
			return false;
		if(value_desc.is_value_node() && ValueNode_Const::Handle::cast_dynamic(value_desc.get_value_node()))
			return false;
		return true;
	}
	return false;
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
	if(value_desc.parent_is_layer_param())
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
