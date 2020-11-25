/* === S Y N F I G ========================================================= */
/*!	\file layercopy.cpp
**	\brief Template File
**
**	$Id$
**
**	\legal
**	......... ... 2014 Ivan Mahonin
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

#include "layercopy.h"
#include "layeradd.h"
#include <synfig/context.h>
#include <synfigapp/canvasinterface.h>
#include <synfigapp/instance.h>
#include <synfigapp/localization.h>
#include <synfig/layers/layer_bitmap.h>

#endif

using namespace std;
using namespace etl;
using namespace synfig;
using namespace synfigapp;
using namespace Action;

/* === M A C R O S ========================================================= */

ACTION_INIT_NO_GET_LOCAL_NAME(Action::LayerCopy);
ACTION_SET_NAME(Action::LayerCopy,"LayerCopy");
ACTION_SET_LOCAL_NAME(Action::LayerCopy,N_("Simple Copy Layer"));
ACTION_SET_TASK(Action::LayerCopy,"copy");
ACTION_SET_CATEGORY(Action::LayerCopy,Action::CATEGORY_LAYER);
ACTION_SET_PRIORITY(Action::LayerCopy,0);
ACTION_SET_VERSION(Action::LayerCopy,"0.0");

/* === G L O B A L S ======================================================= */

/* === P R O C E D U R E S ================================================= */

/* === M E T H O D S ======================================================= */

Action::LayerCopy::LayerCopy()
{
}

synfig::String
Action::LayerCopy::get_local_name()const
{
	return get_layer_descriptions(layers, _("Simple Copy Layer"), _("Simple Copy Layers"));
}

Action::ParamVocab
Action::LayerCopy::get_param_vocab()
{
	ParamVocab ret(Action::CanvasSpecific::get_param_vocab());

	ret.push_back(ParamDesc("layer",Param::TYPE_LAYER)
		.set_local_name(_("Layer"))
		.set_desc(_("Layer to be copied"))
	);

	return ret;
}

bool
Action::LayerCopy::is_candidate(const ParamList &x)
{
	if (!candidate_check(get_param_vocab(),x))
		return false;

	bool there_is_a_bitmap_layer = false;
	for(ParamList::const_iterator i = x.begin(); i != x.end(); i++) {
		if (i->first == "layer") {
			if (i->second.get_type() != Param::TYPE_LAYER)
				return false;
			const Layer_Bitmap* bitmap_layer = dynamic_cast<Layer_Bitmap*>(i->second.get_layer().get());
			if (!bitmap_layer)
				return false;
			there_is_a_bitmap_layer = true;
		}
	}
	return there_is_a_bitmap_layer;
}

bool
Action::LayerCopy::set_param(const synfig::String& name, const Action::Param &param)
{
	if(name=="layer" && param.get_type()==Param::TYPE_LAYER && param.get_layer())
	{
		const Layer_Bitmap* bitmap_layer = dynamic_cast<Layer_Bitmap*>(param.get_layer().get());
		if (!bitmap_layer)
			return false;
		layers.push_back(param.get_layer());
		return true;
	}

	return Action::CanvasSpecific::set_param(name,param);
}

bool
Action::LayerCopy::is_ready()const
{
	if(layers.empty())
		return false;
	return Action::CanvasSpecific::is_ready();
}

void
Action::LayerCopy::prepare()
{
	if(!first_time())
		return;

	for(std::list<Layer::Handle>::iterator i = layers.begin(); i != layers.end(); ++i)
	{
		Layer::Handle layer(*i);

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

		// generate names
		String description, filename, filename_param;
		get_canvas_interface()
			->get_instance()
			->generate_new_name(
				layer,
				description,
				filename,
				filename_param );

		// make copy
		Layer::Handle new_layer = Layer::create(layer->get_name()).get();
		new_layer->add_to_group(layer->get_group());
		new_layer->set_active(layer->active());
		new_layer->set_exclude_from_rendering(layer->get_exclude_from_rendering());
		new_layer->set_param_list(layer->get_param_list());
		new_layer->set_description(description);

		// copy file
		etl::handle<Layer_Bitmap> layer_bitmap = etl::handle<Layer_Bitmap>::cast_dynamic(layer);
		if (layer_bitmap && !filename.empty())
		{
			get_canvas_interface()
				->get_instance()
				->save_surface(layer_bitmap->rendering_surface, filename);
			filenames.push_back(filename);
			new_layer->set_param("filename", filename_param);
		}

		Action::Handle action(Action::create("LayerAdd"));
		action->set_param("canvas",subcanvas);
		action->set_param("canvas_interface",get_canvas_interface());
		action->set_param("new",new_layer);
		add_action(action);
	}
}

void
Action::LayerCopy::undo() {
	Action::Super::undo();
	while(!filenames.empty())
	{
		get_canvas()->get_file_system()->file_remove(filenames.back());
		filenames.pop_back();
	}
}

