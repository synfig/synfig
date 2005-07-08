/* === S Y N F I G ========================================================= */
/*!	\file state_fill.h
**	\brief Template Header
**
**	$Id: state_fill.h,v 1.1.1.1 2005/01/07 03:34:36 darco Exp $
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

#ifndef __SYNFIG_STATE_FILL_H
#define __SYNFIG_STATE_FILL_H

/* === H E A D E R S ======================================================= */

#include "smach.h"


/* === M A C R O S ========================================================= */

/* === T Y P E D E F S ===================================================== */

/* === C L A S S E S & S T R U C T S ======================================= */

namespace studio { 
	
class StateFill_Context;

class StateFill : public Smach::state<StateFill_Context>
{
public:
	StateFill();
	~StateFill();
}; // END of class StateFill

extern StateFill state_fill;

}; // END of namespace studio

/* === E N D =============================================================== */

#endif
