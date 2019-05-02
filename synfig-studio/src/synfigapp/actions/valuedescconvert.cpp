/* === S Y N F I G ========================================================= */
/*!	\file valuedescconvert.cpp
**	\brief Template File
**
**	$Id$
**
**	\legal
**	Copyright (c) 2002-2005 Robert B. Quattlebaum Jr., Adrian Bentley
**	Copyright (c) 2007, 2008 Chris Moore
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

/*
#include "valuenodereplace.h"
#include "layerparamconnect.h"
#include "valuenodelinkconnect.h"
*/

#include <synfig/general.h>

#include "valuedescconnect.h"

#include "valuedescconvert.h"

#include <synfigapp/canvasinterface.h>
#include <synfig/valuenodes/valuenode_const.h>
#include <synfig/valuenode_registry.h>

#include <synfigapp/localization.h>

#endif

using namespace std;
using namespace etl;
using namespace synfig;
using namespace synfigapp;
using namespace Action;

/* === M A C R O S ========================================================= */

ACTION_INIT_NO_GET_LOCAL_NAME(Action::ValueDescConvert);
ACTION_SET_NAME(Action::ValueDescConvert,"ValueDescConvert");
ACTION_SET_LOCAL_NAME(Action::ValueDescConvert,N_("Convert"));
ACTION_SET_TASK(Action::ValueDescConvert,"convert");
ACTION_SET_CATEGORY(Action::ValueDescConvert,Action::CATEGORY_VALUEDESC);
ACTION_SET_PRIORITY(Action::ValueDescConvert,0);
ACTION_SET_VERSION(Action::ValueDescConvert,"0.0");
ACTION_SET_CVS_ID(Action::ValueDescConvert,"$Id$");

/* === G L O B A L S ======================================================= */

/* === P R O C E D U R E S ================================================= */

/* === M E T H O D S ======================================================= */

Action::ValueDescConvert::ValueDescConvert()
{
	time=(Time::begin()-1);
}

synfig::String
Action::ValueDescConvert::get_local_name()const
{
	// TRANSLATORS: This is used in the 'history' dialog when a ValueNode is converted.  The first %s is what is converted, the 2nd is the local name of the ValueNode's type.
	return strprintf(_("Convert '%s' to ValueNode type '%s'"),
					 value_desc.get_description().c_str(),
					 ValueNodeRegistry::book()[type].local_name.c_str());
}

Action::ParamVocab
Action::ValueDescConvert::get_param_vocab()
{
	ParamVocab ret(Action::CanvasSpecific::get_param_vocab());

	ret.push_back(ParamDesc("value_desc",Param::TYPE_VALUEDESC)
		.set_local_name(_("ValueDesc"))
	);

	ret.push_back(ParamDesc("type",Param::TYPE_STRING)
		.set_local_name(_("Type"))
		.set_desc(_("The type of ValueNode that you want to be converted to"))
	);

	ret.push_back(ParamDesc("time",Param::TYPE_TIME)
		.set_local_name(_("Time"))
	);

	return ret;
}

bool
Action::ValueDescConvert::is_candidate(const ParamList &x)
{
	if(candidate_check(get_param_vocab(),x))
	{
		ValueDesc value_desc=x.find("value_desc")->second.get_value_desc();
		if(!value_desc)
			return false;
		// Don't allow to export lower and upper boundaries of the WidthPoint
		if(value_desc.parent_is_linkable_value_node()
			&& value_desc.get_parent_value_node()->get_name()=="composite"
			&& value_desc.get_parent_value_node()->get_type()==type_width_point
			&& (value_desc.get_index()==4 || value_desc.get_index()==5))
		{
			synfig::info("it is not candidate!");
			return false;
		}
		synfig::info("it is candidate!");
		return true;
	}
	return false;
}

