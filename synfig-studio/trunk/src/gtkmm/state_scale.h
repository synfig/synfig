/* === S I N F G =========================================================== */
/*!	\file state_scale.h
**	\brief Template Header
**
**	$Id: state_scale.h,v 1.1.1.1 2005/01/07 03:34:37 darco Exp $
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

#ifndef __SINFG_STUDIO_STATE_SCALE_H
#define __SINFG_STUDIO_STATE_SCALE_H

/* === H E A D E R S ======================================================= */

#include "smach.h"

/* === M A C R O S ========================================================= */

/* === T Y P E D E F S ===================================================== */

/* === C L A S S E S & S T R U C T S ======================================= */

namespace studio {

class StateScale_Context;

class StateScale : public Smach::state<StateScale_Context>
{
public:
	StateScale();
	~StateScale();
}; // END of class StateScale

extern StateScale state_scale;

}; // END of namespace studio

/* === E N D =============================================================== */

#endif
