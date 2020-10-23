/* === S Y N F I G ========================================================= */
/*!	\file layerparamset.cpp
**	\brief Template File
**
**	$Id$
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

#include "layerparamset.h"
#include <synfigapp/canvasinterface.h>

#include <synfigapp/localization.h>

#endif

using namespace etl;
using namespace synfig;
using namespace synfigapp;
using namespace Action;

/* === M A C R O S ========================================================= */

ACTION_INIT(Action::LayerParamSet);
ACTION_SET_NAME(Action::LayerParamSet,"LayerParamSet");
ACTION_SET_LOCAL_NAME(Action::LayerParamSet,N_("Set Layer Parameter"));
ACTION_SET_TASK(Action::LayerParamSet,"set");
ACTION_SET_CATEGORY(Action::LayerParamSet,Action::CATEGORY_LAYER);
ACTION_SET_PRIORITY(Action::LayerParamSet,0);
ACTION_SET_VERSION(Action::LayerParamSet,"0.0");

/* === G L O B A L S ======================================================= */

/* === P R O C E D U R E S ================================================= */

/* === M E T H O D S ======================================================= */

Action::LayerParamSet::LayerParamSet()
{
}

Action::ParamVocab
Action::LayerParamSet::get_param_vocab()
{
	ParamVocab ret(Action::CanvasSpecific::get_param_vocab());

	ret.push_back(ParamDesc("layer",Param::TYPE_LAYER)
		.set_local_name(_("Layer"))
	);

	ret.push_back(ParamDesc("param",Param::TYPE_STRING)
		.set_local_name(_("Param"))
	);

	ret.push_back(ParamDesc("new_value",Param::TYPE_VALUE)
		.set_local_name(_("ValueBase"))
	);

	return ret;
}

bool
Action::LayerParamSet::is_candidate(const ParamList &x)
{
	return candidate_check(get_param_vocab(),x);
}

bool
Action::LayerParamSet::set_param(const synfig::String& name, const Action::Param &param)
{
	if(name=="layer" && param.get_type()==Param::TYPE_LAYER)
	{
		layer=param.get_layer();

		return true;
	}

	if(name=="new_value" && param.get_type()==Param::TYPE_VALUE)
	{
		new_value=param.get_value();

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
Action::LayerParamSet::is_ready()const
{
	if(!layer || !new_value.is_valid() || param_name.empty())
		return false;
	return Action::CanvasSpecific::is_ready();
}

void
Action::LayerParamSet::perform()
{
	// See if the parameter is dynamic
	if(layer->dynamic_param_list().count(param_name))
		throw Error(_("ValueNode attached to Parameter."));

	old_value=layer->get_param(param_name);

	// We shouldn't change the parameters properties when change its value
	new_value.copy_properties_of(old_value);

	if(!layer->set_param(param_name,new_value))
		throw Error(_("Layer did not accept parameter."));

	/*if(layer->active())
		set_dirty(true);
	else
		set_dirty(false);
	*/
	layer->changed();

	// Signal that a layer has been inserted
	if(get_canvas_interface())
	{
		get_canvas_interface()->signal_layer_param_changed()(layer,param_name);
	}
}

void
Action::LayerParamSet::undo()
{
	if(!layer->set_param(param_name,old_value))
		throw Error(_("Layer did not accept parameter."));

	/*
	if(layer->active())
		set_dirty(true);
	else
		set_dirty(false);
	*/

	layer->changed();

	// Signal that a layer has been inserted
	if(get_canvas_interface())
	{
		get_canvas_interface()->signal_layer_param_changed()(layer,param_name);
	}
}
