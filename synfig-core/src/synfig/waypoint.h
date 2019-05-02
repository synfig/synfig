/* === S Y N F I G ========================================================= */
/*!	\file waypoint.h
**	\brief Waypoint class header.
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
**	\brief Waypoint is used to handle variations along the time of the ValueNodes
*
* The Waypoint is a child of a ValueNode (or any of inherited) and it describes the
* Interpolation type (before and after), the ValueNode (usually waypoints are constant
* but in fact they can be animated) and the time where the waypoint is.
* \see Waypoint::get_value(), Waypoint::get_value(const Time &t)
*/
class Waypoint : public UniqueID
{
	/*
 --	** -- T Y P E S -----------------------------------------------------------
	*/

public:

	typedef synfig::Interpolation Interpolation;

/*! \class Waypoint::Model
 * 	\brief Waypoint::Model is a Waypoint model. It is used to store and
 * 	retrieve the values of the waypoint that is going to be modified. Once
 * 	the model is completely modified then it can be applied to the waypoint
 * 	itself by using the \apply_model() member
 */
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

		//! Gets before Interpolation
		Interpolation get_before()const { return before; }
		//! Sets before Interpolation
		void set_before(Interpolation x) { before=x; before_flag=true;}
		//! Gets after Interpolation
		Interpolation get_after()const { return after; }
		//! Sets after Interpolation
		void set_after(Interpolation x) { after=x; after_flag=true;}
		//! Gets tension
		const Real &get_tension()const { return tension; }
		//! Sets tension
		void set_tension(const Real &x) { tension=x; tension_flag=true;}
		//! Gets continuity
		const Real &get_continuity()const { return continuity; }
		//! Sets continuity
		void set_continuity(const Real &x) { continuity=x; continuity_flag=true;}
		//! Gets bias
		const Real &get_bias()const { return bias; }
		//! Sets bias
		void set_bias(const Real &x) { bias=x; bias_flag=true;}
		//! Gets temporal tension
		const Real &get_temporal_tension()const { return temporal_tension; }
		//! Sets temporal tension
		void set_temporal_tension(const Real &x) { temporal_tension=x; temporal_tension_flag=true;}
		//! Gets priority
		int get_priority()const { return priority; }
		//! Sets priority
		void set_priority(int x) { priority=x; priority_flag=true;}

		//! Get & Set members for the flags
		#define FLAG_MACRO(x) bool get_##x##_flag()const { return x##_flag; } void set_##x##_flag(bool y) { x##_flag=y; }
		FLAG_MACRO(priority)
		FLAG_MACRO(before)
		FLAG_MACRO(after)
		FLAG_MACRO(tension)
		FLAG_MACRO(continuity)
		FLAG_MACRO(bias)
		FLAG_MACRO(temporal_tension)
		#undef FLAG_MACRO

		//! Converts the Model in trivial: None of its values will be applied
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

		//! Checks if none of the Model information is relevant for the Waypoint
		//! If all the flags are off, the Model doesn't apply wnything to the
		//! waypoint. \see apply_model(const Model &x)
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
	}; // END of class Model

	enum Side
	{
		SIDE_UNSPECIFIED, SIDE_LEFT, SIDE_RIGHT,

	    SIDE_END=2				//!< \internal
	};

	/*
 --	** -- D A T A -------------------------------------------------------------
	*/

private:

	//! Writeme
	int priority_;
	//! Usually Animated Value Nodes are parents of waypoints
	//! \see class ValueNode_Animated
	etl::loose_handle<ValueNode> parent_;
	//! The two Interpolations before and after
	Interpolation before, after;
	//! The value node that is hold by the waypoint
	etl::rhandle<ValueNode> value_node;
	//! The time of the waypoint
	Time time;

	//! The following are for the INTERPOLATION_TCB type
	Real tension;
	Real continuity;
	Real bias;

	//! Shouldn't be Real?
	float time_tension;

	/*
 --	** -- C O N S T R U C T O R S ---------------------------------------------
	*/

