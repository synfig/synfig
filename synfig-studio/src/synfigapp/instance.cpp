/* === S Y N F I G ========================================================= */
/*!	\file synfigapp/instance.cpp
**	\brief Instance
**
**	\legal
**	Copyright (c) 2002-2005 Robert B. Quattlebaum Jr., Adrian Bentley
**	Copyright (c) 2007, 2008 Chris Moore
**  Copyright (c) 2011 Carlos López
**
**	This file is part of Synfig.
**
**	Synfig is free software: you can redistribute it and/or modify
**	it under the terms of the GNU General Public License as published by
**	the Free Software Foundation, either version 2 of the License, or
**	(at your option) any later version.
**
**	Synfig is distributed in the hope that it will be useful,
**	but WITHOUT ANY WARRANTY; without even the implied warranty of
**	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
**	GNU General Public License for more details.
**
**	You should have received a copy of the GNU General Public License
**	along with Synfig.  If not, see <https://www.gnu.org/licenses/>.
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

#include "instance.h"
#include "canvasinterface.h"
#include <iostream>
#include <synfig/context.h>
#include <synfig/canvasfilenaming.h>
#include <synfig/loadcanvas.h>
#include <synfig/savecanvas.h>
#include <synfig/filecontainerzip.h>
#include <synfig/filesystem.h>
#include <synfig/filesystemnative.h>
#include <synfig/filesystemtemporary.h>
#include <synfig/valuenodes/valuenode_add.h>
#include <synfig/valuenodes/valuenode_composite.h>
#include <synfig/valuenodes/valuenode_const.h>
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
#include <synfig/valuenodes/valuenode_timeloop.h>
#include <synfig/layers/layer_pastecanvas.h>
#include <synfig/layers/layer_bitmap.h>
#include <synfig/rendering/software/surfacesw.h>
#include <synfig/target_scanline.h>
#include "actions/valuedescexport.h"
#include "actions/layerparamset.h"
#include <map>

#include <synfigapp/localization.h>

#include <synfig/importer.h>

#endif

/* === U S I N G =========================================================== */

using namespace etl;
using namespace synfig;
using namespace synfigapp;

/* === M A C R O S ========================================================= */

/* === G L O B A L S ======================================================= */

static std::map<Canvas::LooseHandle, loose_handle<Instance>> instance_map_;

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
		|| ValueNode_TimeLoop::Handle::cast_dynamic(value_node)
	)
		return true;
	return false;
}

etl::handle<Instance>
synfigapp::find_instance(Canvas::Handle canvas)
{
	if(instance_map_.count(canvas)==0)
		return 0;
	return instance_map_[canvas];
}

/* === M E T H O D S ======================================================= */

Instance::Instance(Canvas::Handle canvas, synfig::FileSystem::Handle container):
	canvas_(canvas),
	container_(container)
{
	assert(canvas->is_root());

	unset_selection_manager();

	instance_map_[canvas]=this;
} // END of synfigapp::Instance::Instance()

handle<Instance>
Instance::create(Canvas::Handle canvas, synfig::FileSystem::Handle container)
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
}

Instance::~Instance()
{
	instance_map_.erase(canvas_);

	DEBUG_LOG("SYNFIG_DEBUG_DESTRUCTORS",
		"Instance::~Instance(): Deleted");
}

handle<CanvasInterface>
Instance::find_canvas_interface(synfig::Canvas::Handle canvas)
{
	if(!canvas)
		return 0;
	while(canvas->is_inline())
		canvas=canvas->parent();

	for (CanvasInterfaceList::iterator iter = canvas_interface_list().begin(); iter != canvas_interface_list().end(); ++iter)
		if((*iter)->get_canvas()==canvas)
			return *iter;

	return CanvasInterface::create(this,canvas);
}

