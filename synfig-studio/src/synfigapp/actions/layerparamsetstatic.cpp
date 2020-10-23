/* === S Y N F I G ========================================================= */
/*!	\file layerparamsetstatic.cpp
**	\brief Template File
**
**	$Id$
**
**	\legal
**	Copyright (c) 2002-2005 Robert B. Quattlebaum Jr., Adrian Bentley
**	Copyright (c) 2010 Carlos López
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

#include "layerparamsetstatic.h"
#include <synfigapp/canvasinterface.h>

#include <synfigapp/localization.h>

#endif

using namespace etl;
using namespace synfig;
using namespace synfigapp;
using namespace Action;

/* === M A C R O S ========================================================= */

ACTION_INIT(Action::LayerParamSetStatic);
ACTION_SET_NAME(Action::LayerParamSetStatic,"LayerParamSetStatic");
ACTION_SET_LOCAL_NAME(Action::LayerParamSetStatic,N_("Forbid Animation"));
ACTION_SET_TASK(Action::LayerParamSetStatic,"setstatic");
ACTION_SET_CATEGORY(Action::LayerParamSetStatic,Action::CATEGORY_VALUEDESC);
ACTION_SET_PRIORITY(Action::LayerParamSetStatic,0);
ACTION_SET_VERSION(Action::LayerParamSetStatic,"0.0");

/* === G L O B A L S ======================================================= */

/* === P R O C E D U R E S ================================================= */

/* === M E T H O D S ======================================================= */

Action::LayerParamSetStatic::LayerParamSetStatic():
	old_static_value()
{ }

Action::ParamVocab
Action::LayerParamSetStatic::get_param_vocab()
{
	ParamVocab ret(Action::CanvasSpecific::get_param_vocab());

	ret.push_back(ParamDesc("value_desc",Param::TYPE_VALUEDESC)
		.set_local_name(_("Value Desc"))
	);

	return ret;
}

bool
Action::LayerParamSetStatic::is_candidate(const ParamList &x)
{
	if(!candidate_check(get_param_vocab(),x))
		return false;

	ValueDesc value_desc(x.find("value_desc")->second.get_value_desc());

	if(!value_desc.parent_is_layer())
		return false;

	synfig::ValueBase parameter;
	synfig::Layer::Handle _layer;
	synfig::String _param_name;
	_layer = value_desc.get_layer();
	_param_name = value_desc.get_param_name();

	if(!_layer || _param_name.empty())
		return false;

	//!Check that the parameter is not Value Node (Const, Animated or Linkable)
	if(_layer->dynamic_param_list().count(_param_name))
		return false;
	//! Retrieves the current parameter
	parameter = _layer->get_param(_param_name);
	//! Check that the parameter is not a inline canvas
	if(parameter.get_type()==type_canvas && parameter.get(Canvas::Handle()))
		if(parameter.get(Canvas::Handle())->is_inline())
			return false;
	//! Check if it is already static
	if(parameter.get_static())
		return false;

	return true;

}

bool
Action::LayerParamSetStatic::set_param(const synfig::String& name, const Action::Param &param)
{

	if(!layer && name=="value_desc" && param.get_type()==Param::TYPE_VALUEDESC)
	{
		ValueDesc value_desc(param.get_value_desc());
		if(!value_desc.parent_is_layer())
			return false;

		layer=Layer::Handle::cast_dynamic(value_desc.get_layer());

		if(!layer)
			return false;
	}


	if(param_name.empty() && name=="value_desc" && param.get_type()==Param::TYPE_VALUEDESC)
	{
		ValueDesc value_desc(param.get_value_desc());
		if(!value_desc.parent_is_layer())
			return false;

		param_name=value_desc.get_param_name();

		if(param_name.empty())
			return false;
	}

	return Action::CanvasSpecific::set_param(name,param);
}

bool
Action::LayerParamSetStatic::is_ready()const
{
	if(!layer || param_name.empty())
		return false;

	return Action::CanvasSpecific::is_ready();
}

void
Action::LayerParamSetStatic::perform()
{
	//! See if the parameter is dynamic
	if(layer->dynamic_param_list().count(param_name))
		throw Error(_("This action is not for Value Nodes!"));

	old_static_value=false;

	ValueBase v=layer->get_param(param_name);
	v.set_static(true);
	if(!layer->set_param(param_name,v))
		throw Error(_("Layer did not accept static value."));

	//! Signal layer changed
	layer->changed();
	//! Signal that a layer parameter changed
	if(get_canvas_interface())
		get_canvas_interface()->signal_layer_param_changed()(layer,param_name);
}

void
Action::LayerParamSetStatic::undo()
{
	ValueBase v=layer->get_param(param_name);
	v.set_static(old_static_value);
	if(!layer->set_param(param_name,v))
	   throw Error(_("Layer did not accept static value."));

	//! Signal layer changed
	layer->changed();
	//! Signal that a layer parameter changed
	if(get_canvas_interface())
		get_canvas_interface()->signal_layer_param_changed()(layer,param_name);
}
