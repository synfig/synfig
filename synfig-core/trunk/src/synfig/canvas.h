/* === S Y N F I G ========================================================= */
/*!	\file canvas.h
**	\brief Canvas Class Implementation
**
**	$Id: canvas.h,v 1.1.1.1 2005/01/04 01:23:14 darco Exp $
**
**	\legal
**	Copyright (c) 2002 Robert B. Quattlebaum Jr.
**
**	This software and associated documentation
**	are CONFIDENTIAL and PROPRIETARY property of
**	the above-mentioned copyright holder.
**
**	You may not copy, print, publish, or in any
**	other way distribute this software without
**	a prior written agreement with
**	the copyright holder.
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

#include "vector.h"
#include "string.h"
#include "canvasbase.h"
#include "valuenode.h"
#include "keyframe.h"
#include "renddesc.h"
#include "node.h"
#include "guid.h"

/* === M A C R O S ========================================================= */

/* === T Y P E D E F S ===================================================== */

/* === C L A S S E S & S T R U C T S ======================================= */

namespace synfig {

class Context;
class GUID;
	
/*!	\class Canvas
**	\todo writeme
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

	friend void synfig::optimize_layers(Context, Canvas::Handle);

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

	//! Contains the author's name
	/*!	\see set_author(), get_author() */
	String author_;

	//! Contains the author's email address
	/*!	\todo This private parameter has no binding, so it's unusable at the moment */
	String email_;

	//! File name of Canvas
	/*! \see get_file_name(), set_file_name() */
	String file_name_;

	//! Metadata map for Canvas.
	/*! \see get_meta_data(), set_meta_data(), erase_meta_data() */
	std::map<String, String> meta_data_;

	//! Contains a list of ValueNodes that are in this Canvas
	/*!	\see value_node_list(), find_value_node() */
	ValueNodeList value_node_list_;

	//! \writeme
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

	//! \writeme
	mutable std::map<String,Handle> externals_;
	
	//! This flag is set if this canvas is "inline"
	bool is_inline_;

	mutable bool is_dirty_;
	
	bool op_flag_;
	
	//! Layer Group database
	std::map<String,std::set<etl::handle<Layer> > > group_db_;	

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

	sigc::signal<void, etl::handle<ValueNode>, etl::handle<ValueNode> > signal_value_node_child_added_;	

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
	

	sigc::signal<void, etl::handle<ValueNode> >& signal_value_node_changed() { return signal_value_node_changed_; }	

	//!	Dirty
	sigc::signal<void>& signal_dirty() { return signal_changed();	}

	//! \writeme
	sigc::signal<void, etl::handle<ValueNode>, etl::handle<ValueNode> >& signal_value_node_child_added() { return signal_value_node_child_added_; }

	//! \writeme
	sigc::signal<void, etl::handle<ValueNode>, etl::handle<ValueNode> >& signal_value_node_child_removed() { return signal_value_node_child_removed_; }

	/*
 --	** -- C O N S T R U C T O R S ---------------------------------------------
	*/

protected:

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

	//! \writeme
	bool is_inline()const { return is_inline_; }

	//! Returns a handle to the RendDesc for this Canvas
	RendDesc &rend_desc() { return desc_; }

	//! Returns a handle to the RendDesc for this Canvas
	const RendDesc &rend_desc()const { return desc_; }

	//! Gets the name of the canvas
	const String & get_name()const { return name_; }

	//! Sets the name of the canvas
	void set_name(const String &x);

	//! Gets the author of the canvas
	const String & get_author()const { return author_; }

	//! Sets the author of the canvas
	void set_author(const String &x);

	//! Gets the description of the canvas
	const String & get_description()const { return description_; }

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

	//! \writeme
	String get_relative_id(etl::loose_handle<const Canvas> x)const;

	//! \internal \writeme
	String _get_relative_id(etl::loose_handle<const Canvas> x)const;

