/* === S Y N F I G ========================================================= */
/*!	\file timegather.h
**	\brief Time Gather Header
**
**	$Id: timegather.h,v 1.1.1.1 2005/01/07 03:34:37 darco Exp $
**
**	\legal
**	Copyright (c) 2004 Adrian Bentley
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

#ifndef __SYNFIG_TIMEGATHER_H
#define __SYNFIG_TIMEGATHER_H

/* === H E A D E R S ======================================================= */
#include <synfig/valuenode_animated.h>
#include <synfig/valuenode_dynamiclist.h>
#include <synfig/time.h>
#include "value_desc.h"

/* === M A C R O S ========================================================= */

/* === T Y P E D E F S ===================================================== */

/* === C L A S S E S & S T R U C T S ======================================= */

namespace synfigapp {

class ValueDesc;
class synfig::Time;

struct ValueBaseTimeInfo
{
	synfig::ValueNode_Animated::Handle	val;
	mutable std::set<synfig::Waypoint>	waypoints;

	bool operator<(const ValueBaseTimeInfo &rhs) const
	{
		return val < rhs.val;
	}
};

struct ActiveTimeInfo
{
	struct actcmp
	{
		bool operator()(const synfig::Activepoint &lhs, const synfig::Activepoint &rhs) const
		{
			return lhs.time < rhs.time;
		}
	};

	synfigapp::ValueDesc						val;

	typedef std::set<synfig::Activepoint,actcmp>	set;

	mutable set activepoints;

	bool operator<(const ActiveTimeInfo &rhs) const
	{
		return val.get_parent_value_node() == rhs.val.get_parent_value_node() ?
						val.get_index() < rhs.val.get_index() :
						val.get_parent_value_node() < rhs.val.get_parent_value_node();
	}
};

struct timepoints_ref
{
	typedef std::set<ValueBaseTimeInfo>		waytracker;
	typedef std::set<ActiveTimeInfo>	acttracker;

	waytracker		waypointbiglist;
	acttracker		actpointbiglist;

	void insert(synfig::ValueNode_Animated::Handle v, synfig::Waypoint w);
	void insert(synfigapp::ValueDesc v, synfig::Activepoint a);
};

//assumes they're sorted... (incremental advance)
//checks the intersection of the two sets... might be something better in the stl
template < typename I1, typename I2 >
bool check_intersect(I1 b1, I1 end1, I2 b2, I2 end2)
{
	if(b1 == end1 || b2 == end2)
		return false;

	for(; b1 != end1 && b2 != end2;)
	{
		if(*b1 < *b2) ++b1;
		else if(*b2 < *b1) ++b2;
		else
		{
			assert(*b1 == *b2);
			return true;
		}
	}
	return false;
}

//pointer kind of a hack, gets the accurate times from a value desc
//	(deals with dynamic list member correctly... i.e. gathers activepoints)
const synfig::Node::time_set *get_times_from_vdesc(const synfigapp::ValueDesc &v);

//get's the closest time inside the set
bool get_closest_time(const synfig::Node::time_set &tset, const synfig::Time &t,
						const synfig::Time &range, synfig::Time &out);

//recursion functions based on time restrictions (can be expanded later)...
//builds a list of relevant waypoints and activepoints inside the timepoints_ref structure
void recurse_valuedesc(synfigapp::ValueDesc valdesc, const std::set<synfig::Time> &tlist,
								timepoints_ref &vals);
void recurse_layer(synfig::Layer::Handle layer, const std::set<synfig::Time> &tlist,
								timepoints_ref &vals);
void recurse_canvas(synfig::Canvas::Handle canvas, const std::set<synfig::Time> &tlist,
								timepoints_ref &vals);



}; // END of namespace studio

/* === E N D =============================================================== */

#endif
