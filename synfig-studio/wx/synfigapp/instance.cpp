/* === S Y N F I G ========================================================= */
/*!	\file synfigapp/instance.cpp
**	\brief Instance
**
**	$Id$
**
**	\legal
**	Copyright (c) 2002-2005 Robert B. Quattlebaum Jr., Adrian Bentley
**	Copyright (c) 2007, 2008 Chris Moore
**  Copyright (c) 2011 Carlos López
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

#include "instance.h"
#include "canvasinterface.h"
#include <iostream>
#include <synfig/context.h>
#include <synfig/loadcanvas.h>
#include <synfig/savecanvas.h>
#include <synfig/filesystemnative.h>
#include <synfig/valuenodes/valuenode_add.h>
#include <synfig/valuenodes/valuenode_composite.h>
#include <synfig/valuenodes/valuenode_radialcomposite.h>
#include <synfig/valuenodes/valuenode_reference.h>
#include <synfig/valuenodes/valuenode_boneinfluence.h>
#include <synfig/valuenodes/valuenode_greyed.h>
#include <synfig/valuenodes/valuenode_blinecalctangent.h>
#include <synfig/valuenodes/valuenode_blinecalcvertex.h>
#include <synfig/valuenodes/valuenode_blinecalcwidth.h>
#include <synfig/valuenodes/valuenode_wplist.h>
#include <synfig/valuenodes/valuenode_scale.h>
#include <synfig/valuenodes/valuenode_range.h>
#include <synfig/valuenodes/valuenode_integer.h>
#include <synfig/valuenodes/valuenode_real.h>
#include <synfig/valuenodes/valuenode_bonelink.h>
#include <synfig/valuenodes/valuenode_average.h>
#include <synfig/valuenodes/valuenode_weightedaverage.h>
#include <synfig/layers/layer_pastecanvas.h>
#include <synfig/layers/layer_bitmap.h>
#include <synfig/target_scanline.h>
#include "actions/valuedescexport.h"
#include "actions/layerparamset.h"
#include "actions/layerembed.h"
#include <map>

#include "general.h"

#include <synfig/importer.h>

#endif

/* === U S I N G =========================================================== */

using namespace std;
using namespace etl;
using namespace synfig;
using namespace synfigapp;

/* === M A C R O S ========================================================= */

/* === G L O B A L S ======================================================= */

static std::map<loose_handle<Canvas>, loose_handle<Instance> > instance_map_;

/* === P R O C E D U R E S ================================================= */

bool
synfigapp::is_editable(synfig::ValueNode::Handle value_node)
{
	if(ValueNode_Const::Handle::cast_dynamic(value_node)
		|| ValueNode_Animated::Handle::cast_dynamic(value_node)
		|| ValueNode_Add::Handle::cast_dynamic(value_node)
		|| ValueNode_Composite::Handle::cast_dynamic(value_node)
		|| ValueNode_RadialComposite::Handle::cast_dynamic(value_node)
		||(ValueNode_Reference::Handle::cast_dynamic(value_node) && !ValueNode_Greyed::Handle::cast_dynamic(value_node))
		|| ValueNode_BoneInfluence::Handle::cast_dynamic(value_node)
		|| ValueNode_BLineCalcVertex::Handle::cast_dynamic(value_node)
		|| ValueNode_BLineCalcTangent::Handle::cast_dynamic(value_node)
		|| ValueNode_BLineCalcWidth::Handle::cast_dynamic(value_node)
		|| ValueNode_Scale::Handle::cast_dynamic(value_node)
		|| ValueNode_Range::Handle::cast_dynamic(value_node)
		|| ValueNode_Integer::Handle::cast_dynamic(value_node)
		|| ValueNode_Real::Handle::cast_dynamic(value_node)
		|| ValueNode_BoneLink::Handle::cast_dynamic(value_node)
		|| ValueNode_Average::Handle::cast_dynamic(value_node)
		|| ValueNode_WeightedAverage::Handle::cast_dynamic(value_node)
	)
		return true;
	return false;
}

etl::handle<Instance>
synfigapp::find_instance(etl::handle<synfig::Canvas> canvas)
{
	if(instance_map_.count(canvas)==0)
		return 0;
	return instance_map_[canvas];
}

/* === M E T H O D S ======================================================= */

