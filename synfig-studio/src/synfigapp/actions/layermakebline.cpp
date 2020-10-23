/* === S Y N F I G ========================================================= */
/*!	\file layermakebline.cpp
**	\brief Template File
**
**	$Id$
**
**	\legal
**	......... ... 2013 Ivan Mahonin
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

#include "layermakebline.h"
#include "layeradd.h"
#include "layermove.h"
#include "layerparamconnect.h"
#include <synfig/context.h>
#include <synfigapp/canvasinterface.h>

#include <synfigapp/localization.h>

#endif

using namespace etl;
using namespace synfig;
using namespace synfigapp;
using namespace Action;

/* === M A C R O S ========================================================= */

#define ACTION_LAYERMAKEBLINE_IMPLEMENT(class_name, local_name, bline_layer_name, put_new_layer_behind, ...) \
	ACTION_INIT(Action::class_name); \
	ACTION_SET_NAME(Action::class_name, #class_name); \
	ACTION_SET_LOCAL_NAME(Action::class_name, local_name); \
	ACTION_SET_TASK(Action::class_name,"make_" bline_layer_name); \
	ACTION_SET_CATEGORY(Action::class_name,Action::CATEGORY_LAYER); \
	ACTION_SET_PRIORITY(Action::class_name,0); \
	ACTION_SET_VERSION(Action::class_name,"0.0"); \
	const char * Action::class_name::possible_layer_names__[] = {__VA_ARGS__}; \
	bool Action::class_name::is_candidate(const ParamList &x) \
		{ return is_candidate_for_make_bline(x, possible_layer_names__, sizeof(possible_layer_names__)/sizeof(possible_layer_names__[0])); } \
	void Action::class_name::prepare() { prepare_make_bline(bline_layer_name, put_new_layer_behind); }

ACTION_LAYERMAKEBLINE_IMPLEMENT(
		LayerMakeOutline, N_("Make Outline"), "outline", false,
		"advanced_outline", "region", "plant", "curve_gradient");

ACTION_LAYERMAKEBLINE_IMPLEMENT(
		LayerMakeAdvancedOutline, N_("Make Advanced Outline"), "advanced_outline", false,
		"outline", "region", "plant", "curve_gradient");

ACTION_LAYERMAKEBLINE_IMPLEMENT(
		LayerMakeRegion, N_("Make Region"), "region", true,
		"outline", "advanced_outline", "plant", "curve_gradient");

ACTION_LAYERMAKEBLINE_IMPLEMENT(
		LayerMakeCurveGradient, N_("Make Curve Gradient"), "curve_gradient", true,
		"outline", "advanced_outline", "region", "plant");

ACTION_LAYERMAKEBLINE_IMPLEMENT(
		LayerMakePlant, N_("Make Plant"), "plant", false,
		"outline", "advanced_outline", "region", "curve_gradient");

/* === G L O B A L S ======================================================= */

/* === P R O C E D U R E S ================================================= */

/* === M E T H O D S ======================================================= */

Action::ParamVocab
Action::LayerMakeBLine::get_param_vocab()
{
	ParamVocab ret(Action::CanvasSpecific::get_param_vocab());

	ret.push_back(ParamDesc("layer",Param::TYPE_LAYER)
		.set_local_name(_("Layer"))
		.set_desc(_("Base layer"))
	);

	return ret;
}

bool
Action::LayerMakeBLine::is_candidate_for_make_bline(const ParamList &x, const char **possible_layer_names, size_t possible_layer_names_count)
{
	if(!candidate_check(get_param_vocab(),x))
		return false;

	if(x.count("layer") == 1)
	{
		const Param &param = x.find("layer")->second;
		if(param.get_type() == Param::TYPE_LAYER
		&& param.get_layer()
		&& param.get_layer()->dynamic_param_list().count("bline") == 1)
		{
			for(size_t i = 0; i < possible_layer_names_count; i++)
				if (param.get_layer()->get_name() == possible_layer_names[i])
					return true;
		}
	}

	return false;
}

bool
Action::LayerMakeBLine::set_param(const synfig::String& name, const Action::Param &param)
{
	if(name=="layer" && param.get_type()==Param::TYPE_LAYER)
	{
		layer = param.get_layer();
		return true;
	}

	return Action::CanvasSpecific::set_param(name,param);
}

bool
Action::LayerMakeBLine::is_ready()const
{
	if(!layer)
		return false;
	return Action::CanvasSpecific::is_ready();
}

void
Action::LayerMakeBLine::prepare_make_bline(const synfig::String &bline_layer_name, bool put_new_layer_behind)
{
	if (!layer)
		return;
	if(!layer->dynamic_param_list().count("bline"))
		throw Error(_("This layer doesn't contain linked \"spline\" parameter."));

	Canvas::Handle subcanvas(layer->get_canvas());

	// Find the iterator for the layer
	Canvas::iterator iter=find(subcanvas->begin(),subcanvas->end(),layer);

	// If we couldn't find the region in the canvas, then bail
	if(*iter!=layer)
		throw Error(_("This layer doesn't exist anymore."));

	// If the subcanvas isn't the same as the canvas,
	// then it had better be an inline canvas. If not,
	// bail
	if(get_canvas()!=subcanvas && !subcanvas->is_inline())
		throw Error(_("This layer doesn't belong to this canvas anymore"));

	// todo: which canvas should we use?  subcanvas is the layer's canvas, get_canvas() is the canvas set in the action
	Layer::Handle new_layer(synfig::Layer::create(bline_layer_name));

	// Apply some defaults
	new_layer->set_canvas(subcanvas);
	get_canvas_interface()->layer_set_defaults(new_layer);

	// Set depth
	{
		Action::Handle action(Action::create("LayerMove"));

		action->set_param("canvas",subcanvas);
		action->set_param("canvas_interface",get_canvas_interface());
		action->set_param("layer",new_layer);
		action->set_param("new_index", put_new_layer_behind ? layer->get_depth()+1 : layer->get_depth());

		add_action_front(action);
	}

	// Add into canvas
	{
		Action::Handle action(Action::create("LayerAdd"));

		action->set_param("canvas",subcanvas);
		action->set_param("canvas_interface",get_canvas_interface());
		action->set_param("new",new_layer);

		add_action_front(action);
	}

	// Connect vertices list
	{
		Action::Handle action(Action::create("LayerParamConnect"));

		action->set_param("canvas",subcanvas);
		action->set_param("canvas_interface",get_canvas_interface());
		action->set_param("layer",new_layer);
		action->set_param("param","bline");
		action->set_param("value_node",layer->dynamic_param_list().find("bline")->second);

		add_action_front(action);
	}
	
	// Connect origin
	ValueNode::Handle value_node_origin;
	if (layer->dynamic_param_list().count("origin"))
		value_node_origin=layer->dynamic_param_list().find("origin")->second;
	else
	{
		Vector origin;
		origin = layer->get_param("origin").get(Vector());
		value_node_origin=ValueNode_Const::create(origin);
		{
			Action::Handle action(Action::create("LayerParamConnect"));

			action->set_param("canvas",subcanvas);
			action->set_param("canvas_interface",get_canvas_interface());
			action->set_param("layer",layer);
			action->set_param("param","origin");
			action->set_param("value_node",value_node_origin);

			add_action_front(action);
		}
	}
	{
		Action::Handle action(Action::create("LayerParamConnect"));

		action->set_param("canvas",subcanvas);
		action->set_param("canvas_interface",get_canvas_interface());
		action->set_param("layer",new_layer);
		action->set_param("param","origin");
		action->set_param("value_node",value_node_origin);

		add_action_front(action);
	}
}
