/* === S I N F G =========================================================== */
/*!	\file layerduplicate.cpp
**	\brief Template File
**
**	$Id: layerduplicate.cpp,v 1.1.1.1 2005/01/07 03:34:37 darco Exp $
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

#include "layerduplicate.h"
#include "layeradd.h"
#include <sinfgapp/canvasinterface.h>

#endif

using namespace std;
using namespace etl;
using namespace sinfg;
using namespace sinfgapp;
using namespace Action;

/* === M A C R O S ========================================================= */

ACTION_INIT(Action::LayerDuplicate);
ACTION_SET_NAME(Action::LayerDuplicate,"layer_duplicate");
ACTION_SET_LOCAL_NAME(Action::LayerDuplicate,"Duplicate Layer");
ACTION_SET_TASK(Action::LayerDuplicate,"duplicate");
ACTION_SET_CATEGORY(Action::LayerDuplicate,Action::CATEGORY_LAYER);
ACTION_SET_PRIORITY(Action::LayerDuplicate,0);
ACTION_SET_VERSION(Action::LayerDuplicate,"0.0");
ACTION_SET_CVS_ID(Action::LayerDuplicate,"$Id: layerduplicate.cpp,v 1.1.1.1 2005/01/07 03:34:37 darco Exp $");

/* === G L O B A L S ======================================================= */

/* === P R O C E D U R E S ================================================= */

/* === M E T H O D S ======================================================= */

Action::LayerDuplicate::LayerDuplicate()
{
}

Action::ParamVocab
Action::LayerDuplicate::get_param_vocab()
{
	ParamVocab ret(Action::CanvasSpecific::get_param_vocab());
	
	ret.push_back(ParamDesc("layer",Param::TYPE_LAYER)
		.set_local_name(_("Layer"))
		.set_desc(_("Layer to be duplicated"))
		.set_supports_multiple()
	);
	
	return ret;
}

bool
Action::LayerDuplicate::is_canidate(const ParamList &x)
{
	return canidate_check(get_param_vocab(),x);
}

bool
Action::LayerDuplicate::set_param(const sinfg::String& name, const Action::Param &param)
{
	if(name=="layer" && param.get_type()==Param::TYPE_LAYER)
	{
		layers.push_back(param.get_layer());
		
		return true;
	}

	return Action::CanvasSpecific::set_param(name,param);
}

bool
Action::LayerDuplicate::is_ready()const
{
	if(layers.empty())
		return false;
	return Action::CanvasSpecific::is_ready();
}

void
Action::LayerDuplicate::prepare()
{
	if(!first_time())
		return;
	
	std::list<sinfg::Layer::Handle>::const_iterator iter;
	
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
		if(get_canvas()!=subcanvas && !subcanvas->is_inline())
			throw Error(_("This layer doesn't belong to this canvas anymore"));
		
		Layer::Handle new_layer(layer->clone(guid));
		
		{
			Action::Handle action(Action::create("layer_move"));
			
			action->set_param("canvas",subcanvas);
			action->set_param("canvas_interface",get_canvas_interface());
			action->set_param("layer",new_layer);
			action->set_param("new_index",layers.front()->get_depth());
			
			add_action_front(action);
		}
		{
			Action::Handle action(Action::create("layer_add"));
		
			action->set_param("canvas",subcanvas);
			action->set_param("canvas_interface",get_canvas_interface());
			action->set_param("new",new_layer);
			
			add_action_front(action);
		}
	}
}
