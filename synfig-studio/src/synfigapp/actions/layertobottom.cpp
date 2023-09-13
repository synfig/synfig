/* === S Y N F I G ========================================================= */
/*!	\file layertobottom.cpp
**	\brief Action to lower layer to bottom of the layer stack
**
**	\legal
**	Copyright (c) 2002-2005 Robert B. Quattlebaum Jr., Adrian Bentley
**	Copyright (c) 2008 Chris Moore
**	Copyright (c) 2023 Synfig Contributors
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

#include "layertobottom.h"
#include <synfig/general.h>
#include <synfigapp/canvasinterface.h>
#include <synfigapp/localization.h>
#include "layermove.h"


#endif

using namespace synfig;
using namespace synfigapp;
using namespace Action;

/* === M A C R O S ========================================================= */

ACTION_INIT_NO_GET_LOCAL_NAME(Action::LayerLowerMax);
ACTION_SET_NAME(Action::LayerLowerMax,"LayerLowerMax");
ACTION_SET_LOCAL_NAME(Action::LayerLowerMax,N_("Lower Layer to Bottom"));
ACTION_SET_TASK(Action::LayerLowerMax,"lower to bottom");
ACTION_SET_CATEGORY(Action::LayerLowerMax,Action::CATEGORY_LAYER);
ACTION_SET_PRIORITY(Action::LayerLowerMax,9);
ACTION_SET_VERSION(Action::LayerLowerMax,"0.0");

/* === G L O B A L S ======================================================= */

/* === P R O C E D U R E S ================================================= */

/* === M E T H O D S ======================================================= */

LayerLowerMax::LayerLowerMax()
{

}

synfig::String
Action::LayerLowerMax::get_local_name() const
{
	return get_layer_descriptions(layers, _("Lower Layer to Bottom"), _("Lower Layers to Bottom"));
}

Action::ParamVocab
Action::LayerLowerMax::get_param_vocab()
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
Action::LayerLowerMax::is_candidate(const ParamList &x)
{
	if (!candidate_check(get_param_vocab(), x))
		return false;
	Layer::Handle layer(x.find("layer")->second.get_layer());
	if (layer->get_depth()+1 >= layer->get_canvas()->size())
		return false;
	return true;
}

bool
Action::LayerLowerMax::set_param(const synfig::String& name, const Action::Param &param)
{
	if(name=="layer" && param.get_type()==Param::TYPE_LAYER)
	{
		layers.push_back(param.get_layer());

		return true;
	}

	return Action::CanvasSpecific::set_param(name,param);
}

bool
Action::LayerLowerMax::is_ready()const
{
	if(layers.empty())
		return false;
	return Action::CanvasSpecific::is_ready();
}

void
Action::LayerLowerMax::prepare()
{
	clear();

	for (auto i = layers.rbegin(); i != layers.rend(); ++i)
	{
		Layer::Handle layer(*i);

		Canvas::Handle subcanvas(layer->get_canvas());

		// Find the index of the layer
		int current_index = -1;
		Canvas::iterator iter = subcanvas->find_index(layer, current_index);
		if (*iter != layer)
			throw Error(_("This layer doesn't exist anymore."));

		// If this lowers the layer past the bottom then don't bother
		if(iter == subcanvas->end())
			continue;

		Action::Handle layer_move(LayerMove::create());

		layer_move->set_param("canvas",get_canvas());
		layer_move->set_param("canvas_interface",get_canvas_interface());
		layer_move->set_param("layer",layer);
		layer_move->set_param("new_index",subcanvas->size()-1);

		add_action_front(layer_move);
	}
}
