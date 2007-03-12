/* === S Y N F I G ========================================================= */
/*!	\file node.h
**	\brief Template Header
**
**	$Id: node.h,v 1.3 2005/01/10 07:40:26 darco Exp $
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

#ifndef __SYNFIG_PARENTNODE_H
#define __SYNFIG_PARENTNODE_H

/* === H E A D E R S ======================================================= */

#include <sigc++/signal.h>
#include <set>
#include "time.h"
#include "guid.h"
#include <ETL/handle>
#include "interpolation.h"
#include "mutex.h"

/* === M A C R O S ========================================================= */

/* === T Y P E D E F S ===================================================== */

/* === C L A S S E S & S T R U C T S ======================================= */

namespace synfig {

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

	const GUID& get_guid()const { return guid; }
	const Time& get_time()const { return time; }
	Interpolation get_before()const { return before; }
	Interpolation get_after()const { return after; }

	void set_guid(const GUID& x) { guid=x; }
	void set_time(const Time& x) { time=x; }
	void set_before(Interpolation x) { before=x; }
	void set_after(Interpolation x) { after=x; }

	void absorb(const TimePoint& x);
}; // END of class TimePoint

inline TimePoint operator+(TimePoint lhs,const Time& rhs)
	{ lhs.set_time(lhs.get_time()+rhs); return lhs; }

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

	//! \writeme
	GUID guid_;

	//! cached time values for all the childrens
	mutable time_set	times;

	//! \writeme
	mutable bool		bchanged;

	//! \writeme
	mutable int time_last_changed_;

	//! \writeme
	mutable RWLock rw_lock_;

	//! \writeme
	bool deleting_;

public:

	//! \todo This should really be private
	std::set<Node*> 	parent_set;

	/*
 -- ** -- S I G N A L S -------------------------------------------------------
	*/

private:

	sigc::signal<void> signal_changed_;

	//!	GUID Changed
	/*! \note The second parameter is the *OLD* guid! */
	sigc::signal<void,GUID> signal_guid_changed_;

	//!	Deleted
	sigc::signal<void> signal_deleted_;

	/*
 -- ** -- S I G N A L   I N T E R F A C E -------------------------------------
	*/

public:

	sigc::signal<void>& signal_deleted() { return signal_deleted_; }

	sigc::signal<void>& signal_changed() { return signal_changed_; }

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

	//! Gets the GUID for this value node
	const GUID& get_guid()const;

	//! Sets the GUID for this value node
	void set_guid(const GUID& x);

	int get_time_last_changed()const;

	void add_child(Node*x);

	void remove_child(Node*x);

	int parent_count()const;

	const time_set &get_times() const;

	RWLock& get_rw_lock()const { return rw_lock_; }

protected:

	void begin_delete();

	/*
 --	** -- V I R T U A L   F U N C T I O N S -----------------------------------
	*/

protected:
	virtual void on_changed();

	virtual void on_guid_changed(GUID guid);

	/*!	Function to be overloaded that fills
	*/
	virtual void get_times_vfunc(time_set &set) const = 0;
};

synfig::Node* find_node(const synfig::GUID& guid);

template<typename T> etl::handle<T>
guid_cast(const synfig::GUID& guid)
{
	return etl::handle<T>::cast_dynamic(synfig::find_node(guid));
}

typedef etl::handle<Node> NodeHandle;

}; // END of namespace synfig

/* === E N D =============================================================== */

#endif
