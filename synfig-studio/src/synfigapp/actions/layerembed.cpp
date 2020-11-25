/* === S Y N F I G ========================================================= */
/*!	\file layerembed.cpp
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

#include <synfig/layers/layer_bitmap.h>

#include "layerembed.h"

#include <synfigapp/canvasinterface.h>
#include <synfigapp/localization.h>
#include <synfigapp/instance.h>


#endif

using namespace std;
using namespace etl;
using namespace synfig;
using namespace synfigapp;
using namespace Action;

/* === M A C R O S ========================================================= */

ACTION_INIT(Action::LayerEmbed);
ACTION_SET_NAME(Action::LayerEmbed,"LayerEmbed");
ACTION_SET_LOCAL_NAME(Action::LayerEmbed,N_("Embed Layer"));
ACTION_SET_TASK(Action::LayerEmbed,"embed");
ACTION_SET_CATEGORY(Action::LayerEmbed,Action::CATEGORY_LAYER);
ACTION_SET_PRIORITY(Action::LayerEmbed,0);
ACTION_SET_VERSION(Action::LayerEmbed,"0.0");

/* === G L O B A L S ======================================================= */

/* === P R O C E D U R E S ================================================= */

/* === M E T H O D S ======================================================= */

Action::ParamVocab
Action::LayerEmbed::get_param_vocab()
{
	ParamVocab ret(Action::Super::get_param_vocab());

	ret.push_back(ParamDesc("layer",Param::TYPE_LAYER)
		.set_local_name(_("Layer"))
		.set_desc(_("Layer to be embedded"))
	);

	return ret;
}

bool
Action::LayerEmbed::is_candidate(const ParamList &x)
{
	if (!candidate_check(get_param_vocab(),x)) return false;

	Layer::Handle layer=x.find("layer")->second.get_layer();
	if(!layer) return false;

	etl::handle<synfig::Layer_PasteCanvas> layer_pastecanvas
		= etl::handle<synfig::Layer_PasteCanvas>::cast_dynamic(layer);
	if (layer_pastecanvas)
	{
		Canvas::Handle canvas = layer_pastecanvas->get_sub_canvas();
		if (canvas && canvas->is_root())
			return true;
	}

	Layer::Handle layer_import = layer;
	if (layer_import->get_param_list().count("filename") != 0)
	{
		String filename = layer_import->get_param("filename").get(String());
		if ( !CanvasFileNaming::is_embeded(filename)
	      && !layer_import->dynamic_param_list().count("filename") )
			return true;
	}

	return false;
}

bool
Action::LayerEmbed::set_param(const synfig::String& name, const Action::Param &param)
{
	if(name=="layer" && param.get_type()==Param::TYPE_LAYER)
	{
		Layer::Handle layer = param.get_layer();

		etl::handle<synfig::Layer_PasteCanvas> layer_pastecanvas
			= etl::handle<synfig::Layer_PasteCanvas>::cast_dynamic(layer);
		if (layer_pastecanvas)
		{
			Canvas::Handle canvas = layer_pastecanvas->get_sub_canvas();
			if (canvas && canvas->is_root())
			{
				this->layer_pastecanvas = layer_pastecanvas;
				return true;
			}
		}

		Layer::Handle layer_import = layer;
		if (layer_import->get_param_list().count("filename") != 0)
		{
			String filename = layer_import->get_param("filename").get(String());
			if ( !CanvasFileNaming::is_embeded(filename)
		      && !layer_import->dynamic_param_list().count("filename") )
			{
				this->layer_import = layer_import;
				return true;
			}
		}

		return false;
	}

	return Action::CanvasSpecific::set_param(name,param);
}

bool
Action::LayerEmbed::is_ready()const
{
	if(!layer_pastecanvas && !layer_import)
		return false;
	return Action::CanvasSpecific::is_ready();
}

void
Action::LayerEmbed::prepare()
{
	if(!first_time())
		return;

	if (layer_pastecanvas) {
		Canvas::Handle sub_canvas = layer_pastecanvas->get_sub_canvas();

		// generate name
		std::string fname = filename_sans_extension(basename(sub_canvas->get_file_name()));
		static const char bad_chars[]=" :#@$^&()*";
		for(std::string::iterator j = fname.begin(); j != fname.end(); j++)
			for(const char *k = bad_chars; *k != 0; k++)
				if (*j == *k) { *j = '_'; break; }
		if (fname.empty()) fname = "canvas";
		if (fname[0]>='0' && fname[0]<='9')
			fname = "_" + fname;

		std::string name;
		bool found = false;
		for(int j = 1; j < 1000; j++)
		{
			name = j == 1 ? fname : strprintf("%s_%d", fname.c_str(), j);
			if (get_canvas()->value_node_list().count(name) == false)
			{
				found = true;
				for(std::list<Canvas::Handle>::const_iterator iter=get_canvas()->children().begin();iter!=get_canvas()->children().end();iter++)
					if(name==(*iter)->get_id())
						{ found = false; break; }
				if (found) break;
			}
		}
		if (!found)
			throw Error(_("Cannot generate valid name for new canvas"));

		// create action
		Action::Handle action(Action::create("ValueDescExport"));
		action->set_param("canvas",get_canvas());
		action->set_param("canvas_interface",get_canvas_interface());
		action->set_param("value_desc",ValueDesc(Layer::Handle(layer_pastecanvas),std::string("canvas")));
		action->set_param("name",name);
		add_action_front(action);
	}

	if (layer_import) {
		String new_description;
		String new_filename;
		String new_filename_param;

		get_canvas_interface()->get_instance()->generate_new_name(
			layer_import,
			new_description,
			new_filename,
			new_filename_param );

		String filename_param = layer_import->get_param("filename").get(String());
		String filename = CanvasFileNaming::make_full_filename(get_canvas()->get_file_name(), filename_param);

		FileSystem::Handle file_system = get_canvas()->get_file_system();

		etl::loose_handle<synfigapp::Instance> instance =
			get_canvas_interface()->get_instance();
		etl::handle<Layer_Bitmap> layer_bitmap =
			etl::handle<Layer_Bitmap>::cast_dynamic(layer_import);
		if (layer_bitmap && layer_bitmap->is_surface_modified()) {
			// save surface to new place
			instance->save_surface(layer_bitmap->rendering_surface, new_filename);
			//layer_bitmap->is_surface_modified();
			layer_bitmap->reset_surface_modification_id();
		} else {
			// try to create directory
			if (!file_system->directory_create_recursive(etl::dirname(new_filename)))
				throw Error(_("Cannot create directory in container"));
			// try to copy file
			if (!FileSystem::copy(file_system, filename, file_system, new_filename))
				throw Error(_("Cannot copy file into container"));
		}

		// create action to change layer param
		Action::Handle action(Action::create("LayerParamSet"));
		action->set_param("canvas", get_canvas());
		action->set_param("canvas_interface", get_canvas_interface());
		action->set_param("layer", layer_import);
		action->set_param("param", "filename");
		action->set_param("new_value", new_filename_param);
		add_action_front(action);
	}
}
