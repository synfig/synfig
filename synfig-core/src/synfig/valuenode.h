/* === S Y N F I G ========================================================= */
/*!	\file valuenode.h
**	\brief Header file for implementation of the "Placeholder" valuenode conversion.
**
**	$Id$
**
**	\legal
**	Copyright (c) 2002-2005 Robert B. Quattlebaum Jr., Adrian Bentley
**	Copyright (c) 2008 Chris Moore
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

/* === S T A R T =========================================================== */

#ifndef __SYNFIG_VALUENODE_H
#define __SYNFIG_VALUENODE_H

/* === H E A D E R S ======================================================= */

#include "vector.h"
#include "value.h"
#include "string.h"
#include "releases.h"
#include <ETL/handle>
#include <ETL/stringf>
#include "exception.h"
#include <map>
#include <sigc++/signal.h>
#include "guid.h"
#include <ETL/angle>
#include "paramdesc.h"
#include "interpolation.h"

#include "node.h"

#include <set>

/* === M A C R O S ========================================================= */

#define VALUENODE_CHECK_TYPE(type) 										\
	/* I don't think this ever happens - maybe remove this code? */		\
	if (get_type() == type_nil) {										\
		warning("%s:%d get_type() IS nil sometimes!",					\
				__FILE__, __LINE__);									\
		return false;													\
	}																	\
	if (get_type() != type_nil &&										\
		!(ValueBase::can_copy(value->get_type(), type)) &&				\
		!PlaceholderValueNode::Handle::cast_dynamic(value)) {			\
		error(_("%s:%d wrong type for %s: need %s but got %s"),			\
			  __FILE__, __LINE__,										\
			  link_local_name(i).c_str(),								\
			  type.description.local_name.c_str(),						\
			  value->get_type().description.local_name.c_str() );		\
		return false;													\
	}

#define VALUENODE_SET_VALUE(variable)									\
	variable = value;													\
	signal_child_changed()(i);											\
	signal_value_changed()();											\
	return true

#define CHECK_TYPE_AND_SET_VALUE(variable, type)						\
	VALUENODE_CHECK_TYPE(type)											\
	VALUENODE_SET_VALUE(variable)

/* === T Y P E D E F S ===================================================== */

/* === C L A S S E S & S T R U C T S ======================================= */

namespace synfig {

class Canvas;
class LinkableValueNode;
class Layer;
class ParamVocab;

/*!	\class ValueNode
**	\brief Base class for all Value Nodes
*/
class ValueNode : public synfig::Node
{
	friend class Layer;
	friend class LinkableValueNode;

	/*
 --	** -- T Y P E S -----------------------------------------------------------
	*/

public:

	typedef etl::handle<ValueNode> Handle;

	typedef etl::loose_handle<ValueNode> LooseHandle;

	typedef etl::handle<const ValueNode> ConstHandle;

	typedef etl::rhandle<ValueNode> RHandle;

	//!Instantiates the book of ValaueNodes and register all the valid valuenodes on it
	static bool subsys_init();
	//!Deletes the book of ValueNodes
	static bool subsys_stop();

	static void breakpoint();

	/*
 --	** -- D A T A -------------------------------------------------------------
	*/

private:
	//! The type of the Value Node
	//! \see ValueBase
	Type *type;
	//! The name of the Value Node. This is the string that is used in the
	//! sif file to define the value type: i.e. <param name="amount">
	String name;
	//! The canvas this Value Node belongs to
	etl::loose_handle<Canvas> canvas_;
	//! The root canvas this Value Node belongs to
	etl::loose_handle<Canvas> root_canvas_;

	/*
 -- ** -- S I G N A L S -------------------------------------------------------
	*/

private:

	//!	ValueBase Changed
	sigc::signal<void> signal_value_changed_;

	//!	Children Reordered
	sigc::signal<void,int*> signal_children_reordered_;

	//!	Child Changed
	sigc::signal<void,int> signal_child_changed_;

