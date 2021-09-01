/* === S Y N F I G ========================================================= */
/*!	\file layerparamconnect.cpp
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

#include "layerparamconnect.h"
#include <synfigapp/canvasinterface.h>

#include <synfigapp/localization.h>

#endif

using namespace synfig;
using namespace synfigapp;
using namespace Action;

/* === M A C R O S ========================================================= */

ACTION_INIT(Action::LayerParamConnect);
ACTION_SET_NAME(Action::LayerParamConnect,"LayerParamConnect");
ACTION_SET_LOCAL_NAME(Action::LayerParamConnect,N_("Connect Layer Parameter"));
ACTION_SET_TASK(Action::LayerParamConnect,"connect");
ACTION_SET_CATEGORY(Action::LayerParamConnect,Action::CATEGORY_LAYER|Action::CATEGORY_VALUENODE);
ACTION_SET_PRIORITY(Action::LayerParamConnect,0);
ACTION_SET_VERSION(Action::LayerParamConnect,"0.0");

/* === G L O B A L S ======================================================= */

/* === P R O C E D U R E S ================================================= */

/* === M E T H O D S ======================================================= */

Action::LayerParamConnect::LayerParamConnect()
{
}

Action::ParamVocab
Action::LayerParamConnect::get_param_vocab()
{
	ParamVocab ret(Action::CanvasSpecific::get_param_vocab());

	ret.push_back(ParamDesc("layer",Param::TYPE_LAYER)
		.set_local_name(_("Layer"))
	);

	ret.push_back(ParamDesc("param",Param::TYPE_STRING)
		.set_local_name(_("Param"))
	);

	ret.push_back(ParamDesc("value_node",Param::TYPE_VALUENODE)
		.set_local_name(_("ValueNode"))
	);

	return ret;
}

bool
Action::LayerParamConnect::is_candidate(const ParamList &x)
{
	return candidate_check(get_param_vocab(),x);
}

bool
Action::LayerParamConnect::set_param(const synfig::String& name, const Action::Param &param)
{
	if(name=="layer" && param.get_type()==Param::TYPE_LAYER)
	{
		layer=param.get_layer();

		return true;
	}

	if(name=="value_node" && param.get_type()==Param::TYPE_VALUENODE)
	{
		value_node=param.get_value_node();

		return true;
	}

	if(name=="param" && param.get_type()==Param::TYPE_STRING)
	{
		param_name=param.get_string();

		return true;
	}

	return Action::CanvasSpecific::set_param(name,param);
}

bool
Action::LayerParamConnect::is_ready()const
{
	if(!layer)
		synfig::warning("Action::LayerParamConnect: Missing \"layer\"");
	if(!value_node)
		synfig::warning("Action::LayerParamConnect: Missing \"value_node\"");
	if(param_name.empty())
		synfig::warning("Action::LayerParamConnect: Missing \"param\"");

	if(!layer || !value_node || param_name.empty())
		return false;
	return Action::CanvasSpecific::is_ready();
}

void
Action::LayerParamConnect::perform()
{
	// See if the parameter is dynamic
	if(layer->dynamic_param_list().count(param_name))
		old_value_node=layer->dynamic_param_list().find(param_name)->second;
	else
	{
		old_value_node=0;
	}

	old_value=layer->get_param(param_name);
	if(!old_value.is_valid())
		throw Error(_("Layer did not recognize parameter name"));

	if(!layer->set_param(param_name,(*value_node)(0)))
		throw Error(_("Bad connection"));

	layer->connect_dynamic_param(param_name,value_node);

	layer->changed();
	value_node->changed();
/*	if(get_canvas_interface())
	{
		get_canvas_interface()->signal_layer_param_changed()(layer,param_name);
		//get_canvas_interface()->signal_value_node_changed()(value_node);
	}
*/
}

void
Action::LayerParamConnect::undo()
{
	if(old_value_node)
		layer->connect_dynamic_param(param_name,old_value_node);
	else
	{
		layer->disconnect_dynamic_param(param_name);
		layer->set_param(param_name,old_value);
	}

	layer->changed();
	if(old_value_node)
		old_value_node->changed();
	/*
	if(layer->active())
		set_dirty(true);
	else
		set_dirty(false);
	*/

	if(get_canvas_interface())
	{
		get_canvas_interface()->signal_layer_param_changed()(layer,param_name);
		//if(old_value_node)get_canvas_interface()->signal_value_node_changed()(old_value_node);
	}
}
