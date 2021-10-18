/* === S Y N F I G ========================================================= */
/*!	\file layerextract.cpp
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

#include <synfig/general.h>
#include <synfig/string.h>
#include <synfig/canvasfilenaming.h>

#include "layerextract.h"

#include <synfigapp/canvasinterface.h>
#include <synfigapp/localization.h>

#endif

using namespace synfig;
using namespace synfigapp;
using namespace Action;

/* === M A C R O S ========================================================= */

ACTION_INIT(Action::LayerExtract);
ACTION_SET_NAME(Action::LayerExtract,"LayerExtract");
ACTION_SET_LOCAL_NAME(Action::LayerExtract,N_("Extract Layer"));
ACTION_SET_TASK(Action::LayerExtract,"extract");
ACTION_SET_CATEGORY(Action::LayerExtract,Action::CATEGORY_LAYER);
ACTION_SET_PRIORITY(Action::LayerExtract,0);
ACTION_SET_VERSION(Action::LayerExtract,"0.0");

/* === G L O B A L S ======================================================= */

/* === P R O C E D U R E S ================================================= */

/* === M E T H O D S ======================================================= */

Action::ParamVocab
Action::LayerExtract::get_param_vocab()
{
	ParamVocab ret(Action::Super::get_param_vocab());

	ret.push_back(ParamDesc("layer",Param::TYPE_LAYER)
		.set_local_name(_("Layer"))
		.set_desc(_("Layer to be extracted"))
	);

	ret.push_back(ParamDesc("filename",Param::TYPE_STRING)
		.set_local_name(_("File name"))
		.set_desc(_("File name with path to store exported file"))
		.set_user_supplied()
	);

	return ret;
}

bool
Action::LayerExtract::is_candidate(const ParamList &x)
{
	if (!candidate_check(get_param_vocab(),x)) return false;

	Layer::Handle layer=x.find("layer")->second.get_layer();
	if(!layer) return false;

	if (layer->get_param_list().count("filename") != 0)
	{
		String filename = layer->get_param("filename").get(String());
		if ( !CanvasFileNaming::is_embeded(filename)
	      && !layer->dynamic_param_list().count("filename") )
			return true;
	}

	return false;
}

bool
Action::LayerExtract::set_param(const synfig::String& name, const Action::Param &param)
{
	if(name=="layer" && param.get_type()==Param::TYPE_LAYER)
	{
		Layer::Handle layer = param.get_layer();

		if (layer->get_param_list().count("filename") != 0)
		{
			String filename = layer->get_param("filename").get(String());
			if ( !CanvasFileNaming::is_embeded(filename)
		      && !layer->dynamic_param_list().count("filename") )
			{
				this->layer = layer;
				return true;
			}
		}

		return false;
	}

	if(name=="filename" && param.get_type()==Param::TYPE_STRING)
	{
		filename = param.get_string();
		return true;
	}

	return Action::CanvasSpecific::set_param(name,param);
}

bool
Action::LayerExtract::is_ready()const
{
	if(!layer)
		return false;
	return Action::CanvasSpecific::is_ready();
}

void
Action::LayerExtract::prepare()
{
	if(!first_time())
		return;

	if (layer) {
		String old_filename_param = layer->get_param("filename").get(String());
		String old_filename = CanvasFileNaming::make_full_filename(get_canvas()->get_file_name(), old_filename_param);

		String new_filename = filename;
		if (new_filename.empty())
			new_filename = get_canvas()->get_file_path() + ETL_DIRECTORY_SEPARATOR + old_filename;
		else
		if (CanvasFileNaming::is_embeded(new_filename) && !etl::is_absolute_path(new_filename))
			new_filename = etl::absolute_path(get_canvas()->get_file_path(), new_filename);

		String new_filename_param = CanvasFileNaming::make_short_filename(get_canvas()->get_file_name(), new_filename);

		FileSystem::Handle file_system = get_canvas()->get_file_system();

		// try to create directory
		if (!file_system->directory_create_recursive(etl::dirname(new_filename)))
			throw Error(_("Cannot create directory"));
		// try to copy file
		if (!FileSystem::copy(file_system, old_filename, file_system, new_filename))
			throw Error(_("Cannot copy file"));

		// create action to change layer param
		Action::Handle action(Action::create("LayerParamSet"));
		action->set_param("canvas", get_canvas());
		action->set_param("canvas_interface", get_canvas_interface());
		action->set_param("layer", layer);
		action->set_param("param", "filename");
		action->set_param("new_value", new_filename_param);
		add_action_front(action);
	}
}
