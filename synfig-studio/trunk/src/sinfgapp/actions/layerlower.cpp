/* === S I N F G =========================================================== */
/*!	\file layerlower.cpp
**	\brief Template File
**
**	$Id: layerlower.cpp,v 1.1.1.1 2005/01/07 03:34:37 darco Exp $
**
**	\legal
**	Copyright (c) 2002 Robert B. Quattlebaum Jr.
**
**	This software and associated documentation
**	are CONFIDENTIAL and PROPRIETARY property of
**	the above-mentioned copyright holder.
**
**	You may not copy, print, publish, or in any
**	other way distribute this software without
**	a prior written agreement with
**	the copyright holder.
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

#include "layerlower.h"
#include "layermove.h"
#include <sinfgapp/canvasinterface.h>

#endif

using namespace std;
using namespace etl;
using namespace sinfg;
using namespace sinfgapp;
using namespace Action;

/* === M A C R O S ========================================================= */

ACTION_INIT(Action::LayerLower);
ACTION_SET_NAME(Action::LayerLower,"layer_lower");
ACTION_SET_LOCAL_NAME(Action::LayerLower,"Lower Layer");
ACTION_SET_TASK(Action::LayerLower,"lower");
ACTION_SET_CATEGORY(Action::LayerLower,Action::CATEGORY_LAYER);
ACTION_SET_PRIORITY(Action::LayerLower,10);
ACTION_SET_VERSION(Action::LayerLower,"0.0");
ACTION_SET_CVS_ID(Action::LayerLower,"$Id: layerlower.cpp,v 1.1.1.1 2005/01/07 03:34:37 darco Exp $");

/* === G L O B A L S ======================================================= */

/* === P R O C E D U R E S ================================================= */

/* === M E T H O D S ======================================================= */

Action::LayerLower::LayerLower()
{
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
Action::LayerLower::is_canidate(const ParamList &x)
{
	if(!canidate_check(get_param_vocab(),x))
		return false;
	
	Layer::Handle layer(x.find("layer")->second.get_layer());
	//sinfg::info("layer->get_depth()= %d ; layer->get_canvas()->size()=%d ;",layer->get_depth(),layer->get_canvas()->size());
	if(layer->get_depth()+1>=layer->get_canvas()->size())
		return false;
	return true;
}

bool
Action::LayerLower::set_param(const sinfg::String& name, const Action::Param &param)
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
	std::list<sinfg::Layer::Handle>::const_iterator iter;

	clear();
	
	for(iter=layers.begin();iter!=layers.end();++iter)
	{
		Layer::Handle layer(*iter);
		
		Canvas::Handle subcanvas(layer->get_canvas());
		
		// Find the iterator for the layer
		Canvas::iterator iter=find(subcanvas->begin(),subcanvas->end(),layer);
		
		// If we couldn't find the layer in the canvas, then bail
		if(*iter!=layer)
			throw Error(_("This layer doesn't exist anymore."));
	
		// If the subcanvas isn't the same as the canvas,
		// then it had better be an inline canvas. If not,
		// bail
		//if(get_canvas()!=subcanvas && !subcanvas->is_inline())
		//	throw Error(_("This layer doesn't belong to this canvas anymore"));
		
		int new_index=iter-subcanvas->begin();
				
		new_index++;
		
		// If this lowers the layer past the bottom then don't bother
		if(new_index==subcanvas->size())
			continue;
		
		Action::Handle layer_move(LayerMove::create());
		
		layer_move->set_param("canvas",get_canvas());
		layer_move->set_param("canvas_interface",get_canvas_interface());
		layer_move->set_param("layer",layer);
		layer_move->set_param("new_index",new_index);
		
		add_action_front(layer_move);
	}
}
