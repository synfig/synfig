/* === S Y N F I G ========================================================= */
/*!	\file layerlower.cpp
**	\brief Template File
**
**	$Id$
**
**	\legal
**	Copyright (c) 2002-2005 Robert B. Quattlebaum Jr., Adrian Bentley
**	Copyright (c) 2008 Chris Moore
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

#include <synfig/general.h>

#include "layerlower.h"
#include "layermove.h"
#include <synfigapp/canvasinterface.h>

#include <synfigapp/localization.h>

#endif

using namespace std;
using namespace etl;
using namespace synfig;
using namespace synfigapp;
using namespace Action;

/* === M A C R O S ========================================================= */

ACTION_INIT_NO_GET_LOCAL_NAME(Action::LayerLower);
ACTION_SET_NAME(Action::LayerLower,"LayerLower");
ACTION_SET_LOCAL_NAME(Action::LayerLower,N_("Lower Layer"));
ACTION_SET_TASK(Action::LayerLower,"lower");
ACTION_SET_CATEGORY(Action::LayerLower,Action::CATEGORY_LAYER);
ACTION_SET_PRIORITY(Action::LayerLower,10);
ACTION_SET_VERSION(Action::LayerLower,"0.0");
ACTION_SET_CVS_ID(Action::LayerLower,"$Id$");

/* === G L O B A L S ======================================================= */

/* === P R O C E D U R E S ================================================= */

/* === M E T H O D S ======================================================= */

Action::LayerLower::LayerLower()
{
}

synfig::String
Action::LayerLower::get_local_name()const
{
	return get_layer_descriptions(layers, _("Lower Layer"), _("Lower Layers"));
}

Action::ParamVocab
Action::LayerLower::get_param_vocab()
{
	ParamVocab ret(Action::CanvasSpecific::get_param_vocab());

	ret.push_back(ParamDesc("layer",Param::TYPE_LAYER)
		.set_local_name(_("Layer"))
		.set_desc(_("Layer to be lowered"))
		.set_supports_multiple()
	);

	return ret;
}

bool
Action::LayerLower::is_candidate(const ParamList &x)
{
	if(!candidate_check(get_param_vocab(),x))
		return false;

	Layer::Handle layer(x.find("layer")->second.get_layer());
	//synfig::info("layer->get_depth()= %d ; layer->get_canvas()->size()=%d ;",layer->get_depth(),layer->get_canvas()->size());
	if(layer->get_depth()+1>=layer->get_canvas()->size())
		return false;
	return true;
}

bool
Action::LayerLower::set_param(const synfig::String& name, const Action::Param &param)
{
	if(name=="layer" && param.get_type()==Param::TYPE_LAYER)
	{
		layers.push_back(param.get_layer());

		return true;
	}

	return Action::CanvasSpecific::set_param(name,param);
}

bool
Action::LayerLower::is_ready()const
{
	if(layers.empty())
		return false;
	return Action::CanvasSpecific::is_ready();
}

void
Action::LayerLower::prepare()
{
	std::list<synfig::Layer::Handle>::const_iterator i;

	clear();

	for(i=layers.begin();i!=layers.end();++i)
	{
		Layer::Handle layer(*i);

		Canvas::Handle subcanvas(layer->get_canvas());

		// Find the index of the layer
		int new_index = -1;
		Canvas::iterator iter = subcanvas->find_index(layer, new_index);

		// If we couldn't find the layer in the canvas, then bail
		if(*iter!=layer)
			throw Error(_("This layer doesn't exist anymore."));

		// If the subcanvas isn't the same as the canvas,
		// then it had better be an inline canvas. If not,
		// bail
		//if(get_canvas()!=subcanvas && !subcanvas->is_inline())
		//	throw Error(_("This layer doesn't belong to this canvas anymore"));

		++iter;
		++new_index;

		// If this lowers the layer past the bottom then don't bother
		if(iter == subcanvas->end())
			continue;

		Action::Handle layer_move(LayerMove::create());

		layer_move->set_param("canvas",get_canvas());
		layer_move->set_param("canvas_interface",get_canvas_interface());
		layer_move->set_param("layer",layer);
		layer_move->set_param("new_index",new_index);

		add_action_front(layer_move);
	}
}
