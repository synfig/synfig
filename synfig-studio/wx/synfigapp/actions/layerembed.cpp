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

#include "layerembed.h"

#include <synfig/layers/layer_bitmap.h>

#include <synfigapp/canvasinterface.h>
#include <synfigapp/general.h>
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
ACTION_SET_CVS_ID(Action::LayerEmbed,"$Id$");

/* === G L O B A L S ======================================================= */

/* === P R O C E D U R E S ================================================= */

/* === M E T H O D S ======================================================= */

Action::ParamVocab
Action::LayerEmbed::get_param_vocab()
{
	ParamVocab ret(Action::Super::get_param_vocab());

	ret.push_back(ParamDesc("layer",Param::TYPE_LAYER)
		.set_local_name(_("Layer"))
		.set_desc(_("Layer to be embed"))
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
		Canvas::Handle canvas = layer_pastecanvas->get_sub_canvas();;
		if (canvas && canvas->is_root())
			return true;
	}

	Layer::Handle layer_import = layer;
	if (layer_import->get_param_list().count("filename") != 0)
	{
		String filename = layer_import->get_param("filename").get(String());
		// TODO: literal "container:"
		if (!filename.empty()
		  && filename.substr(0, String("#").size()) != "#"
	      && layer_import->dynamic_param_list().count("filename") == 0)
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
			// TODO: literal "container:"
			if (!filename.empty()
			  && filename.substr(0, String("#").size()) != "#"
		      && layer_import->dynamic_param_list().count("filename") == 0)
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
		// TODO: "container:" and "images" literals
		std::string dir = "#images/";

		std::string filename = layer_import->get_param("filename").get(String());
		std::string src_dir = get_canvas()->get_file_path();
		if (!is_absolute_path(src_dir))
			src_dir = absolute_path(src_dir);

		std::string absolute_filename
			  =	filename.empty()           ? src_dir
			  : is_absolute_path(filename) ? filename
			  : cleanup_path(src_dir+ETL_DIRECTORY_SEPARATOR+filename);

		FileSystem::Handle file_system = get_canvas()->get_identifier().file_system;

		// try to create directory
		if (!file_system->directory_create(dir.substr(0,dir.size()-1)))
			throw Error(_("Cannot create directory in container"));

		// generate new filename
		int i = 0;
		std::string new_filename = basename(filename);
		while(file_system->is_exists(dir + new_filename))
		{
			new_filename = filename_sans_extension(basename(filename))
					     + strprintf("_%d", ++i)
					     + filename_extension(filename);
		}

		etl::loose_handle<synfigapp::Instance> instance =
			get_canvas_interface()->get_instance();
		etl::handle<Layer_Bitmap> layer_bitmap =
			etl::handle<Layer_Bitmap>::cast_dynamic(layer_import);
		if (layer_bitmap && instance->is_layer_registered_to_save(layer_bitmap)) {
			// save surface
			get_canvas_interface()->get_instance()->save_surface(layer_bitmap->surface, dir + new_filename);
		} else {
			// try to copy file
			if (!FileSystem::copy(file_system, absolute_filename, file_system, dir + new_filename))
				throw Error(_("Cannot copy file into container"));
		}

		// create action to change layer param
		ValueBase value;
		value.set("#" + new_filename);
		Action::Handle action(Action::create("LayerParamSet"));
		action->set_param("canvas",get_canvas());
		action->set_param("canvas_interface",get_canvas_interface());
		action->set_param("layer",layer_import);
		action->set_param("param","filename");
		action->set_param("new_value",value);
		add_action_front(action);
	}
}
