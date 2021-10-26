/* === S Y N F I G ========================================================= */
/*!	\file layerencapsulatefilter.cpp
**	\brief Template File
**
**	\legal
**	......... ... 2017 Ivan Mahonin
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

#include "layerencapsulatefilter.h"
#include "layeradd.h"
#include "layerremove.h"
#include <synfigapp/canvasinterface.h>

#include <synfigapp/localization.h>

#endif

using namespace synfig;
using namespace synfigapp;
using namespace Action;

/* === M A C R O S ========================================================= */

ACTION_INIT_NO_GET_LOCAL_NAME(Action::LayerEncapsulateFilter);
ACTION_SET_NAME(Action::LayerEncapsulateFilter,"LayerEncapsulateFilter");
ACTION_SET_LOCAL_NAME(Action::LayerEncapsulateFilter,N_("Group Layers into Filter"));
ACTION_SET_TASK(Action::LayerEncapsulateFilter,"encapsulate_filter");
ACTION_SET_CATEGORY(Action::LayerEncapsulateFilter,Action::CATEGORY_LAYER);
ACTION_SET_PRIORITY(Action::LayerEncapsulateFilter,0);
ACTION_SET_VERSION(Action::LayerEncapsulateFilter,"0.0");

/* === G L O B A L S ======================================================= */

/* === P R O C E D U R E S ================================================= */

/* === M E T H O D S ======================================================= */

Action::LayerEncapsulateFilter::LayerEncapsulateFilter()
{
}

synfig::String
Action::LayerEncapsulateFilter::get_local_name()const
{
	return get_layer_descriptions(layers, _("Group Layer into Filter"), _("Group Layers into Filter"));
}

Action::ParamVocab
Action::LayerEncapsulateFilter::get_param_vocab()
{
	ParamVocab ret(Action::CanvasSpecific::get_param_vocab());

	ret.push_back(ParamDesc("layer",Param::TYPE_LAYER)
		.set_local_name(_("Layer"))
		.set_desc(_("Layer to be grouped"))
		.set_supports_multiple()
	);
	ret.push_back(ParamDesc("description",Param::TYPE_STRING)
		.set_local_name(_("Description"))
		.set_desc(_("Description of new filter"))
		.set_optional()
	);

	return ret;
}

bool
Action::LayerEncapsulateFilter::is_candidate(const ParamList &x)
{
	return candidate_check(get_param_vocab(),x);
}

bool
Action::LayerEncapsulateFilter::set_param(const synfig::String& name, const Action::Param &param)
{
	if(name=="layer" && param.get_type()==Param::TYPE_LAYER)
	{
		layers.push_back(param.get_layer());
		return true;
	}
	if(name=="description" && param.get_type()==Param::TYPE_STRING)
	{
		description = param.get_string();
		return true;
	}

	return Action::CanvasSpecific::set_param(name,param);
}

bool
Action::LayerEncapsulateFilter::is_ready()const
{
	if(layers.empty())
		return false;
	return Action::CanvasSpecific::is_ready();
}

int
Action::LayerEncapsulateFilter::lowest_depth()const
{
	std::list<synfig::Layer::Handle>::const_iterator iter;
	int lowest_depth(0x7fffffff);

	for(iter=layers.begin();iter!=layers.end();++iter)
	{
		int depth((*iter)->get_depth());
		if(depth<lowest_depth)
			lowest_depth=depth;
	}
	if(lowest_depth==0x7fffffff)
		return 0;
	return lowest_depth;
}

void
Action::LayerEncapsulateFilter::prepare()
{

	if(!first_time())
		return;

	if(layers.empty())
		throw Error(_("No layers to group"));

	// First create the new canvas and layer
	if(!child_canvas)
		child_canvas=Canvas::create_inline(get_canvas());

	Layer::Handle new_layer(Layer::create("filter_group"));

	if (!description.empty()) new_layer->set_description(description);
	new_layer->set_param("canvas",child_canvas);

	int target_depth(lowest_depth());

	// Add the layer
	{
		Action::Handle action(LayerAdd::create());

		action->set_param("canvas",get_canvas());
		action->set_param("canvas_interface",get_canvas_interface());
		action->set_param("new",new_layer);

		add_action(action);
	}

	// Move the layer
	{
		Action::Handle action(Action::create("LayerMove"));

		assert(action);

		action->set_param("canvas",get_canvas());
		action->set_param("canvas_interface",get_canvas_interface());
		action->set_param("layer",new_layer);
		action->set_param("new_index",target_depth);

		add_action(action);
	}

	std::list<synfig::Layer::Handle>::reverse_iterator i;

	for(i=layers.rbegin();i!=layers.rend();++i)
	{
		Layer::Handle layer(*i);

		Canvas::Handle subcanvas(layer->get_canvas());

		// Find the iterator for the layer
		Canvas::iterator iter=find(subcanvas->begin(),subcanvas->end(),layer);

		// If we couldn't find the layer in the canvas, then bail
		if(*iter!=layer)
			throw Error(_("This layer doesn't exist anymore."));

		if(!subcanvas)
			throw Error(_("This layer doesn't have a parent canvas"));

		// If the subcanvas isn't the same as the canvas,
		// then it had better be an inline canvas. If not,
		// bail
		if(get_canvas()!=subcanvas && !subcanvas->is_inline())
			throw Error(_("This layer doesn't belong to this canvas anymore"));

		if(get_canvas()!=subcanvas)
			throw Error(_("get_canvas()!=subcanvas"));

		// Remove the layer from the old canvas
		{
			Action::Handle action(LayerRemove::create());

			action->set_param("canvas",subcanvas);
			action->set_param("canvas_interface",get_canvas_interface());
			action->set_param("layer",layer);

			add_action(action);
		}
		// Add the layer to the new canvas
		{
			Action::Handle action(LayerAdd::create());

			action->set_param("canvas",child_canvas);
			action->set_param("canvas_interface",get_canvas_interface());
			action->set_param("new",layer);

			add_action(action);
		}
	}
}
