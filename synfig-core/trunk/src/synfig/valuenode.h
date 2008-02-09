/* === S Y N F I G ========================================================= */
/*!	\file valuenode.h
**	\brief Header file for implementation of the "Placeholder" valuenode conversion.
**
**	$Id$
**
**	\legal
**	Copyright (c) 2002-2005 Robert B. Quattlebaum Jr., Adrian Bentley
**	Copyright (c) 2008 Chris Moore
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

#ifndef SYNFIG_NO_ANGLE
#include <ETL/angle>
#endif

#include "node.h"

#include <set>

/* === M A C R O S ========================================================= */

// This is a hack for GCC 3.0.4... which has a broken dynamic_cast<>
// It is deprecated, and will be removed soon.
#if ( __GNUC__ == 3 ) && ( __GNUC__MINOR__ == 0 )
# define DCAST_HACK_BASECLASS()	int cast__
# define DCAST_HACK_ID(x)		static const int my_cast__(void) { return x; }
# define DCAST_HACK_ENABLE()	cast__=my_cast__()
#else
# define DCAST_HACK_BASECLASS()
# define DCAST_HACK_ID(x)
# define DCAST_HACK_ENABLE()
#endif

#define CHECK_TYPE_AND_SET_VALUE(variable, type)						\
	/* I don't think this ever happens - maybe remove this code? */		\
	if (get_type() == ValueBase::TYPE_NIL) {							\
		warning("%s:%d get_type() IS nil sometimes!",					\
				__FILE__, __LINE__);									\
		return false;													\
	}																	\
	if (get_type() != ValueBase::TYPE_NIL &&							\
		!(ValueBase::same_type_as(value->get_type(), type)) &&			\
		!PlaceholderValueNode::Handle::cast_dynamic(value)) {			\
		error(_("%s:%d wrong type for %s: need %s but got %s"),			\
			  __FILE__, __LINE__,										\
			  link_local_name(i).c_str(),								\
			  ValueBase::type_local_name(type).c_str(),					\
			  ValueBase::type_local_name(value->get_type()).c_str());	\
		return false;													\
	}																	\
	variable = value;													\
	signal_child_changed()(i);											\
	signal_value_changed()();											\
	return true

/* === T Y P E D E F S ===================================================== */

/* === C L A S S E S & S T R U C T S ======================================= */

namespace synfig {

class Canvas;
class LinkableValueNode;
class Layer;

/*!	\class ValueNode
**	\todo writeme
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


	static bool subsys_init();

	static bool subsys_stop();

	/*
 --	** -- D A T A -------------------------------------------------------------
	*/

private:
	ValueBase::Type type;
	String name;
	etl::loose_handle<Canvas> canvas_;
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

	ValueNode(ValueBase::Type type=ValueBase::TYPE_NIL);

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


	//! \writeme
	virtual ValueNode* clone(const GUID& deriv_guid=GUID())const=0;

	//! \writeme
	bool is_exported()const { return !get_id().empty(); }

	//! Returns the type of the ValueNode
	ValueBase::Type get_type()const { return type; }

	//! Returns a handle to the parent canvas, if it has one.
	etl::loose_handle<Canvas> get_parent_canvas()const { return canvas_; }

	//! Returns a handle to the parent canvas, if it has one.
	etl::loose_handle<Canvas> get_root_canvas()const { return root_canvas_; }

	//! \writeme
	void set_parent_canvas(etl::loose_handle<Canvas> x);

	//! \writeme
	void set_root_canvas(etl::loose_handle<Canvas> x);

	//! \writeme
	String get_relative_id(etl::loose_handle<const Canvas> x)const;

	int replace(etl::handle<ValueNode> x);

protected:
	//! Sets the type of the ValueNode
	void set_type(ValueBase::Type t) { type=t; }

	virtual void on_changed();

public:
	DCAST_HACK_BASECLASS();
	DCAST_HACK_ID(0);
}; // END of class ValueNode

/*!	\class PlaceholderValueNode
**	\todo writeme
*/
class PlaceholderValueNode : public ValueNode
{
public:
	typedef etl::handle<PlaceholderValueNode> Handle;
	typedef etl::loose_handle<PlaceholderValueNode> LooseHandle;
	typedef etl::handle<const PlaceholderValueNode> ConstHandle;
	typedef etl::rhandle<PlaceholderValueNode> RHandle;

private:

	PlaceholderValueNode(ValueBase::Type type=ValueBase::TYPE_NIL);

public:

	virtual ValueBase operator()(Time t)const;

	virtual String get_name()const;

	virtual String get_local_name()const;

	virtual ValueNode* clone(const GUID& deriv_guid=GUID())const;

	static Handle create(ValueBase::Type type=ValueBase::TYPE_NIL);

protected:
	virtual void get_times_vfunc(Node::time_set &/*set*/) const {}
}; // END of class PlaceholderValueNode


/*!	\class LinkableValueNode
**	\todo writeme
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
	typedef LinkableValueNode* (*Factory)(const ValueBase&);

	typedef bool (*CheckType)(ValueBase::Type);

	struct BookEntry
	{
		String local_name;
		Factory factory;
		CheckType check_type;
		ReleaseVersion release_version; // which version of synfig introduced this valuenode type
	};

	typedef std::map<String,BookEntry> Book;

	static Book& book();

	static Handle create(const String &name, const ValueBase& x);

	static bool check_type(const String &name, ValueBase::Type x);

public:
	LinkableValueNode(ValueBase::Type type=ValueBase::TYPE_NIL):
		ValueNode(type) { }

protected:
	virtual bool set_link_vfunc(int i,ValueNode::Handle x)=0;

	void unlink_all();

public:

	virtual int link_count()const=0;

	virtual String link_local_name(int i)const=0;

	virtual String link_name(int i)const=0;

	virtual int get_link_index_from_name(const String &name)const=0;

	virtual ValueNode* clone(const GUID& deriv_guid=GUID())const;

	bool set_link(int i,ValueNode::Handle x);
	bool set_link(const String &name,ValueNode::Handle x) {	return set_link(get_link_index_from_name(name),x);	}

	ValueNode::LooseHandle get_link(int i)const;
	ValueNode::LooseHandle get_link(const String &name)const { return get_link(get_link_index_from_name(name)); }

	String
	get_description(int index, bool show_exported_name = true)const;

protected:
	//! Sets the type of the ValueNode
	void set_type(ValueBase::Type t) { ValueNode::set_type(t); }

	virtual ValueNode::LooseHandle get_link_vfunc(int i)const=0;

	// Wrapper for new operator, used by clone()
	virtual LinkableValueNode* create_new()const=0;

	virtual void get_times_vfunc(Node::time_set &set) const;
}; // END of class LinkableValueNode

/*!	\class ValueNodeList
**	\brief A searchable value_node list container
**	\warning Do not confuse with ValueNode_DynamicList!
**	\todo writeme
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
	ValueNode::Handle find(const String &name);

	//! Finds the ValueNode in the list with the given \a name
	/*!	\return If found, returns a handle to the ValueNode.
	**		Otherwise, returns an empty handle.
	*/
	ValueNode::ConstHandle find(const String &name)const;

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
