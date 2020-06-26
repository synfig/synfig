/* === S Y N F I G ========================================================= */
/*!	\file state_circle.h
**	\brief Circle creation state
**
**	$Id$
**
**	\legal
**	Copyright (c) 2002-2005 Robert B. Quattlebaum Jr., Adrian Bentley
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

#ifndef __SYNFIG_STUDIO_STATE_CIRCLE_H
#define __SYNFIG_STUDIO_STATE_CIRCLE_H

/* === H E A D E R S ======================================================= */

#include "smach.h"


/* === M A C R O S ========================================================= */

/* === T Y P E D E F S ===================================================== */

/* === C L A S S E S & S T R U C T S ======================================= */

namespace studio {

class StateCircle_Context;

class StateCircle : public Smach::state<StateCircle_Context>
{
public:
	StateCircle();
	~StateCircle();
	virtual void* enter_state(studio::CanvasView* machine_context) const;
}; // END of class StateCircle

extern StateCircle state_circle;

}; // END of namespace studio

/* === E N D =============================================================== */

#endif
