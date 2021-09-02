/* === S Y N F I G ========================================================= */
/*!	\file template.cpp
**	\brief LayerFit
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

#include <synfig/general.h>

#include "layerfit.h"
#include <synfigapp/canvasinterface.h>
#include <synfigapp/localization.h>

#endif

/* === U S I N G =========================================================== */

using namespace etl;
using namespace synfig;
using namespace synfigapp;
using namespace Action;

/* === M A C R O S ========================================================= */

ACTION_INIT(Action::LayerFit);
ACTION_SET_NAME(Action::LayerFit,"LayerFit");
ACTION_SET_LOCAL_NAME(Action::LayerFit,N_("Fit image"));
ACTION_SET_TASK(Action::LayerFit,"fit");
ACTION_SET_CATEGORY(Action::LayerFit,Action::CATEGORY_LAYER);
ACTION_SET_PRIORITY(Action::LayerFit,0);
ACTION_SET_VERSION(Action::LayerFit,"0.0");

/* === G L O B A L S ======================================================= */

/* === P R O C E D U R E S ================================================= */

/* === M E T H O D S ======================================================= */

Action::LayerFit::LayerFit()
{
}

Action::ParamVocab
Action::LayerFit::get_param_vocab()
{
	ParamVocab ret(Action::CanvasSpecific::get_param_vocab());

	ret.push_back(ParamDesc("layer",Param::TYPE_LAYER)
		.set_local_name(_("Layer"))
	);

	return ret;
}

bool
Action::LayerFit::is_candidate(const ParamList &x)
{
	if (!candidate_check(get_param_vocab(),x))
		return false;
	for(ParamList::const_iterator i = x.begin(); i != x.end(); i++) {
		if (i->first == "layer") {
			if (i->second.get_type() != Param::TYPE_LAYER) return false;
			const Layer::Handle layer = i->second.get_layer();
			if (layer.empty()
			 || layer->get_param("tl").empty()
			 || layer->get_param("br").empty())
				return false;
		}
	}
	return true;
}

bool
Action::LayerFit::set_param(const synfig::String& name, const Action::Param &param)
{
	if(name=="layer" && param.get_type()==Param::TYPE_LAYER)
	{
		layer=param.get_layer();
		return true;
	}

	return Action::CanvasSpecific::set_param(name,param);
}

bool
Action::LayerFit::is_ready()const
{
	if(!layer)
		return false;
	return Action::CanvasSpecific::is_ready();
}

void
Action::LayerFit::perform()
{
	Canvas::Handle subcanvas(layer->get_canvas());

	// Find the iterator for the layer
	Canvas::iterator iter=find(subcanvas->begin(),subcanvas->end(),layer);

	// If we couldn't find the layer in the canvas, then bail
	if(*iter!=layer)
		throw Error(_("This layer doesn't exist anymore."));

	if (layer->dynamic_param_list().count("tl") > 0
	 || layer->dynamic_param_list().count("br") > 0)
		throw Error(_("You cannot fit animated layers"));

	// remember values
	prev_tl = layer->get_param("tl");
	prev_br = layer->get_param("br");

	set_dirty();

	// new coordinates
	Vector size = get_canvas()->rend_desc().get_br()-get_canvas()->rend_desc().get_tl();
	ValueBase new_tl(Point(size*(-0.5f)));
	ValueBase new_br(Point(size*0.5f));

	// recalculate coordinates to keep proportions
	int w = layer->get_param("_width").get(int());
	int h = layer->get_param("_height").get(int());
	if(w > 0 && h > 0)
	{
		Vector x;

		if(abs(size[0])<abs(size[1]))	// if canvas is tall and thin
		{
			x[0]=size[0];	// use full width
			x[1]=size[0]/w*h; // and scale for height
			if((size[0]<0) ^ (size[1]<0))
				x[1]=-x[1];
		}
		else				// else canvas is short and fat (or maybe square)
		{
			x[1]=size[1];	// use full height
			x[0]=size[1]/h*w; // and scale for width
			if((size[0]<0) ^ (size[1]<0))
				x[0]=-x[0];
		}

		new_tl = -x/2;
		new_br = x/2;
	}

	layer->set_param("tl", new_tl);
	layer->set_param("br", new_br);
	layer->changed();

	if(get_canvas_interface())
	{
		get_canvas_interface()->signal_layer_param_changed()(layer, "tl");
		get_canvas_interface()->signal_layer_param_changed()(layer, "br");
	}
	else synfig::warning("CanvasInterface not set on action");
}

void
Action::LayerFit::undo()
{
	set_dirty();

	// restore the old state
	layer->set_param("tl", prev_tl);
	layer->set_param("br", prev_br);
	layer->changed();

	if(get_canvas_interface())
	{
		get_canvas_interface()->signal_layer_param_changed()(layer, "tl");
		get_canvas_interface()->signal_layer_param_changed()(layer, "br");
	}
	else synfig::warning("CanvasInterface not set on action");
}

/* === E N T R Y P O I N T ================================================= */


