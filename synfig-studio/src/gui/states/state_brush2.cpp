/* === S Y N F I G ========================================================= */
/*!	\file state_brush2.cpp
**	\brief Template File
**
**	\legal
**	......... ... 2014 Ivan Mahonin
**	......... ... 2014 Jerome Blanchi
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

/* === H E A D E R S ======================================================= */

#ifdef USING_PCH
#	include "pch.h"
#else
#ifdef HAVE_CONFIG_H
#	include <config.h>
#endif

// Own header
#include "state_brush2.h"

// Includes needed for the minimal implementation
#include "state_normal.h" // To switch back to the normal tool
#include <gui/canvasview.h>
#include <gui/event_mouse.h>
#include <gui/workarea.h>
#include <synfig/rendering/surface.h>
#include <synfigapp/main.h> // For settings, etc. in the future
#include <gui/localization.h>

#endif

/* === U S I N G =========================================================== */

using namespace studio;
using namespace synfig;

/* === M A C R O S ========================================================= */

/* === G L O B A L S ======================================================= */

StateBrush2 studio::state_brush2;

/* === C L A S S E S & S T R U C T S ======================================= */

/**
 * The StateBrush2_Context class holds all the data and logic for the tool
 * while it is active on a canvas. An instance of this class is created
 * when the user selects the tool.
 */
class studio::StateBrush2_Context : public sigc::trackable
{
private:
	CanvasView::Handle canvas_view_;
	WorkArea::PushState push_state;

	// for the overlay system
	synfig::Surface overlay_surface_;
	bool is_drawing_;

public:
	// Constructor and Destructor
	StateBrush2_Context(CanvasView* canvas_view);
	~StateBrush2_Context();

	// Event Handlers
	Smach::event_result event_mouse_down_handler(const Smach::event& x);
	Smach::event_result event_mouse_up_handler(const Smach::event& x);
	Smach::event_result event_mouse_draw_handler(const Smach::event& x);
	Smach::event_result event_stop_handler(const Smach::event& x);

	WorkArea* get_work_area() const { return canvas_view_->get_work_area(); }
};

/* === M E T H O D S ======================================================= */

StateBrush2::StateBrush2() :
	Smach::state<StateBrush2_Context>("brush2", N_("Brush Tool 2"))
{
	// Connect the application events to their handler methods in the context class.
	insert(event_def(EVENT_WORKAREA_MOUSE_BUTTON_DOWN, &StateBrush2_Context::event_mouse_down_handler));
	insert(event_def(EVENT_WORKAREA_MOUSE_BUTTON_UP,   &StateBrush2_Context::event_mouse_up_handler));
	insert(event_def(EVENT_WORKAREA_MOUSE_BUTTON_DRAG, &StateBrush2_Context::event_mouse_draw_handler));
	insert(event_def(EVENT_STOP,                       &StateBrush2_Context::event_stop_handler));
}

StateBrush2::~StateBrush2() {}

void* StateBrush2::enter_state(studio::CanvasView* machine_context) const
{
	return new StateBrush2_Context(machine_context);
}

StateBrush2_Context::StateBrush2_Context(CanvasView* canvas_view) :
	canvas_view_(canvas_view),
	push_state(*get_work_area()),
	is_drawing_(false)
{

}

StateBrush2_Context::~StateBrush2_Context()
{

}

Smach::event_result StateBrush2_Context::event_mouse_down_handler(const Smach::event& x)
{
	is_drawing_ = true;
	return Smach::RESULT_ACCEPT;
}

Smach::event_result StateBrush2_Context::event_mouse_up_handler(const Smach::event& x)
{
	if (!is_drawing_) return Smach::RESULT_OK;
	is_drawing_ = false;
	return Smach::RESULT_ACCEPT;
}

Smach::event_result StateBrush2_Context::event_mouse_draw_handler(const Smach::event& x)
{
	if (!is_drawing_) return Smach::RESULT_OK;
	return Smach::RESULT_ACCEPT;
}

Smach::event_result StateBrush2_Context::event_stop_handler(const Smach::event& x)
{
	throw &state_normal;
	return Smach::RESULT_OK;
}
