/* === S Y N F I G ========================================================= */
/*!	\file valuenode_animated.h
**	\brief Template Header
**
**	$Id: valuenode_animated.h,v 1.1.1.1 2005/01/04 01:23:15 darco Exp $
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
