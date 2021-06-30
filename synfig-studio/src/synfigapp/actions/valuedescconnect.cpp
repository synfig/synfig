/* === S Y N F I G ========================================================= */
/*!	\file valuedescconnect.cpp
**	\brief Template File
**
**	$Id$
**
**	\legal
**	Copyright (c) 2002-2005 Robert B. Quattlebaum Jr., Adrian Bentley
**	Copyright (c) 2008 Chris Moore
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

#include "layerparamconnect.h"
#include "waypointconnect.h"
#include "valuenodelinkconnect.h"
#include "valuenodereplace.h"

#include "valuedescconnect.h"
#include <synfigapp/canvasinterface.h>

#include <synfigapp/localization.h>

#endif

using namespace etl;
using namespace synfig;
using namespace synfigapp;
using namespace Action;

/* === M A C R O S ========================================================= */

ACTION_INIT_NO_GET_LOCAL_NAME(Action::ValueDescConnect);
ACTION_SET_NAME(Action::ValueDescConnect,"ValueDescConnect");
ACTION_SET_LOCAL_NAME(Action::ValueDescConnect,N_("Connect"));
ACTION_SET_TASK(Action::ValueDescConnect,"connect");
ACTION_SET_CATEGORY(Action::ValueDescConnect,Action::CATEGORY_VALUEDESC|Action::CATEGORY_VALUENODE);
ACTION_SET_PRIORITY(Action::ValueDescConnect,0);
ACTION_SET_VERSION(Action::ValueDescConnect,"0.0");

/* === G L O B A L S ======================================================= */

/* === P R O C E D U R E S ================================================= */

/* === M E T H O D S ======================================================= */

Action::ValueDescConnect::ValueDescConnect()
{
}

synfig::String
Action::ValueDescConnect::get_local_name()const
{
	// TRANSLATORS: This is used in the 'history' dialog when a connection is made.
	return strprintf(_("Connect '%s' to '%s'"),
					 value_desc.get_description(false).c_str(),
					 value_node->get_id().c_str());
}

Action::ParamVocab
Action::ValueDescConnect::get_param_vocab()
{
	ParamVocab ret(Action::CanvasSpecific::get_param_vocab());

	ret.push_back(ParamDesc("dest",Param::TYPE_VALUEDESC)
		.set_local_name(_("Destination ValueDesc"))
	);

	ret.push_back(ParamDesc("src",Param::TYPE_VALUENODE)
		.set_local_name(_("Source ValueNode"))
		.set_mutual_exclusion("src_name")
	);

	ret.push_back(ParamDesc("src_name",Param::TYPE_STRING)
		.set_local_name(_("Source ValueNode Name"))
		.set_mutual_exclusion("src")
		.set_user_supplied()
	);

	return ret;
}

bool
Action::ValueDescConnect::is_candidate(const ParamList &x)
{
	if(candidate_check(get_param_vocab(),x))
	{
	    ValueDesc value_desc(x.find("dest")->second.get_value_desc());
	    ValueNode::Handle value_node(x.find("src")->second.get_value_node());

	    //! forbid recursive linking (fix #48)
	    if (value_desc.parent_is_value_node())
	    {
	        ValueNode* vn = dynamic_cast<ValueNode*>(value_node.get());
	        if (vn && vn->is_descendant(value_desc.get_parent_value_node()))
	            return false;
	    }


		// don't show the option of connecting to an existing Index parameter of the Duplicate layer
		if(x.count("dest"))
		{
			if (value_desc.parent_is_layer() &&
				value_desc.get_layer()->get_name() == "duplicate" &&
				value_desc.get_param_name() == "index")
				return false;
		}

		if(x.count("src"))
		{
			if(value_desc.get_value_type()==value_node->get_type())
				return true;
		}
		return true;
	}
	return false;
}

bool
Action::ValueDescConnect::set_param(const synfig::String& name, const Action::Param &param)
{
	if(name=="dest" && param.get_type()==Param::TYPE_VALUEDESC)
	{
		value_desc=param.get_value_desc();

		return true;
	}

	if(name=="src" && param.get_type()==Param::TYPE_VALUENODE)
	{
		value_node=param.get_value_node();

		return true;
	}

	if(!value_node_name.empty() && !value_node && name=="canvas" && param.get_type()==Param::TYPE_CANVAS)
	{
		value_node=param.get_canvas()->find_value_node(value_node_name, false);
	}

	if(name=="src_name" && param.get_type()==Param::TYPE_STRING)
	{
		value_node_name=param.get_string();

		if(get_canvas())
		{
			value_node=get_canvas()->find_value_node(value_node_name, false);
			if(!value_node)
				return false;
		}

		return true;
	}

	return Action::CanvasSpecific::set_param(name,param);
}

bool
Action::ValueDescConnect::is_ready()const
{
	if(!value_desc || !value_node)
		return false;
	return Action::CanvasSpecific::is_ready();
}

void
Action::ValueDescConnect::prepare()
{
	clear();

	if(value_desc.parent_is_canvas())
	{
		ValueNode::Handle dest_value_node;
		dest_value_node=value_desc.get_value_node();

		Action::Handle action(ValueNodeReplace::create());

		action->set_param("canvas",get_canvas());
		action->set_param("canvas_interface",get_canvas_interface());
		action->set_param("src",value_node);
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
		Action::Handle action(WaypointConnect::create());

		action->set_param("canvas",get_canvas());
		action->set_param("canvas_interface",get_canvas_interface());
		action->set_param("parent_value_node",value_desc.get_parent_value_node());
		action->set_param("value_node", value_node);
		action->set_param("waypoint_time",value_desc.get_waypoint_time());

		assert(action->is_ready());
		if(!action->is_ready())
			throw Error(Error::TYPE_NOTREADY);

		add_action_front(action);
		return;
	}
	if(value_desc.parent_is_linkable_value_node())
	{
		Action::Handle action(ValueNodeLinkConnect::create());

		action->set_param("canvas",get_canvas());
		action->set_param("canvas_interface",get_canvas_interface());
		action->set_param("parent_value_node",value_desc.get_parent_value_node());
		action->set_param("value_node", value_node);
		action->set_param("index",value_desc.get_index());

		assert(action->is_ready());
		if(!action->is_ready())
			throw Error(Error::TYPE_NOTREADY);

		add_action_front(action);
		return;
	}
	else
	if(value_desc.parent_is_layer())
	{
		Action::Handle action(LayerParamConnect::create());

		action->set_param("canvas",get_canvas());
		action->set_param("canvas_interface",get_canvas_interface());
		action->set_param("layer",value_desc.get_layer());
		action->set_param("param",value_desc.get_param_name());
		action->set_param("value_node",value_node);

		assert(action->is_ready());
		if(!action->is_ready())
			throw Error(Error::TYPE_NOTREADY);

		add_action_front(action);
		return;
	}

	throw Error(_("ValueDesc is not recognized or supported."));
}
