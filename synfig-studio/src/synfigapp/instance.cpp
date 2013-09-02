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
	file_system_->register_system("container:", container_);

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

	// skip already packed files
	if (filename.substr(0, std::string("container:").size()) == "container:")
		return false;

	// try to create directory
	if (!instance->container_->directory_create("images"))
		return false;

	// generate new filename
	int i = 0;
	std::string new_filename = "container:images/"+basename(filename);
	while(instance->container_->is_exists(new_filename))
	{
		new_filename = "container:images/"
				     + filename_sans_extension(basename(filename))
				     + strprintf("_%d", ++i)
				     + filename_extension(filename);
	}

	// try to copy file
	if (!FileSystem::copy(instance->file_system_, filename, instance->file_system_, new_filename))
		return false;

	// save information about copied file
	FileReference r;
	r.layer = layer;
	r.param_name = param_name;
	r.old_filename = filename;
	r.new_filename = new_filename;
	instance->save_canvas_references_.push_back(r);

	filename = new_filename;
	return true;
}

void
Instance::update_references_in_canvas()
{
	while(!save_canvas_references_.empty())
	{
		FileReference &r = save_canvas_references_.front();
		for(IndependentContext c = get_canvas()->get_independent_context(); c != get_canvas()->end(); c++)
		{
			if (*c == r.layer)
			{
				ValueBase value;
				value.set(r.new_filename);
				(*c)->set_param(r.param_name, value);
				break;
			}
		}
		save_canvas_references_.pop_front();
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
	bool save_container = false;
	std::string canvas_filename = file_name;
	if (filename_extension(file_name) == ".zip")
	{
		canvas_filename = "container:project.sifz";
		save_container = true;
	}

	bool ret;

	String old_file_name(get_file_name());

	set_file_name(file_name);

	set_save_canvas_external_file_callback(save_canvas_callback, this);
	ret = save_canvas(file_system_->get_identifier(canvas_filename),canvas_,!save_container)
	   && container_->save_changes(file_name, false);

	if (ret) update_references_in_canvas();
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