	//!	Child Removed
	sigc::signal<void,int> signal_child_removed_;

	//!	Child Inserted
	sigc::signal<void,int> signal_child_inserted_;

	//!	ID Changed
	sigc::signal<void> signal_id_changed_;

	/*
 -- ** -- S I G N A L   I N T E R F A C E -------------------------------------
	*/

public:

	//!	ValueBase Changed
	sigc::signal<void>& signal_value_changed() { return signal_value_changed_; }

	//!	Children Reordered
	sigc::signal<void,int*>& signal_children_reordered() { return signal_children_reordered_; }

	//!	Child Changed
	sigc::signal<void,int>& signal_child_changed() { return signal_child_changed_; }

	//!	Child Removed
	sigc::signal<void,int>& signal_child_removed() { return signal_child_removed_; }

	//!	Child Inserted
	sigc::signal<void,int>& signal_child_inserted() { return signal_child_inserted_; }

	//!	ID Changed
	sigc::signal<void>& signal_id_changed() { return signal_id_changed_; }

	/*
 --	** -- C O N S T R U C T O R S ---------------------------------------------
	*/

protected:

	ValueNode(Type &type=type_nil);

public:

	virtual ~ValueNode();

	/*
 --	** -- M E M B E R   F U N C T I O N S -------------------------------------
	*/

public:

	//! Returns the value of the ValueNode at time \a t
	virtual ValueBase operator()(Time /*t*/)const
		{ return ValueBase(); }

	//! \internal Sets the id of the ValueNode
	void set_id(const String &x);

	//! Returns the id of the ValueNode
	/*!	The ID is used for keeping track of a
	**	specific instance of a ValueNode. */
	const String &get_id()const { return name; }

	//! Returns the name of the ValueNode type
	virtual String get_name()const=0;

	//! Returns the localized name of the ValueNode type
	virtual String get_local_name()const=0;

	//! Return a full description of the ValueNode and its parentage
	virtual String get_description(bool show_exported_name = true)const;

	String get_string()const;

	//! Clones a Value Node
	virtual ValueNode::Handle clone(etl::loose_handle<Canvas> canvas, const GUID& deriv_guid=GUID())const=0;

	//! Returns \true if the Value Node has an ID (has been exported)
	bool is_exported()const { return !get_id().empty(); }

	//! Check recursively if \value_node_dest is a descendant of the Value Node
	bool is_descendant(ValueNode::Handle value_node_dest);

	//! Returns the type of the ValueNode
	Type& get_type()const { return *type; }

	//! Returns a handle to the parent canvas, if it has one.
	etl::loose_handle<Canvas> get_parent_canvas()const;

	//! Returns a handle to the parent canvas, if it has one.
	etl::loose_handle<Canvas> get_root_canvas()const;

	//! Returns a handle to the parent canvas, if it has one.
	etl::loose_handle<Canvas> get_non_inline_ancestor_canvas()const;

	//! Sets the parent canvas for the Value Node
	void set_parent_canvas(etl::loose_handle<Canvas> x);

	//! Sets the root canvas parent for the Value Node
	virtual void set_root_canvas(etl::loose_handle<Canvas> x);

	//! Returns the relative ID of a Node when accessed form the \x Canvas
	String get_relative_id(etl::loose_handle<const Canvas> x)const;

	//! Replaces the Value Node with a given one. It look up all its parents
	//! remove it self from them and adds the given Value Node
	//! Notice that it is called twice and the second time it uses
	//! a replaceable handle to the Node
	//! \see etl::rhandle
	int replace(etl::handle<ValueNode> x);
	
	//! Get the default interpolation for Value Nodes
	virtual Interpolation get_interpolation()const { return INTERPOLATION_UNDEFINED; }
	//! Set the default interpolation for Value Nodes
	virtual void set_interpolation(Interpolation /* i*/) { }

protected:
	//! Sets the type of the ValueNode
	void set_type(Type &t) { type=&t; }

