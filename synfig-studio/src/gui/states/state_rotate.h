/* === S Y N F I G ========================================================= */
/*!	\file state_rotate.h
**	\brief Template Header
**
**	\legal
**	Copyright (c) 2002-2005 Robert B. Quattlebaum Jr., Adrian Bentley
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

#ifndef __SYNFIG_STUDIO_STATE_ROTATE_H
#define __SYNFIG_STUDIO_STATE_ROTATE_H

/* === H E A D E R S ======================================================= */

#include "smach.h"
#include <gui/workarea.h>
#include <gui/canvasview.h>
#include <synfig/angle.h>


/* === M A C R O S ========================================================= */

/* === T Y P E D E F S ===================================================== */

/* === C L A S S E S & S T R U C T S ======================================= */

namespace studio {

class StateRotate_Context;

class StateRotate : public Smach::state<StateRotate_Context>
{
public:
	StateRotate();
	~StateRotate();
	virtual void* enter_state(studio::CanvasView* machine_context) const;
}; // END of class StateRotate

extern StateRotate state_rotate;

class DuckDrag_Rotate : public DuckDrag_Base
{
	friend class DuckDrag_NonVertex_Rotate;

	synfig::Vector last_rotate;
	synfig::Vector drag_offset;
	synfig::Vector center;
	synfig::Vector snap;

	synfig::Angle original_angle;
	synfig::Real original_mag;

	std::vector<synfig::Vector> positions;


	bool bad_drag;
	bool move_only;

public:
	etl::handle<CanvasView> canvas_view_;
	bool use_magnitude;
	DuckDrag_Rotate();
	void begin_duck_drag(Duckmatic* duckmatic, const synfig::Vector& begin);
	bool end_duck_drag(Duckmatic* duckmatic);
	void duck_drag(Duckmatic* duckmatic, const synfig::Vector& vector);

	etl::handle<synfigapp::CanvasInterface> get_canvas_interface()const{return canvas_view_->canvas_interface();}
};


}; // END of namespace studio

/* === E N D =============================================================== */

#endif
