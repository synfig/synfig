/* === S I N F G =========================================================== */
/*!	\file timegather.h
**	\brief Time Gather Header
**
**	$Id: timegather.h,v 1.1.1.1 2005/01/07 03:34:37 darco Exp $
**
**	\legal
**	Copyright (c) 2004 Adrian Bentley
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

#ifndef __SINFG_TIMEGATHER_H
#define __SINFG_TIMEGATHER_H

/* === H E A D E R S ======================================================= */
#include <sinfg/valuenode_animated.h>
#include <sinfg/valuenode_dynamiclist.h>
#include <sinfg/time.h>
#include "value_desc.h"

/* === M A C R O S ========================================================= */

/* === T Y P E D E F S ===================================================== */

/* === C L A S S E S & S T R U C T S ======================================= */

namespace sinfgapp {

class ValueDesc;
class sinfg::Time;

struct ValueBaseTimeInfo
{
	sinfg::ValueNode_Animated::Handle	val;
	mutable std::set<sinfg::Waypoint>	waypoints;
	
	bool operator<(const ValueBaseTimeInfo &rhs) const
	{
		return val < rhs.val;
	}
};

struct ActiveTimeInfo
{
	struct actcmp
	{
		bool operator()(const sinfg::Activepoint &lhs, const sinfg::Activepoint &rhs) const
		{
			return lhs.time < rhs.time;
		}		
	};
	
	sinfgapp::ValueDesc						val;
	
	typedef std::set<sinfg::Activepoint,actcmp>	set;
	
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
	
	void insert(sinfg::ValueNode_Animated::Handle v, sinfg::Waypoint w);	
	void insert(sinfgapp::ValueDesc v, sinfg::Activepoint a);
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
const sinfg::Node::time_set *get_times_from_vdesc(const sinfgapp::ValueDesc &v);

//get's the closest time inside the set
bool get_closest_time(const sinfg::Node::time_set &tset, const sinfg::Time &t, 
						const sinfg::Time &range, sinfg::Time &out);

//recursion functions based on time restrictions (can be expanded later)...
//builds a list of relevant waypoints and activepoints inside the timepoints_ref structure
void recurse_valuedesc(sinfgapp::ValueDesc valdesc, const std::set<sinfg::Time> &tlist,
								timepoints_ref &vals);
void recurse_layer(sinfg::Layer::Handle layer, const std::set<sinfg::Time> &tlist, 
								timepoints_ref &vals);
void recurse_canvas(sinfg::Canvas::Handle canvas, const std::set<sinfg::Time> &tlist, 
								timepoints_ref &vals);



}; // END of namespace studio

/* === E N D =============================================================== */

#endif