Instance::Instance(etl::handle<synfig::Canvas> canvas, etl::handle< synfig::FileContainerTemporary > container):
	CVSInfo(canvas->get_file_name()),
	canvas_(canvas),
	file_system_(new FileSystemGroup(FileSystemNative::instance())),
	container_(container)
{
	file_system_->register_system("#", container_);

	assert(canvas->is_root());

	unset_selection_manager();

	instance_map_[canvas]=this;
} // END of synfigapp::Instance::Instance()

handle<Instance>
Instance::create(etl::handle<synfig::Canvas> canvas, etl::handle< synfig::FileContainerTemporary > container)
{
	// Construct a new instance
	handle<Instance> instance(new Instance(canvas, container));

	return instance;
} // END of synfigapp::Instance::create()

synfig::String
Instance::get_file_name()const
{
	return get_canvas()->get_file_name();
}

void
Instance::set_file_name(const synfig::String &name)
{
	get_canvas()->set_file_name(name);
	CVSInfo::set_file_name(name);
}

Instance::~Instance()
{
	instance_map_.erase(canvas_);

	if (getenv("SYNFIG_DEBUG_DESTRUCTORS"))
		synfig::info("Instance::~Instance(): Deleted");
}

handle<CanvasInterface>
Instance::find_canvas_interface(synfig::Canvas::Handle canvas)
{
	if(!canvas)
		return 0;
	while(canvas->is_inline())
		canvas=canvas->parent();

	CanvasInterfaceList::iterator iter;

	for(iter=canvas_interface_list().begin();iter!=canvas_interface_list().end();iter++)
		if((*iter)->get_canvas()==canvas)
			return *iter;

	return CanvasInterface::create(this,canvas);
}

bool
Instance::save_canvas_callback(void *instance_ptr, synfig::Layer::ConstHandle layer, const std::string &param_name, std::string &filename)
{
	// todo: "container:" and "images" literals
	Instance *instance = (Instance*)instance_ptr;

	std::string actual_filename = filename;
	if (actual_filename.substr(0, std::string("#").size()) == "#")
		actual_filename = "#images/" + actual_filename.substr(std::string("#").size());

	// skip already packed (or unpacked) files
	bool file_already_in_container = actual_filename.substr(0, std::string("#").size()) == "#";
	if (file_already_in_container && instance->save_canvas_into_container_) return false;
	if (!file_already_in_container && !instance->save_canvas_into_container_) return false;

	const std::string src_dir = instance->get_canvas()->get_file_path();
	const std::string &dir = instance->save_canvas_reference_directory_;
	const std::string &localdir = instance->save_canvas_reference_local_directory_;

	std::string absolute_filename
		  =	file_already_in_container  ? actual_filename
		  : actual_filename.empty()           ? src_dir
		  : is_absolute_path(actual_filename) ? actual_filename
		  : cleanup_path(src_dir+ETL_DIRECTORY_SEPARATOR+actual_filename);

	// is file already copied?
	for(FileReferenceList::iterator i = instance->save_canvas_references_.begin(); i != instance->save_canvas_references_.end(); i++)
	{
		if (i->old_filename == absolute_filename)
		{
			FileReference r = *i;
			r.layer = layer;
			r.param_name = param_name;
			instance->save_canvas_references_.push_back(r);
			filename = r.new_filename;
			return true;
		}
	}

	// try to create directory
	if (!instance->file_system_->directory_create(dir.substr(0,dir.size()-1)))
		return false;

	// generate new actual_filename
	int i = 0;
	std::string new_filename = basename(actual_filename);
	while(instance->file_system_->is_exists(dir + new_filename))
	{
		new_filename = filename_sans_extension(basename(actual_filename))
				     + strprintf("_%d", ++i)
				     + filename_extension(actual_filename);
	}

	// try to copy file
	if (!FileSystem::copy(instance->file_system_, absolute_filename, instance->file_system_, dir + new_filename))
		return false;

	// save information about copied file
	FileReference r;
	r.layer = layer;
	r.param_name = param_name;
	r.old_filename = absolute_filename;
	r.new_filename = localdir + new_filename;
	if (r.new_filename.substr(0, String("#images/").size())=="#images/")
		r.new_filename = "#" + r.new_filename.substr(String("#images/").size());
	instance->save_canvas_references_.push_back(r);

	filename = r.new_filename;
	return true;
}

