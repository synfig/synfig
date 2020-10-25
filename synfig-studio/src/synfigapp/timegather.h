/* === S Y N F I G ========================================================= */
/*!	\file timegather.h
**	\brief Time Gather Header
**
**	\legal
**	Copyright (c) 2004 Adrian Bentley
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

#ifndef __SYNFIG_TIMEGATHER_H
#define __SYNFIG_TIMEGATHER_H

/* === H E A D E R S ======================================================= */
#include <synfig/valuenodes/valuenode_animated.h>
#include <synfig/valuenodes/valuenode_dynamiclist.h>
#include <synfig/time.h>
#include "value_desc.h"

/* === M A C R O S ========================================================= */

/* === T Y P E D E F S ===================================================== */

/* === C L A S S E S & S T R U C T S ======================================= */

namespace synfig {
	class Time;
}

namespace synfigapp {

class ValueDesc;

struct ValueBaseTimeInfo
{
	synfig::ValueNode_Animated::Handle	val;
	synfig::Real						time_dilation;
	mutable std::set<synfig::Waypoint>	waypoints;

	bool operator<(const ValueBaseTimeInfo &rhs) const
	{
		return val == rhs.val ? time_dilation < rhs.time_dilation : val < rhs.val;
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

	synfigapp::ValueDesc							val;

	synfig::Real									time_dilation;

	typedef std::set<synfig::Activepoint,actcmp>	set;

	mutable set activepoints;

	bool operator<(const ActiveTimeInfo &rhs) const
	{
		return val.get_parent_value_node() == rhs.val.get_parent_value_node() ?
						val.get_index() == rhs.val.get_index() ?
							time_dilation < rhs.time_dilation :
							val.get_index() < rhs.val.get_index() :
						val.get_parent_value_node() < rhs.val.get_parent_value_node();
	}
};

struct timepoints_ref
{
	typedef std::set<ValueBaseTimeInfo>		waytracker;
	typedef std::set<ActiveTimeInfo>		acttracker;

	waytracker		waypointbiglist;
	acttracker		actpointbiglist;

	void insert(synfig::ValueNode_Animated::Handle v, synfig::Waypoint w, synfig::Real time_dilation = 1);
	void insert(synfigapp::ValueDesc v, synfig::Activepoint a, synfig::Real time_dilation = 1);
};

//assumes they're sorted... (incremental advance)
//checks the intersection of the two sets... might be something better in the stl
template < typename I1, typename I2 >
bool check_intersect(I1 b1, I1 end1, I2 b2, I2 end2, synfig::Time time_offset = 0, synfig::Real time_dilation = 1)
{
	if(b1 == end1 || b2 == end2)
		return false;

	for(; b1 != end1 && b2 != end2;)
	{
		if(*b1 < *b2 * time_dilation + time_offset) ++b1;
		else if(*b2 * time_dilation + time_offset < *b1) ++b2;
		else
		{
			assert(*b1 == *b2 * time_dilation + time_offset);
			return true;
		}
	}
	return false;
}

//gets the closest time inside the set
bool get_closest_time(const synfig::Node::time_set &tset, const synfig::Time &t,
						const synfig::Time &range, synfig::Time &out);

//recursion functions based on time restrictions (can be expanded later)...
//builds a list of relevant waypoints and activepoints inside the timepoints_ref structure
void recurse_valuedesc(synfigapp::ValueDesc valdesc, const std::set<synfig::Time> &tlist,
								timepoints_ref &vals, synfig::Time time = 0, synfig::Real time_dilation = 1);
void recurse_layer(synfig::Layer::Handle layer, const std::set<synfig::Time> &tlist,
								timepoints_ref &vals, synfig::Time time = 0, synfig::Real time_dilation = 1);
void recurse_canvas(synfig::Canvas::Handle canvas, const std::set<synfig::Time> &tlist,
								timepoints_ref &vals, synfig::Time time = 0, synfig::Real time_dilation = 1);



}; // END of namespace studio

/* === E N D =============================================================== */

#endif
