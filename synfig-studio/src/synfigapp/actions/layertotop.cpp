/* === S Y N F I G ========================================================= */
/*!	\file layertotop.cpp
**	\brief Template File
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

#include "layertotop.h"
#include <synfig/general.h>
#include <synfigapp/canvasinterface.h>
#include <synfigapp/localization.h>
#include "layermove.h"

#endif

using namespace synfig;
using namespace synfigapp;
using namespace Action;

/* === M A C R O S ========================================================= */

ACTION_INIT_NO_GET_LOCAL_NAME(Action::LayerToTop);
ACTION_SET_NAME(Action::LayerToTop,"LayerToTop");
ACTION_SET_LOCAL_NAME(Action::LayerToTop,N_("Raise Layer to Top"));
ACTION_SET_TASK(Action::LayerToTop,"raise to top");
ACTION_SET_CATEGORY(Action::LayerToTop,Action::CATEGORY_LAYER);
ACTION_SET_PRIORITY(Action::LayerToTop,9);
ACTION_SET_VERSION(Action::LayerToTop,"0.0");

/* === G L O B A L S ======================================================= */

/* === P R O C E D U R E S ================================================= */

/* === M E T H O D S ======================================================= */

LayerToTop::LayerToTop()
{

}

synfig::String
Action::LayerToTop::get_local_name() const
{
	return get_layer_descriptions(layers, _("Raise Layer to Top"), _("Raise Layers to Top"));
}

Action::ParamVocab
Action::LayerToTop::get_param_vocab()
{
	ParamVocab ret(Action::CanvasSpecific::get_param_vocab());

	ret.push_back(ParamDesc("layer",Param::TYPE_LAYER)
		.set_local_name(_("Layer"))
		.set_desc(_("Layer to be raised"))
		.set_supports_multiple()
	);

	return ret;
}

bool
Action::LayerToTop::is_candidate(const ParamList &x)
{
	if (!candidate_check(get_param_vocab(), x))
		return false;
	if (x.find("layer")->second.get_layer()->get_depth() == 0)
		return false;
	return true;
}

bool
Action::LayerToTop::set_param(const synfig::String& name, const Action::Param &param)
{
	if(name=="layer" && param.get_type() == Param::TYPE_LAYER)
	{
		layers.push_back(param.get_layer());

		return true;
	}

	return Action::CanvasSpecific::set_param(name,param);
}

bool
Action::LayerToTop::is_ready()const
{
	if (layers.empty())
		return false;
	return Action::CanvasSpecific::is_ready();
}

void
Action::LayerToTop::prepare()
{
	clear();

	for (auto i = layers.begin(); i!=layers.end(); ++i)
	{
		Layer::Handle layer(*i);

		Canvas::Handle subcanvas(layer->get_canvas());

		// Find the index of the layer
		int current_index = -1;
		Canvas::iterator iter = subcanvas->find_index(layer, current_index);
		if (*iter != layer)
			throw Error(_("This layer doesn't exist anymore."));

		if (current_index == 0)
			continue;

		Action::Handle layer_move(LayerMove::create());

		layer_move->set_param("canvas", get_canvas());
		layer_move->set_param("canvas_interface", get_canvas_interface());
		layer_move->set_param("layer", layer);
		layer_move->set_param("new_index", 0);

		add_action_front(layer_move);
	}
}
