/* === S Y N F I G ========================================================= */
/*!	\file canvas.h
**	\brief Canvas Class Implementation
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

#ifndef __SYNFIG_CANVAS_H
#define __SYNFIG_CANVAS_H

/* === H E A D E R S ======================================================= */

#include <map>
#include <list>
#include <ETL/handle>
#include <sigc++/signal.h>
#include <sigc++/connection.h>

#include "vector.h"
#include "string.h"
#include "canvasbase.h"
#include "valuenode.h"
#include "keyframe.h"
#include "renddesc.h"
#include "node.h"
#include "guid.h"
#include "filesystem.h"

/* === M A C R O S ========================================================= */

/* version change history:
 *
 * 0.1: the original version
 *
 *      if a waypoint goes from -179 to 179 degrees, that is a 2
 *      degree change.  there's no way to express a 720 degree
 *      rotation with a single pair of waypoints
 *
 * 0.2: svn r1227
 *
 *      angles no longer wrap at -180 degrees back to 180 degrees; if
 *      a waypoint goes from -179 to 179 degrees, that is a rotation
 *      of 358 degrees.  loading a version 0.1 canvas will modify
 *      constant angle waypoints to that they are within 180 degrees
 *      of the previous waypoint's value
 *
 *      the 'straight' blend method didn't used to work properly.  it
 *      didn't work at all on transparent pixels in layers other than
 *      the PasteCanvas layer.  for example, the examples/japan.sifz
 *      file has a red circle (straight, amount=1.0) on top of a
 *      striped conical gradient.  if 'straight' was working, the
 *      conical gradient would be entirely obscured by the circle
 *      layer (even by its transparent pixels)
 *
 * 0.3: svn r1422
 *
 *      the 'straight' blend method was fixed.  loading a version 0.2
 *      or older canvas will replace the 'straight' blend method in
 *      non-pastecanvas layers with 'composite', unless they're
 *      completely transparent, in which case it will replace them
 *      with an  'alpha over' blend instead.  Images like
 *      examples/logo.sifz use transparent straight blends to do
 *      masking, which no longer works now that 'straight' blending is
 *      fixed.
 *
 *      Tangent lengths calculated by the "Segment Tangent" and "BLine
 *      Tangent" ValueNodes were scaled by a factor of 0.5.
 *
 * 0.4: svn r1856
 *
 *      Stop scaling tangents by 0.5.
 *
 * 0.5: svn r1863
 *
 *      Added "offset", "scale", and "fixed_length" links to the
 *      "BLine Tangent" ValueNode.
 *
 * 0.6: svn r2067
 *
 *      Added "scale" link to the "BLine Width" ValueNode in svn r1872.
 *
 *      Added "loop" link to the "Gradient Color" ValueNode in svn r1901.
 *
 * 0.7: svn r2315
 *
 *      Added "loop" link to the "Random" ValueNode in svn r2315.
 *
 * 0.8: git 82baee2702a65a9866f3dc4a28ef163dcf43795a
 *
 *      Added "homogeneous" link to "BLineCalcVertex", "BLineCalcTangent"
 *      and "BLineCalcWidth" valuenodes.
 *
 * 0.9: git 6922776b8129fdae6cb42953b2715decc810786c
 *
 *		Added "split_radius" and "split_angle" to BLinePoint Composite
 *		Value Node
 *
 * 1.0 git
 *
 *      Added a canvas component called
 */

#define CURRENT_CANVAS_VERSION "1.0"

/* === T Y P E D E F S ===================================================== */

/* === C L A S S E S & S T R U C T S ======================================= */

