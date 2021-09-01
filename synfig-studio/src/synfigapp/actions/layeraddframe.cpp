/* === S Y N F I G ========================================================= */
/*!	\file layeraddframe.cpp
**	\brief Template File
**
**	$Id$
**
**	\legal
**	Copyright (c) 2014 Ivan Mahonin
**	Copyright (c) 2016 Jérôme Blanchi
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

#include "layeraddframe.h"
#include "layercopy.h"
#include <synfig/context.h>
#include <synfigapp/canvasinterface.h>
#include <synfigapp/instance.h>
#include <synfigapp/localization.h>

#endif

using namespace synfig;
using namespace synfigapp;
using namespace Action;

/* === M A C R O S ========================================================= */

ACTION_INIT(Action::LayerAddFrame);
ACTION_SET_NAME(Action::LayerAddFrame,"LayerAddFrame");
ACTION_SET_LOCAL_NAME(Action::LayerAddFrame,N_("Make New Frame"));
ACTION_SET_TASK(Action::LayerAddFrame,"add_frame");
ACTION_SET_CATEGORY(Action::LayerAddFrame,Action::CATEGORY_LAYER);
ACTION_SET_PRIORITY(Action::LayerAddFrame,0);
ACTION_SET_VERSION(Action::LayerAddFrame,"0.0");

/* === G L O B A L S ======================================================= */

/* === P R O C E D U R E S ================================================= */

/* === M E T H O D S ======================================================= */

Action::LayerAddFrame::LayerAddFrame():
	time(0)
{
}

Action::ParamVocab
Action::LayerAddFrame::get_param_vocab()
{
	ParamVocab ret(Action::CanvasSpecific::get_param_vocab());

	ret.push_back(ParamDesc("layer",Param::TYPE_LAYER)
		.set_local_name(_("Layer"))
		.set_desc(_("New frame should be added into this Switch Layer"))
	);
	ret.push_back(ParamDesc("time",Param::TYPE_TIME)
		.set_local_name(_("Time"))
		.set_optional()
	);

	return ret;
}

bool
Action::LayerAddFrame::is_candidate(const ParamList &x)
{
	if (!candidate_check(get_param_vocab(),x)) return false;

	Layer::Handle layer=x.find("layer")->second.get_layer();
	if(!layer) return false;

	if (!etl::handle<Layer_Switch>::cast_dynamic(layer)) return false;

	return true;
}

bool
Action::LayerAddFrame::set_param(const synfig::String& name, const Action::Param &param)
{
	if(name=="layer" && param.get_type()==Param::TYPE_LAYER)
	{
		layer_switch = etl::handle<Layer_Switch>::cast_dynamic(param.get_layer());
		if (layer_switch)
		{
			layer_base = layer_switch->get_current_layer();
			if (!layer_base) layer_switch = NULL;
		}
		return layer_switch;
	}
	if(name=="time" && param.get_type()==Param::TYPE_TIME)
	{
		time=param.get_time();
		return true;
	}

	return Action::CanvasSpecific::set_param(name,param);
}

bool
Action::LayerAddFrame::is_ready()const
{
	if(!layer_switch)
		return false;
	return Action::CanvasSpecific::is_ready();
}

void
Action::LayerAddFrame::prepare()
{
	if(!first_time())
		return;

	// check canvas for switch
	Canvas::Handle subcanvas(layer_switch->get_canvas());
	Canvas::iterator iter=find(subcanvas->begin(),subcanvas->end(),layer_switch);
	if(*iter!=layer_switch)
		throw Error(_("Switch layer doesn't exist anymore."));
	if(get_canvas()!=subcanvas && !subcanvas->is_inline())
		throw Error(_("Switch layer doesn't belong to this canvas anymore"));

	// check canvas for base
	subcanvas = layer_base->get_canvas();
	if (!subcanvas->is_inline())
		throw Error(_("Only inline canvas supported"));
	iter=find(subcanvas->begin(),subcanvas->end(),layer_base);
	if(*iter!=layer_base)
		throw Error(_("Base frame layer doesn't exist anymore."));
	if(layer_switch->get_sub_canvas() != subcanvas)
		throw Error(_("Base frame layer doesn't belong to switch layer canvas anymore"));

	// generate name
	String description, filename, filename_param;
	get_canvas_interface()
		->get_instance()
		->generate_new_name(
			layer_base,
			description,
			filename,
			filename_param );

	{
		Action::Handle action(Action::create("LayerCopy"));
		action->set_param("canvas",subcanvas);
		action->set_param("canvas_interface",get_canvas_interface());
		action->set_param("layer",layer_base);
		add_action(action);
	}
	{
		Action::Handle action(Action::create("ValueDescSet"));
		action->set_param("canvas",get_canvas());
		action->set_param("canvas_interface",get_canvas_interface());
		action->set_param("value_desc",ValueDesc(Layer::Handle(layer_switch), "layer_name"));
		action->set_param("time",time);
		action->set_param("new_value",ValueBase(description));
		action->set_param("animate",true);
		add_action(action);
	}
}