	virtual void on_changed();
}; // END of class ValueNode

/*!	\class PlaceholderValueNode
**	Seems to be a Place to hold a Value Node temporarly.
*
* 	Doesn't seem to implement any functionality. Seems to be used when the
* 	value node cannot be created using the Const, Animated or Linkable
* 	Value Nodes.
*
*/
class PlaceholderValueNode : public ValueNode
{
public:
	typedef etl::handle<PlaceholderValueNode> Handle;
	typedef etl::loose_handle<PlaceholderValueNode> LooseHandle;
	typedef etl::handle<const PlaceholderValueNode> ConstHandle;
	typedef etl::rhandle<PlaceholderValueNode> RHandle;

private:

	PlaceholderValueNode(Type &type=type_nil);

public:

	virtual ValueBase operator()(Time t)const;

	virtual String get_name()const;

	virtual String get_local_name()const;

	String get_string()const;

	virtual ValueNode::Handle clone(etl::loose_handle<Canvas> canvas, const GUID& deriv_guid=GUID())const;

	static Handle create(Type &type=type_nil);

protected:
	virtual void get_times_vfunc(Node::time_set &/*set*/) const {}
}; // END of class PlaceholderValueNode


/*!	\class LinkableValueNode
**	\brief Specialized Class of Value Nodes that has links to other
** Value Nodes
*
* 	This Value Node is calculated based on a math calculation or a time
* 	evaluation of the linked Value Nodes. It is commonly known as
* 	Converted Value Nodes. The derived clases defines the behavior.
*/
class LinkableValueNode : public ValueNode
{
	friend class ValueNode;
public:

	typedef etl::handle<LinkableValueNode> Handle;

	typedef etl::loose_handle<LinkableValueNode> LooseHandle;

	typedef etl::handle<const LinkableValueNode> ConstHandle;

	typedef etl::rhandle<LinkableValueNode> RHandle;


	//! Type that represents a pointer to a ValueNode's constructor
	/*! As a pointer to the constructor, it represents a "factory" of
	**  objects of this class.
	*/
	typedef LinkableValueNode* (*Factory)(const ValueBase&, etl::loose_handle<Canvas> canvas);

	//! This represents a pointer to a Type check member fucntion
	/*! As a pointer to the member, it represents a fucntion that checks
	**  the type of the provided ValueBase
	*/
	typedef bool (*CheckType)(Type &type);

	struct BookEntry
	{
		String local_name;
		Factory factory;
		CheckType check_type;
		ReleaseVersion release_version; // which version of synfig introduced this valuenode type
	};

	//! Book of types of linkable value nodes indexed by type name.
	/*! While the sifz file is read, each time a new LinkableValueNode entry
	**  is found, the factory constructor that the "factory" pointer member
	**  of the "BookEntry" struct points to, is called, and a new object of
	**  that type is created.
	**  \sa LinkableValueNode::Factory
	*/
	typedef std::map<String,BookEntry> Book;

	//! The vocabulary of the children
	/*! \see synfig::Paramdesc
	 */
	typedef ParamVocab Vocab;

	static Book& book();

	//! Creates a Linkable Value Node based on the name and the returned
	//! value type. Returns a valid Handle if both (name and type) match
	static Handle create(const String &name, const ValueBase& x, etl::loose_handle<Canvas> canvas /* = 0 */);

	//! Each derived Linkable Value Node has to implement this fucntion and
	//! should return true only if the type matches. \name is the name of
	//! the linked value node and \x is the returned value type
	static bool check_type(const String &name, Type &x);

public:
	LinkableValueNode(Type &type=type_nil):
		ValueNode(type) { }

protected:
	//! Stores the Value Node \x in the sub parameter i after check if the
	//! type is the same.
	//! It has to be defined by the derived class.
	virtual bool set_link_vfunc(int i,ValueNode::Handle x)=0;