	//! Returns \c true if the Canvas is a root Canvas. \c false otherwise
	bool is_root()const { return !parent_; }

	//! Returns a handle to the parent Canvas.
	/*! The returned handle will be empty if this is a root canvas */
	LooseHandle parent()const { return parent_; }

	LooseHandle get_root()const;
	
	//! Returns a list of all child canvases in this canvas
	std::list<Handle> &children() { return children_; }

	//! Returns a list of all child canvases in this canvas
	const std::list<Handle> &children()const { return children_; }

	//! Gets the color at the specified point
	//Color get_color(const Point &pos)const;

	//! Sets the time for all the layers in the canvas
	void set_time(Time t)const;

	//! \writeme
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
	ValueNode::Handle find_value_node(const String &id);

	//! \internal \writeme
	ValueNode::Handle surefind_value_node(const String &id);

	//! Finds the ValueNode in the Canvas with the given \a id
	/*!	\return If found, returns a handle to the ValueNode.
	**		Otherwise, returns an empty handle.
	*/
	ValueNode::ConstHandle find_value_node(const String &id)const;

	//! \writeme
	void add_value_node(ValueNode::Handle x, const String &id);

	//! \writeme
	//void rename_value_node(ValueNode::Handle x, const String &id);

	//! \writeme
	void remove_value_node(ValueNode::Handle x);

	//! \writeme
	void remove_value_node(const String &id) { remove_value_node(find_value_node(id)); }

	//! Finds a child Canvas in the Canvas with the given \a name
	/*!	\return If found, returns a handle to the child Canvas.
	**		If not found, it creates a new Canvas and returns it
	**		If an error occurs, it returns an empty handle
	*/
	Handle surefind_canvas(const String &id);

	//! Finds a child Canvas in the Canvas with the given \a id
	/*!	\return If found, returns a handle to the child Canvas.
	**		Otherwise, returns an empty handle.
	*/
	Handle find_canvas(const String &id);

	//! Finds a child Canvas in the Canvas with the given \a id
	/*!	\return If found, returns a handle to the child Canvas.
	**		Otherwise, returns an empty handle.
	*/
	ConstHandle find_canvas(const String &id)const;

	//! Sets the file path for the Canvas
	//void set_file_path(const String &);

	//! Returns the file path from the file name
	String get_file_path()const;

	//! Sets the filename (with path)
	void set_file_name(const String &);

	//! Gets the filename (with path)
	String get_file_name()const;
	
	//! Creates a new child canvas, and returns its handle
	Handle new_child_canvas();

	//! Creates a new child canvas with an ID of \aid, and returns its handle
	Handle new_child_canvas(const String &id);

	//! Adds the given canvas as a child
	Handle add_child_canvas(Handle child_canvas, const String &id);

	void remove_child_canvas(Handle child_canvas);

	etl::handle<Layer> find_layer(const Point &pos);

	int get_depth(etl::handle<Layer>)const;

	Context get_context()const;

	iterator end();

	const_iterator end()const;

	reverse_iterator rbegin();

	const_reverse_iterator rbegin()const;

	etl::handle<Layer> &back();
	
	void push_back(etl::handle<Layer> x);

	void push_front(etl::handle<Layer> x);

	void push_back_simple(etl::handle<Layer> x);

	void insert(iterator iter,etl::handle<Layer> x);
	void erase(iterator iter);
	
	const etl::handle<Layer> &back()const;

	void set_inline(LooseHandle parent);

	static Handle create();

	static Handle create_inline(Handle parent);
	
	Handle clone(const GUID& deriv_guid=GUID())const;

private:
	void add_group_pair(String group, etl::handle<Layer> layer);
	void remove_group_pair(String group, etl::handle<Layer> layer);

protected:
	virtual void on_changed();
	virtual void get_times_vfunc(Node::time_set &set) const;
}; // END of class Canvas

void optimize_layers(Context context, Canvas::Handle op_canvas);


}; // END of namespace synfig

/* === E N D =============================================================== */

#endif
