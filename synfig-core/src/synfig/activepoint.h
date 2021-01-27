/* === S Y N F I G ========================================================= */
/*!	\file activepoint.h
**	\brief Template Header
**
**	$Id$
**
**	\legal
**	Copyright (c) 2002-2005 Robert B. Quattlebaum Jr., Adrian Bentley
**	Copyright (c) 2007 Chris Moore
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

#ifndef __SYNFIG_ACTIVEPOINT_H
#define __SYNFIG_ACTIVEPOINT_H

/* === H E A D E R S ======================================================= */

#include <cstdio>

#include <ETL/handle>

#include "time.h"
#include "uniqueid.h"

/* === M A C R O S ========================================================= */

/* === T Y P E D E F S ===================================================== */

/* === C L A S S E S & S T R U C T S ======================================= */

namespace synfig {
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

	bool operator<(const Activepoint& rhs) const { return time<rhs.time; }
	bool operator<(const Time& rhs) const { return time<rhs; }

	Activepoint(const Time &time, const bool &state, int p=0):
		index(), time(time), priority(p),state(state) { }
	//! \todo Should priority be initialized here, or elsewhere?  This avoids a valgrind warning for now.
	Activepoint(): index(), time(0), priority(0), state() { }

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

}; // END of namespace synfig

/* === E N D =============================================================== */

#endif
