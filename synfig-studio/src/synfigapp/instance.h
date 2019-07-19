/* === S Y N F I G ========================================================= */
/*!	\file synfigapp/instance.h
**	\brief writeme
**
**	$Id$
**
**	\legal
**	Copyright (c) 2002-2005 Robert B. Quattlebaum Jr., Adrian Bentley
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

/* === S T A R T =========================================================== */

#ifndef __SYNFIG_APP_INSTANCE_H
#define __SYNFIG_APP_INSTANCE_H

/* === H E A D E R S ======================================================= */

#include "action.h"
#include <ETL/handle>
#include <synfig/canvas.h>
#include <synfig/string.h>
#include <synfig/filesystemtemporary.h>
#include <synfig/filesystemgroup.h>
#include <list>
#include <set>
#include <sigc++/sigc++.h>
#include "action_system.h"
#include "selectionmanager.h"
#include "cvs.h"
#include <synfig/rendering/surface.h>

/* === M A C R O S ========================================================= */

/* === T Y P E D E F S ===================================================== */

/* === C L A S S E S & S T R U C T S ======================================= */

namespace synfigapp {

class CanvasInterface;


class Instance : public Action::System , public CVSInfo
{
	friend class PassiveGrouper;
	/*
 -- ** -- P U B L I C   T Y P E S ---------------------------------------------
	*/

public:

	typedef std::list< etl::handle<CanvasInterface> > CanvasInterfaceList;

	struct FileReference
	{
		synfig::Layer::ConstHandle layer;
		std::string param_name;
		std::string old_filename;
		std::string new_filename;
	};

	typedef std::list< FileReference > FileReferenceList;

	using etl::shared_object::ref;
	using etl::shared_object::unref;

	/*
 -- ** -- P U B L I C  D A T A ------------------------------------------------
	*/

public:

	/*
 -- ** -- P R I V A T E   D A T A ---------------------------------------------
	*/

private:

	//! Handle for root canvas
	synfig::Canvas::Handle canvas_;

	synfig::FileSystem::Handle container_;

	CanvasInterfaceList canvas_interface_list_;

	sigc::signal<void> signal_filename_changed_;
	sigc::signal<void> signal_saved_;
	etl::handle<SelectionManager> selection_manager_;

	std::list< synfig::Layer::Handle > layers_to_save;

	bool import_external_canvas(synfig::Canvas::Handle canvas, std::map<synfig::Canvas*, synfig::Canvas::Handle> &imported);
	etl::handle<Action::Group> import_external_canvases();

	struct ProcessFilenamesParams
	{
		synfig::Canvas::Handle canvas;
		synfig::FileSystem::Handle previous_canvas_filesystem;
		synfig::String previous_canvas_filename;
		bool embed_files;
		bool save_files;

		mutable std::set<synfig::NodeHandle> processed_nodes;
		mutable std::map<synfig::String, synfig::String> processed_files;

		mutable std::map<std::pair<synfig::Layer::Handle, synfig::String>, synfig::String> processed_params;
		mutable std::map<synfig::ValueNode_Const::Handle, synfig::String> processed_valuenodes;

		ProcessFilenamesParams(): embed_files(), save_files() { }
		ProcessFilenamesParams(
			const synfig::Canvas::Handle &canvas,
			const synfig::FileSystem::Handle &previous_canvas_filesystem,
			const synfig::String &previous_canvas_filename,
			bool embed_files,
			bool save_files
		):
			canvas(canvas),
			previous_canvas_filesystem(previous_canvas_filesystem),
			previous_canvas_filename(previous_canvas_filename),
			embed_files(embed_files),
			save_files(save_files)
		{ }
	};

	// embed and refine filenames used in layers and valuenodes in current instance
	void process_filename(const ProcessFilenamesParams &params, const synfig::String &filename, synfig::String &out_filename);
	void process_filenames(const ProcessFilenamesParams &params, const synfig::NodeHandle &node, bool self = false);
	void process_filenames_undo(const ProcessFilenamesParams &params);

protected:
	Instance(etl::handle<synfig::Canvas>, synfig::FileSystem::Handle container);

	/*
 -- ** -- P U B L I C   M E T H O D S -----------------------------------------
	*/

public:
	~Instance();

	void set_selection_manager(const etl::handle<SelectionManager> &sm) { assert(sm); selection_manager_=sm; }
	void unset_selection_manager() { selection_manager_=new NullSelectionManager(); }
	const etl::handle<SelectionManager> &get_selection_manager() { return selection_manager_; }

	synfig::FileSystem::Handle get_container() const { return container_; };
	bool save_surface(const synfig::rendering::SurfaceResource::Handle &surface, const synfig::String &filename);
	bool save_surface(const synfig::Surface &surface, const synfig::String &filename);
	bool save_layer(const synfig::Layer::Handle &layer);
	void save_all_layers();
	void find_unsaved_layers(std::vector<synfig::Layer::Handle> &out_layers, const synfig::Canvas::Handle canvas);
	void find_unsaved_layers(std::vector<synfig::Layer::Handle> &out_layers)
		{ find_unsaved_layers(out_layers, get_canvas()); }

	etl::handle<CanvasInterface> find_canvas_interface(synfig::Canvas::Handle canvas);

	const synfig::Canvas::Handle& get_canvas()const { return canvas_; }

	void convert_animated_filenames(const synfig::Canvas::Handle &canvas, const synfig::String &old_path, const synfig::String &new_path);

	//! Saves the instance to filename_
	bool save();

	bool save_as(const synfig::String &filename);

	//! Saves the instance to current temporary container
	bool backup();

	//! generate layer name (also known in code as 'description')
	synfig::String generate_new_description(const synfig::Layer::Handle &layer);

	//! create unique file name for an embedded image layer (if image filename is empty, description layer is used)
	void generate_new_name(
		const synfig::Layer::Handle &layer,
		synfig::String &out_description,
		synfig::String &out_filename,
		synfig::String &out_filename_param );

public:	// Interfaces to internal information
	sigc::signal<void>& signal_filename_changed() { return signal_filename_changed_; }
	sigc::signal<void>& signal_saved() { return signal_saved_; }

	CanvasInterfaceList & canvas_interface_list() { return canvas_interface_list_; }
	const CanvasInterfaceList & canvas_interface_list()const { return canvas_interface_list_; }

	synfig::String get_file_name()const;

	void set_file_name(const synfig::String &name);

public:


public:	// Constructor interfaces
	static etl::handle<Instance> create(etl::handle<synfig::Canvas> canvas, synfig::FileSystem::Handle container);
}; // END class Instance

etl::handle<Instance> find_instance(etl::handle<synfig::Canvas> canvas);

bool is_editable(synfig::ValueNode::Handle value_node);

}; // END namespace studio

/* === E N D =============================================================== */

#endif
