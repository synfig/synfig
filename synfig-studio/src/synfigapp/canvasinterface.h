/* === S Y N F I G ========================================================= */
/*!	\file canvasinterface.h
**	\brief Template Header
**
**	$Id$
**
**	\legal
**	Copyright (c) 2002-2005 Robert B. Quattlebaum Jr., Adrian Bentley
**	Copyright (c) 2007, 2008 Chris Moore
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

#ifndef __SYNFIG_APP_CANVASINTERFACE_H
#define __SYNFIG_APP_CANVASINTERFACE_H

/* === H E A D E R S ======================================================= */

//#include <synfig/canvas.h>
#include <synfig/value.h>
#include <sigc++/sigc++.h>
#include <list>
#include "selectionmanager.h"
#include "uimanager.h"
#include "value_desc.h"
#include "editmode.h"

/* === M A C R O S ========================================================= */

/* === T Y P E D E F S ===================================================== */

/* === C L A S S E S & S T R U C T S ======================================= */

namespace synfig { class ValueNode_DynamicList; class Waypoint; class GUIDSet; class Canvas; };

namespace synfigapp {

namespace Action { class ParamList; class Param; class EditModeSet; };

class Instance;
class ValueDesc;

class CanvasInterface : public etl::shared_object, public sigc::trackable
{
	friend class Instance;
	friend class Action::EditModeSet;

public:

	typedef EditMode Mode;

private:
	// Constructor is private to force the use of the "create()" constructor.
	CanvasInterface(etl::loose_handle<Instance> instance,etl::handle<synfig::Canvas> canvas);

private:
	etl::loose_handle<Instance> instance_;
	etl::handle<synfig::Canvas> canvas_;
	etl::handle<SelectionManager> selection_manager_;
	etl::handle<UIInterface> ui_interface_;
	synfig::Time cur_time_;
	Mode mode_;

	sigc::signal<void,synfig::Layer::Handle> signal_layer_raised_;
	sigc::signal<void,synfig::Layer::Handle> signal_layer_lowered_;
	sigc::signal<void,synfig::Layer::Handle,int> signal_layer_inserted_;
	sigc::signal<void,synfig::Layer::Handle,int,synfig::Canvas::Handle> signal_layer_moved_;
	sigc::signal<void,synfig::Layer::Handle> signal_layer_removed_;
	sigc::signal<void,synfig::Layer::Handle,bool> signal_layer_status_changed_;
	sigc::signal<void,synfig::Layer::Handle,bool> signal_layer_exclude_from_rendering_changed_;
	sigc::signal<void,synfig::Layer::Handle,bool> signal_layer_z_range_changed_;
	sigc::signal<void,synfig::Layer::Handle,synfig::String> signal_layer_new_description_;
	sigc::signal<void,synfig::Canvas::Handle> signal_canvas_added_;
	sigc::signal<void,synfig::Canvas::Handle> signal_canvas_removed_;

	sigc::signal<void,synfig::ValueNode::Handle> signal_value_node_added_;
	sigc::signal<void,synfig::ValueNode::Handle> signal_value_node_deleted_;
	sigc::signal<void,synfig::ValueNode::Handle,synfig::ValueNode::Handle> signal_value_node_replaced_;

	sigc::signal<void,synfig::Keyframe> signal_keyframe_added_;
	sigc::signal<void,synfig::Keyframe> signal_keyframe_removed_;
	sigc::signal<void,synfig::Keyframe> signal_keyframe_changed_;
	sigc::signal<void,synfig::Keyframe> signal_keyframe_selected_;
	sigc::signal<void> signal_keyframe_properties_;

	sigc::signal<void> signal_id_changed_;

	sigc::signal<void> signal_time_changed_;

	sigc::signal<void> signal_rend_desc_changed_;

	sigc::signal<void,Mode> signal_mode_changed_;

	//sigc::signal<void> signal_dirty_preview_;

	sigc::signal<void,synfig::Layer::Handle,synfig::String> signal_layer_param_changed_;

public:	// Signal Interface

	sigc::signal<void,synfig::Layer::Handle,int,synfig::Canvas::Handle>& signal_layer_moved() { return signal_layer_moved_; }

	sigc::signal<void,synfig::Layer::Handle,synfig::String>& signal_layer_new_description() { return signal_layer_new_description_; }

	//! Signal called when layer is raised.
	sigc::signal<void,synfig::Layer::Handle>& signal_layer_raised() { return signal_layer_raised_; }

	//! Signal called when layer is lowered.
	sigc::signal<void,synfig::Layer::Handle>& signal_layer_lowered() { return signal_layer_lowered_; }

	//! Signal called when layer has been inserted at a given position.
	sigc::signal<void,synfig::Layer::Handle,int>& signal_layer_inserted() { return signal_layer_inserted_; }

	//! Signal called when a layer has been removed from the canvas.
	sigc::signal<void,synfig::Layer::Handle>& signal_layer_removed() { return signal_layer_removed_; }