void
Instance::update_references_in_canvas(synfig::Canvas::Handle canvas)
{
	for(std::list<Canvas::Handle>::const_iterator i = canvas->children().begin(); i != canvas->children().end(); i++)
		update_references_in_canvas(*i);

	for(IndependentContext c = canvas->get_independent_context(); *c; c++)
	{
		for(FileReferenceList::iterator j = save_canvas_references_.begin(); j != save_canvas_references_.end();)
		{
			if (*c == j->layer)
			{
				ValueBase value;
				value.set(j->new_filename);
				(*c)->set_param(j->param_name, value);
				(*c)->changed();
				find_canvas_interface(get_canvas())->signal_layer_param_changed()(*c,j->param_name);
				j = save_canvas_references_.erase(j);
			}
			else j++;
		}
	}
}

bool
Instance::import_external_canvas(Canvas::Handle canvas, std::map<Canvas*, Canvas::Handle> &imported)
{
	etl::handle<CanvasInterface> canvas_interface;

	for(IndependentContext i = canvas->get_independent_context(); *i; i++)
	{
		etl::handle<Layer_PasteCanvas> paste_canvas = etl::handle<Layer_PasteCanvas>::cast_dynamic(*i);
		if (!paste_canvas) continue;

		Canvas::Handle sub_canvas = paste_canvas->get_sub_canvas();
		if (!sub_canvas) continue;
		if (!sub_canvas->is_root()) continue;

		if (imported.count(sub_canvas.get()) != 0) {
			// link already exported canvas
			Canvas::Handle new_canvas = imported[sub_canvas.get()];
			if (!new_canvas) continue;

			// Action to link canvas
			try
			{
				Action::Handle action(Action::LayerParamSet::create());
				if (!action) continue;
				canvas_interface = find_canvas_interface(canvas);
				action->set_param("canvas",canvas);
				action->set_param("canvas_interface",canvas_interface);
				action->set_param("layer",Layer::Handle(paste_canvas));
				action->set_param("param","canvas");
				action->set_param("new_value",ValueBase(new_canvas));
				if(!action->is_ready()) continue;
				if(!perform_action(action)) continue;
			}
			catch(...)
			{
				continue;
			}
		} else {
			imported[sub_canvas.get()] = NULL;

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
				if (canvas->value_node_list().count(name) == false)
				{
					found = true;
					for(std::list<Canvas::Handle>::const_iterator iter=canvas->children().begin();iter!=canvas->children().end();iter++)
						if(name==(*iter)->get_id())
							{ found = false; break; }
					if (found) break;
				}
			}
			if (!found) continue;

			// Action to import canvas
			try {
				Action::Handle action(Action::ValueDescExport::create());
				if (!action) continue;

				canvas_interface = find_canvas_interface(canvas);
				action->set_param("canvas",canvas);
				action->set_param("canvas_interface",canvas_interface);
				action->set_param("value_desc",ValueDesc(Layer::Handle(paste_canvas),std::string("canvas")));
				action->set_param("name",name);
				if(!action->is_ready()) continue;
				if(!perform_action(action)) continue;
				std::string warnings;
				imported[sub_canvas.get()] = canvas->find_canvas(name, warnings);
			}
			catch(...)
			{
				continue;
			}

			return true;
		}
	}

	for(std::list<Canvas::Handle>::const_iterator i = canvas->children().begin(); i != canvas->children().end(); i++)
		if (import_external_canvas(*i, imported))
			return true;

	return false;
}

void
Instance::import_external_canvases()
{
	std::map<Canvas*, Canvas::Handle> imported;
	while(import_external_canvas(get_canvas(), imported));
}

void Instance::save_surface(const synfig::Surface &surface, const synfig::String &filename)
{
	if (surface.get_h() <= 0 || surface.get_w() <= 0) return;

	String ext = filename_extension(filename);
	if (ext.empty()) return;
	ext.erase(0, 1);
	String tmpfile = FileContainerTemporary::generate_temporary_filename();

	etl::handle<Target_Scanline> target
		= etl::handle<Target_Scanline>(Target::create(Target::ext_book()[ext],tmpfile,TargetParam()));
	if (!target) return;
	target->set_canvas(get_canvas());
	RendDesc desc;
	desc.set_w(surface.get_w());
	desc.set_h(surface.get_h());
	desc.set_x_res(1);
	desc.set_y_res(1);
	desc.set_frame_rate(1);
	desc.set_frame(0);
	desc.set_frame_start(0);
	desc.set_frame_end(0);
	target->set_rend_desc(&desc);
	target->add_frame(&surface);
	target = NULL;

	FileSystem::copy(FileSystemNative::instance(), tmpfile, get_file_system(), filename);
	FileSystemNative::instance()->file_remove(tmpfile);
}

