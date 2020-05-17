/* === S Y N F I G ========================================================= */
/*!	\file state_text.h
**	\brief Template Header
**
**	$Id$
**
**	\legal
**	Copyright (c) 2002-2005 Robert B. Quattlebaum Jr., Adrian Bentley
**  Copyright (c) 2008 Chris Moore
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

#ifndef __SYNFIG_STATE_TEXT_H
#define __SYNFIG_STATE_TEXT_H

/* === H E A D E R S ======================================================= */

#include "smach.h"


/* === M A C R O S ========================================================= */

/* === T Y P E D E F S ===================================================== */

/* === C L A S S E S & S T R U C T S ======================================= */

namespace studio {

class StateText_Context;

class StateText : public Smach::state<StateText_Context>
{
public:
	StateText();
	~StateText();
	virtual void* enter_state(studio::CanvasView* machine_context) const;
}; // END of class StateText

extern StateText state_text;

}; // END of namespace studio

/* === E N D =============================================================== */

#endif
