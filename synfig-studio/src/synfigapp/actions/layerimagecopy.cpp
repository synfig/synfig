/* === S Y N F I G ========================================================= */
/*!	\file layerimagecopy.cpp
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

#include "layerimagecopy.h"
#include "layeradd.h"
#include <synfig/context.h>
#include <synfigapp/canvasinterface.h>
#include <synfigapp/instance.h>
#include <synfigapp/general.h>

#endif

using namespace std;
using namespace etl;
using namespace synfig;
using namespace synfigapp;
using namespace Action;

/* === M A C R O S ========================================================= */

ACTION_INIT(Action::LayerImageCopy);
ACTION_SET_NAME(Action::LayerImageCopy,"LayerImageCopy");
ACTION_SET_LOCAL_NAME(Action::LayerImageCopy,N_("Make New Frame"));
ACTION_SET_TASK(Action::LayerImageCopy,"image_copy");
ACTION_SET_CATEGORY(Action::LayerImageCopy,Action::CATEGORY_LAYER);
ACTION_SET_PRIORITY(Action::LayerImageCopy,0);
ACTION_SET_VERSION(Action::LayerImageCopy,"0.0");
ACTION_SET_CVS_ID(Action::LayerImageCopy,"$Id$");

/* === G L O B A L S ======================================================= */

/* === P R O C E D U R E S ================================================= */

/* === M E T H O D S ======================================================= */

Action::LayerImageCopy::LayerImageCopy():
	time(0)
{
}

Action::ParamVocab
Action::LayerImageCopy::get_param_vocab()
{
	ParamVocab ret(Action::CanvasSpecific::get_param_vocab());

	ret.push_back(ParamDesc("layer",Param::TYPE_LAYER)
		.set_local_name(_("Layer"))
		.set_desc(_("Layer to be duplicated"))
	);

	return ret;
}

bool
Action::LayerImageCopy::is_candidate(const ParamList &x)
{
	return candidate_check(get_param_vocab(),x);
}

bool
Action::LayerImageCopy::set_param(const synfig::String& name, const Action::Param &param)
{
	if(name=="layer" && param.get_type()==Param::TYPE_LAYER)
	{
		layer_bitmap = etl::handle<Layer_Bitmap>::cast_dynamic(param.get_layer());

		if (layer_bitmap
		 && layer_bitmap->surface.get_w() > 0
		 && layer_bitmap->surface.get_h() > 0
		 && layer_bitmap->get_param_list().count("filename"))
		{
			ValueBase value = layer_bitmap->get_param("filename");
			if (value.same_type_as(String()) && filename_extension(value.get(String())) == ".png")
				layer_switch = etl::handle<Layer_Switch>::cast_dynamic(layer_bitmap->get_parent_paste_canvas_layer());
		}

		if (!layer_switch) layer_bitmap = NULL;

		return layer_bitmap;
	}
	if(name=="time" && param.get_type()==Param::TYPE_TIME)
	{
		time=param.get_time();

		return true;
	}

	return Action::CanvasSpecific::set_param(name,param);
}

bool
Action::LayerImageCopy::is_ready()const
{
	if(!layer_bitmap)
		return false;
	return Action::CanvasSpecific::is_ready();
}

void
Action::LayerImageCopy::prepare()
{
	if(!first_time())
		return;

	Canvas::Handle subcanvas(layer_bitmap->get_canvas());

	// Find the iterator for the layer
	Canvas::iterator iter=find(subcanvas->begin(),subcanvas->end(),layer_bitmap);

	// If we couldn't find the layer in the canvas, then bail
	if(*iter!=layer_bitmap)
		throw Error(_("This layer doesn't exist anymore."));

	// If the subcanvas isn't the same as the canvas,
	// then it had better be an inline canvas. If not,
	// bail
	if(get_canvas()!=subcanvas && !subcanvas->is_inline())
		throw Error(_("This layer doesn't belong to this canvas anymore"));


	// generate filename
	String prev_filename = layer_bitmap->get_param("filename").get(String());

	if (prev_filename.empty())
		throw Error(_("This layer doesn't associated with image file"));

	filename.clear();
	String desc;
	String param;
	String name = filename_sans_extension(basename(prev_filename));
	String ext = filename_extension(prev_filename);
	etl::handle<FileSystemGroup> file_system = get_canvas_interface()->get_instance()->get_file_system();
	for(int i = 0; i < 10000; i++) {
		bool valid = true;
		String number = strprintf("%04d", i);
		// TODO: literal '#'
		String fn = "#images/" + name + "." + number + ext;
		String d = name + number;
		for(IndependentContext ic = subcanvas->get_independent_context(); *ic; ic++)
			if ((*ic)->get_description() == d)
				{ valid = false; break; }
		if (valid && file_system->is_exists(fn))
			valid = false;
		if (valid) { filename = fn; desc = d; param = "#" + name + "." + number + ext; break; }
	}

	if (filename.empty())
		throw Error(_("Cannot generate filename"));

	// TODO: literal '#images'
	get_canvas_interface()
		->get_instance()
		->get_file_system()
		->directory_create("#images");
	get_canvas_interface()
		->get_instance()
		->save_surface(layer_bitmap->surface, filename);

	// todo: which canvas should we use?  subcanvas is the layer's canvas, get_canvas() is the canvas set in the action
	Layer::Handle new_layer(layer_bitmap->clone(subcanvas));
	new_layer->set_description(desc);
	new_layer->set_param("filename", param);
	{
		Action::Handle action(Action::create("LayerAdd"));

		action->set_param("canvas",subcanvas);
		action->set_param("canvas_interface",get_canvas_interface());
		action->set_param("new",new_layer);

		add_action(action);
	}
	{
		Action::Handle action(Action::create("ValueDescSet"));

		action->set_param("canvas",subcanvas);
		action->set_param("canvas_interface",get_canvas_interface());
		action->set_param("value_desc",ValueDesc(Layer::Handle(layer_switch), "layer_name"));
		action->set_param("time",time);
		action->set_param("new_value",ValueBase(desc));

		add_action(action);
	}
}

void
Action::LayerImageCopy::undo() {
	Action::Super::undo();
	get_canvas_interface()
		->get_instance()
		->get_file_system()
		->file_remove(filename);
}