void
Instance::embed_all(synfig::Canvas::Handle canvas, bool &success, bool &restart) {
	etl::handle<CanvasInterface> canvas_interface = find_canvas_interface(canvas);

	Action::ParamList paramList;
	paramList.add("canvas",canvas);
	paramList.add("canvas_interface",canvas_interface);

	for(synfig::Canvas::iterator i = canvas->begin(); i != canvas->end(); ++i) {
		// process layer
		paramList.remove_all("layer").add("layer",*i);
		if (Action::LayerEmbed::is_candidate(paramList)) {
			Action::Handle action(Action::LayerEmbed::create());
			if (action) {
				action->set_param_list(paramList);
				if(action->is_ready()) {
					if(perform_action(action)) {
						restart = true;
						return;
					}
				}
			}
			success = false;
		}

		// process sub-canvas
		etl::handle<Layer_PasteCanvas> layer_pastecanvas =
			etl::handle<Layer_PasteCanvas>::cast_dynamic(*i);
		if (layer_pastecanvas) {
			synfig::Canvas::Handle sub_canvas = layer_pastecanvas->get_sub_canvas();
			if (sub_canvas) {
				embed_all(sub_canvas, success, restart);
				if (restart) return;
			}
		}
	}
}


bool
Instance::embed_all() {
	bool success = true;
	bool restart = true;
	while(restart) {
		restart = false;
		embed_all(get_canvas(), success, restart);
	}
	return success;
}

//! make relative filenames from animated valuenodes
void Instance::convert_animated_filenames(const Canvas::Handle &canvas, const synfig::String &old_path, const synfig::String &new_path)
{
	for(Canvas::iterator i = canvas->begin(); i != canvas->end(); ++i)
	{
		const Layer::DynamicParamList &dynamic_param_list = (*i)->dynamic_param_list();
		Layer::DynamicParamList::const_iterator j = dynamic_param_list.find("filename");
		if (j != dynamic_param_list.end())
		{
			ValueNode_Animated::Handle valuenode_animated = ValueNode_Animated::Handle::cast_dynamic(j->second);
			if (valuenode_animated)
			{
				WaypointList &waypoint_list = valuenode_animated->editable_waypoint_list();
				for(WaypointList::iterator k = waypoint_list.begin(); k != waypoint_list.end(); ++k)
				{
					ValueNode_Const::Handle valuenode_const = ValueNode_Const::Handle::cast_dynamic(k->get_value_node());
					if (valuenode_const && valuenode_const->get_type() == type_string)
					{
						String s = valuenode_const->get_value().get(String());
						if (!s.empty() && s[0] != '#')
						{
							warning(old_path);
							warning(new_path);
							if (!is_absolute_path(s) && !old_path.empty()) s = old_path + ETL_DIRECTORY_SEPARATOR + s;
							s = relative_path(new_path, s);
							valuenode_const->set_value(s);
						}
					}
				}
			}
		}

		etl::handle<Layer_PasteCanvas> layer_paste_canvas = etl::handle<Layer_PasteCanvas>::cast_dynamic(*i);
		if (layer_paste_canvas && layer_paste_canvas->get_sub_canvas() && !layer_paste_canvas->get_sub_canvas()->is_root())
			convert_animated_filenames(Canvas::Handle(layer_paste_canvas->get_sub_canvas()), old_path, new_path);
	}

}

bool
Instance::save()
{
	return save_as(get_canvas()->get_file_name());
}

