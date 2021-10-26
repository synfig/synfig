/* === S Y N F I G ========================================================= */
/*!	\file layerparamdisconnect.cpp
**	\brief Template File
**
**	\legal
**	Copyright (c) 2002-2005 Robert B. Quattlebaum Jr., Adrian Bentley
**
**	This file is part of Synfig.
**
**	Synfig is free software: you can redistribute it and/or modify
**	it under the terms of the GNU General Public License as published by
**	the Free Software Foundation, either version 2 of the License, or
**	(at your option) any later version.
**
**	Synfig is distributed in the hope that it will be useful,
**	but WITHOUT ANY WARRANTY; without even the implied warranty of
**	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
**	GNU General Public License for more details.
**
**	You should have received a copy of the GNU General Public License
**	along with Synfig.  If not, see <https://www.gnu.org/licenses/>.
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
#include <synfigapp/canvasinterface.h>
#include <synfig/valuenodes/valuenode_dynamiclist.h>

#include <synfigapp/localization.h>

#endif

using namespace synfig;
using namespace synfigapp;
using namespace Action;

/* === M A C R O S ========================================================= */

ACTION_INIT(Action::LayerParamDisconnect);
ACTION_SET_NAME(Action::LayerParamDisconnect,"LayerParamDisconnect");
ACTION_SET_LOCAL_NAME(Action::LayerParamDisconnect,N_("Disconnect Layer Parameter"));
ACTION_SET_TASK(Action::LayerParamDisconnect,"disconnect");
ACTION_SET_CATEGORY(Action::LayerParamDisconnect,Action::CATEGORY_LAYER|Action::CATEGORY_VALUENODE);
ACTION_SET_PRIORITY(Action::LayerParamDisconnect,0);
ACTION_SET_VERSION(Action::LayerParamDisconnect,"0.0");

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
Action::LayerParamDisconnect::is_candidate(const ParamList &x)
{
	return candidate_check(get_param_vocab(),x);
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
			new_value_node=old_value_node->clone(get_canvas());
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