	//! Frees all the subparameters of the Linkable Value Node.
	//! Used by the derived classed destructors.
	void unlink_all();

public:

	//! Returns the number of linked Value Nodes
	virtual int link_count()const;

	//! Returns the local name of the 'i' linked Value Node
	virtual String link_local_name(int i)const;

	//! Returns the name of the 'i' linked Value Node
	virtual String link_name(int i)const;

	//! Returns the child index Value Node based on the name
	virtual int get_link_index_from_name(const String &name)const;

	//! Clones a Value Node
	virtual ValueNode::Handle clone(etl::loose_handle<Canvas> canvas, const GUID& deriv_guid=GUID())const;

	//! Sets a new Value Node link by its index
	bool set_link(int i,ValueNode::Handle x);
	//! Sets a new Value Node link by its name
	bool set_link(const String &name,ValueNode::Handle x) {	return set_link(get_link_index_from_name(name),x);	}

	//! Returns a Loose Handle to the Value Node based on the link's index
	ValueNode::LooseHandle get_link(int i)const;
	//! Returns a Loose Handle to the Value Node based on the link's name
	ValueNode::LooseHandle get_link(const String &name)const { return get_link(get_link_index_from_name(name)); }
	//! Return a full description of the linked ValueNode given by the index
	String get_description(int index = -1, bool show_exported_name = true)const;
	//! Return a full description of the linked ValueNode given by the index
	//! Proper overload of the inherited function
	String get_description(bool show_exported_name = true)const;

	//! Gets the children vocabulary for linkable value nodes
	virtual Vocab get_children_vocab()const;

	virtual void set_root_canvas(etl::loose_handle<Canvas> x);

protected:
	//! Member to store the children vocabulary
	Vocab children_vocab;
	//! Sets the type of the ValueNode
	void set_type(Type &t) { ValueNode::set_type(t); }

	//! Virtual member to get the linked Value Node Handle
	virtual ValueNode::LooseHandle get_link_vfunc(int i)const=0;

	//! Wrapper for new operator, used by clone()
	virtual LinkableValueNode* create_new()const=0;

	//! Returns the cached times values for all the children (linked Value Nodes)
	virtual void get_times_vfunc(Node::time_set &set) const;

	//! Pure Virtual member to get the children vocabulary
	virtual Vocab get_children_vocab_vfunc()const=0;

	//! Virtual memebr to set the children vocabulary to a given value
	virtual void set_children_vocab(const Vocab& rvocab);
}; // END of class LinkableValueNode

/*!	\class ValueNodeList
**	\brief A searchable value_node list container
**	\warning Do not confuse with ValueNode_DynamicList!
*
*  Used by Canvas class to access to the exported value nodes.
*/
class ValueNodeList : public std::list<ValueNode::RHandle>
{
	int placeholder_count_;
public:
	ValueNodeList();

	//! Finds the ValueNode in the list with the given \a name
	/*!	\return If found, returns a handle to the ValueNode.
	**		Otherwise, returns an empty handle.
	*/
	ValueNode::Handle find(const String &name, bool might_fail);

	//! Finds the ValueNode in the list with the given \a name
	/*!	\return If found, returns a handle to the ValueNode.
	**		Otherwise, returns an empty handle.
	*/
	ValueNode::ConstHandle find(const String &name, bool might_fail)const;

	//! Removes the \a value_node from the list
	bool erase(ValueNode::Handle value_node);

	//! \writeme
	bool add(ValueNode::Handle value_node);

	//! \writeme
	bool count(const String &id)const;

	//! Similar to find, but will create a placeholder value_node if it cannot be found.
	ValueNode::Handle surefind(const String &name);

	//! Removes any value_nodes with reference counts of 1.
	void audit();

	//! Placeholder Count
	int placeholder_count()const { return placeholder_count_; }
};

ValueNode::LooseHandle find_value_node(const GUID& guid);

}; // END of namespace synfig

/* === E N D =============================================================== */

#endif