namespace synfig {

class IndependentContext;
class ContextParams;
class Context;
class GUID;
class Canvas;
class SoundProcessor;

typedef        etl::handle<Canvas>     CanvasHandle;

//! Optimize layers based on its calculated Z depth to perform a quick
//! render of the layers to the output.
void optimize_layers(Time, Context, CanvasHandle, bool seen_motion_blur=false);

/*!	\class Canvas
**	\brief Canvas is a double ended queue of Layers. It is the base class
* for a Synfig document.
*
* As a node it inherits all the parent child relationship and the GUID
* methods. As a double queue it allows insertion and deletion of Layers
* and can access to the layers on the queue easily.
*/
class Canvas : public CanvasBase, public Node
{
	/*
 --	** -- T Y P E S -----------------------------------------------------------
	*/

public:
	typedef etl::handle<Canvas> Handle;
	typedef etl::loose_handle<Canvas> LooseHandle;
	typedef etl::handle<const Canvas> ConstHandle;

	typedef std::list<Handle> Children;

	friend void synfig::optimize_layers(Time, Context, Canvas::Handle, bool seen_motion_blur);

	/*
 --	** -- D A T A -------------------------------------------------------------
	*/

private:

	//! Contains the ID string for the Canvas
	/*!	\see get_id(), set_id() */
	String id_;

	//! Contains the name of the Canvas
	/*!	\see set_name(), get_name() */
	String name_;

	//! Contains a description of the Canvas
	/*!	\see set_description(), get_description() */
	String description_;

	//! Contains the canvas' version string
	/*!	\see set_version(), get_version() */
	String version_;

	//! Contains the author's name
	/*!	\see set_author(), get_author() */
	String author_;

	//! Contains the author's email address
	/*!	\todo This private parameter has no binding, so it's unusable at the moment */
	String email_;

	//! File name of Canvas
	/*! \see get_file_name(), set_file_name() */
	String file_name_;

	//! File identifier of Canvas
	/*! \see get_identifier(), set_identifier() */
	FileSystem::Identifier identifier_;

	//! Metadata map for Canvas.
	/*! \see get_meta_data(), set_meta_data(), erase_meta_data() */
	std::map<String, String> meta_data_;

	//! Contains a list of ValueNodes that are in this Canvas
	/*!	\see value_node_list(), find_value_node() */
	ValueNodeList value_node_list_;

	//! Contains a list of Keyframes that are in the Canvas
	/*! \see keyframe_list()*/
	KeyframeList keyframe_list_;

	//! A handle to the parent canvas of this canvas.
	/*!	If canvas is a root canvas, then this handle is empty
	**	\see parent()
	*/
	LooseHandle parent_;

	//! List containing any child Canvases
	/*!	\see children() */
	Children children_;

	//! Render Description for Canvas
	/*!	\see rend_desc() */
    RendDesc desc_;

	//! Contains the value of the last call to set_time()
	Time cur_time_;

	//! Map of external Canvases used in this Canvas
	mutable std::map<String,Handle> externals_;

	//! This flag is set if this canvas is "inline"
	bool is_inline_;

	//! True if the Canvas properties has changed
	mutable bool is_dirty_;

	//! It is set to true when synfig::optimize_layers is called
	bool op_flag_;

	//! Layer Group database
	std::map<String,std::set<etl::handle<Layer> > > group_db_;

	//! Layer Signal Connection database. Seems to be unused.
	std::map<etl::loose_handle<Layer>,std::vector<sigc::connection> > connections_;

	//! Value to store temporarily the grow value for the child outline type layers
	/*! \see get_grow_value set_grow_value */
	Real outline_grow;


	/*
 -- ** -- S I G N A L S -------------------------------------------------------
	*/

private:

	//!	Group Added
	sigc::signal<void,String> signal_group_added_;

	//!	Group Removed
	sigc::signal<void,String> signal_group_removed_;

	//! Group Changed
	sigc::signal<void,String> signal_group_changed_;

	sigc::signal<void,String,etl::handle<synfig::Layer> > signal_group_pair_added_;
	sigc::signal<void,String,etl::handle<synfig::Layer> > signal_group_pair_removed_;

