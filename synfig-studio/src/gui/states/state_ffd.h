/* === S Y N F I G ========================================================= */
/*!	\file state_ffd.h
**	\brief FFD Tool Header
**
**	\legal
**	Copyright (c) 2026 Ahmed Fathy
**
**	This file is part of Synfig.
**
**	Synfig is free software: you can redistribute it and/or modify
**	it under the terms of the GNU General Public License as published by
**	the Free Software Foundation, either version 2 of the License, or
**	(at your option) any later version.
**
**	Synfig is distributed in the hope that it will be useful,
**	but WITHOUT ANY WARRANTY; without even the implied warranty of
**	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
**	GNU General Public License for more details.
**
**	You should have received a copy of the GNU General Public License
**	along with Synfig.  If not, see <https://www.gnu.org/licenses/>.
**	\endlegal
*/
/* ========================================================================= */

#ifndef SYNFIG_STUDIO_STATE_FFD_H
#define SYNFIG_STUDIO_STATE_FFD_H

/* === H E A D E R S ======================================================= */

#include "smach.h"

/* === C L A S S E S & S T R U C T S ======================================= */

namespace studio {

class StateFFD_Context;

class StateFFD : public Smach::state<StateFFD_Context>
{
public:
	StateFFD();
	virtual void* enter_state(studio::CanvasView* machine_context) const;
}; // END of class StateFFD

extern StateFFD state_ffd;

} // END of namespace studio

/* === E N D =============================================================== */

#endif
