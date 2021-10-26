/* === S Y N F I G ========================================================= */
/*!	\file valuenode_animatedinterface.h
**	\brief Header file for implementation of the "Animated" valuenode interface.
**
**	\legal
**	Copyright (c) 2002-2005 Robert B. Quattlebaum Jr., Adrian Bentley
**	......... ... 2016 Ivan Mahonin
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

#ifndef __SYNFIG_VALUENODE_ANIMATEDBASE_H
#define __SYNFIG_VALUENODE_ANIMATEDBASE_H

/* === H E A D E R S ======================================================= */

#include <list>

#include <synfig/valuenode.h>
#include <synfig/uniqueid.h>
#include <synfig/waypoint.h>

/* === M A C R O S ========================================================= */

/* === C L A S S E S & S T R U C T S ======================================= */

namespace synfig {

/*! \class ValueNode_AnimatedInterface
 *  \brief Virtual class for the derived ValueNode Animated implementations.
 *
 * It stores the list of waypoints and defines the base methods for:
 * add a given waypoint, return a new waypoint at a given time, some find
 * functions to find waypoints in the list, and to create a handles to
 * ValueNode_AnimatedBase based on the ValueBase Type of the given value node.
 * Also defines virtual methods to add a new waypoint at a given time using
 * a value node. They must be redefined by the inherited classes and will
 * be different depending on the type of value being animated.
*/
class ValueNode_AnimatedInterfaceConst: public ValueNode_Interface
{
private:
	class Internal;

public:
	class Interpolator;

	typedef synfig::Waypoint Waypoint;
	typedef synfig::WaypointList WaypointList;

	typedef	std::pair<WaypointList::iterator,bool>	findresult;
	typedef	std::pair<WaypointList::const_iterator,bool>	const_findresult;

private:
	Interpolation interpolation_;
	Interpolator *interpolator_;

	//! List of Waypoints. \see waypoint.h
	WaypointList waypoint_list_;

protected:
	explicit ValueNode_AnimatedInterfaceConst(ValueNode &node);

public:
	virtual ~ValueNode_AnimatedInterfaceConst();

protected:
	WaypointList &editable_waypoint_list() { return waypoint_list_; }

	//! Creates a new waypoint at a Time \t with a given ValueBase \value
	//! Must be redefined in the inherited class
	WaypointList::iterator new_waypoint(Time t, ValueBase value);
	//! Creates a new waypoint at a Time \t with a given ValueNode handle \value_node
	//! Must be redefined in the inherited class
	WaypointList::iterator new_waypoint(Time t, ValueNode::Handle value_node);
	//! Adds a waypoint \x
	//! \see : Waypoint new_waypoint_at_time(const Time& t)const;
	WaypointList::iterator add(const Waypoint &x);
	//! Inserts time \delta from time \location to the waypoints.
	//! used to move waypoints in the time line.
	void insert_time(const Time& location, const Time& delta);
	//! Removes a waypoint based on its UniqueId from the waypoint list
	void erase(const UniqueID &x);
	//! Removes all waypoints
	void erase_all();

	//! Finds Waypoint iterator and associated boolean if found. Find by UniqueID
	findresult 			   find_uid(const UniqueID &x);
	//! Finds Waypoint iterator and associated boolean if found. Find by Time
	findresult			   find_time(const Time &x);
	//! Finds a Waypoint by given UniqueID \x
	WaypointList::iterator find(const UniqueID &x);
	//! Finds a Waypoint by given Time \x
	WaypointList::iterator find(const Time &x);
	//! Finds next Waypoint at a given time \x starting from current waypoint
	WaypointList::iterator find_next(const Time &x);
	//! Finds previous Waypoint at a given time \x starting from current waypoint
	WaypointList::iterator find_prev(const Time &x);
	//! Fills the \list with the waypoints between \begin and \end
	int find(const Time& begin, const Time& end, std::vector<Waypoint*>& list);

	void set_type(Type &t);

	virtual void animated_changed() { }

	void on_changed();
	ValueBase operator()(Time t) const;
	void get_times_vfunc(Node::time_set &set) const;
	void get_values_vfunc(std::map<Time, ValueBase> &x) const;

	void assign(const ValueNode_AnimatedInterfaceConst &animated, const synfig::GUID& deriv_guid);

	//! Get/Set the default interpolation for Value Nodes
	Interpolation get_interpolation()const { return interpolation_; }
	void set_interpolation(Interpolation i) { interpolation_=i; }

public:
	const WaypointList &waypoint_list()const { return waypoint_list_; }
	bool waypoint_is_only_use_of_valuenode(Waypoint &waypoint) const;
	//! Returns a new waypoint at a given time but it is not inserted in the Waypoint List.
	/*! \note this does not add any waypoint to the ValueNode! */
	Waypoint new_waypoint_at_time(const Time& t)const;

	//! Finds Waypoint iterator and associated boolean if found. Find by UniqueID
	const_findresult 	         find_uid(const UniqueID &x)const;
	//! Finds Waypoint iterator and associated boolean if found. Find by Time
	const_findresult	         find_time(const Time &x)const;
	//! Finds a Waypoint by given UniqueID \x
	WaypointList::const_iterator find(const UniqueID &x)const;
	//! Finds a Waypoint by given Time \x
	WaypointList::const_iterator find(const Time &x)const;
	//! Finds next Waypoint at a given time \x starting from current waypoint
	WaypointList::const_iterator find_next(const Time &x)const;
	//! Finds previous Waypoint at a given time \x starting from current waypoint
	WaypointList::const_iterator find_prev(const Time &x)const;
	//! Fills the \list with the waypoints between \begin and \end
	int find(const Time& begin, const Time& end, std::vector<const Waypoint*>& list) const;
};

class ValueNode_AnimatedInterface: public ValueNode_AnimatedInterfaceConst
{
protected:
	explicit ValueNode_AnimatedInterface(ValueNode &node): ValueNode_AnimatedInterfaceConst(node) { }
	virtual void animated_changed() { node().changed(); }

public:
	using ValueNode_AnimatedInterfaceConst::editable_waypoint_list;
	using ValueNode_AnimatedInterfaceConst::new_waypoint;
	using ValueNode_AnimatedInterfaceConst::add;
	using ValueNode_AnimatedInterfaceConst::insert_time;
	using ValueNode_AnimatedInterfaceConst::erase;
	using ValueNode_AnimatedInterfaceConst::erase_all;

	using ValueNode_AnimatedInterfaceConst::find_uid;
	using ValueNode_AnimatedInterfaceConst::find_time;
	using ValueNode_AnimatedInterfaceConst::find;
	using ValueNode_AnimatedInterfaceConst::find_next;
	using ValueNode_AnimatedInterfaceConst::find_prev;
};


}; // END of namespace synfig

/* === E N D =============================================================== */

#endif