bool
Instance::save_as(const synfig::String &file_name)
{
	if (filename_extension(file_name) == ".sfg") embed_all();

	save_canvas_into_container_ = false;
	bool embed_data = false;
	bool extract_data = false;
	std::string canvas_filename = file_name;

	convert_animated_filenames(get_canvas(), absolute_path(get_canvas()->get_file_path()), absolute_path(dirname(file_name)));

	// save bitmaps
	std::set<Layer::Handle> layers_to_save_set;
	for(std::list<Layer::Handle>::iterator i = layers_to_save.begin(); i != layers_to_save.end(); i++)
		layers_to_save_set.insert(*i);
	for(std::set<Layer::Handle>::iterator i = layers_to_save_set.begin(); i != layers_to_save_set.end(); i++)
	{
		etl::handle<Layer_Bitmap> layer_bitmap = etl::handle<Layer_Bitmap>::cast_dynamic(*i);
		if (!layer_bitmap) continue;
		if (!layer_bitmap->get_canvas()) continue;
		if (!(*i)->get_param_list().count("filename")) continue;
		ValueBase value = (*i)->get_param("filename");
		if (!value.same_type_as(String())) continue;
		String filename = value.get(String());
		// TODO: literals '#' and 'images/'
		if (!filename.empty() && filename[0] == '#')
			filename.insert(1, "images/");
		save_surface(layer_bitmap->surface, filename);
	}

	if (filename_extension(file_name) == ".sfg")
	{
		save_canvas_reference_directory_ = "#images/";
		save_canvas_reference_local_directory_ = "#images/";
		canvas_filename = "#project.sifz";
		save_canvas_into_container_ = true;
		embed_data = filename_extension(get_canvas()->get_file_name()) != ".sfg";
	} else
	{
		save_canvas_reference_directory_ =
			filename_sans_extension(file_name)
		  + ".images"
		  + ETL_DIRECTORY_SEPARATOR;
		save_canvas_reference_local_directory_ =
			filename_sans_extension(basename(file_name))
		  + ".images"
		  + ETL_DIRECTORY_SEPARATOR;
		extract_data = filename_extension(get_canvas()->get_file_name()) == ".sfg";
	}

	if (embed_data) import_external_canvases();

	bool ret;

	String old_file_name(get_file_name());

	set_file_name(file_name);
	get_canvas()->set_identifier(file_system_->get_identifier(canvas_filename));

	if (embed_data || extract_data)
		set_save_canvas_external_file_callback(save_canvas_callback, this);
	else
		set_save_canvas_external_file_callback(NULL, NULL);

	ret = save_canvas(file_system_->get_identifier(canvas_filename),canvas_,!save_canvas_into_container_);

	if (ret && save_canvas_into_container_)
	   ret = container_->save_changes(file_name, false);

	if (ret && (embed_data || extract_data))
		update_references_in_canvas(get_canvas());
	set_save_canvas_external_file_callback(NULL, NULL);
	save_canvas_references_.clear();

	if(ret)
	{
		reset_action_count();
		signal_saved_();
	}
	else
		set_file_name(old_file_name);

	signal_filename_changed_();

	return ret;
}

bool
Instance::generate_new_name(
		synfig::Layer::Handle layer,
		synfig::Canvas::Handle canvas,
		synfig::FileSystem::Handle file_system,
		synfig::String &out_description,
		synfig::String &out_filename,
		synfig::String &out_filename_param)
{
	out_description.clear();
	out_filename.clear();
	out_filename_param.clear();

	String description = layer->get_description();
	String filename;

	etl::handle<Layer_Bitmap> layer_bitmap = etl::handle<Layer_Bitmap>::cast_dynamic(layer);
	if (layer_bitmap
	 && layer_bitmap->surface.get_w() > 0
	 && layer_bitmap->surface.get_h() > 0
	 && layer_bitmap->get_param_list().count("filename"))
	{
		ValueBase value = layer_bitmap->get_param("filename");
		if (value.same_type_as(String()) && filename_extension(value.get(String())) == ".png")
			filename = basename(value.get(String()));
	}

	// extract name from filename or from description
	String name = filename.empty() ? description : filename_sans_extension(filename);
	String ext = filename_extension(name);
	if (ext.find_first_not_of(".0123456789") == String::npos)
		name = filename_sans_extension(name);
	for(size_t i = name.find("#", 0); i != String::npos; i = name.find("#", i))
		name.erase(i, 1);
	// if name based on description add extension
	ext = filename.empty() ? ".png" : filename_extension(filename);

	// generate new names
	for(int i = 0; i < 10000; i++) {
		bool valid = true;
		String number = strprintf("%04d", i);
		// TODO: literal '#'
		String current_description = name + "." + number;
		String current_filename = "#images/" + name + "." + number + ext;
		String current_filename_param = "#" + name + "." + number + ext;
		if (current_description == description || current_filename == filename)
			valid = false;
		if (valid && canvas)
			for(IndependentContext ic = canvas->get_independent_context(); *ic; ic++)
				if ((*ic)->get_description() == current_description)
					{ valid = false; break; }
		if (valid && file_system && file_system->is_exists(current_filename))
			valid = false;
		if (valid) {
			out_description = current_description;
			out_filename = current_filename;
			out_filename_param = current_filename_param;
			break;
		}
	}

	return true;
}