	//!	Layers Reordered
	sigc::signal<void,int*> signal_layers_reordered_;

	//!	RendDesc Changed
	sigc::signal<void> signal_rend_desc_changed_;

	//!	ID Changed
	sigc::signal<void> signal_id_changed_;

	//!	Dirty
	//sigc::signal<void> signal_dirty_;

	//!	FileName Changed
	sigc::signal<void> signal_file_name_changed_;

	//!	Metadata Changed
	sigc::signal<void, String> signal_meta_data_changed_;

	//! Key-Specific meta data changed signals
	std::map<String, sigc::signal<void> > signal_map_meta_data_changed_;


	//!	ValueBasenode Changed
	sigc::signal<void, etl::handle<ValueNode> > signal_value_node_changed_;
	//!	ValueBasenode Renamed
	sigc::signal<void, etl::handle<ValueNode> > signal_value_node_renamed_;
	//!	Child Value Node Added. Used in Dynamic List Value Nodes
	sigc::signal<void, etl::handle<ValueNode>, etl::handle<ValueNode> > signal_value_node_child_added_;
	//!	Child Value Node Removed. Used in Dynamic List Value Nodes
	sigc::signal<void, etl::handle<ValueNode>, etl::handle<ValueNode> > signal_value_node_child_removed_;

	/*
 -- ** -- S I G N A L   I N T E R F A C E -------------------------------------
	*/

public:

	sigc::signal<void,String,etl::handle<synfig::Layer> >& signal_group_pair_added() { return signal_group_pair_added_; }
	sigc::signal<void,String,etl::handle<synfig::Layer> >& signal_group_pair_removed() { return signal_group_pair_removed_; }

	//!	Group Added
	sigc::signal<void,String>& signal_group_added() { return signal_group_added_; }

	//!	Group Removed
	sigc::signal<void,String>& signal_group_removed() { return signal_group_removed_; }

	//! Group Changed
	sigc::signal<void,String>& signal_group_changed() { return signal_group_changed_; }

	//!	Layers Reordered
	sigc::signal<void,int*>& signal_layers_reordered() { return signal_layers_reordered_; }

	//!	RendDesc Changed
	sigc::signal<void>& signal_rend_desc_changed() { return signal_rend_desc_changed_; }

	//!	ID Changed
	sigc::signal<void>& signal_id_changed() { return signal_id_changed_; }

	//!	File name Changed
	sigc::signal<void>& signal_file_name_changed();

	//!	Metadata Changed
	sigc::signal<void, String>& signal_meta_data_changed() { return signal_meta_data_changed_; }

	//!	Metadata Changed
	sigc::signal<void>& signal_meta_data_changed(const String& key) { return signal_map_meta_data_changed_[key]; }

	//! Value Node Changed
	sigc::signal<void, etl::handle<ValueNode> >& signal_value_node_changed() { return signal_value_node_changed_; }
	//! Value Node Renamed
	sigc::signal<void, etl::handle<ValueNode> >& signal_value_node_renamed() { return signal_value_node_renamed_; }

	//! Dirty
	sigc::signal<void>& signal_dirty() { return signal_changed();	}

	//! Child Value Node Added
	sigc::signal<void, etl::handle<ValueNode>, etl::handle<ValueNode> >& signal_value_node_child_added() { return signal_value_node_child_added_; }

	//! Child Value Node Removed
	sigc::signal<void, etl::handle<ValueNode>, etl::handle<ValueNode> >& signal_value_node_child_removed() { return signal_value_node_child_removed_; }

	void invoke_signal_value_node_child_removed(etl::handle<ValueNode>, etl::handle<ValueNode>);

	/*
 --	** -- C O N S T R U C T O R S ---------------------------------------------
	*/

protected:
	//! Canvas constructor by Canvas name
	Canvas(const String &name);

public:

	~Canvas();

	/*
 --	** -- M E M B E R   F U N C T I O N S -------------------------------------
	*/

public:

