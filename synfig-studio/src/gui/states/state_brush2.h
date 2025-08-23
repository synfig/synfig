/* === S Y N F I G ========================================================= */
/*!	\file state_brush2.h
**	\brief Template Header
**
**	\legal
**	Copyright (c) 2025 Abdelhadi Wael
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

/* === S T A R T =========================================================== */

#ifndef SYNFIG_STUDIO_STATE_BRUSH2_H
#define SYNFIG_STUDIO_STATE_BRUSH2_H

/* === H E A D E R S ======================================================= */

#include "smach.h"

/* === M A C R O S ========================================================= */

/* === T Y P E D E F S ===================================================== */

/* === C L A S S E S & S T R U C T S ======================================= */

namespace studio {

	class StateBrush2_Context;

	class StateBrush2 : public Smach::state<StateBrush2_Context>
	{
	public:
		StateBrush2();
		~StateBrush2();
		void* enter_state(studio::CanvasView* machine_context) const override;
	}; // END of class StateBrush2

	extern StateBrush2 state_brush2;

}; // END of namespace studio

/* === E N D =============================================================== */

#endif
