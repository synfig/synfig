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
#include <synfig/filecontainertemporary.h>
#include <synfig/filesystemgroup.h>
#include <list>
#include <set>
#include <sigc++/signal.h>
#include <sigc++/object.h>
#include "action_system.h"
#include "selectionmanager.h"
#include "cvs.h"

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

	etl::handle< synfig::FileSystemGroup > file_system_;
	etl::handle< synfig::FileContainerTemporary > container_;

	CanvasInterfaceList canvas_interface_list_;

	sigc::signal<void> signal_filename_changed_;
	sigc::signal<void> signal_saved_;
	etl::handle<SelectionManager> selection_manager_;

	bool save_canvas_into_container_;
	std::string save_canvas_reference_directory_;
	std::string save_canvas_reference_local_directory_;
	FileReferenceList save_canvas_references_;
	std::list< synfig::Layer::Handle > layers_to_save;

	static bool save_canvas_callback(void *instance_ptr, synfig::Layer::ConstHandle layer, const std::string &param_name, std::string &filename);
	void update_references_in_canvas(synfig::Canvas::Handle canvas);
	bool import_external_canvas(synfig::Canvas::Handle canvas, std::map<synfig::Canvas*, synfig::Canvas::Handle> &imported);
	void import_external_canvases();
	void embed_all(synfig::Canvas::Handle canvas, bool &success, bool &restart);

protected:
	Instance(etl::handle<synfig::Canvas>, etl::handle< synfig::FileContainerTemporary > container);

	/*
 -- ** -- P U B L I C   M E T H O D S -----------------------------------------
	*/

public:
	~Instance();

	bool is_layer_registered_to_save(synfig::Layer::Handle layer) {
		for(std::list<synfig::Layer::Handle>::iterator i = layers_to_save.begin(); i != layers_to_save.end(); i++)
			if (*i == layer) return true;
		return false;
	}
	void register_layer_to_save(synfig::Layer::Handle layer) { layers_to_save.push_back(layer); }
	void unregister_layer_to_save(synfig::Layer::Handle layer)
	{
		for(std::list<synfig::Layer::Handle>::iterator i = layers_to_save.begin(); i != layers_to_save.end(); i++)
			if (*i == layer) { layers_to_save.erase(i); break; }
	}

	void set_selection_manager(const etl::handle<SelectionManager> &sm) { assert(sm); selection_manager_=sm; }
	void unset_selection_manager() { selection_manager_=new NullSelectionManager(); }
	const etl::handle<SelectionManager> &get_selection_manager() { return selection_manager_; }

	etl::handle< synfig::FileSystemGroup > get_file_system() const { return file_system_; };
	etl::handle< synfig::FileContainerTemporary > get_container() const { return container_; };
	void save_surface(const synfig::Surface &surface, const synfig::String &filename);

	etl::handle<CanvasInterface> find_canvas_interface(synfig::Canvas::Handle canvas);

	synfig::Canvas::Handle get_canvas()const { return canvas_; }

	bool embed_all();

	void convert_animated_filenames(const synfig::Canvas::Handle &canvas, const synfig::String &old_path, const synfig::String &new_path);

	//! Saves the instance to filename_
	bool save();

	bool save_as(const synfig::String &filename);

	//! create unique file name for an embedded image layer (if image filename is empty, description layer is used)
	bool generate_new_name(
			synfig::Layer::Handle layer,
			synfig::Canvas::Handle canvas,
			synfig::FileSystem::Handle file_system,
			synfig::String &out_description,
			synfig::String &out_filename,
			synfig::String &out_filename_param);

public:	// Interfaces to internal information
	sigc::signal<void>& signal_filename_changed() { return signal_filename_changed_; }
	sigc::signal<void>& signal_saved() { return signal_saved_; }

	CanvasInterfaceList & canvas_interface_list() { return canvas_interface_list_; }
	const CanvasInterfaceList & canvas_interface_list()const { return canvas_interface_list_; }

	synfig::String get_file_name()const;

	void set_file_name(const synfig::String &name);

public:


public:	// Constructor interfaces
	static etl::handle<Instance> create(etl::handle<synfig::Canvas> canvas, etl::handle< synfig::FileContainerTemporary > container);
}; // END class Instance

etl::handle<Instance> find_instance(etl::handle<synfig::Canvas> canvas);

bool is_editable(synfig::ValueNode::Handle value_node);

}; // END namespace studio

/* === E N D =============================================================== */

#endif