	//! Returns the set of layers in group
	std::set<etl::handle<Layer> > get_layers_in_group(const String&group);

	//! Gets all the groups
	std::set<String> get_groups()const;

	//! Gets the number of groups in this canvas
	int get_group_count()const;

	//! Renames the given group
	void rename_group(const String&old_name,const String&new_name);

	//! Returns true if the Canvas is in line
	bool is_inline()const { return is_inline_; }

	//! Returns a handle to the RendDesc for this Canvas
	RendDesc &rend_desc() { return desc_; }

	//! Returns a handle to the RendDesc for this Canvas
	const RendDesc &rend_desc()const { return desc_; }

	//! Gets the name of the canvas
	const String & get_name()const { return name_; }

	//! Sets the name of the canvas
	void set_name(const String &x);

	//! Gets the version string of the canvas
	const String get_version()const { return version_; }

	//! Sets the version string of the canvas
	void set_version(const String &x) { version_ = x; }

	//! Gets the author of the canvas
	const String & get_author()const { return author_; }

	//! Sets the author of the canvas
	void set_author(const String &x);

	//! Gets the description of the canvas
	const String & get_description()const { return description_; }

	String get_string()const;

	//! Sets the name of the canvas
	void set_description(const String &x);

	//! Gets the ID of the canvas
	const String & get_id()const { return id_; }

	//! Sets the ID of the canvas
	void set_id(const String &x);

	//!	Returns the data string for the given meta data key
	String get_meta_data(const String& key)const;

	//!	Returns a list of meta data keys
	std::list<String> get_meta_data_keys()const;

	//! Sets a meta data key to a specific string
	void set_meta_data(const String& key, const String& data);

	//! Removes a meta data key
	void erase_meta_data(const String& key);

	//! Gets the relative ID string for an ancestor Canvas
	String get_relative_id(etl::loose_handle<const Canvas> x)const;

	//! Gets the relative ID string for an ancestor Canvas. Don't call it directly
	String _get_relative_id(etl::loose_handle<const Canvas> x)const;

	//! Returns \c true if the Canvas is a root Canvas. \c false otherwise
	bool is_root()const { return !parent_; }

	//! Returns a handle to the parent Canvas.
	/*! The returned handle will be empty if this is a root canvas */
	LooseHandle parent()const { return parent_; }

	//! Returns a handle to the root Canvas
	LooseHandle get_root()const;

	LooseHandle get_non_inline_ancestor()const;

	//! Returns a list of all child canvases in this canvas
	std::list<Handle> &children() { return children_; }

	//! Returns a list of all child canvases in this canvas
	const std::list<Handle> &children()const { return children_; }

	//! Gets the color at the specified point
	//Color get_color(const Point &pos)const;

	//! Sets the time for all the layers in the canvas
	void set_time(Time t)const;
	
	//! Loads resources (frames) for all the external layers in the canvas
	void load_resources(Time t)const;

	//! Returns the current time of the Canvas
	Time get_time()const { return cur_time_; }

	//! Returns the number of layers in the canvas
	int size()const;

	//! Removes all the layers from the canvas
	void clear();

	//! Returns true if the canvas has no layers
	bool empty()const;

	//! Returns a reference to the ValueNodeList for this Canvas
	// ValueNodeList &value_node_list() { return value_node_list_; }

	//! Returns a reference to the ValueNodeList for this Canvas
	const ValueNodeList &value_node_list()const;

	//! Returns a reference to the KeyframeList for this Canvas
	KeyframeList &keyframe_list();

	//! Returns a reference to the KeyframeList for this Canvas
	const KeyframeList &keyframe_list()const;

	//! Finds the ValueNode in the Canvas with the given \a id
	/*!	\return If found, returns a handle to the ValueNode.
	**		Otherwise, returns an empty handle.
	*/
	ValueNode::Handle find_value_node(const String &id, bool might_fail);

