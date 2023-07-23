/* === S Y N F I G ========================================================= */
/*!	\file state_normal.h
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

#ifndef __SYNFIG_STUDIO_STATE_NORMAL_H
#define __SYNFIG_STUDIO_STATE_NORMAL_H

/* === H E A D E R S ======================================================= */

#include "smach.h"
#include <gui/workarea.h>
#include <gui/canvasview.h>

/* === M A C R O S ========================================================= */

/* === T Y P E D E F S ===================================================== */

/* === C L A S S E S & S T R U C T S ======================================= */

namespace studio {

class StateNormal_Context;
class DuckDrag_Base;

class StateNormal : public Smach::state<StateNormal_Context>
{
public:
	StateNormal();
	~StateNormal();
	virtual void* enter_state(studio::CanvasView* machine_context) const;
}; // END of class StateNormal

extern StateNormal state_normal;

//better to just have this class in a separate or other file
class DuckDrag_Combo : public DuckDrag_Base
{
	synfig::Vector last_move;
	synfig::Vector drag_offset;
	synfig::Vector center;
	synfig::Vector snap;

	synfig::Angle original_angle;
	synfig::Real original_mag;

	std::vector<synfig::Vector> last_;
	std::vector<synfig::Vector> positions;
	std::set<Duck::Handle> selectedDucks;

	bool duck_independent_move;

	bool bad_drag;
	bool move_only;

	bool is_moving;

	DuckList get_selected_ducks(const Duckmatic& duckmatic) const;
	void select_all_ducks(const Duckmatic& duckmatic);

public:
	CanvasView* canvas_view_;
	bool scale;
	bool rotate;
	bool constrain;
	DuckDrag_Combo(bool duck_independent_drag = false);
	void begin_duck_drag(Duckmatic* duckmatic, const synfig::Vector& begin);
	bool end_duck_drag(Duckmatic* duckmatic);
	void duck_drag(Duckmatic* duckmatic, const synfig::Vector& vector);

	etl::handle<synfigapp::CanvasInterface> get_canvas_interface()const{return canvas_view_->canvas_interface();}
};

}; // END of namespace studio

/* === E N D =============================================================== */

#endif
