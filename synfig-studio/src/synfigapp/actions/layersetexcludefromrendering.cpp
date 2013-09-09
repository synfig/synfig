/* === S Y N F I G ========================================================= */
/*!	\file layersetexcludefromrendering.cpp
**	\brief Template File
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

#include "layersetexcludefromrendering.h"
#include <synfigapp/canvasinterface.h>

#include <synfigapp/general.h>

#endif

using namespace std;
using namespace etl;
using namespace synfig;
using namespace synfigapp;
using namespace Action;

/* === M A C R O S ========================================================= */

ACTION_INIT_NO_GET_LOCAL_NAME(Action::LayerSetExcludeFromRendering);
ACTION_SET_NAME(Action::LayerSetExcludeFromRendering,"LayerSetExcludeFromRendering");
ACTION_SET_LOCAL_NAME(Action::LayerSetExcludeFromRendering,N_("Toggle Exclude from Rendering"));
ACTION_SET_TASK(Action::LayerSetExcludeFromRendering,"setexcludefromrendering");
ACTION_SET_CATEGORY(Action::LayerSetExcludeFromRendering,Action::CATEGORY_LAYER);
ACTION_SET_PRIORITY(Action::LayerSetExcludeFromRendering,0);
ACTION_SET_VERSION(Action::LayerSetExcludeFromRendering,"0.0");
ACTION_SET_CVS_ID(Action::LayerSetExcludeFromRendering,"$Id$");

/* === G L O B A L S ======================================================= */

/* === P R O C E D U R E S ================================================= */

/* === M E T H O D S ======================================================= */

Action::LayerSetExcludeFromRendering::LayerSetExcludeFromRendering():
	new_state_set(false),
	old_state(false),
	new_state(false)
{
}

synfig::String
Action::LayerSetExcludeFromRendering::get_local_name()const
{
	if(!layer)
		return _("Toggle Exclude from Rendering");

	return strprintf("%s '%s'",
					 new_state
					 ? _("Exclude from Rendering")
					 : _("Enable Rendering of"),
					 layer->get_non_empty_description().c_str());
}

Action::ParamVocab
Action::LayerSetExcludeFromRendering::get_param_vocab()
{
	ParamVocab ret(Action::CanvasSpecific::get_param_vocab());

	ret.push_back(ParamDesc("layer",Param::TYPE_LAYER)
		.set_local_name(_("Layer"))
	);

	ret.push_back(ParamDesc("new_state",Param::TYPE_BOOL)
		.set_local_name(_("New State"))
		.set_desc(_("The new state of the layer exclusion"))
		.set_optional(true)
	);

	return ret;
}

bool
Action::LayerSetExcludeFromRendering::is_candidate(const ParamList &x)
{
	return candidate_check(get_param_vocab(),x);
}

bool
Action::LayerSetExcludeFromRendering::set_param(const synfig::String& name, const Action::Param &param)
{
	if(name=="layer" && param.get_type()==Param::TYPE_LAYER)
	{
		layer=param.get_layer();
		if (layer && !new_state_set)
			new_state=!layer->get_exclude_from_rendering();
		return true;
	}

	if(name=="new_state" && param.get_type()==Param::TYPE_BOOL)
	{
		new_state=param.get_bool();
		new_state_set=true;
		return true;
	}

	return Action::CanvasSpecific::set_param(name,param);
}

bool
Action::LayerSetExcludeFromRendering::is_ready()const
{
	if(!layer)
		return false;
	return Action::CanvasSpecific::is_ready();
}

void
Action::LayerSetExcludeFromRendering::perform()
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

	old_state=layer->get_exclude_from_rendering();

	// If we are changing the state to what it already is,
	// the go ahead and return
	if(new_state==old_state)
	{
		set_dirty(false);
		return;
	}
	else
		set_dirty();

	layer->set_exclude_from_rendering(new_state);

	if(get_canvas_interface())
	{
		get_canvas_interface()->signal_layer_exclude_from_rendering_changed()(layer,new_state);
	}
	else synfig::warning("CanvasInterface not set on action");
}

void
Action::LayerSetExcludeFromRendering::undo()
{
	// If we are changing the state to what it already is,
	// the go ahead and return
	if(new_state==old_state)
	{
		set_dirty(false);
		return;
	}
	else
		set_dirty();

	// restore the old state
	layer->set_exclude_from_rendering(old_state);

	if(get_canvas_interface())
	{
		get_canvas_interface()->signal_layer_exclude_from_rendering_changed()(layer,old_state);
	}
	else synfig::warning("CanvasInterface not set on action");
}
