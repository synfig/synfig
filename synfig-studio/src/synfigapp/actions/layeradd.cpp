/* === S Y N F I G ========================================================= */
/*!	\file layeradd.cpp
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

#include "layeradd.h"
#include <synfigapp/canvasinterface.h>
#include <synfigapp/localization.h>
#include <synfig/layers/layer_pastecanvas.h>
#include <synfig/valuenodes/valuenode_composite.h>

#endif

using namespace std;
using namespace etl;
using namespace synfig;
using namespace synfigapp;
using namespace Action;

/* === M A C R O S ========================================================= */

ACTION_INIT_NO_GET_LOCAL_NAME(Action::LayerAdd);
ACTION_SET_NAME(Action::LayerAdd,"LayerAdd");
ACTION_SET_LOCAL_NAME(Action::LayerAdd,N_("Add Layer"));
ACTION_SET_TASK(Action::LayerAdd,"add");
ACTION_SET_CATEGORY(Action::LayerAdd,Action::CATEGORY_LAYER);
ACTION_SET_PRIORITY(Action::LayerAdd,0);
ACTION_SET_VERSION(Action::LayerAdd,"0.0");

/* === G L O B A L S ======================================================= */

/* === P R O C E D U R E S ================================================= */

/* === M E T H O D S ======================================================= */

Action::LayerAdd::LayerAdd()
{
}

synfig::String
Action::LayerAdd::get_local_name()const
{
	if (layer)
		return strprintf("%s '%s'", _("Add Layer"), layer->get_local_name().c_str());
	else
		return _("Add Layer");
}

Action::ParamVocab
Action::LayerAdd::get_param_vocab()
{
	ParamVocab ret(Action::CanvasSpecific::get_param_vocab());

	ret.push_back(ParamDesc("new",Param::TYPE_LAYER)
		.set_local_name(_("New Layer"))
		.set_desc(_("Layer to be added"))
	);

	return ret;
}

bool
Action::LayerAdd::is_candidate(const ParamList &x)
{
	return candidate_check(get_param_vocab(),x);
}

bool
Action::LayerAdd::set_param(const synfig::String& name, const Action::Param &param)
{
	if(name=="new" && param.get_type()==Param::TYPE_LAYER)
	{
		layer=param.get_layer();

		return true;
	}

	return Action::CanvasSpecific::set_param(name,param);
}

bool
Action::LayerAdd::is_ready()const
{
	if(!layer)
		return false;
	return Action::CanvasSpecific::is_ready();
}

void
Action::LayerAdd::perform()
{
	// Set the layer's canvas
	layer->set_canvas(get_canvas());

	// Push the layer onto the front of the canvas
	get_canvas()->push_front(layer);

	// Mark ourselves as dirty if necessary
	//set_dirty(layer->active());

	if (etl::handle<Layer_PasteCanvas>::cast_dynamic(layer)
	 && layer->dynamic_param_list().count("transformation") == 0)
			layer->connect_dynamic_param("transformation",
				ValueNode_Composite::create(
					layer->get_param("transformation"),
					get_canvas() ));

	// Signal that a layer has been inserted
	if(get_canvas_interface())
	{
		get_canvas_interface()->signal_layer_inserted()(layer,0);
	}
	else synfig::warning("CanvasInterface not set on action");
}

void
Action::LayerAdd::undo()
{
	// Find the iterator for the layer
	Canvas::iterator iter=find(get_canvas()->begin(),get_canvas()->end(),layer);

	// If we couldn't find the layer in the canvas, then bail
	if(*iter!=layer)
		throw Error(_("This layer doesn't exist anymore."));

	// Remove the layer from the canvas
	get_canvas()->erase(iter);

	// Mark ourselves as dirty if necessary
	//set_dirty(layer->active());

	// Signal that a layer has been inserted
	if(get_canvas_interface())
	{
		get_canvas_interface()->signal_layer_removed()(layer);
	}
	else synfig::warning("CanvasInterface not set on action");
}
