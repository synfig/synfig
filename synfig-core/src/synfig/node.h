/* === S Y N F I G ========================================================= */
/*!	\file node.h
**	\brief Base class for Layers and Value Nodes.
**	It defines the base members for the parent - child relationship,
**	the times where the node is modified and the handling of
**	the GUID on deletion and changing.
**
**	\legal
**	Copyright (c) 2002-2005 Robert B. Quattlebaum Jr., Adrian Bentley
**	Copyright (c) 2007 Chris Moore
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

/* === S T A R T =========================================================== */

#ifndef __SYNFIG_PARENTNODE_H
#define __SYNFIG_PARENTNODE_H

/* === H E A D E R S ======================================================= */

#include <ctime>
#include <mutex>
#include <set>
#include <vector>

#include <glibmm/threads.h>
#include <sigc++/signal.h>

#include <ETL/handle>
#include "guid.h"
#include "interpolation.h"
#include "time.h"

/* === M A C R O S ========================================================= */

// When a PasteCanvas layer has a non-zero 'time offset' parameter, should
// the waypoints shown for the canvas be adjusted?  This currently only
// partially works - see the TODO at the end of layer_pastecanvas.cpp
#define ADJUST_WAYPOINTS_FOR_TIME_OFFSET

/* === T Y P E D E F S ===================================================== */

/* === C L A S S E S & S T R U C T S ======================================= */

namespace synfig {

//!\brief TimePoint class: holds the time and the before and after interpolation mode
/**
 * It includes a GUID, to make it unique
 * \see guid.h interpolation.h
**/
class TimePoint
{
	GUID guid;
	Time time;
	Interpolation before,after;
public:

	TimePoint(const Time& x=Time::begin()):
		guid(GUID::zero()),
		time(x),
		before(INTERPOLATION_NIL),
		after(INTERPOLATION_NIL)
	{
	}

	const GUID& get_guid()const { return guid; }
	const Time& get_time()const { return time; }
	Interpolation get_before()const { return before; }
	Interpolation get_after()const { return after; }

	void set_guid(const GUID& x) { guid=x; }
	void set_time(const Time& x) { time=x; }
	void set_before(Interpolation x) { before=x; }
	void set_after(Interpolation x) { after=x; }

	//! Modify the TimePoint based on the values of \x "merging"
	//! the interpolations. Used to insert a Time Point in a Time Points Set
	//! \see TimePointSet::iterator TimePointSet::insert(const TimePoint& x)
	void absorb(const TimePoint& x);
}; // END of class TimePoint

inline TimePoint operator+(TimePoint lhs,const Time& rhs)
	{ lhs.set_time(lhs.get_time()+rhs); return lhs; }

inline TimePoint operator-(TimePoint lhs,const Time& rhs)
	{ lhs.set_time(lhs.get_time()-rhs); return lhs; }

inline bool operator<(const TimePoint& lhs,const TimePoint& rhs)
	{ return lhs.get_time()<rhs.get_time(); }

inline bool operator<(const TimePoint& lhs,const Time& rhs)
	{ return lhs.get_time()<rhs; }

inline bool operator<(const Time& lhs,const TimePoint& rhs)
	{ return lhs<rhs.get_time(); }

inline bool operator==(const TimePoint& lhs,const TimePoint& rhs)
	{ return lhs.get_time()==rhs.get_time(); }

inline bool operator!=(const TimePoint& lhs,const TimePoint& rhs)
	{ return lhs.get_time()!=rhs.get_time(); }

class TimePointSet : public std::set<TimePoint>
{
public:
	iterator insert(const TimePoint& x);

	template <typename ITER> void insert(ITER begin, ITER end)
		{ for(;begin!=end;++begin) insert(*begin); }

}; // END of class TimePointSet


//! Base class for dealing with parent-child relationship, time points and basic signals
//! Historically, it was designed primarily for handling ValueNodes and their link features
class Node : public etl::rshared_object
{
	/*
 --	** -- T Y P E S -----------------------------------------------------------
	*/

public:

	typedef	TimePointSet time_set;

	/*
 --	** -- D A T A -------------------------------------------------------------
	*/

private:

	mutable std::mutex guid_mutex_;
	//! The GUID of the node
	mutable GUID guid_;

	//! Cached time values for this node and all the children
	mutable time_set times;

	//! Indicates if \p times cache is not updated since last changed() call
	mutable bool bchanged;

	//! The last time the node was modified since the program started
	mutable clock_t time_last_changed_;

	//! \writeme
	mutable Glib::Threads::RWLock rw_lock_;

	//! Variable used to remember that a signal_deleted has been thrown
	bool deleting_;

	//! Mutex for parent_set protection
	mutable std::mutex parent_set_mutex_;

	//! A set of pointers to parent nodes
	std::set<Node*> parent_set;

	/*
 -- ** -- S I G N A L S -------------------------------------------------------
	*/

private:

	//! Node changed signal
	sigc::signal<void> signal_changed_;

	//! Child node changed signal
	sigc::signal<void, const Node*> signal_child_changed_;

	//! GUID changed signal
	/*! \note The parameter is the *OLD* guid! */
	sigc::signal<void,GUID> signal_guid_changed_;

	//! Node deleted signal
	sigc::signal<void> signal_deleted_;

	/*
 -- ** -- S I G N A L   I N T E R F A C E -------------------------------------
	*/

public:

	//! Signal emitted when node is about to be deleted
	sigc::signal<void>& signal_deleted() { return signal_deleted_; }

	//! Signal emitted when node is flagged as changed via changed() or child_changed()
	sigc::signal<void>& signal_changed() { return signal_changed_; }

