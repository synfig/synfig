/* === S Y N F I G ========================================================= */
/*!	\file node.h
**	\brief Base class for Layers and Value Nodes.
**	It defines the base members for the parent - child relationship,
**	the times where the node is modified and the handling of
**	the GUID on deletion and changing.
**
**	$Id$
**
**	\legal
**	Copyright (c) 2002-2005 Robert B. Quattlebaum Jr., Adrian Bentley
**	Copyright (c) 2007 Chris Moore
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

#ifndef __SYNFIG_PARENTNODE_H
#define __SYNFIG_PARENTNODE_H

/* === H E A D E R S ======================================================= */

#include <sigc++/signal.h>
#include <set>
#include "time.h"
#include "guid.h"
#include <ETL/handle>
#include "interpolation.h"
#include <mutex>
#include "mutex.h"

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
		guid(0),
		time(x),
		before(INTERPOLATION_NIL),
		after(INTERPOLATION_NIL)
	{
	}

#ifdef _DEBUG
	const char *c_str()const;
#endif

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

class Node : public etl::rshared_object
{
	/*
 --	** -- T Y P E S -----------------------------------------------------------
	*/

public:

	//! \writeme
	typedef	TimePointSet	time_set;

	/*
 --	** -- D A T A -------------------------------------------------------------
	*/

private:

	//! \ The GUID of the node
	mutable std::mutex guid_mutex_;
	mutable GUID guid_;

	//! cached time values for all the children
	mutable time_set	times;

	//! \writeme
	mutable bool		bchanged;

	//! The last time the node was modified since the program started
	//! \see __sys_clock
	mutable int time_last_changed_;

	//! \writeme
	//! \see mutex.h
	mutable RWLock rw_lock_;

	//! Variable used to remember that a signal_deleted has been thrown
	bool deleting_;

public:

	//! A set of pointers to parent nodes
	//! \todo This should really be private
	std::set<Node*> 	parent_set;

	/*
 -- ** -- S I G N A L S -------------------------------------------------------
	*/

private:

	//! Node changed signal
	sigc::signal<void> signal_changed_;

	//! Child node changed signal
	sigc::signal<void, const Node*> signal_child_changed_;

	//!	GUID changed signal
	/*! \note The second parameter is the *OLD* guid! */
	sigc::signal<void,GUID> signal_guid_changed_;

	//!	Node deleted signal
	sigc::signal<void> signal_deleted_;

	/*
 -- ** -- S I G N A L   I N T E R F A C E -------------------------------------
	*/

public:

	sigc::signal<void>& signal_deleted() { return signal_deleted_; }

	sigc::signal<void>& signal_changed() { return signal_changed_; }

	sigc::signal<void, const Node*>& signal_child_changed() { return signal_child_changed_; }

	//!	GUID Changed
	/*! \note The second parameter is the *OLD* guid! */
	sigc::signal<void,GUID>& signal_guid_changed() { return signal_guid_changed_; }

	/*
 --	** -- C O N S T R U C T O R S ---------------------------------------------
	*/

protected:

	Node();

	// This class cannot be copied -- use clone() if necessary
private:
	Node(const Node &x);

public:
	virtual ~Node();

	/*
 --	** -- M E M B E R   F U N C T I O N S -------------------------------------
	*/

public:

	void changed();
	void child_changed(const Node *x);

	//! Gets the GUID for this Node
	const GUID& get_guid()const;

	//! Sets the GUID for this Node
	virtual void set_guid(const GUID& x);

	//! Gets the time when the Node was changed
	int get_time_last_changed()const;

	//! Adds the parameter \x as the child of the current Node
	void add_child(Node*x);

	//! Removes the parameter \x as a child of the current Node
	void remove_child(Node*x);

	//!Returns how many parenst has the current Node
	int parent_count()const;

	//! Returns the cached times values for all the children
	const time_set &get_times() const;

	//! Writeme!
	RWLock& get_rw_lock()const { return rw_lock_; }

	virtual String get_string()const = 0;
protected:

	void begin_delete();

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

	//!	Function to be overloaded that fills the Time Point Set with
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

#ifdef _DEBUG
template <typename T>
synfig::String set_string(T start, T end)
{
	synfig::String ret("[");
	bool started = false;

	while (start != end)
	{
		if (started)	ret += ", ";
		else			started = true;

		ret += synfig::String((*start).c_str());
		start++;
	}

	return ret + "]";
}

template <typename T>
synfig::String set_string(T set)
{
	return set_string(set.begin(), set.end());
}
#endif // _DEBUG

typedef etl::handle<Node> NodeHandle;

}; // END of namespace synfig

/* === E N D =============================================================== */

#endif
