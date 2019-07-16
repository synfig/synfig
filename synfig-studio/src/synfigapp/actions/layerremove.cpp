/* === S Y N F I G ========================================================= */
/*!	\file layerremove.cpp
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

#include "layerremove.h"
#include <synfigapp/canvasinterface.h>

#include <synfigapp/localization.h>

#endif

using namespace std;
using namespace etl;
using namespace synfig;
using namespace synfigapp;
using namespace Action;

/* === M A C R O S ========================================================= */

ACTION_INIT_NO_GET_LOCAL_NAME(Action::LayerRemove);
ACTION_SET_NAME(Action::LayerRemove,"LayerRemove");
ACTION_SET_LOCAL_NAME(Action::LayerRemove,N_("Delete Layer"));
ACTION_SET_TASK(Action::LayerRemove,"remove");
ACTION_SET_CATEGORY(Action::LayerRemove,Action::CATEGORY_LAYER);
ACTION_SET_PRIORITY(Action::LayerRemove,0);
ACTION_SET_VERSION(Action::LayerRemove,"0.0");
ACTION_SET_CVS_ID(Action::LayerRemove,"$Id$");

/* === G L O B A L S ======================================================= */

/* === P R O C E D U R E S ================================================= */

/* === M E T H O D S ======================================================= */

Action::LayerRemove::LayerRemove()
{
}

synfig::String
Action::LayerRemove::get_local_name()const
{
	return get_layer_descriptions(layer_list, _("Delete Layer"), _("Delete Layers"));
}

Action::ParamVocab
Action::LayerRemove::get_param_vocab()
{
	ParamVocab ret(Action::CanvasSpecific::get_param_vocab());

	ret.push_back(ParamDesc("layer",Param::TYPE_LAYER)
		.set_local_name(_("Layer"))
		.set_desc(_("Layer to be deleted"))
		.set_supports_multiple()
	);

	return ret;
}

bool
Action::LayerRemove::is_candidate(const ParamList &x)
{
	return candidate_check(get_param_vocab(),x);
}

bool
Action::LayerRemove::set_param(const synfig::String& name, const Action::Param &param)
{
	if(name=="layer" && param.get_type()==Param::TYPE_LAYER)
	{
		std::pair<synfig::Layer::Handle,int> layer_pair;
		layer_pair.first=param.get_layer();
		layer_list.push_back(layer_pair);

		return true;
	}

	return Action::CanvasSpecific::set_param(name,param);
}

bool
Action::LayerRemove::is_ready()const
{
	if(layer_list.empty())
		return false;
	return Action::CanvasSpecific::is_ready();
}

void
Action::LayerRemove::perform()
{
	std::list<std::pair<synfig::Layer::Handle,int> >::iterator iter;
	for(iter=layer_list.begin();iter!=layer_list.end();++iter)
	{
		Layer::Handle layer(iter->first);
//		int& depth(iter->second);
		Canvas::Handle subcanvas(layer->get_canvas());

		// Find the iterator for the layer
		Canvas::iterator iter2=find(subcanvas->begin(),subcanvas->end(),layer);

		// If we couldn't find the layer in the canvas, then bail
		if(*iter2!=layer)
		{
			/*!	\todo We should really undo all prior removals
			**	before we go throwing shit around */
			throw Error(_("This layer doesn't exist anymore."));
		}

		// If the subcanvas isn't the same as the canvas,
		// then it had better be an inline canvas. If not,
		// bail
		if(get_canvas()!=subcanvas && !subcanvas->is_inline())
		{
			/*!	\todo We should really undo all prior removals
			**	before we go throwing shit around */
			throw Error(_("This layer doesn't belong to this canvas anymore"));
		}

		set_canvas(subcanvas);

		// Calculate the depth that the layer was at (For the undo)
		iter->second=layer->get_depth();

		// Mark ourselves as dirty if necessary
		set_dirty(layer->active());

		// Remove the layer from the canvas
		subcanvas->erase(iter2);

		// Signal that a layer has been removed
		if(get_canvas_interface())
			get_canvas_interface()->signal_layer_removed()(layer);
	}
}

void
Action::LayerRemove::undo()
{
	std::list<std::pair<synfig::Layer::Handle,int> >::reverse_iterator iter;
	for(iter=layer_list.rbegin();iter!=layer_list.rend();++iter)
	{
		Layer::Handle layer(iter->first);
		int& depth(iter->second);

		// Set the layer's canvas
		layer->set_canvas(get_canvas());

		// Make sure that the depth is valid
		if(get_canvas()->size()<depth)
			depth=get_canvas()->size();

		// Mark ourselves as dirty if necessary
		set_dirty(layer->active());

		// Insert the layer into the canvas at the desired depth
		get_canvas()->insert(get_canvas()->byindex(depth), layer);

		// Signal that a layer has been inserted
		if(get_canvas_interface())
			get_canvas_interface()->signal_layer_inserted()(layer,depth);
	}
}