	//! Signal called when the layer's active status has changed.
	sigc::signal<void,synfig::Layer::Handle,bool>& signal_layer_status_changed() { return signal_layer_status_changed_; }

	//! Signal called when the layer's "exclude from rendering" flag has changed.
	sigc::signal<void,synfig::Layer::Handle,bool>& signal_layer_exclude_from_rendering_changed() { return signal_layer_exclude_from_rendering_changed_; }

	//! Signal called when the layer's zdepth range has changed. This layer has to be Layer_PasteCanvas
	sigc::signal<void,synfig::Layer::Handle,bool>& signal_layer_z_range_changed() { return signal_layer_z_range_changed_; }
	
	//! Signal called when a canvas has been added.
	sigc::signal<void,etl::handle<synfig::Canvas> >& signal_canvas_added() { return signal_canvas_added_; }

	//! Signal called when a canvas has been removed.
	sigc::signal<void,etl::handle<synfig::Canvas> >& signal_canvas_removed() { return signal_canvas_removed_; }

	//! Signal called when a layer's parameter has been changed
	sigc::signal<void,synfig::Layer::Handle,synfig::String>& signal_layer_param_changed() { return signal_layer_param_changed_; }

	//! Signal called when the canvas's preview needs to be updated
	//sigc::signal<void>& signal_dirty_preview() { return signal_dirty_preview_; }
	sigc::signal<void>& signal_dirty_preview() { return get_canvas()->signal_dirty(); }

	sigc::signal<void,etl::handle<synfig::ValueNode>,etl::handle<synfig::ValueNode> >&
		signal_value_node_child_added() { return get_canvas()->signal_value_node_child_added(); }
	sigc::signal<void,etl::handle<synfig::ValueNode>,etl::handle<synfig::ValueNode> >&
		signal_value_node_child_removed() { return get_canvas()->signal_value_node_child_removed(); }

	//! Signal called when a ValueNode has changed
	sigc::signal<void,etl::handle<synfig::ValueNode> >& signal_value_node_added() { return signal_value_node_added_; }

	//! Signal called when a ValueNode has been deleted
	sigc::signal<void,etl::handle<synfig::ValueNode> >& signal_value_node_deleted() { return signal_value_node_deleted_; }

	//! Signal called when a ValueNode has been changed
	sigc::signal<void,etl::handle<synfig::ValueNode> >& signal_value_node_changed() { return get_canvas()->signal_value_node_changed(); }

	//! Signal called when a ValueNode has been renamed
	sigc::signal<void,etl::handle<synfig::ValueNode> >& signal_value_node_renamed() { return get_canvas()->signal_value_node_renamed(); }

	//! Signal called when the mode has changed
	sigc::signal<void,Mode> signal_mode_changed() { return signal_mode_changed_; }

	//! Signal called when a the ID has been changed
	sigc::signal<void>& signal_id_changed() { return signal_id_changed_; }

	//! Signal called whenever the time changes
	sigc::signal<void> signal_time_changed() { return signal_time_changed_; }

	//! Signal called whenever a data node has been replaced.
	/*!	Second ValueNode replaces first */
	sigc::signal<void,synfig::ValueNode::Handle,synfig::ValueNode::Handle>& signal_value_node_replaced()
		{ return signal_value_node_replaced_; }

	//! Signal called whenever the RendDesc changes
	sigc::signal<void>& signal_rend_desc_changed() { return signal_rend_desc_changed_; }

	//! Signal called when a keyframe is added
	sigc::signal<void,synfig::Keyframe>& signal_keyframe_added() { return signal_keyframe_added_; }
	//! Signal called when a keyframe is removed
	sigc::signal<void,synfig::Keyframe>& signal_keyframe_removed() { return signal_keyframe_removed_; }
	//! Signal called when a keyframe is changed
	sigc::signal<void,synfig::Keyframe>& signal_keyframe_changed() { return signal_keyframe_changed_; }
	//! Signal called when a keyframe is selected
	/*!	Second parameter (void*) hold 'this*' of the signal emiter class (to prevent endless loop)*/
	sigc::signal<void,synfig::Keyframe>& signal_keyframe_selected() { return signal_keyframe_selected_; }
	//! Signal called when the properties dialog of the selected keyframe must be shown
	sigc::signal<void>& signal_keyframe_properties() { return signal_keyframe_properties_; }

public:

	void auto_export(const ValueDesc& value_desc);

	void auto_export(synfig::ValueNode::Handle value_node);

	void set_meta_data(const synfig::String& key,const synfig::String& data);

	void erase_meta_data(const synfig::String& key);

	//! Changes the current SelectionManager object
	void set_selection_manager(const etl::handle<SelectionManager> &sm) { selection_manager_=sm; }

	//! Disables the selection manager
	void unset_selection_manager() { selection_manager_=new NullSelectionManager(); }

	//! Returns a handle to the current SelectionManager
	const etl::handle<SelectionManager> &get_selection_manager()const { return selection_manager_; }

