/* === S I N F G =========================================================== */
/*!	\file state_gradient.h
**	\brief Template Header
**
**	$Id: state_gradient.h,v 1.1.1.1 2005/01/07 03:34:36 darco Exp $
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

#ifndef __SINFG_STUDIO_STATE_GRADIENT_H
#define __SINFG_STUDIO_STATE_GRADIENT_H

/* === H E A D E R S ======================================================= */

#include "smach.h"


/* === M A C R O S ========================================================= */

/* === T Y P E D E F S ===================================================== */

/* === C L A S S E S & S T R U C T S ======================================= */

namespace studio {

class StateGradient_Context;

class StateGradient : public Smach::state<StateGradient_Context>
{
public:
	StateGradient();
	~StateGradient();
}; // END of class StateGradient

extern StateGradient state_gradient;

}; // END of namespace studio

/* === E N D =============================================================== */

#endif