bool
Instance::import_external_canvas(Canvas::Handle canvas, std::map<Canvas*, Canvas::Handle> &imported)
{
	etl::handle<CanvasInterface> canvas_interface;

	for(IndependentContext i = canvas->get_independent_context(); *i; i++)
	{
		Layer_PasteCanvas::Handle paste_canvas = Layer_PasteCanvas::Handle::cast_dynamic(*i);
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
			imported[sub_canvas.get()] = nullptr;

			// generate name
			std::string fname = filesystem::Path::filename_sans_extension(filesystem::Path::basename(sub_canvas->get_file_name()));
			static const char bad_chars[]=" :#@$^&()*";
			for(std::string::iterator j = fname.begin(); j != fname.end(); ++j)
				for (const char* k = bad_chars; *k != 0; ++k)
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
					for (std::list<Canvas::Handle>::const_iterator iter = canvas->children().begin(); iter != canvas->children().end(); ++iter)
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

	for (std::list<Canvas::Handle>::const_iterator i = canvas->children().begin(); i != canvas->children().end(); ++i)
		if (import_external_canvas(*i, imported))
			return true;

	return false;
}

etl::handle<Action::Group>
Instance::import_external_canvases()
{
	synfigapp::Action::PassiveGrouper group(this, _("Import external canvases"));
	std::map<Canvas*, Canvas::Handle> imported;
	while(import_external_canvas(get_canvas(), imported));
	return group.finish();
}

bool Instance::save_surface(const rendering::SurfaceResource::Handle &surface, const synfig::String &filename)
{
	rendering::SurfaceResource::LockRead<rendering::SurfaceSW> lock(surface);
	if (!lock) return false;
	return save_surface(lock->get_surface(), filename);
}

bool Instance::save_surface(const synfig::Surface &surface, const synfig::String &filename)
{
	if (surface.get_h() <= 0 || surface.get_w() <= 0)
		return false;

	String ext = filesystem::Path::filename_extension(filename);
	if (ext.empty())
		return false;

	ext.erase(0, 1);
	String tmpfile = FileSystemTemporary::generate_system_temporary_filename("surface");

	Target_Scanline::Handle target =
		Target_Scanline::Handle::cast_dynamic(
			Target::create(Target::ext_book()[ext],tmpfile,TargetParam()) );
	if (!target)
		return false;

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
	bool success = target->add_frame(&surface, nullptr);
	target = nullptr;

	if (success)
		success = get_canvas()->get_file_system()->directory_create(filesystem::Path::dirname(filename));
	if (success)
		success = FileSystem::copy(FileSystemNative::instance(), tmpfile, get_canvas()->get_file_system(), filename);

	FileSystemNative::instance()->file_remove(tmpfile);

	return success;
}

void
Instance::process_filename(const ProcessFilenamesParams &params, const synfig::String &filename, synfig::String &out_filename)
{
	String full_filename = CanvasFileNaming::make_full_filename(params.previous_canvas_filename, filename);
	std::map<String, String>::const_iterator i = params.processed_files.find(full_filename);
	if (i != params.processed_files.end())
		{ out_filename = i->second; return; }

	if (params.embed_files)
	{
		if ( CanvasFileNaming::can_embed(filename)
		  && !CanvasFileNaming::is_embeded(params.previous_canvas_filename, filename))
		{
			String new_filename = CanvasFileNaming::generate_container_filename(get_canvas()->get_identifier().file_system, filename);
			String new_full_filename = CanvasFileNaming::make_full_filename(get_canvas()->get_file_name(), out_filename);
			if (FileSystem::copy(
					params.previous_canvas_filesystem,
					full_filename,
					get_canvas()->get_identifier().file_system,
					new_full_filename ))
			{
				out_filename = new_filename;
				params.processed_files[full_filename] = out_filename;
				info("embed file: %s -> %s", filename.c_str(), out_filename.c_str());
				return;
			}
			else
			{
				warning("Cannot embed file: %s", filename.c_str());
			}
		}
	}

	out_filename = CanvasFileNaming::make_short_filename(params.canvas->get_file_name(), full_filename);
	params.processed_files[full_filename] = out_filename;
	info("refine filename: %s -> %s", filename.c_str(), out_filename.c_str());
}

void
Instance::process_filenames(const ProcessFilenamesParams &params, const synfig::NodeHandle &node, bool self)
{
	if (!node || params.processed_nodes.count(node)) return;
	params.processed_nodes.insert(node);

	// ValueNodeConst
	if (ValueNode_Const::Handle value_node = ValueNode_Const::Handle::cast_dynamic(node))
	{
		// allow to process valuenodes without canvas
		if ( value_node->get_parent_canvas()
		  && value_node->get_parent_canvas()->get_root() != params.canvas)
			return;

		ValueBase value = value_node->get_value();

		if (self)
		{
			if (value.same_type_as(String()))
			{
				String filename = value.get(String());
				String new_filename;
				process_filename(params, filename, new_filename);
				if (filename != new_filename)
				{
					params.processed_valuenodes[value_node] = filename;
					value_node->set_value(new_filename);
				}
				return;
			}
			warning("Cannot process filename for node: %s", node->get_string().c_str());
		}

		if (value.can_get(Canvas::Handle()))
			process_filenames(params, value.get(Canvas::Handle()));
		return;
	}

	// ValueNode_Animated
	if (ValueNode_Animated::Handle animated = ValueNode_Animated::Handle::cast_dynamic(node))
	{
		const WaypointList &waypoints = animated->waypoint_list();
		for(WaypointList::const_iterator i = waypoints.begin(); i != waypoints.end(); ++i)
			process_filenames(params, i->get_value_node(), self);
		return;
	}

	// Followed node-types cannot by processed by it self (childs only)
	if (self)
		warning("Cannot process filename for node: %s", node->get_string().c_str());

	// Canvas
	if (Canvas::Handle canvas = Canvas::Handle::cast_dynamic(node))
	{
		if (canvas->get_root() != params.canvas) return;

		// exported values
		if (canvas->is_inline())
		{
			const ValueNodeList &list = canvas->value_node_list();
			for(ValueNodeList::const_iterator i = list.begin(); i != list.end(); ++i)
				process_filenames(params, *i);
		}

		// layers
		for(Canvas::const_iterator i = canvas->begin(); i != canvas->end(); ++i)
			process_filenames(params, *i);

		return;
	}

	// Layer
	if (Layer::Handle layer = Layer::Handle::cast_dynamic(node))
	{
		// skip layers without canvas
		if ( !layer->get_canvas()
		   || layer->get_canvas()->get_root() != params.canvas)
			return;

		const ParamVocab vocab = layer->get_param_vocab();
		const Layer::DynamicParamList &dynamic_params = layer->dynamic_param_list();
		for(ParamVocab::const_iterator i = vocab.begin(); i != vocab.end(); ++i)
		{
			Layer::DynamicParamList::const_iterator j = dynamic_params.find(i->get_name());
			ValueNode::Handle value_node = j == dynamic_params.end() ? ValueNode::Handle() : ValueNode::Handle(j->second);

			bool is_filename = i->get_hint() == "filename";
			if (value_node)
			{
				process_filenames(params, value_node, is_filename);
				continue;
			}

			ValueBase value = layer->get_param(i->get_name());

			if (value.can_get(Canvas::Handle()))
				process_filenames(params, value.get(Canvas::Handle()));

			if (!is_filename || !value.same_type_as(String()))
				continue;

			String filename = value.get(String());
			String new_filename;
			process_filename(params, filename, new_filename);
			if (filename != new_filename)
			{
				params.processed_params[std::make_pair(layer, i->get_name())] = filename;
				layer->set_param(i->get_name(), new_filename);
			}
		}
		return;
	}

	// LinkableValueNode
	if (LinkableValueNode::Handle linkable = LinkableValueNode::Handle::cast_dynamic(node))
	{
		// allow to process valuenodes without canvas
		if ( linkable->get_parent_canvas()
		  && linkable->get_parent_canvas()->get_root() != params.canvas)
			return;

		const ParamVocab& vocab = linkable->get_children_vocab();
		for(ParamVocab::const_iterator i = vocab.begin(); i != vocab.end(); ++i)
			process_filenames(params, ValueNode::Handle(linkable->get_link(i->get_name())), i->get_hint() == "filename");

		return;
	}
}

void
Instance::process_filenames_undo(const ProcessFilenamesParams &params)
{
	// restore layer params
	for(std::map<std::pair<Layer::Handle, String>, String>::const_iterator i = params.processed_params.begin(); i != params.processed_params.end(); ++i)
		i->first.first->set_param(i->first.second, i->second);
	// restore value nodes
	for(std::map<ValueNode_Const::Handle, String>::const_iterator i = params.processed_valuenodes.begin(); i != params.processed_valuenodes.end(); ++i)
		i->first->set_value(i->second);
}

void
Instance::find_unsaved_layers(std::vector<synfig::Layer::Handle> &out_layers, const synfig::Canvas::Handle canvas)
{
	for(Canvas::const_iterator i = canvas->begin(); i != canvas->end(); ++i)
	{
		if (Layer_PasteCanvas::Handle layer_pastecanvas = Layer_PasteCanvas::Handle::cast_dynamic(*i))
			if (Canvas::Handle sub_canvas = layer_pastecanvas->get_sub_canvas())
				find_unsaved_layers(out_layers, sub_canvas);
		if (Layer_Bitmap::Handle layer_bitmap = Layer_Bitmap::Handle::cast_dynamic(*i))
			if (layer_bitmap->is_surface_modified())
				out_layers.push_back(layer_bitmap);
	}
}

bool
Instance::save()
{
	return save_as(get_canvas()->get_file_name());
}

bool
Instance::save_layer(const synfig::Layer::Handle &layer)
{
	if (Layer_Bitmap::Handle layer_bitmap = Layer_Bitmap::Handle::cast_dynamic(layer))
	{
		if ( layer_bitmap->is_surface_modified()
		  && layer_bitmap->get_param_list().count("filename"))
		{
			ValueBase value = layer_bitmap->get_param("filename");
			if (value.same_type_as(String()))
			{
				String filename = value.get(String());
				if (save_surface(layer_bitmap->rendering_surface, filename))
					return true;
				error("Cannot save image: %s", filename.c_str());
				return false;
			}
		}
	}
	error("Don't know how to save layer type: %s", layer->get_name().c_str());
	return false;
}

void
Instance::save_all_layers()
{
	std::vector<Layer::Handle> layers;
	find_unsaved_layers(layers);
	for(std::vector<Layer::Handle>::const_iterator i = layers.begin(); i != layers.end(); ++i)
		save_layer(*i);
}

bool
Instance::backup(bool save_even_if_unchanged)
{
	if (!get_action_count() && !save_even_if_unchanged)
		return true;
	FileSystemTemporary::Handle temporary_filesystem = FileSystemTemporary::Handle::cast_dynamic(get_canvas()->get_file_system());

	if (!temporary_filesystem)
	{
		warning("Cannot backup, canvas was not attached to temporary file system: %s", get_file_name().c_str());
		return false;
	}
	// don't save images while backup
	//if (success)
	//	save_all_layers();
	if (!save_canvas(get_canvas()->get_identifier(), get_canvas(), false))
		return false;
	
	return temporary_filesystem->save_temporary();
}

bool
Instance::save_as(const synfig::String &file_name)
{
	Canvas::Handle canvas = get_canvas();

	FileSystem::Identifier previous_canvas_identifier = canvas->get_identifier();
	FileSystem::Handle previous_canvas_filesystem = previous_canvas_identifier.file_system;
	FileSystem::Handle previous_container = get_container();
	String previous_canvas_filename = canvas->get_file_name();

	FileSystem::Identifier new_canvas_identifier = previous_canvas_identifier;
	FileSystem::Handle new_canvas_filesystem = previous_canvas_filesystem;
	FileSystem::Handle new_container = previous_container;
	String new_canvas_filename = file_name;
	bool is_filename_changed = previous_canvas_filename != new_canvas_filename;

	if (is_filename_changed)
	{
		// new canvas filesystem
		FileSystem::Handle new_container = CanvasFileNaming::make_filesystem_container(new_canvas_filename, 0, true);
		if (!new_container)
		{
			warning("Cannot create container: %s", new_canvas_filename.c_str());
			return false;
		}
		new_canvas_filesystem = CanvasFileNaming::make_filesystem(new_container);
		if (!new_canvas_filesystem)
		{
			warning("Cannot create canvas filesysem for: %s", new_canvas_filename.c_str());
			return false;
		}

		// wrap into temporary file system
		if (FileSystemTemporary::Handle previous_temporary_filesystem = FileSystemTemporary::Handle::cast_dynamic(previous_canvas_filesystem))
		{
			FileSystemTemporary::Handle new_temporary_filesystem = new FileSystemTemporary(
				previous_temporary_filesystem->get_tag(),
				previous_temporary_filesystem->get_temporary_directory(),
				new_canvas_filesystem );
			new_temporary_filesystem->set_meta("filename", new_canvas_filename);
			new_temporary_filesystem->set_meta("as", new_canvas_filename);
			new_temporary_filesystem->set_meta("truncate", "0");
			new_canvas_filesystem = new_temporary_filesystem;
		}

		new_canvas_identifier = new_canvas_filesystem->get_identifier(CanvasFileNaming::project_file(new_canvas_filename));

		// copy embedded files
		if (!FileSystem::copy_recursive(
			previous_canvas_filesystem,
			CanvasFileNaming::container_prefix,
			new_canvas_filesystem,
			CanvasFileNaming::container_prefix ))
		{
			//new_canvas_filesystem->remove_recursive(CanvasFileNaming::container_prefix);
			new_canvas_filesystem.reset();
			new_container.reset();
			//FileSystemNative::instance()->file_remove(new_canvas_filename);
			return false;
		}

		// remove previous canvas file
		if (previous_canvas_identifier.filename != new_canvas_identifier.filename)
			new_canvas_filesystem->file_remove(previous_canvas_identifier.filename.u8string());

		// set new canvas filename
		canvas->set_file_name(new_canvas_filename);
		canvas->set_identifier(new_canvas_identifier);
		container_ = new_container;
	}

	// save bitmaps
	save_all_layers();

	// find zip-container
	FileContainerZip::Handle new_container_zip = FileContainerZip::Handle::cast_dynamic(new_container);
	bool embed_files = (bool)new_container_zip;
	bool save_files = true;

	etl::handle<Action::Group> import_external_canvases_action;
	if (embed_files)
		import_external_canvases_action = import_external_canvases();

	// process filenames
	ProcessFilenamesParams params(
		canvas,
		previous_canvas_filesystem,
		previous_canvas_filename,
		(bool)embed_files,
		save_files );
	process_filenames(params, canvas);

	// save
	bool success = true;
	if (success)
		success = save_canvas(new_canvas_identifier, canvas, false);
	if (success)
		if (FileSystemTemporary::Handle temporary_filesystem = FileSystemTemporary::Handle::cast_dynamic(new_canvas_filesystem))
			success = temporary_filesystem->save_changes();
	if (success && new_container_zip)
		success = new_container_zip->save();
	if (success)
		reset_action_count();

	if (success)
	{
		signal_saved_();
		signal_filename_changed_();
		return true;
	}

	// undo
	canvas->set_file_name(previous_canvas_filename);
	canvas->set_identifier(previous_canvas_identifier);
	container_ = previous_container;
	process_filenames_undo(params);
	//new_canvas_filesystem->remove_recursive(CanvasFileNaming::container_prefix);
	new_canvas_filesystem.reset();
	new_container.reset();
	//FileSystemNative::instance()->file_remove(new_canvas_filename);
	if (import_external_canvases_action)
		import_external_canvases_action->undo();

	return false;
}

void
Instance::generate_new_name(
	const Layer::Handle &layer,
	String &out_description,
	String &out_filename,
	String &out_filename_param )
{
	String filename;
	if (layer->get_param_list().count("filename")) {
		ValueBase value = layer->get_param("filename");
		if (value.same_type_as(String()))
			filename = filesystem::Path::basename(value.get(String()));
	}

	if (filename.empty())
		filename = !layer->get_description().empty()
        		 ? layer->get_description()
        		 : layer->get_local_name();
	if (CanvasFileNaming::filename_extension_lower(filename) != "png")
		filename += ".png";

	assert(get_canvas()->get_file_system());
	String short_filename = CanvasFileNaming::generate_container_filename(get_canvas()->get_file_system(), filename);
	String full_filename = CanvasFileNaming::make_full_filename(get_canvas()->get_file_name(), short_filename);
	String base = filesystem::Path::filename_sans_extension(CanvasFileNaming::filename_base(short_filename));

	out_description = base;
	out_filename = full_filename;
	out_filename_param = short_filename;
}