public:

	//! Constructor for constant Waypoint
	Waypoint(ValueBase value, Time time);
	//! Constructor for animated Waypoint
	//! Is is called anytime?
	Waypoint(etl::handle<ValueNode> value_node, Time time);

	//! Default constructor. Leaves unset the Value Node
	Waypoint();

	/*
 --	** -- M E M B E R   F U N C T I O N S -------------------------------------
	*/

public:

	//! Applies the content of the Model to the Waypoint. It doesn't alter
	//! the Value Node hold or the time.
	void apply_model(const Model &x);

	//! Gets the before Interpolation
	Interpolation get_before()const { return before; }
	//! Sets the before Interpolation
	void set_before(Interpolation x) { before=x; }
	//! Gets the after Interpolation
	Interpolation get_after()const { return after; }
	//! Sets the after Interpolation
	void set_after(Interpolation x) { after=x; }
	//! Gets the value hold by the Waypoint
	ValueBase get_value()const;
	//!Gets the value hold by the Waypoint at time \t when it is animated
	ValueBase get_value(const Time &t)const;
	//!Sets the value of the Waypoint.
	//!Maybe it would be possible to define set_value(const ValueBase &x, Time &t) ?
	void set_value(const ValueBase &x);
	//! Returns the handle to the value node
	const etl::rhandle<ValueNode> &get_value_node()const { return value_node; }
	//! Sets the value node by handle
	void set_value_node(const etl::handle<ValueNode> &x);

	//! Gets tension
	const Real &get_tension()const { return tension; }
	//! Sets tension
	void set_tension(const Real &x) { tension=x; }
	//! Gets continuity
	const Real &get_continuity()const { return continuity; }
	//! Sets continuity
	void set_continuity(const Real &x) { continuity=x; }
	//! Gets bias
	const Real &get_bias()const { return bias; }
	//! Sets bias
	void set_bias(const Real &x) { bias=x; }

	//! Gets the time of the waypoint
	const Time &get_time()const { return time; }
	//! Sets the time of the waypoint
	void set_time(const Time &x);

	int get_priority()const { return priority_; }
	void set_priority(int x) { priority_=x; }

	//! Gets parent Value Node
	const etl::loose_handle<ValueNode> &get_parent_value_node()const { return parent_; }

	//! Sets parent Value Node
	void set_parent_value_node(const etl::loose_handle<ValueNode> &x);

	//! \true if the Value Node is constant, not null and not exported
	bool is_static()const;

	//!! Gets temporal tension
	float get_temporal_tension()const { return time_tension; }
	//!! Sets temporal tension
	void set_temporal_tension(const float& x) { time_tension=x; }

	//! True if the current waypoint's time is earlier than the compared waypoint's time
	bool operator<(const Waypoint &rhs)const
	{ return time<rhs.time; }
	//! True if the current waypoint's time is earlier than the given time
	bool operator<(const Time &rhs)const
	{ return time.is_less_than(rhs); }
	//! True if the current waypoint's time is later than the given time
	bool operator>(const Time &rhs)const
	{ return time.is_more_than(rhs); }
	//! True if the waypoint's time is the same than the given time
	bool operator==(const Time &rhs)const
	{ return time.is_equal(rhs); }
	//! True if the waypoint's time is different than the given time
	bool operator!=(const Time &rhs)const
	{ return !time.is_equal(rhs); }

	//! True if the Waypoint's Unique Id is the same than the argument's Unique ID
	bool operator==(const UniqueID &rhs)const
	{ return get_uid()==rhs.get_uid(); }
	//! True if the Waypoint's Unique Id is different than the argument's Unique ID
	bool operator!=(const UniqueID &rhs)const
	{ return get_uid()!=rhs.get_uid(); }


	//! Clones the Value Node if it is not exported and returns a Waypoint
	//! with no parent.
	Waypoint clone(etl::loose_handle<Canvas> canvas, const GUID& deriv_guid=GUID())const;

	//! Returns a hack GUID using the UniqueID's value
	GUID get_guid()const;
}; // END of class Waypoint

typedef std::vector< Waypoint > WaypointList;

}; // END of namespace synfig

/* === E N D =============================================================== */

#endif
