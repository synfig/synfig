/* === S Y N F I G ========================================================= */
/*!	\file layeroutlineregion.cpp
**	\brief Template File
**
**	$Id$
**
**	\legal
**	......... ... 2013 Ivan Mahonin
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

#include "layeroutlineregion.h"
#include "layeradd.h"
#include <synfig/context.h>
#include <synfigapp/canvasinterface.h>

#include <synfigapp/general.h>

#endif

using namespace std;
using namespace etl;
using namespace synfig;
using namespace synfigapp;
using namespace Action;

/* === M A C R O S ========================================================= */

ACTION_INIT_NO_GET_LOCAL_NAME(Action::LayerOutlineRegion);
ACTION_SET_NAME(Action::LayerOutlineRegion,"LayerOutlineRegion");
ACTION_SET_LOCAL_NAME(Action::LayerOutlineRegion,N_("Outline Region"));
ACTION_SET_TASK(Action::LayerOutlineRegion,"outline_region");
ACTION_SET_CATEGORY(Action::LayerOutlineRegion,Action::CATEGORY_LAYER);
ACTION_SET_PRIORITY(Action::LayerOutlineRegion,0);
ACTION_SET_VERSION(Action::LayerOutlineRegion,"0.0");
ACTION_SET_CVS_ID(Action::LayerOutlineRegion,"$Id$");

/* === G L O B A L S ======================================================= */

/* === P R O C E D U R E S ================================================= */

/* === M E T H O D S ======================================================= */

Action::LayerOutlineRegion::LayerOutlineRegion()
{
}

synfig::String
Action::LayerOutlineRegion::get_local_name()const
{
	if (layer)
		return strprintf("%s '%s'", _("Outline Region"), layer->get_non_empty_description().c_str());
	else
		return _("Outline Region");
}

Action::ParamVocab
Action::LayerOutlineRegion::get_param_vocab()
{
	ParamVocab ret(Action::CanvasSpecific::get_param_vocab());

	ret.push_back(ParamDesc("layer",Param::TYPE_LAYER)
		.set_local_name(_("Region"))
		.set_desc(_("Region to be outlined"))
		.set_supports_multiple()
	);

	return ret;
}

bool
Action::LayerOutlineRegion::is_candidate(const ParamList &x)
{
	return candidate_check(get_param_vocab(),x);
}

bool
Action::LayerOutlineRegion::set_param(const synfig::String& name, const Action::Param &param)
{
	if(name=="layer" && param.get_type()==Param::TYPE_LAYER)
	{
		if (param.get_layer()->get_name() == "region")
		{
			layer = param.get_layer();
			return true;
		}
		else return false;
	}

	return Action::CanvasSpecific::set_param(name,param);
}

bool
Action::LayerOutlineRegion::is_ready()const
{
	if(!layer)
		return false;
	return Action::CanvasSpecific::is_ready();
}

void
Action::LayerOutlineRegion::prepare()
{
	//todo: why?
	if(!first_time())
		return;

	if (!layer)
		return;

	Canvas::Handle subcanvas(layer->get_canvas());

	// Find the iterator for the region
	Canvas::iterator iter=find(subcanvas->begin(),subcanvas->end(),layer);

	// If we couldn't find the region in the canvas, then bail
	if(*iter!=layer)
		throw Error(_("This region doesn't exist anymore."));

	// If the subcanvas isn't the same as the canvas,
	// then it had better be an inline canvas. If not,
	// bail
	if(get_canvas()!=subcanvas && !subcanvas->is_inline())
		throw Error(_("This region doesn't belong to this canvas anymore"));

	// todo: which canvas should we use?  subcanvas is the region's canvas, get_canvas() is the canvas set in the action
	Layer::Handle outline(synfig::Layer::create("outline"));

	// Apply some defaults
	outline->set_canvas(subcanvas);
	get_canvas_interface()->apply_layer_param_defaults(outline);

	{
		Action::Handle action(Action::create("LayerMove"));

		action->set_param("canvas",subcanvas);
		action->set_param("canvas_interface",get_canvas_interface());
		action->set_param("layer",outline);
		action->set_param("new_index",layer->get_depth());

		add_action_front(action);
	}
	{
		Action::Handle action(Action::create("LayerAdd"));

		action->set_param("canvas",subcanvas);
		action->set_param("canvas_interface",get_canvas_interface());
		action->set_param("new",outline);

		add_action_front(action);
	}
	{
		Action::Handle action(Action::create("LayerParamConnect"));

		action->set_param("canvas",subcanvas);
		action->set_param("canvas_interface",get_canvas_interface());
		action->set_param("layer",outline);
		action->set_param("param","bline");
		action->set_param("value_node",layer->dynamic_param_list().find("bline")->second);

		add_action_front(action);
	}
}
