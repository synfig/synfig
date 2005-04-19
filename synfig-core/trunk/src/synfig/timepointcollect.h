/* === S Y N F I G ========================================================= */
/*!	\file timepointcollect.h
**	\brief Template Header
**
**	$Id: timepointcollect.h,v 1.1.1.1 2005/01/04 01:23:15 darco Exp $
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

#ifndef __SYNFIG_TIMEPOINTCOLLECT_H
#define __SYNFIG_TIMEPOINTCOLLECT_H

/* === H E A D E R S ======================================================= */

#include <set>
#include "activepoint.h"
#include "waypoint.h"
#include "node.h"
#include "time.h"

/* === M A C R O S ========================================================= */

/* === T Y P E D E F S ===================================================== */

/* === C L A S S E S & S T R U C T S ======================================= */

namespace synfig {

//! \writeme
int waypoint_collect(std::set<Waypoint, std::less<UniqueID> >& waypoint_set,const Time& time, const etl::handle<Node>& node);

//! \writeme
int activepoint_collect(std::set<Activepoint, std::less<UniqueID> >& activepoint_set,const Time& time, const etl::handle<Node>& node);
	
}; // END of namespace synfig

/* === E N D =============================================================== */

#endif
