/* === S Y N F I G ========================================================= */
/*!	\file state_select.h
**	\brief Template Header
**
**	\legal
**	Copyright (c) 2002-2005 Robert B. Quattlebaum Jr., Adrian Bentley
**	Copyright (c) 2009 Nikita Kitaev
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

#ifndef __SYNFIG_STUDIO_STATE_SELECT_H
#define __SYNFIG_STUDIO_STATE_SELECT_H

/* === H E A D E R S ======================================================= */

#include "smach.h"
#include <gui/states/state_normal.h>

/* === M A C R O S ========================================================= */

/* === T Y P E D E F S ===================================================== */

/* === C L A S S E S & S T R U C T S ======================================= */

namespace studio {

class StateSelect_Context;

class StateSelect : public Smach::state<StateSelect_Context>
{
public:
	StateSelect();
	~StateSelect();
	virtual void* enter_state(studio::CanvasView* machine_context) const;
}; // END of class StateSelect

extern StateSelect state_select;

class DuckDrag_Select : public DuckDrag_Combo
{
public:
	DuckDrag_Select();
	void begin_duck_drag(Duckmatic* duckmatic, const synfig::Vector& begin);
	bool end_duck_drag(Duckmatic* duckmatic);
	void duck_drag(Duckmatic* duckmatic, const synfig::Vector& vector);
};

}; // END of namespace studio

/* === E N D =============================================================== */

#endif
