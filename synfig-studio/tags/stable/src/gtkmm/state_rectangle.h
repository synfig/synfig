/* === S I N F G =========================================================== */
/*!	\file state_rectangle.h
**	\brief Rectangle creation state
**
**	$Id: state_rectangle.h,v 1.1.1.1 2005/01/07 03:34:37 darco Exp $
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

#ifndef __SINFG_STUDIO_STATE_RECTANGLE_H
#define __SINFG_STUDIO_STATE_RECTANGLE_H

/* === H E A D E R S ======================================================= */

#include "smach.h"


/* === M A C R O S ========================================================= */

/* === T Y P E D E F S ===================================================== */

/* === C L A S S E S & S T R U C T S ======================================= */

namespace studio {

class StateRectangle_Context;

class StateRectangle : public Smach::state<StateRectangle_Context>
{
public:
	StateRectangle();
	~StateRectangle();
}; // END of class StateRectangle

extern StateRectangle state_rectangle;
	
}; // END of namespace studio

/* === E N D =============================================================== */

#endif