	//! Changes the current UIInterface object
	void set_ui_interface(const etl::handle<UIInterface> &uim) { ui_interface_=uim; }

	//! Disables the UIInterface
	void unset_ui_interface() { ui_interface_=new DefaultUIInterface(); }

	//! Returns a handle to the current UIInterface
	const etl::handle<UIInterface> &get_ui_interface() { return ui_interface_; }

	//! Returns the Canvas associated with this interface
	const etl::handle<synfig::Canvas>& get_canvas()const { return canvas_; }

	//! Returns the Instance associated with this interface
	const etl::loose_handle<Instance>& get_instance()const { return instance_; }

	//! Changes the name of the canvas. Undoable.
	void set_name(const synfig::String &x);

	//! Changes the description of the canvas. Undoable.
	void set_description(const synfig::String &x);

	//! Changes the ID of the canvas. Undoable.
	void set_id(const synfig::String &x);

	//! Convenience function to retrieve the name of the canvas
	synfig::String get_name()const { return get_canvas()->get_name(); }

	//! Convenience function to retrieve the description of the canvas
	synfig::String get_description()const { return get_canvas()->get_description(); }

	//! Convenience function to retrieve the ID of the canvas
	synfig::String get_id()const { return get_canvas()->get_id(); }

	//! Sets the current time
	void set_time(synfig::Time x);

	//! Retrieves the current time
	synfig::Time get_time()const;

	//! Changes the current time to the next keyframe
	void jump_to_next_keyframe();

	//! Changes the current time to the next keyframe
	void jump_to_prev_keyframe();

	void seek_frame(int frames);

	void seek_time(synfig::Time time);

	//! \writeme
	void refresh_current_values();

	//! Sets the current editing mode
	/*! \see Mode */
	void set_mode(Mode x);

	//! Retrieves the current editing mode
	/*! \see Mode */
	Mode get_mode()const;



	//! Creates a new layer, of type \c id at the top of the layer stack
	synfig::Layer::Handle add_layer_to(const synfig::String &id, const synfig::Canvas::Handle &canvas, int depth = 0);

	//! Stage 1/4 of add_layer_to. Create new layer and assign canvas
	synfig::Layer::Handle layer_create(const synfig::String &id, const synfig::Canvas::Handle &canvas);
	//! Stage 2/4 of add_layer_to. Apply default parameters (canvas should be assigned before)
	void layer_set_defaults(const synfig::Layer::Handle &layer);
	//! Stage 3/4 of add_layer_to. Perform action to add the layer
	bool layer_add_action(const synfig::Layer::Handle &layer);
	//! Stage 4/4 of add_layer_to. Perform action to move the layer (set depth)
	bool layer_move_action(const synfig::Layer::Handle &layer, int depth);


	bool convert(ValueDesc value_desc, synfig::String type);
	//! Adds the given ValueNode to the canvas.
	bool add_value_node(synfig::ValueNode::Handle value_node, synfig::String name);


	Action::ParamList generate_param_list(const synfigapp::ValueDesc &);

	Action::ParamList generate_param_list(const std::list<synfigapp::ValueDesc> &);

	void set_rend_desc(const synfig::RendDesc &rend_desc);

	bool import(const synfig::String &filename, synfig::String &errors, synfig::String &warnings, bool resize_image=false);


	void waypoint_duplicate(synfigapp::ValueDesc value_desc,synfig::Waypoint waypoint);
	void waypoint_duplicate(synfig::ValueNode::Handle value_node,synfig::Waypoint waypoint);

	void waypoint_remove(synfigapp::ValueDesc value_desc,synfig::Waypoint waypoint);
	void waypoint_remove(synfig::ValueNode::Handle value_node,synfig::Waypoint waypoint);

	bool change_value(synfigapp::ValueDesc value_desc,synfig::ValueBase new_value,bool lock_animation = false);


	int find_important_value_descs(std::vector<synfigapp::ValueDesc>& out);
	static int find_important_value_descs(synfig::Canvas::Handle canvas,std::vector<synfigapp::ValueDesc>& out,synfig::GUIDSet& guid_set);

	~CanvasInterface();

	static etl::handle<CanvasInterface> create(etl::loose_handle<Instance> instance,etl::handle<synfig::Canvas> canvas);
}; // END of class CanvasInterface

/*!	\class PushMode
**	\brief Class that changes the mode of a CanvasInterface, and restores it on destruction.
*/
class PushMode
{
	CanvasInterface* canvas_interface_;
	CanvasInterface::Mode old_mode_;
public:
	PushMode(etl::loose_handle<CanvasInterface> c, CanvasInterface::Mode mode):
		canvas_interface_(c.get()), old_mode_(canvas_interface_->get_mode())
	{ canvas_interface_->set_mode(mode); }

	~PushMode() { canvas_interface_->set_mode(old_mode_); }
}; // END of class PushMode

}; // END of namespace synfigapp

/* === E N D =============================================================== */

#endif
