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
#include <synfig/valuenode_composite.h>
#include <synfig/valuenode_radialcomposite.h>
#include <synfig/valuenode_reference.h>
#include <synfig/valuenode_boneinfluence.h>
#include <synfig/valuenode_greyed.h>
#include <synfig/valuenode_blinecalctangent.h>
#include <synfig/valuenode_blinecalcvertex.h>
#include <synfig/valuenode_blinecalcwidth.h>
#include <synfig/valuenode_wplist.h>
#include <synfig/valuenode_scale.h>
#include <synfig/valuenode_range.h>
#include <synfig/valuenode_integer.h>
#include <synfig/valuenode_real.h>
#include <synfig/layer_pastecanvas.h>
#include "actions/valuedescexport.h"
#include "actions/layerparamset.h"
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


bool
Instance::save()
{
	return save_as(get_canvas()->get_file_name());
}

bool
Instance::save_as(const synfig::String &file_name)
{
	save_canvas_into_container_ = false;
	bool embed_data = false;
	bool extract_data = false;
	std::string canvas_filename = file_name;
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
