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

#include <synfig/valuenode.h>
#include <synfig/uniqueid.h>
#include <synfig/waypoint.h>

/* === M A C R O S ========================================================= */

/* === C L A S S E S & S T R U C T S ======================================= */

namespace synfig {

/*! \class ValueNode_Animated
 *  \brief Virtual class for the derived ValueNode Animated implementations.
 *
 * It stores the list of waypoints and defines the base methods for:
 * add a given waypoint, return a new waypoint at a given time, some find
 * functions to find waypoints in the list, and to create a handles to
 * ValueNode_Animated based on the ValueBase Type of the given value node.
 * Also defines virtual methods to add a new waypoint at a given time using
 * a value node. They must be redefined by the inherited classes and will
 * be different depending on the type of value being animated.
*/
struct ValueNode_Animated : public ValueNode
{
private:
	Interpolation interpolation_;
public:
	typedef etl::handle<ValueNode_Animated> Handle;
	typedef etl::handle<const ValueNode_Animated> ConstHandle;

	typedef synfig::Waypoint Waypoint;
	typedef synfig::WaypointList WaypointList;

	typedef	std::pair<WaypointList::iterator,bool>	findresult;
	typedef	std::pair<WaypointList::const_iterator,bool>	const_findresult;

protected:
	//! List of Waypoints. \see waypoint.h
	WaypointList waypoint_list_;

public:
	WaypointList &editable_waypoint_list() { return waypoint_list_; }

	const WaypointList &waypoint_list()const { return waypoint_list_; }

	//! Creates a new waypoint at a Time \t with a given ValueBase \value
	//! Must be redefined in the inherited class
	virtual WaypointList::iterator new_waypoint(Time t, ValueBase value)=0;
	//! Creates a new waypoint at a Time \t with a given ValueNode handle \value_node
	//! Must be redefined in the inherited class
	virtual WaypointList::iterator new_waypoint(Time t, ValueNode::Handle value_node)=0;

	//! Returns a new waypoint at a given time but it is not inserted in the Waypoint List.
	/*! \note this does not add any waypoint to the ValueNode! */
	Waypoint new_waypoint_at_time(const Time& t)const;

	//! Adds a waypoint \x
	//! \see : Waypoint new_waypoint_at_time(const Time& t)const;
	WaypointList::iterator add(const Waypoint &x);

	bool waypoint_is_only_use_of_valuenode(Waypoint &waypoint);

	//! Removes a waypoint based on its UniqueId from the waypoint list
	void erase(const UniqueID &x);

	//! Finds Waypoint iterator and associated boolean if found. Find by UniqueID
	findresult 			find_uid(const UniqueID &x);
	//! Finds Waypoint iterator and associated boolean if found. Find by UniqueID
	const_findresult 	find_uid(const UniqueID &x)const;
	//! Finds Waypoint iterator and associated boolean if found. Find by Time
	findresult			find_time(const Time &x);
	//! Finds Waypoint iterator and associated boolean if found. Find by Time
	const_findresult	find_time(const Time &x)const;

	//! Finds a Waypoint by given UniqueID \x
	WaypointList::iterator find(const UniqueID &x);
	//! Finds a Waypoint by given UniqueID \x
	WaypointList::const_iterator find(const UniqueID &x)const;
	//! Finds a Waypoint by given Time \x
	WaypointList::iterator find(const Time &x);
	//! Finds a Waypoint by given Time \x
	WaypointList::const_iterator find(const Time &x)const;

	//! Finds next Waypoint at a given time \x starting from current waypoint
	WaypointList::iterator find_next(const Time &x);
	//! Finds next Waypoint at a given time \x starting from current waypoint
	WaypointList::const_iterator find_next(const Time &x)const;
	//! Finds previous Waypoint at a given time \x starting from current waypoint
	WaypointList::iterator find_prev(const Time &x);
	//! Finds previous Waypoint at a given time \x starting from current waypoint
	WaypointList::const_iterator find_prev(const Time &x)const;

	virtual ~ValueNode_Animated();

	//! Virtual member to be filled by inherited classes
	virtual String get_name()const;
	//! Virtual member to be filled by inherited classes
	virtual String get_local_name()const;

	//! Creates a Valuenode_Animated by type
	static Handle create(Type &type);
	//! Creates a Valuenode_Animated by ValueBase and Time
	static Handle create(const ValueBase& value, const Time& time);
	//! Creates a Valuenode_Animated by ValueNode and Time
	static Handle create(ValueNode::Handle value_node, const Time& time);

	//! Fills the \list with the waypoints between \begin and \end
	int find(const Time& begin,const Time& end,std::vector<Waypoint*>& list);

	//! Inserts time \delta from time \location to the waypoints.
	//! used to move waypoints in the time line.
	void insert_time(const Time& location, const Time& delta);

	String get_string()const;

	//! Get/Set the default interpolation for Value Nodes
	virtual Interpolation get_interpolation()const { return interpolation_; }
	virtual void set_interpolation(Interpolation i) { interpolation_=i; }

	

protected:
	ValueNode_Animated();

	//! Sets the type of the Animated Value Node
	void set_type(Type &t);
	//!	Function to be overloaded that fills the Time Point Set with
	//! all the children Time Points. Time Point is like Waypoint but
	//! without value node
	virtual void get_times_vfunc(Node::time_set &set) const;
};

}; // END of namespace synfig

/* === E N D =============================================================== */

#endif
