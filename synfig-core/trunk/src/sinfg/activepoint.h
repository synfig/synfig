/* === S I N F G =========================================================== */
/*!	\file activepoint.h
**	\brief Template Header
**
**	$Id: activepoint.h,v 1.1.1.1 2005/01/04 01:23:14 darco Exp $
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

#ifndef __SINFG_ACTIVEPOINT_H
#define __SINFG_ACTIVEPOINT_H

/* === H E A D E R S ======================================================= */

#include "time.h"
#include "uniqueid.h"
#include <ETL/handle>

/* === M A C R O S ========================================================= */

/* === T Y P E D E F S ===================================================== */

/* === C L A S S E S & S T R U C T S ======================================= */

namespace sinfg {
class GUID;
class ValueNode;
	
struct Activepoint : public UniqueID
{
private:
	etl::loose_handle<ValueNode> parent_;
	int index;
	
public:
	//! Time of the activepoint
	Time time;
	
	//! Priority
	int priority;

	//! Does this activepoint turn the entry on, or off?
	bool state;
		
	bool operator<(const Activepoint& rhs) { return time<rhs.time; }
	bool operator<(const Time& rhs) { return time<rhs; }
	
	Activepoint(const Time &time, const bool &state, int p=0): time(time), priority(p),state(state) { }
	Activepoint() { }
	
	const Time& get_time()const { return time; }
	void set_time(const Time& x) { time=x; }

	bool get_state()const { return state; }
	void set_state(bool x) { state=x; }

	int get_priority()const { return priority; }
	void set_priority(int x) { priority=x; }

	const etl::loose_handle<ValueNode> &get_parent_value_node()const { return parent_; }
	void set_parent_value_node(const etl::loose_handle<ValueNode> &x) { parent_=x; }	
	
	int get_parent_index()const { return index; }
	void set_parent_index(int x) { index=x; }
	
	GUID get_guid()const;
}; // END of struct ValueNode_BLine::Activepoint

}; // END of namespace sinfg

/* === E N D =============================================================== */

#endif
