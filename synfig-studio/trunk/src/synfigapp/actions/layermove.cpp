/* === S Y N F I G ========================================================= */
/*!	\file layermove.cpp
**	\brief Template File
**
**	$Id: layermove.cpp,v 1.1.1.1 2005/01/07 03:34:37 darco Exp $
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

#include "layermove.h"
#include <synfigapp/canvasinterface.h>

#endif

using namespace std;
using namespace etl;
using namespace synfig;
using namespace synfigapp;
using namespace Action;

/* === M A C R O S ========================================================= */

ACTION_INIT(Action::LayerMove);
ACTION_SET_NAME(Action::LayerMove,"layer_move");
ACTION_SET_LOCAL_NAME(Action::LayerMove,_("Move Layer"));
ACTION_SET_TASK(Action::LayerMove,"move");
ACTION_SET_CATEGORY(Action::LayerMove,Action::CATEGORY_LAYER);
ACTION_SET_PRIORITY(Action::LayerMove,0);
ACTION_SET_VERSION(Action::LayerMove,"0.0");
ACTION_SET_CVS_ID(Action::LayerMove,"$Id: layermove.cpp,v 1.1.1.1 2005/01/07 03:34:37 darco Exp $");

/* === G L O B A L S ======================================================= */

static const int nindex=-1;

/* === P R O C E D U R E S ================================================= */

/* === M E T H O D S ======================================================= */

Action::LayerMove::LayerMove():
	new_index(0xdeadbeef)
{
}

Action::ParamVocab
Action::LayerMove::get_param_vocab()
{
	ParamVocab ret(Action::CanvasSpecific::get_param_vocab());
	
	ret.push_back(ParamDesc("layer",Param::TYPE_LAYER)
		.set_local_name(_("Layer"))
		.set_desc(_("Layer to be moved"))
	);

	ret.push_back(ParamDesc("new_index",Param::TYPE_INTEGER)
		.set_local_name(_("New Index"))
		.set_desc(_("Where the layer is to be moved to"))
	);

	ret.push_back(ParamDesc("dest_canvas",Param::TYPE_CANVAS)
		.set_local_name(_("Destination Canvas"))
		.set_desc(_("The canvas the layer is to be moved to"))
		.set_optional()
	);
	
	return ret;
}

bool
Action::LayerMove::is_canidate(const ParamList &x)
{
	return canidate_check(get_param_vocab(),x);
}

bool
Action::LayerMove::set_param(const synfig::String& name, const Action::Param &param)
{
	if(name=="layer" && param.get_type()==Param::TYPE_LAYER)
	{

		layer=param.get_layer();
		
		return true;
	}

	if(name=="new_index" && param.get_type()==Param::TYPE_INTEGER)
	{
		new_index=param.get_integer();
		
		return true;
	}

	if(name=="dest_canvas" && param.get_type()==Param::TYPE_CANVAS)
	{
		dest_canvas=param.get_canvas();
		
		return true;
	}

	return Action::CanvasSpecific::set_param(name,param);
}

bool
Action::LayerMove::is_ready()const
{
	synfig::info(__FILE__":%d: layer->count()=%d",__LINE__,layer.count());
	if(!layer || (unsigned)new_index==0xdeadbeef)
		return false;
	return Action::CanvasSpecific::is_ready();
}

void
Action::LayerMove::perform()
{		
	synfig::info(__FILE__":%d: layer->count()=%d",__LINE__,layer.count());

	Canvas::Handle subcanvas(layer->get_canvas());
	src_canvas=subcanvas;
	if(!dest_canvas)
		dest_canvas=subcanvas;
		
	// Find the iterator for the layer
	Canvas::iterator iter=find(src_canvas->begin(),src_canvas->end(),layer);
	
	// If we couldn't find the layer in the canvas, then bail
	if(*iter!=layer)
		throw Error(_("This layer doesn't exist anymore."));

	synfig::info(__FILE__":%d: layer->count()=%d",__LINE__,layer.count());

	// If the subcanvas isn't the same as the canvas,
	// then it had better be an inline canvas. If not,
	// bail
	//if(get_canvas()!=subcanvas && !subcanvas->is_inline())
	if(get_canvas()->get_root()!=dest_canvas->get_root() || get_canvas()->get_root()!=src_canvas->get_root())
		throw Error(_("You cannot directly move layers across compositions"));
	
	old_index=iter-src_canvas->begin();
	int depth;
	
	if(new_index<0)
		depth=dest_canvas->size()+new_index+1;
	else
		depth=new_index;
	
	set_dirty(layer->active());

	synfig::info(__FILE__":%d: layer->count()=%d",__LINE__,layer.count());

	// If we were to move it to where it is
	if(old_index==depth && src_canvas==dest_canvas)
		return;
	
	if(depth>dest_canvas->size())
		depth=dest_canvas->size();
	if(depth<0)
		depth=0;
		
	src_canvas->erase(iter);
	
	dest_canvas->insert(dest_canvas->begin()+depth,layer);		
	layer->set_canvas(dest_canvas);
	
	layer->changed();
	dest_canvas->changed(); if(dest_canvas!=src_canvas) src_canvas->changed();
	
	synfig::info(__FILE__":%d: layer->count()=%d",__LINE__,layer.count());
	
	if(get_canvas_interface())
	{
		if(src_canvas==dest_canvas)
		{
			if(new_index==old_index-1)	// Raise
				get_canvas_interface()->signal_layer_raised()(layer);
			else if(new_index==old_index+1)	// Lower
				get_canvas_interface()->signal_layer_lowered()(layer);
			else		// Moved
			{
				get_canvas_interface()->signal_layer_moved()(layer,depth,dest_canvas);
			}
		}
		else
		{
			get_canvas_interface()->signal_layer_moved()(layer,depth,dest_canvas);
		}
	}
	else synfig::warning("CanvasInterface not set on action");

	synfig::info(__FILE__":%d: layer->count()=%d",__LINE__,layer.count());
}

void
Action::LayerMove::undo()
{
	// Find the iterator for the layer
	Canvas::iterator iter=find(dest_canvas->begin(),dest_canvas->end(),layer);
	
	// If we couldn't find the layer in the canvas, then bail
	if(*iter!=layer || (get_canvas()!=dest_canvas && !dest_canvas->is_inline()))
		throw Error(_("This layer doesn't exist anymore."));
	
	// If we were to move it to where it is
	if(old_index==new_index && src_canvas==dest_canvas)
		return;

	// Mark ourselves as dirty if necessary
	set_dirty(layer->active());

	dest_canvas->erase(iter);
	
	src_canvas->insert(src_canvas->begin()+old_index,layer);
	layer->set_canvas(src_canvas);

	layer->changed();
	dest_canvas->changed(); if(dest_canvas!=src_canvas) src_canvas->changed();
	
	// Execute any signals
	if(get_canvas_interface())
	{
		if(src_canvas==dest_canvas)
		{
			if(new_index==old_index+1)	// Raise
				get_canvas_interface()->signal_layer_raised()(layer);
			else if(new_index==old_index-1)	// Lower
				get_canvas_interface()->signal_layer_lowered()(layer);
			else		// Moved
			{
			get_canvas_interface()->signal_layer_moved()(layer,old_index,src_canvas);
			}
		}
		else
		{
			get_canvas_interface()->signal_layer_moved()(layer,old_index,src_canvas);
			//get_canvas_interface()->signal_layer_removed()(layer);	
			//get_canvas_interface()->signal_layer_inserted()(layer,old_index);
		}
	}
	else synfig::warning("CanvasInterface not set on action");
}
