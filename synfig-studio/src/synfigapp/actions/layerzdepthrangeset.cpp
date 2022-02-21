/* === S Y N F I G ========================================================= */
/*!	\file layerzdepthrangeset.cpp
**	\brief Template File
**
**	\legal
**	Copyright (c) 2002-2005 Robert B. Quattlebaum Jr., Adrian Bentley
**	Copyright (c) 2007, 2008 Chris Moore
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

#include "layerzdepthrangeset.h"
#include "layeradd.h"
#include "layerremove.h"
#include <synfigapp/canvasinterface.h>

#include <synfigapp/localization.h>

#include <synfig/layers/layer_group.h>

#endif

using namespace synfig;
using namespace synfigapp;
using namespace Action;

/* === M A C R O S ========================================================= */

ACTION_INIT_NO_GET_LOCAL_NAME(Action::LayerZDepthRangeSet);
ACTION_SET_NAME(Action::LayerZDepthRangeSet,"LayerZDepthRangeSet");
ACTION_SET_LOCAL_NAME(Action::LayerZDepthRangeSet,N_("Make Z Range visible"));
ACTION_SET_TASK(Action::LayerZDepthRangeSet,"zdetph_range_set");
ACTION_SET_CATEGORY(Action::LayerZDepthRangeSet,Action::CATEGORY_LAYER);
ACTION_SET_PRIORITY(Action::LayerZDepthRangeSet,0);
ACTION_SET_VERSION(Action::LayerZDepthRangeSet,"0.0");

#define ZDEPTH_MAX 1e8
#define ZDEPTH_EPS 0.0
/* === G L O B A L S ======================================================= */

/* === P R O C E D U R E S ================================================= */

/* === M E T H O D S ======================================================= */

Action::LayerZDepthRangeSet::LayerZDepthRangeSet()
{
	z_depth=ZDEPTH_EPS;
	z_position=ZDEPTH_MAX;
}

synfig::String
Action::LayerZDepthRangeSet::get_local_name()const
{
	return get_layer_descriptions(layers, _("Make Z Range visible"), _("Make Z Range visible"));
}

Action::ParamVocab
Action::LayerZDepthRangeSet::get_param_vocab()
{
	ParamVocab ret(Action::CanvasSpecific::get_param_vocab());

	ret.push_back(ParamDesc("layer",Param::TYPE_LAYER)
		.set_local_name(_("Layer"))
		.set_desc(_("Layer to make Z Range visible"))
		.set_supports_multiple()
	);

	return ret;
}

bool
Action::LayerZDepthRangeSet::is_candidate(const ParamList &x)
{
	if(!candidate_check(get_param_vocab(),x))
		return false;
	// Check if all layers belong to the same canvas
	Canvas::Handle canvas=0;
	for(ParamList::const_iterator i = x.begin(); i != x.end(); i++) 
	{
		if (i->first == "layer" && i->second.get_type() == Param::TYPE_LAYER) 
		{
			const Layer::Handle layer = i->second.get_layer();
			if(layer.empty())
				return false;
			if(!canvas)
				canvas=layer->get_canvas();
			if(canvas && canvas->is_root())
				return false;
			if(canvas && !canvas->is_inline())
				return false;
			if(layer->get_canvas() && canvas && layer->get_canvas()!=canvas)
				return false;
		}
	}
	return true;
}

bool
Action::LayerZDepthRangeSet::set_param(const synfig::String& name, const Action::Param &param)
{
	if(name=="layer" && param.get_type()==Param::TYPE_LAYER)
	{
		layers.push_back(param.get_layer());
		Layer::Handle layer=param.get_layer();
		if(layer)
		{
			// Expand position and depth to include the given layer
			float layer_z_depth=layer->get_true_z_depth(Time(0));
			if(z_position > layer_z_depth)
				z_position=layer_z_depth;
			if(z_position + z_depth <= layer_z_depth)
				z_depth=layer_z_depth - z_position;
		}
		return true;
	}
	return Action::CanvasSpecific::set_param(name,param);
}

bool
Action::LayerZDepthRangeSet::is_ready()const
{
	if(layers.empty())
		return false;
	if(z_position == ZDEPTH_MAX)
		return false;
	return Action::CanvasSpecific::is_ready();
}

void
Action::LayerZDepthRangeSet::prepare()
{
	if(!first_time())
		return;

	if(layers.empty())
		throw Error(_("No layers selected"));

	Layer::Handle layer=layers.front();

	etl::handle<Layer_Group> paste=
		etl::handle<Layer_Group>::cast_dynamic(
			layer->get_parent_paste_canvas_layer() );
	if(!paste)
		throw Error(_("No Parent Group found!"));
	// Z ENABLE
	{
		ValueBase new_value(true);
		Action::Handle action(Action::create("ValueDescSet"));
		if(!action)
			throw Error(_("Unable to find action ValueDescSet (bug)"));
		action->set_param("canvas",get_canvas());
		action->set_param("canvas_interface",get_canvas_interface());
		action->set_param("new_value",new_value);
		action->set_param("value_desc",ValueDesc(Layer::Handle(paste), "z_range"));
		action->set_param("recursive", true);
		if(!action->is_ready())
			throw Error(Error::TYPE_NOTREADY);
		add_action(action);
	}
		// Z POSITION
	{
		ValueBase new_value(z_position);
		Action::Handle action(Action::create("ValueDescSet"));
		if(!action)
			throw Error(_("Unable to find action ValueDescSet (bug)"));
		action->set_param("canvas",get_canvas());
		action->set_param("canvas_interface",get_canvas_interface());
		action->set_param("new_value",new_value);
		action->set_param("value_desc",ValueDesc(Layer::Handle(paste), "z_range_position"));
		action->set_param("recursive", false);
		if(!action->is_ready())
			throw Error(Error::TYPE_NOTREADY);
		add_action(action);
	}
	// Z DEPTH
	{
		ValueBase new_value(z_depth);
		Action::Handle action(Action::create("ValueDescSet"));
		if(!action)
			throw Error(_("Unable to find action ValueDescSet (bug)"));
		action->set_param("canvas",get_canvas());
		action->set_param("canvas_interface",get_canvas_interface());
		action->set_param("new_value",new_value);
		action->set_param("value_desc",ValueDesc(Layer::Handle(paste), "z_range_depth"));
		action->set_param("recursive", false);
		if(!action->is_ready())
			throw Error(Error::TYPE_NOTREADY);
		add_action(action);
	}
	return;
}
