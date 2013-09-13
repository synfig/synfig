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

#include "layerfit.h"
#include <synfigapp/canvasinterface.h>
#include <synfigapp/general.h>

#endif

/* === U S I N G =========================================================== */

using namespace std;
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
ACTION_SET_CVS_ID(Action::LayerFit,"$Id$");

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
	for(ParamList::const_iterator i = x.begin(); i != x.end(); i++) {
		if (i->first == "layer") {
			if (i->second.get_type() != Param::TYPE_LAYER) return false;
			const Layer::Handle layer = i->second.get_layer();
			if (layer.empty()
			 || layer->get_param("tl").empty()
			 || layer->get_param("br").empty()) return false;
		}
	}
	return candidate_check(get_param_vocab(),x);
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

	prev_tl = layer->get_param("tl");
	prev_br = layer->get_param("br");

	set_dirty();

	Point canvas_tl = subcanvas->rend_desc().get_br()-get_canvas()->rend_desc().get_tl();
	//Point canvas_br = subcanvas->rend_desc().get_br()-get_canvas()->rend_desc().get_br();
	ValueBase new_tl(Point(canvas_tl*(-0.5f)));
	ValueBase new_br(Point(canvas_tl*0.5f));

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


