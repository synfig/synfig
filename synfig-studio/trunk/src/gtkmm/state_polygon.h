/* === S Y N F I G ========================================================= */
/*!	\file rotoscope_polygon.h
**	\brief Template Header
**
**	$Id: state_polygon.h,v 1.1.1.1 2005/01/07 03:34:37 darco Exp $
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

#ifndef __SYNFIG_STUDIO_ROTOSCOPE_POLYGON_H
#define __SYNFIG_STUDIO_ROTOSCOPE_POLYGON_H

/* === H E A D E R S ======================================================= */

#include "smach.h"

/* === M A C R O S ========================================================= */

/* === T Y P E D E F S ===================================================== */

/* === C L A S S E S & S T R U C T S ======================================= */

namespace studio {

class StatePolygon_Context;

class StatePolygon : public Smach::state<StatePolygon_Context>
{
public:
	StatePolygon();
	~StatePolygon();
}; // END of class StatePolygon

extern StatePolygon state_polygon;

}; // END of namespace studio

/* === E N D =============================================================== */

#endif
