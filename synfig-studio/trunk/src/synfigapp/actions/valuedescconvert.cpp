/* === S Y N F I G ========================================================= */
/*!	\file valuedescconvert.cpp
**	\brief Template File
**
**	$Id: valuedescconvert.cpp,v 1.1.1.1 2005/01/07 03:34:37 darco Exp $
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

/*
#include "valuenodereplace.h"
#include "layerparamconnect.h"
#include "valuenodelinkconnect.h"
*/

#include "valuedescconnect.h"

#include "valuedescconvert.h"

#include <synfigapp/canvasinterface.h>
#include <synfig/valuenode_const.h>

#endif

using namespace std;
using namespace etl;
using namespace synfig;
using namespace synfigapp;
using namespace Action;

/* === M A C R O S ========================================================= */

ACTION_INIT(Action::ValueDescConvert);
ACTION_SET_NAME(Action::ValueDescConvert,"value_desc_convert");
ACTION_SET_LOCAL_NAME(Action::ValueDescConvert,"Convert");
ACTION_SET_TASK(Action::ValueDescConvert,"convert");
ACTION_SET_CATEGORY(Action::ValueDescConvert,Action::CATEGORY_VALUEDESC);
ACTION_SET_PRIORITY(Action::ValueDescConvert,0);
ACTION_SET_VERSION(Action::ValueDescConvert,"0.0");
ACTION_SET_CVS_ID(Action::ValueDescConvert,"$Id: valuedescconvert.cpp,v 1.1.1.1 2005/01/07 03:34:37 darco Exp $");

/* === G L O B A L S ======================================================= */

/* === P R O C E D U R E S ================================================= */

/* === M E T H O D S ======================================================= */

Action::ValueDescConvert::ValueDescConvert()
{
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
	
	return ret;
}

bool
Action::ValueDescConvert::is_candidate(const ParamList &x)
{
	return candidate_check(get_param_vocab(),x);
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

	return Action::CanvasSpecific::set_param(name,param);
}

bool
Action::ValueDescConvert::is_ready()const
{
	if(!value_desc || type.empty())
		return false;
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
		value=(*value_desc.get_value_node())(0);
	else
		throw Error(_("Unable to decipher ValueDesc (Bug?)"));
	
	ValueNode::Handle src_value_node(LinkableValueNode::create(type,value));

	if(!src_value_node)
		throw Error(_("Unable to create new value node"));

	
	ValueNode::Handle dest_value_node;
	dest_value_node=value_desc.get_value_node();

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
	if(value_desc.parent_is_layer_param())
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
	
	
	
	throw Error(_("ValueDesc is not recognised or supported."));	
*/
}
