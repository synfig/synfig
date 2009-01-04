/* === S Y N F I G ========================================================= */
/*!	\file valuenode_animated.h
**	\brief Header file for implementation of the "Animated" valuenode conversion.
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

#ifndef __SYNFIG_VALUENODE_ANIMATED_H
#define __SYNFIG_VALUENODE_ANIMATED_H

/* === H E A D E R S ======================================================= */

#include <list>

#include "valuenode.h"
#include "uniqueid.h"
#include "waypoint.h"

/* === M A C R O S ========================================================= */

/* === C L A S S E S & S T R U C T S ======================================= */

namespace synfig {

struct ValueNode_Animated : public ValueNode
{
public:
	typedef etl::handle<ValueNode_Animated> Handle;
	typedef etl::handle<const ValueNode_Animated> ConstHandle;

	typedef synfig::Waypoint Waypoint;
	typedef synfig::WaypointList WaypointList;

	typedef	std::pair<WaypointList::iterator,bool>	findresult;
	typedef	std::pair<WaypointList::const_iterator,bool>	const_findresult;

protected:
	WaypointList waypoint_list_;

public:
	WaypointList &waypoint_list() { return waypoint_list_; }

	const WaypointList &waypoint_list()const { return waypoint_list_; }

	virtual WaypointList::iterator new_waypoint(Time t, ValueBase value)=0;

	virtual WaypointList::iterator new_waypoint(Time t, ValueNode::Handle value_node)=0;

	/*! \note this does not add any waypoint to the ValueNode! */
	Waypoint new_waypoint_at_time(const Time& t)const;

	WaypointList::iterator add(const Waypoint &x);

	bool waypoint_is_only_use_of_valuenode(Waypoint &waypoint);

	void erase(const UniqueID &x);

	//either use find result (return bool and iterator) or
	findresult 			find_uid(const UniqueID &x);
	const_findresult 	find_uid(const UniqueID &x)const;
	findresult			find_time(const Time &x);
	const_findresult	find_time(const Time &x)const;

	WaypointList::iterator find(const UniqueID &x);
	WaypointList::const_iterator find(const UniqueID &x)const;
	WaypointList::iterator find(const Time &x);
	WaypointList::const_iterator find(const Time &x)const;

	WaypointList::iterator find_next(const Time &x);
	WaypointList::const_iterator find_next(const Time &x)const;
	WaypointList::iterator find_prev(const Time &x);
	WaypointList::const_iterator find_prev(const Time &x)const;

	virtual ~ValueNode_Animated();

	virtual String get_name()const;
	virtual String get_local_name()const;

	static Handle create(ValueBase::Type type);

	static Handle create(const ValueBase& value, const Time& time);

	static Handle create(ValueNode::Handle value_node, const Time& time);

	int find(const Time& begin,const Time& end,std::vector<Waypoint*>& list);

	void insert_time(const Time& location, const Time& delta);

	String get_string()const;

protected:
	ValueNode_Animated();

	void set_type(ValueBase::Type t);
	virtual void get_times_vfunc(Node::time_set &set) const;
public:
	DCAST_HACK_ID(4);
};

}; // END of namespace synfig

/* === E N D =============================================================== */

#endif
