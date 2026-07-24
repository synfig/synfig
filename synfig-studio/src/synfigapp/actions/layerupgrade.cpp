/* === S Y N F I G ========================================================= */
/*!	\file layerupgrade.cpp
**	\brief Upgrade a deprecated layer to its current replacement
**
**	\legal
**	Copyright (C) 2026 Synfig Contributors
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

#include <algorithm>
#include <utility>

#include <synfig/general.h>

#include "layerupgrade.h"

#include <synfigapp/canvasinterface.h>
#include <synfigapp/localization.h>

#endif

using namespace synfig;
using namespace synfigapp;
using namespace Action;

/* === M A C R O S ========================================================= */

ACTION_INIT(Action::LayerUpgrade);
ACTION_SET_NAME(Action::LayerUpgrade,"LayerUpgrade");
ACTION_SET_LOCAL_NAME(Action::LayerUpgrade,N_("Upgrade Layer"));
ACTION_SET_TASK(Action::LayerUpgrade,"upgrade");
ACTION_SET_CATEGORY(Action::LayerUpgrade,Action::CATEGORY_LAYER);
ACTION_SET_PRIORITY(Action::LayerUpgrade,0);
ACTION_SET_VERSION(Action::LayerUpgrade,"0.0");

/* === G L O B A L S ======================================================= */

/* === P R O C E D U R E S ================================================= */

/* === M E T H O D S ======================================================= */

// ponytail: the deprecated->current layer name map. Add a pair here to enable
// its upgrade; no other code in this file needs to change. Linear scan of a
// tiny list is fine while pairs stay few -- switch to std::map if it grows.
synfig::String
Action::LayerUpgrade::get_target_layer_name(const synfig::String &source_layer_name)
{
	static const std::pair<const char*, const char*> table[] = {
		{ "bevel_deprecated", "bevel" }
	};
	for(const auto &entry : table)
		if(source_layer_name == entry.first)
			return synfig::String(entry.second);
	return synfig::String();
}

Action::ParamVocab
Action::LayerUpgrade::get_param_vocab()
{
	ParamVocab ret(Action::CanvasSpecific::get_param_vocab());

	ret.push_back(ParamDesc("layer",Param::TYPE_LAYER)
		.set_local_name(_("Layer"))
		.set_desc(_("Deprecated layer to upgrade"))
		.set_supports_multiple()
	);

	return ret;
}

bool
Action::LayerUpgrade::is_candidate(const ParamList &x)
{
	if(!candidate_check(get_param_vocab(),x))
		return false;

	// Candidate only when EVERY selected layer has a known upgrade target.
	std::pair<ParamList::const_iterator, ParamList::const_iterator> range(x.equal_range("layer"));
	if(range.first==range.second)
		return false;
	for(ParamList::const_iterator it(range.first); it!=range.second; ++it)
	{
		if(it->second.get_type()!=Param::TYPE_LAYER)
			return false;
		const Layer::Handle layer(it->second.get_layer());
		if(!layer || get_target_layer_name(layer->get_name()).empty())
			return false;
	}
	return true;
}

bool
Action::LayerUpgrade::set_param(const synfig::String& name, const Action::Param &param)
{
	if(name=="layer" && param.get_type()==Param::TYPE_LAYER)
	{
		layers.push_back(param.get_layer());
		return true;
	}

	return Action::CanvasSpecific::set_param(name,param);
}

bool
Action::LayerUpgrade::is_ready()const
{
	if(layers.empty())
		return false;
	return Action::CanvasSpecific::is_ready();
}

void
Action::LayerUpgrade::prepare()
{
	if(!first_time())
		return;

	if(layers.empty())
		throw Error(_("No layers to upgrade"));

	// ponytail: process descending by depth so mutations at higher indices
	// never shift the captured depth of lower layers (multi-select drift guard).
	layers.sort([](const Layer::Handle &a, const Layer::Handle &b)
		{ return a->get_depth() > b->get_depth(); });

	for(const Layer::Handle &layer : layers)
	{
		const String target_name(get_target_layer_name(layer->get_name()));
		if(target_name.empty())
			continue; // shouldn't happen -- is_candidate already filtered
		prepare_upgrade_layer(layer, target_name);
	}
}

void
Action::LayerUpgrade::prepare_upgrade_layer(const synfig::Layer::Handle &layer, const synfig::String &target_name)
{
	if(!layer)
		return;

	Canvas::Handle subcanvas(layer->get_canvas());

	// Find the iterator for the layer
	Canvas::iterator iter(find(subcanvas->begin(),subcanvas->end(),layer));

	// If we couldn't find the layer in the canvas, then bail
	if(*iter!=layer)
		throw Error(_("This layer doesn't exist anymore."));

	// If the subcanvas isn't the same as the canvas,
	// then it had better be an inline canvas. If not, bail.
	if(get_canvas()!=subcanvas && !subcanvas->is_inline())
		throw Error(_("This layer doesn't belong to this canvas anymore"));

	const int depth(layer->get_depth());

	// create the replacement layer and carry over its state.
	// Deprecated and current layers are expected to share an identical
	// parameter vocabulary, so set_param_list transfers everything
	// (amount, blend_method, z_depth, type, colors, angle, ...). name__/
	// version__ are not part of the vocab, so the new layer keeps its own
	// register identity.
	Layer::Handle new_layer(Layer::create(target_name));
	new_layer->set_canvas(subcanvas);
	get_canvas_interface()->layer_set_defaults(new_layer);
	new_layer->set_param_list(layer->get_param_list());

	// description: keep only user-set values; an empty description falls back
	// to the new layer's local name. Drop text that equals the old layer's
	// default display name too, so the upgrade is invisible.
	const String &desc(layer->get_description());
	if(!desc.empty() && desc != layer->get_local_name())
		new_layer->set_description(desc);

	new_layer->set_active(layer->active());
	new_layer->set_exclude_from_rendering(layer->get_exclude_from_rendering());

	// group membership: group_ is synced into the canvas group_db_ when the
	// layer is inserted (Canvas::insert), so setting it before LayerAdd is enough.
	if(!layer->get_group().empty())
		new_layer->add_to_group(layer->get_group());

	// Reconnect animated (dynamic) parameters by SHARING the value nodes,
	// not cloning them, so exported/animated links stay referenced.
	for(const auto &dp : layer->dynamic_param_list())
	{
		Action::Handle action(Action::create("LayerParamConnect"));
		action->set_param("canvas",subcanvas);
		action->set_param("canvas_interface",get_canvas_interface());
		action->set_param("layer",new_layer);
		action->set_param("param",dp.first);
		action->set_param("value_node",ValueNode::Handle(dp.second));
		add_action(action);
	}

	// add the new layer
	{
		Action::Handle action(Action::create("LayerAdd"));
		action->set_param("canvas",subcanvas);
		action->set_param("canvas_interface",get_canvas_interface());
		action->set_param("new",new_layer);
		add_action(action);
	}

	// move it into the old layer's position
	{
		Action::Handle action(Action::create("LayerMove"));
		action->set_param("canvas",subcanvas);
		action->set_param("canvas_interface",get_canvas_interface());
		action->set_param("layer",new_layer);
		action->set_param("new_index",depth);
		add_action(action);
	}

	// finally, remove the deprecated layer
	{
		Action::Handle action(Action::create("LayerRemove"));
		action->set_param("canvas",subcanvas);
		action->set_param("canvas_interface",get_canvas_interface());
		action->set_param("layer",layer);
		add_action(action);
	}
}
