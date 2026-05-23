/* === S Y N F I G ========================================================= */
/*!	\file layerparamreset.cpp
**	\brief Reset the layer parameter value to its default
**
**	\legal
**	Copyright (c) 2002-2005 Robert B. Quattlebaum Jr., Adrian Bentley
**                2025      Synfig Contributors
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

#include "layerparamreset.h"

#include <synfig/general.h>

#include <synfigapp/canvasinterface.h>
#include <synfigapp/localization.h>

#endif

using namespace synfig;
using namespace synfigapp;
using namespace Action;

/* === M A C R O S ========================================================= */

ACTION_INIT(Action::LayerParamReset);
ACTION_SET_NAME(Action::LayerParamReset,"LayerParamReset");
ACTION_SET_LOCAL_NAME(Action::LayerParamReset,N_("Reset Layer Parameter"));
ACTION_SET_TASK(Action::LayerParamReset,"set");
ACTION_SET_CATEGORY(Action::LayerParamReset,Action::CATEGORY_LAYER|Action::CATEGORY_VALUENODE|Action::CATEGORY_VALUEDESC);
ACTION_SET_PRIORITY(Action::LayerParamReset,0);
ACTION_SET_VERSION(Action::LayerParamReset,"0.0");

/* === G L O B A L S ======================================================= */

/* === P R O C E D U R E S ================================================= */

/* === M E T H O D S ======================================================= */

Action::LayerParamReset::LayerParamReset()
{
}

Action::ParamVocab
Action::LayerParamReset::get_param_vocab()
{
	ParamVocab ret(Action::CanvasSpecific::get_param_vocab());

	ret.push_back(ParamDesc("layer", Param::TYPE_LAYER)
		.set_local_name(_("Layer"))
		.set_optional()
	);

	ret.push_back(ParamDesc("param", Param::TYPE_STRING)
		.set_local_name(_("Param"))
		.set_optional()
	);

	ret.push_back(ParamDesc("value_desc", Param::TYPE_VALUEDESC)
		.set_local_name(_("ValueDesc"))
		.set_optional()
	);

	return ret;
}

bool
Action::LayerParamReset::is_candidate(const ParamList& x)
{
	return candidate_check(get_param_vocab(), x);
}

bool
Action::LayerParamReset::set_param(const synfig::String& name, const Action::Param& param)
{
	if (name == "layer" && param.get_type() == Param::TYPE_LAYER) {
		layer = param.get_layer();

		return true;
	}

	if (name == "param" && param.get_type() == Param::TYPE_STRING) {
		param_name = param.get_string();

		return true;
	}

	if (name == "value_desc" && param.get_type() == Param::TYPE_VALUEDESC)
	{
		// Grab the value_desc
		ValueDesc value_desc = param.get_value_desc();
		if (!value_desc.parent_is_layer())
			return false;

		layer = value_desc.get_layer();
		param_name = value_desc.get_param_name();

		return true;
	}
	return Action::CanvasSpecific::set_param(name,param);
}

bool
Action::LayerParamReset::is_ready() const
{
	if (!layer)
		synfig::warning("Action::LayerParamConnect: Missing \"layer\"");
	if (param_name.empty())
		synfig::warning("Action::LayerParamConnect: Missing \"param\"");

	if (!layer || param_name.empty())
		return false;
	return Action::CanvasSpecific::is_ready();
}

void
Action::LayerParamReset::perform()
{
	if (default_value.get_type() == type_nil) {
		auto default_layer = Layer::create(layer->get_name());
		default_value = default_layer->get_param(param_name);
	}

	// See if the parameter is dynamic
	auto it = layer->dynamic_param_list().find(param_name);
	if (it != layer->dynamic_param_list().end()) {
		old_value_node = it->second;
		layer->disconnect_dynamic_param(param_name);
	} else {
		old_value_node = nullptr;
	}

	old_value = layer->get_param(param_name);
	if (!old_value.is_valid())
		throw Error(_("Layer did not recognize parameter name"));

	//new_value.copy_properties_of(old_value);

	if (!layer->set_param(param_name, default_value))
		throw Error(_("Bad connection"));

	layer->changed();
	if (get_canvas_interface())
		get_canvas_interface()->signal_layer_param_changed()(layer, param_name);
}

void
Action::LayerParamReset::undo()
{
	if (old_value_node) {
		layer->connect_dynamic_param(param_name, old_value_node);
	} else {
		layer->disconnect_dynamic_param(param_name);
		layer->set_param(param_name, old_value);
	}

	layer->changed();
	if (old_value_node)
		old_value_node->changed();

	if (get_canvas_interface())
		get_canvas_interface()->signal_layer_param_changed()(layer, param_name);
}
