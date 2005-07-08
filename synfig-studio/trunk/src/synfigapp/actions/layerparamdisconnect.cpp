/* === S Y N F I G ========================================================= */
/*!	\file layerparamdisconnect.cpp
**	\brief Template File
**
**	$Id: layerparamdisconnect.cpp,v 1.1.1.1 2005/01/07 03:34:37 darco Exp $
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

#include "layerparamdisconnect.h"
#include <synfigapp/canvasinterface.h>
#include <synfig/valuenode_dynamiclist.h>

#endif

using namespace std;
using namespace etl;
using namespace synfig;
using namespace synfigapp;
using namespace Action;

/* === M A C R O S ========================================================= */

ACTION_INIT(Action::LayerParamDisconnect);
ACTION_SET_NAME(Action::LayerParamDisconnect,"layer_param_disconnect");
ACTION_SET_LOCAL_NAME(Action::LayerParamDisconnect,_("Disconnect Layer Parameter"));
ACTION_SET_TASK(Action::LayerParamDisconnect,"disconnect");
ACTION_SET_CATEGORY(Action::LayerParamDisconnect,Action::CATEGORY_LAYER|Action::CATEGORY_VALUENODE);
ACTION_SET_PRIORITY(Action::LayerParamDisconnect,0);
ACTION_SET_VERSION(Action::LayerParamDisconnect,"0.0");
ACTION_SET_CVS_ID(Action::LayerParamDisconnect,"$Id: layerparamdisconnect.cpp,v 1.1.1.1 2005/01/07 03:34:37 darco Exp $");

/* === G L O B A L S ======================================================= */

/* === P R O C E D U R E S ================================================= */

/* === M E T H O D S ======================================================= */

Action::LayerParamDisconnect::LayerParamDisconnect():
	time(0)
{
	
}

Action::ParamVocab
Action::LayerParamDisconnect::get_param_vocab()
{
	ParamVocab ret(Action::CanvasSpecific::get_param_vocab());
	
	ret.push_back(ParamDesc("layer",Param::TYPE_LAYER)
		.set_local_name(_("Layer"))
	);

	ret.push_back(ParamDesc("param",Param::TYPE_STRING)
		.set_local_name(_("Param"))
	);

	ret.push_back(ParamDesc("time",Param::TYPE_STRING)
		.set_local_name(_("Time"))
		.set_optional()
	);
	
	return ret;
}

bool
Action::LayerParamDisconnect::is_canidate(const ParamList &x)
{
	return canidate_check(get_param_vocab(),x);
}

bool
Action::LayerParamDisconnect::set_param(const synfig::String& name, const Action::Param &param)
{
	if(name=="layer" && param.get_type()==Param::TYPE_LAYER)
	{
		layer=param.get_layer();
		
		return true;
	}

	if(name=="param" && param.get_type()==Param::TYPE_STRING)
	{
		param_name=param.get_string();
		
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
Action::LayerParamDisconnect::is_ready()const
{
	if(!layer || param_name.empty())
		return false;
	return Action::CanvasSpecific::is_ready();
}

void
Action::LayerParamDisconnect::perform()
{
	if(!layer->dynamic_param_list().count(param_name))
		throw Error(_("Layer Parameter is not connected to anything"));

	old_value_node=layer->dynamic_param_list().find(param_name)->second;
	layer->disconnect_dynamic_param(param_name);
	
	if(new_value_node || ValueNode_DynamicList::Handle::cast_dynamic(old_value_node))
	{
		if(!new_value_node)
			new_value_node=old_value_node.clone();
		layer->connect_dynamic_param(param_name,new_value_node);
	}
	else
		layer->set_param(param_name,(*old_value_node)(time));
	
	layer->changed();
	old_value_node->changed();

	set_dirty(false);
	
	if(get_canvas_interface())
	{
		get_canvas_interface()->signal_layer_param_changed()(layer,param_name);
	}
}

void
Action::LayerParamDisconnect::undo()
{
	layer->connect_dynamic_param(param_name,old_value_node);
	
/*	if(layer->active() && get_canvas()->get_time()!=time)
		set_dirty(true);
	else
		set_dirty(false);
*/
	layer->changed();
	old_value_node->changed();

	set_dirty(false);

	if(get_canvas_interface())
	{
		get_canvas_interface()->signal_layer_param_changed()(layer,param_name);
	}
}
