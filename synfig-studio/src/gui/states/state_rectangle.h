/* === S Y N F I G ========================================================= */
/*!	\file state_rectangle.h
**	\brief Rectangle tool state (header)
**
**	$Id$
**
**	\legal
**	Copyright (c) 2002-2005 Robert B. Quattlebaum Jr., Adrian Bentley
**	Copyright (c) 2016 caryoscelus
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

#ifndef __SYNFIG_STUDIO_STATE_RECTANGLE_H
#define __SYNFIG_STUDIO_STATE_RECTANGLE_H

/* === H E A D E R S ======================================================= */

#include "smach.h"
#include "state_shape.h"

/* === M A C R O S ========================================================= */

/* === T Y P E D E F S ===================================================== */

/* === C L A S S E S & S T R U C T S ======================================= */

namespace studio {

class StateRectangle_Context : public StateShape_Context
{
	// expansion
	Gtk::Label expand_label;
	Widget_Distance expand_dist;

public:

	synfig::Real get_expand_size() const {
		return expand_dist.get_value().get(
			synfig::Distance::SYSTEM_UNITS,
			get_canvas_view()->get_canvas()->rend_desc()
		);
	}
	void set_expand_size(synfig::Distance x) { return expand_dist.set_value(x);}

protected:
	virtual const synfig::String get_name_lower() const { return "rectangle"; }
	virtual const synfig::String get_name() const { return "Rectangle"; }
	virtual const synfig::String get_local_name() const { return _("Rectangle tool"); }

	virtual const Gdk::CursorType get_cursor() const { return Gdk::DOTBOX; }

	virtual void do_load_settings();
	virtual void do_save_settings();

public:
	virtual Smach::event_result event_mouse_click_handler(const Smach::event& x);

	void make_rectangle(const synfig::Point& p1, const synfig::Point& p2);

	virtual void toggle_layer_creation();

	virtual void enter();

	//constructor destructor
	StateRectangle_Context(CanvasView* canvas_view);
	virtual ~StateRectangle_Context();

};	// END of class StateGradient_Context

class StateRectangle : public StateShape<StateRectangle_Context>
{
public:
	StateRectangle();
	~StateRectangle();
}; // END of class StateRectangle

extern StateRectangle state_rectangle;

}; // END of namespace studio

/* === E N D =============================================================== */

#endif
