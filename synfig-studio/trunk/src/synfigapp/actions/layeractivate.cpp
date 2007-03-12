/* === S Y N F I G ========================================================= */
/*!	\file layeractivate.cpp
**	\brief Template File
**
**	$Id: layeractivate.cpp,v 1.1.1.1 2005/01/07 03:34:37 darco Exp $
**
**	\legal
**	Copyright (c) 2002-2005 Robert B. Quattlebaum Jr., Adrian Bentley
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

#include "layeractivate.h"
#include <synfigapp/canvasinterface.h>

#endif

using namespace std;
using namespace etl;
using namespace synfig;
using namespace synfigapp;
using namespace Action;

/* === M A C R O S ========================================================= */
#define ACTION_INIT2(class) \
	Action::Base* class::create() { return new class(); }	\
	synfig::String class::get_name()const { return name__; }

ACTION_INIT2(Action::LayerActivate);
ACTION_SET_NAME(Action::LayerActivate,"layer_activate");
ACTION_SET_LOCAL_NAME(Action::LayerActivate,_("Activate Layer"));
ACTION_SET_TASK(Action::LayerActivate,"activate");
ACTION_SET_CATEGORY(Action::LayerActivate,Action::CATEGORY_LAYER);
ACTION_SET_PRIORITY(Action::LayerActivate,0);
ACTION_SET_VERSION(Action::LayerActivate,"0.0");
ACTION_SET_CVS_ID(Action::LayerActivate,"$Id: layeractivate.cpp,v 1.1.1.1 2005/01/07 03:34:37 darco Exp $");

/* === G L O B A L S ======================================================= */

/* === P R O C E D U R E S ================================================= */

/* === M E T H O D S ======================================================= */

Action::LayerActivate::LayerActivate()
{
}

synfig::String
Action::LayerActivate::get_local_name()const
{
	if(!layer)
		return _("Activate Layer");
	String name;
	if(layer->get_description().empty())
		name=layer->get_local_name();
	else
		name=layer->get_description();

	return (new_status?_("Activate "):_("Deactivate "))+name;
}

Action::ParamVocab
Action::LayerActivate::get_param_vocab()
{
	ParamVocab ret(Action::CanvasSpecific::get_param_vocab());

	ret.push_back(ParamDesc("layer",Param::TYPE_LAYER)
		.set_local_name(_("Layer"))
	);

	ret.push_back(ParamDesc("new_status",Param::TYPE_BOOL)
		.set_local_name(_("New Status"))
		.set_desc(_("The new status of the layer"))
	);

	return ret;
}

bool
Action::LayerActivate::is_candidate(const ParamList &x)
{
	return candidate_check(get_param_vocab(),x);
}

bool
Action::LayerActivate::set_param(const synfig::String& name, const Action::Param &param)
{
	if(name=="layer" && param.get_type()==Param::TYPE_LAYER)
	{
		layer=param.get_layer();

		return true;
	}

	if(name=="new_status" && param.get_type()==Param::TYPE_BOOL)
	{
		new_status=param.get_bool();

		return true;
	}

	return Action::CanvasSpecific::set_param(name,param);
}

bool
Action::LayerActivate::is_ready()const
{
	if(!layer)
		return false;
	return Action::CanvasSpecific::is_ready();
}

void
Action::LayerActivate::perform()
{
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
	//if(get_canvas()->get_root()!=subcanvas->get_root())
	//	throw Error(_("This layer doesn't belong to this composition"));

	old_status=layer->active();

	// If we are changing the status to what it already is,
	// the go ahead and return
	if(new_status==old_status)
	{
		set_dirty(false);
		return;
	}
	else
		set_dirty();

	if(new_status)
		layer->enable();
	else
		layer->disable();

	if(get_canvas_interface())
	{
		get_canvas_interface()->signal_layer_status_changed()(layer,new_status);
	}
	else synfig::warning("CanvasInterface not set on action");
}

void
Action::LayerActivate::undo()
{
	// If we are changing the status to what it already is,
	// the go ahead and return
	if(new_status==old_status)
	{
		set_dirty(false);
		return;
	}
	else
		set_dirty();

	// restore the old status
	if(old_status)
		layer->enable();
	else
		layer->disable();

	if(get_canvas_interface())
	{
		get_canvas_interface()->signal_layer_status_changed()(layer,old_status);
	}
	else synfig::warning("CanvasInterface not set on action");
}