	//! Signal emitted when a child node is flagged as changed via child_changed()
	sigc::signal<void, const Node*>& signal_child_changed() { return signal_child_changed_; }

	//! Signal emitted when GUID changes
	/*! \note The parameter is the *OLD* guid! */
	sigc::signal<void,GUID>& signal_guid_changed() { return signal_guid_changed_; }

	/*
 --	** -- C O N S T R U C T O R S ---------------------------------------------
	*/

protected:

	Node();

	Node(const Node &x) = delete;

public:
	virtual ~Node();

	/*
 --	** -- M E M B E R   F U N C T I O N S -------------------------------------
	*/

public:

	//! Flag this node has changed.
	//! This way programmer can batch its changes and call it only once
	//! It emits signal_changed()
	void changed();
	//! Flag the child node \p x has changed.
	//! This way programmer can batch its changes and call it only once
	//! It emits signal_child_changed() and signal_changed()
	void child_changed(const Node *x);

	//! Gets the GUID for this Node
	const GUID& get_guid()const;

	//! Sets the GUID for this Node
	virtual void set_guid(const GUID& x);

	//! Gets the time when the Node was changed
	int get_time_last_changed()const;

	//! Adds the parameter \p x as the child of the current Node
	void add_child(Node *x);

	//! Removes the parameter \p x as a child of the current Node
	void remove_child(Node *x);

	//! Returns how many parents has the current Node
	std::size_t parent_count() const;

	//! Checks if node \p x is parent of this node
	bool is_child_of(const Node* x) const;

	//! Returns the first parent Node.
	//! Note that parents are stored as a FIFO. Do not rely on parent order!
	Node* get_first_parent() const;

	//! Callback function for a foreach method.
	//! If it returns true, the foreach iteration is halted.
	using ForeachFunc = sigc::slot<bool(Node*)>;
	using ConstForeachFunc = sigc::slot<bool(const Node*)>;

	//! Call function func for each of the parents of the current Node
	//! Do not add or remove any parent node while doing this foreach call
	void foreach_parent(const ConstForeachFunc& func) const;

	//! Call function func for each of the parents of the current Node
	//! Do not add or remove any parent node while doing this foreach call
	void foreach_parent(const ForeachFunc& func);

	//! Return a list of all parents of a given type
	//! Example: node->find_all_parents_of_type<MyType>()
	template<typename T> std::vector<etl::handle<T>> find_all_parents_of_type() const {
		std::vector<etl::handle<T>> list;
		const_cast<Node*>(this)->foreach_parent([&list](Node* parent) -> bool {
			if (auto item = dynamic_cast<T*>(parent))
				list.push_back(item);
			return false;
		});
		return list;
	}

	//! Return the first parent of a given type.
	//! Example: node->find_first_parent_of_type<MyType>()
	template<typename T> etl::handle<T> find_first_parent_of_type() const {
		T* parent = nullptr;
		foreach_parent([&parent](const Node* node) -> bool {
			if (auto item = dynamic_cast<T*>(const_cast<Node*>(node)))
				parent = item;
			return parent;
		});
		return parent;
	}

	//! Return the first parent of a given type with an additional match condition
	//! The CompareFunc should return true when match is satisfied
	//! Example: node->find_first_parent_of_type<MyType>([](MyType::Handle v) -> bool { return v.prop == 1;});
	template<typename T> etl::handle<T> find_first_parent_of_type(const sigc::slot<bool(const etl::handle<T>&)> &CompareFunc) const {
		T* parent = nullptr;
		foreach_parent([&parent, CompareFunc](const Node* node) -> bool {
			if (auto item = dynamic_cast<T*>(const_cast<Node*>(node)))
				if (CompareFunc(item))
					parent = item;
			return parent;
		});
		return parent;
	}

	//! Returns the cached times values for all the children
	const time_set &get_times() const;

	//! Writeme!
	Glib::Threads::RWLock& get_rw_lock()const { return rw_lock_; }

	virtual String get_string()const = 0;

protected:
	void begin_delete();

private:
	//! Add a new parent Node to parent_set
	void add_parent(Node* new_parent);
	//! Remove a Node from parent_set.
	//! No error is reported if it is not a parent node
	void remove_parent(Node* parent);
	/*
 --	** -- V I R T U A L   F U N C T I O N S -----------------------------------
	*/

protected:
	//! Used when the node has changed. Makes changed the parent too.
	//! To be overloaded by the derivative classes. Emits a signal where the
	//! the GUI can be connected to.
	virtual void on_changed();

	//! Used when child node has changed. Calls changed() too.
	//! To be overloaded by the derivative classes. Emits a signal where the
	//! the GUI can be connected to.
	virtual void on_child_changed(const Node *x);

	//! Used when the node's GUID has changed.
	//! To be overloaded by the derivative classes. Emits a signal where the
	//! the GUI can be connected to.
	virtual void on_guid_changed(GUID guid);

	//! Function to be overloaded that fills the Time Point Set with
	//! all the children Time Points.
	virtual void get_times_vfunc(time_set &set) const = 0;
}; // End of Node class

//! Finds a node by its GUID.
//! \see global_node_map()
synfig::Node* find_node(const synfig::GUID& guid);

//! Returns a Handle to the Node by its GUID
template<typename T> etl::handle<T>
guid_cast(const synfig::GUID& guid)
{
	return etl::handle<T>::cast_dynamic(synfig::find_node(guid));
}

typedef etl::handle<Node> NodeHandle;

}; // END of namespace synfig

/* === E N D =============================================================== */

#endif