bool
Action::ValueDescConvert::set_param(const synfig::String& name, const Action::Param &param)
{
	if(name=="value_desc" && param.get_type()==Param::TYPE_VALUEDESC)
	{
		value_desc=param.get_value_desc();

		return true;
	}

	if(name=="type" && param.get_type()==Param::TYPE_STRING)
	{
		type=param.get_string();

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
Action::ValueDescConvert::is_ready()const
{
	if(!value_desc || type.empty())
		return false;
	if(time==(Time::begin()-1))
	{
		synfig::error("Missing time");
		return false;
	}
	return Action::CanvasSpecific::is_ready();
}

void
Action::ValueDescConvert::prepare()
{
	clear();

	ValueBase value;

	if(value_desc.is_const())
		value=value_desc.get_value();
	else if(value_desc.is_value_node())
		value=(*value_desc.get_value_node())(time);
	else
		throw Error(_("Unable to decipher ValueDesc (Bug?)"));
#ifdef _DEBUG
	printf("%s:%d Action::ValueDescConvert::prepare() with value %s type %s\n", __FILE__, __LINE__, value.get_string().c_str(), type.c_str());
//	printf("%s:%d canvas = %lx\n", __FILE__, __LINE__, uintptr_t(value_desc.get_value_node()->get_parent_canvas().get()));
//	printf("%s:%d parent canvas = %lx\n", __FILE__, __LINE__, uintptr_t(value_desc.get_parent_value_node()->get_parent_canvas().get()));
#endif	// _DEBUG
	ValueNode::Handle src_value_node(ValueNodeRegistry::create(type,value));

	if(!src_value_node)
		throw Error(_("Unable to create new value node"));
#ifdef _DEBUG
//	printf("%s:%d src_value_node canvas was %lx\n", __FILE__, __LINE__, uintptr_t(src_value_node->get_parent_canvas().get()));
//	if (!src_value_node->get_parent_canvas())
//		src_value_node->set_parent_canvas();
	printf("%s:%d CONVERT %s: desc canvas = %lx\n", __FILE__, __LINE__, type.c_str(), uintptr_t(value_desc.get_canvas().get()));
	printf("%s:%d CONVERT %s: src_value_node canvas is %lx\n", __FILE__, __LINE__, type.c_str(), uintptr_t(src_value_node->get_parent_canvas().get()));
#endif	// _DEBUG
	ValueNode::Handle dest_value_node;
	dest_value_node=value_desc.get_value_node();
	// printf("%s:%d dest_value_node canvas = %lx\n", __FILE__, __LINE__, uintptr_t(dest_value_node->get_parent_canvas().get()));

	Action::Handle action(ValueDescConnect::create());

	action->set_param("canvas",get_canvas());
	action->set_param("canvas_interface",get_canvas_interface());
	action->set_param("src",src_value_node);
	action->set_param("dest",value_desc);

	assert(action->is_ready());
	if(!action->is_ready())
		throw Error(Error::TYPE_NOTREADY);

	add_action_front(action);

/*
	return;


	if(value_desc.parent_is_canvas())
	{
		ValueNode::Handle dest_value_node;
		dest_value_node=value_desc.get_value_node();

		Action::Handle action(ValueNodeReplace::create());

		action->set_param("canvas",get_canvas());
		action->set_param("canvas_interface",get_canvas_interface());
		action->set_param("src",src_value_node);
		action->set_param("dest",dest_value_node);

		assert(action->is_ready());
		if(!action->is_ready())
			throw Error(Error::TYPE_NOTREADY);

		add_action_front(action);
		return;
	}
	else
	if(value_desc.parent_is_linkable_value_node())
	{
		Action::Handle action(ValueNodeLinkConnect::create());

		action->set_param("canvas",get_canvas());
		action->set_param("canvas_interface",get_canvas_interface());
		action->set_param("parent_value_node",value_desc.get_parent_value_node());
		action->set_param("value_node", src_value_node);
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
		action->set_param("value_node",src_value_node);

		assert(action->is_ready());
		if(!action->is_ready())
			throw Error(Error::TYPE_NOTREADY);

		add_action_front(action);
		return;
	}



	throw Error(_("ValueDesc is not recognized or supported."));
*/
}