	//! \internal \writeme
	ValueNode::Handle surefind_value_node(const String &id);

	//! Finds the ValueNode in the Canvas with the given \a id
	/*!	\return If found, returns a handle to the ValueNode.
	**		Otherwise, returns an empty handle.
	*/
	ValueNode::ConstHandle find_value_node(const String &id, bool might_fail)const;

	//! Adds a Value node by its Id.
	/*! Throws an error if the Id is not
	//! correct or the Value node is already exported
	**/
	void add_value_node(ValueNode::Handle x, const String &id);

	//! writeme
	//void rename_value_node(ValueNode::Handle x, const String &id);

	//! Removes a Value Node from the Canvas by its Handle
	void remove_value_node(ValueNode::Handle x, bool might_fail);

	//! Removes a Value Node from the Canvas by its Id
	void remove_value_node(const String &id, bool might_fail) { remove_value_node(find_value_node(id, might_fail), might_fail); }

	//! Finds a child Canvas in the Canvas with the given \a name
	/*!	\return If found, returns a handle to the child Canvas.
	**		If not found, it creates a new Canvas and returns it
	**		If an error occurs, it returns an empty handle
	*/
	Handle surefind_canvas(const String &id,String &warnings);

	//! Finds a child Canvas in the Canvas with the given \a id
	/*!	\return If found, returns a handle to the child Canvas.
	**		Otherwise, returns an empty handle.
	*/
	Handle find_canvas(const String &id, String &warnings);

	//! Finds a child Canvas in the Canvas with the given \a id
	/*!	\return If found, returns a handle to the child Canvas.
	**		Otherwise, returns an empty handle.
	*/
	ConstHandle find_canvas(const String &id, String &warnings)const;

	//! Returns the file path from the file name
	String get_file_path()const;

	//! Sets the filename (with path)
	void set_file_name(const String &);

	//! Gets the filename (with path)
	String get_file_name()const;

	//! Gets file_system of the canvas
	FileSystem::Handle get_file_system()const;

	//! Sets the file identifier
	void set_identifier(const FileSystem::Identifier &);

	//! Gets the file identifier
	const FileSystem::Identifier& get_identifier()const;

	//! Creates a new child canvas, and returns its handle
	Handle new_child_canvas();

	//! Creates a new child canvas with an ID of \a id, and returns its handle
	Handle new_child_canvas(const String &id);

	//! Adds the given canvas as a child
	Handle add_child_canvas(Handle child_canvas, const String &id);

	//! Remove Child Canvas by its handle. If Current canvas is a child of a parent
	//! it ask to the parent to remove the Child canvas.
	void remove_child_canvas(Handle child_canvas);

	//! Finds a Layer by its position.
	//! \see get_context()
	etl::handle<Layer> find_layer(const ContextParams &context_params, const Point &pos);

	//! Gets the depth of a particular Layer by its handle
	int get_depth(etl::handle<Layer>)const;

	//! Retireves the first layer of the double queue of Layers
	IndependentContext get_independent_context()const;

	//! Retireves the first layer of the double queue of Layers assigned with rendering parameters
	Context get_context(const ContextParams &params)const;

	//! Retireves the first layer of the double queue of Layers assigned with rendering parameters
	Context get_context(const Context &parent_context)const;

	//! Retireves sorted double queue of Layers and Context of the first layer with rendering parameters
	Context get_context_sorted(const ContextParams &params, CanvasBase &out_queue) const;

	int indexof(const const_iterator &iter) const;
	iterator byindex(int index);
	const_iterator byindex(int index) const;
	
	iterator find_index(const etl::handle<Layer> &layer, int &index);
	const_iterator find_index(const etl::handle<Layer> &layer, int &index) const;
	
