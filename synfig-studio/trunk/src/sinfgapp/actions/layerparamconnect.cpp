/* === S I N F G =========================================================== */
/*!	\file layerparamconnect.cpp
**	\brief Template File
**
**	$Id: layerparamconnect.cpp,v 1.2 2005/01/16 19:55:57 darco Exp $
**
**	\legal
**	Copyright (c) 2002 Robert B. Quattlebaum Jr.
**
**	This software and associated documentation
**	are CONFIDENTIAL and PROPRIETARY property of
**	the above-mentioned copyright holder.
**
**	You may not copy, print, publish, or in any
**	other way distribute this software without
**	a prior written agreement with
**	the copyright holder.
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

#include "layerparamconnect.h"
#include <sinfgapp/canvasinterface.h>

#endif

using namespace std;
using namespace etl;
using namespace sinfg;
using namespace sinfgapp;
using namespace Action;

/* === M A C R O S ========================================================= */

ACTION_INIT(Action::LayerParamConnect);
ACTION_SET_NAME(Action::LayerParamConnect,"layer_param_connect");
ACTION_SET_LOCAL_NAME(Action::LayerParamConnect,_("Connect Layer Parameter"));
ACTION_SET_TASK(Action::LayerParamConnect,"connect");
ACTION_SET_CATEGORY(Action::LayerParamConnect,Action::CATEGORY_LAYER|Action::CATEGORY_VALUENODE);
ACTION_SET_PRIORITY(Action::LayerParamConnect,0);
ACTION_SET_VERSION(Action::LayerParamConnect,"0.0");
ACTION_SET_CVS_ID(Action::LayerParamConnect,"$Id: layerparamconnect.cpp,v 1.2 2005/01/16 19:55:57 darco Exp $");

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
Action::LayerParamConnect::is_canidate(const ParamList &x)
{
	return canidate_check(get_param_vocab(),x);
}

bool
Action::LayerParamConnect::set_param(const sinfg::String& name, const Action::Param &param)
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
		sinfg::warning("Action::LayerParamConnect: Missing \"layer\"");
	if(!value_node)
		sinfg::warning("Action::LayerParamConnect: Missing \"value_node\"");
	if(param_name.empty())
		sinfg::warning("Action::LayerParamConnect: Missing \"param\"");
	
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
		throw Error(_("Layer did not recognise parameter name"));		

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
