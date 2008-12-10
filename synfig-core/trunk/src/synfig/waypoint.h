/* === S Y N F I G ========================================================= */
/*!	\file waypoint.h
**	\brief Template Header
**
**	$Id$
**
**	\legal
**	Copyright (c) 2002-2005 Robert B. Quattlebaum Jr., Adrian Bentley
**	Copyright (c) 2008 Chris Moore
**	Copyright (c) 2008 Paul Wise
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

#ifndef __SYNFIG_WAYPOINT_H
#define __SYNFIG_WAYPOINT_H

/* === H E A D E R S ======================================================= */

#include "time.h"
#include "real.h"
#include "value.h"
//#include "valuenode.h"
#include "uniqueid.h"
#include <vector>
#include "guid.h"
#include "interpolation.h"

/* === M A C R O S ========================================================= */

/* === T Y P E D E F S ===================================================== */

/* === C L A S S E S & S T R U C T S ======================================= */

namespace synfig {

class ValueNode;
class GUID;


/*!	\class Waypoint
**	\brief \writeme
*/
class Waypoint : public UniqueID
{
	/*
 --	** -- T Y P E S -----------------------------------------------------------
	*/

public:

	typedef synfig::Interpolation Interpolation;

	class Model
	{
		friend class Waypoint;

		int priority;
		Interpolation before;
		Interpolation after;
		Real tension;
		Real continuity;
		Real bias;
		Real temporal_tension;

		bool priority_flag,before_flag,after_flag,tension_flag,continuity_flag,bias_flag,temporal_tension_flag;

	public:
		Model():
			// we don't need to initialise these 5, but the compiler thinks they're used uninitialised if we don't
			// and this constructor isn't called often, so it's ok
			priority(0), before(INTERPOLATION_NIL), after(INTERPOLATION_NIL), tension(0), continuity(0), bias(0), temporal_tension(0),

			priority_flag(false),
			before_flag(false),
			after_flag(false),
			tension_flag(false),
			continuity_flag(false),
			bias_flag(false),
			temporal_tension_flag(false) { }

		Interpolation get_before()const { return before; }
		void set_before(Interpolation x) { before=x; before_flag=true;}

		Interpolation get_after()const { return after; }
		void set_after(Interpolation x) { after=x; after_flag=true;}

		const Real &get_tension()const { return tension; }
		void set_tension(const Real &x) { tension=x; tension_flag=true;}

		const Real &get_continuity()const { return continuity; }
		void set_continuity(const Real &x) { continuity=x; continuity_flag=true;}

		const Real &get_bias()const { return bias; }
		void set_bias(const Real &x) { bias=x; bias_flag=true;}

		const Real &get_temporal_tension()const { return temporal_tension; }
		void set_temporal_tension(const Real &x) { temporal_tension=x; temporal_tension_flag=true;}

		int get_priority()const { return priority; }
		void set_priority(int x) { priority=x; priority_flag=true;}

		#define FLAG_MACRO(x) bool get_##x##_flag()const { return x##_flag; } void set_##x##_flag(bool y) { x##_flag=y; }
		FLAG_MACRO(priority)
		FLAG_MACRO(before)
		FLAG_MACRO(after)
		FLAG_MACRO(tension)
		FLAG_MACRO(continuity)
		FLAG_MACRO(bias)
		FLAG_MACRO(temporal_tension)
		#undef FLAG_MACRO

		void reset()
		{
			priority_flag=false;
			before_flag=false;
			after_flag=false;
			tension_flag=false;
			continuity_flag=false;
			bias_flag=false;
			temporal_tension_flag=false;
		}

		bool is_trivial()const
		{
			return !(
				priority_flag||
				before_flag||
				after_flag||
				tension_flag||
				continuity_flag||
				bias_flag||
				temporal_tension_flag
			);
		}
	};

	enum Side
	{
		SIDE_UNSPECIFIED, SIDE_LEFT, SIDE_RIGHT,

	    SIDE_END=2				//!< \internal
	};

	/*
 --	** -- D A T A -------------------------------------------------------------
	*/

private:

	int priority_;
	etl::loose_handle<ValueNode> parent_;

	Interpolation before, after;

	etl::rhandle<ValueNode> value_node;

	Time time;

	// The following are for the INTERPOLATION_TCB type
	Real tension;
	Real continuity;
	Real bias;

	// The following are for the INTERPOLATION_MANUAL type
	ValueBase cpoint_before,cpoint_after;


	float time_tension;

	/*
 --	** -- C O N S T R U C T O R S ---------------------------------------------
	*/

public:

	Waypoint(ValueBase value, Time time);
	Waypoint(etl::handle<ValueNode> value_node, Time time);

	Waypoint();

	/*
 --	** -- M E M B E R   F U N C T I O N S -------------------------------------
	*/

public:

	void apply_model(const Model &x);

	Interpolation get_before()const { return before; }
	void set_before(Interpolation x) { before=x; }

	Interpolation get_after()const { return after; }
	void set_after(Interpolation x) { after=x; }

	ValueBase get_value()const;
	ValueBase get_value(const Time &t)const;
	void set_value(const ValueBase &x);

	const etl::rhandle<ValueNode> &get_value_node()const { return value_node; }
	void set_value_node(const etl::handle<ValueNode> &x);

	const Real &get_tension()const { return tension; }
	void set_tension(const Real &x) { tension=x; }

	const Real &get_continuity()const { return continuity; }
	void set_continuity(const Real &x) { continuity=x; }

	const Real &get_bias()const { return bias; }
	void set_bias(const Real &x) { bias=x; }

	const Time &get_time()const { return time; }
	void set_time(const Time &x);

	int get_priority()const { return priority_; }
	void set_priority(int x) { priority_=x; }

	const etl::loose_handle<ValueNode> &get_parent_value_node()const { return parent_; }
	void set_parent_value_node(const etl::loose_handle<ValueNode> &x) { parent_=x; }

	bool is_static()const;

	float get_temporal_tension()const { return time_tension; }
	void set_temporal_tension(const float& x) { time_tension=x; }

	bool operator<(const Waypoint &rhs)const
	{ return time<rhs.time; }

	bool operator<(const Time &rhs)const
	{ return time.is_less_than(rhs); }
	bool operator>(const Time &rhs)const
	{ return time.is_more_than(rhs); }

	bool operator==(const Time &rhs)const
	{ return time.is_equal(rhs); }
	bool operator!=(const Time &rhs)const
	{ return !time.is_equal(rhs); }

	bool operator==(const UniqueID &rhs)const
	{ return get_uid()==rhs.get_uid(); }
	bool operator!=(const UniqueID &rhs)const
	{ return get_uid()!=rhs.get_uid(); }

	Waypoint clone(etl::loose_handle<Canvas> canvas, const GUID& deriv_guid=GUID())const;

	GUID get_guid()const;
}; // END of class Waypoint

typedef std::vector< Waypoint > WaypointList;

}; // END of namespace synfig

/* === E N D =============================================================== */

#endif