	//! Returns the last Canvas layer queue iterator. Notice that it
	/*! overrides the std::end() member that would return an iterator
	 * just past the last element of the queue.*/
	iterator end();
	//! Returns the last Canvas layer queue const_iterator. Notice that it
	/*! overrides the std::end() member that would return an iterator
	 * just past the last element of the queue.*/
	const_iterator end()const;
	//! Returns the last Canvas layer queue reverse iterator. Notice that it
	/*! overrides the std::rbegin() member that would return an iterator
	 * just past the last element of the queue.*/
	reverse_iterator rbegin();
	//! Returns the last Canvas layer queue reverse const iterator. Notice that it
	/*! overrides the std::rbegin() member that would return an iterator
	 * just past the last element of the queue.*/
	const_reverse_iterator rbegin()const;
	//! Returns last layer in Canvas layer stack
	etl::handle<Layer> &back();
	//! Returns last layer in Canvas layer stack
	const etl::handle<Layer> &back()const;
	//! Inserts a layer just before the last layer.
	//! \see end(), insert(iterator iter,etl::handle<Layer> x)
	void push_back(etl::handle<Layer> x);
	//! Inserts a layer just at the beginning of the Canvas layer dqueue
	void push_front(etl::handle<Layer> x);
	//! Inserts a layer in the last position of the Canvas layer dqueue
	//! Uses the standard methods and doesn't perform any parentship
	//! or signal update
	void push_back_simple(etl::handle<Layer> x);
	//! Inserts a layer before the given position by \iter and performs
	//! the proper child parent relationships and signals update
	void insert(iterator iter,etl::handle<Layer> x);
	//! Removes a layer from the Canvas layer dqueue and its group and parent
	//! relatioship. Although it is not already used, it clears the connections
	//! see connections_
	void erase(iterator iter);
	//! Sets to be a inline canvas of a given Canvas \parent. The inline
	//! Canvas inherits the groups and the render description.
	//! \see rend_desc()
	void set_inline(LooseHandle parent);
	//! Returns a Canvas handle with "Untitled" as ID
	static Handle create();
	//! Creates an inline Canvas for a given Canvas \parent
	static Handle create_inline(Handle parent);

	//! Clones (copies) the Canvas
	Handle clone(const GUID& deriv_guid=GUID(), bool for_export=false)const;

	//! Stores the external canvas by its file name and the Canvas handle
	void register_external_canvas(String file, Handle canvas);

	//! Set/Get members for the outline grow value
	Real get_outline_grow()const;
	void set_outline_grow(Real x);

#if 0
	void show_canvas_ancestry(String file, int line, String note)const;
	void show_canvas_ancestry()const;
#endif

#ifdef _DEBUG
	void show_externals(String file, int line, String text) const;
	void show_structure(int i) const;
#endif	// _DEBUG

private:
	//! Sets parent and raises on_parent_set event
	void set_parent(const Canvas::LooseHandle &parent);
	//! Adds a \layer to a group given by its \group string to the group
	//! database
	void add_group_pair(String group, etl::handle<Layer> layer);
	//! Removes a \layer from a group given by its \group string to the group
	//! database
	void remove_group_pair(String group, etl::handle<Layer> layer);
	//! Seems to be used to add the stored signals connections of the layers.
	//! \see connections_
	void add_connection(etl::loose_handle<Layer> layer, sigc::connection connection);
	//! Seems to be used to disconnect the stored signals connections of the layers.
	//! \see connections_
	void disconnect_connections(etl::loose_handle<Layer> layer);

protected:
	//! Parent changed
	virtual void on_parent_set();
	//! Sets the Canvas to dirty and calls Node::on_changed()
	virtual void on_changed();
	//! Collects the times (TimePoints) of the Layers of the Canvas and
	//! stores it in the passed Time Set \set
	//! \see Node::get_times()
	virtual void get_times_vfunc(Node::time_set &set) const;

public:
	void fill_sound_processor(SoundProcessor &soundProcessor) const;
}; // END of class Canvas

}; // END of namespace synfig

/* === E N D =============================================================== */

#endif
