/* === S Y N F I G ========================================================= */
/*!	\file state_roto.h
**	\brief Template Header
**
**	$Id: state_draw.h,v 1.1.1.1 2005/01/07 03:34:36 darco Exp $
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

#ifndef __SYNFIG_STUDIO_STATE_ROTOSCOPE_H
#define __SYNFIG_STUDIO_STATE_ROTOSCOPE_H

/* === H E A D E R S ======================================================= */

#include "smach.h"


/* === M A C R O S ========================================================= */

/* === T Y P E D E F S ===================================================== */

/* === C L A S S E S & S T R U C T S ======================================= */

namespace studio {

class StateDraw_Context;

class StateDraw : public Smach::state<StateDraw_Context>
{
public:
	StateDraw();
	~StateDraw();
}; // END of class StateDraw

extern StateDraw state_draw;
	
}; // END of namespace studio

/* === E N D =============================================================== */

#endif
