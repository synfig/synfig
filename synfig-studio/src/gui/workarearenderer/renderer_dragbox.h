/* === S Y N F I G ========================================================= */
/*!	\file renderer_dragbox.h
**	\brief Renderer_Dragbox class is used to display in the workarea
**  the interactive selection box, and select workarea objects (actually handles)
**  accordingly to the shift/control modifier keys.
**
**	\legal
**	Copyright (c) 2002-2005 Robert B. Quattlebaum Jr., Adrian Bentley
**  Copyright (c) 2015 Blanchi Jérôme
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

#ifndef __SYNFIG_RENDERER_DRAGBOX_H
#define __SYNFIG_RENDERER_DRAGBOX_H

/* === H E A D E R S ======================================================= */

#include "workarearenderer.h"
#include <gui/duckmatic.h>
#include <synfig/vector.h>
#include <synfig/guidset.h>


/* === M A C R O S ========================================================= */

/* === T Y P E D E F S ===================================================== */

/* === C L A S S E S & S T R U C T S ======================================= */

namespace studio {

class Renderer_Dragbox : public studio::WorkAreaRenderer
{

public:
	~Renderer_Dragbox();

	const synfig::Point& get_drag_point()const;
	const synfig::Point& get_curr_point()const;

private:
	//! Context of workarea objects for selection process
	DuckList handles_selected_;
    DuckList handles_all_;
    synfig::GUIDSet handles_selected_guid_;

	//! Used to catch a new drag box sequence
	bool drag_paused = false;

protected:
	bool get_enabled_vfunc()const;

	//! Redraw the drag box
	void render_vfunc(const Glib::RefPtr<Gdk::Window>& drawable,const Gdk::Rectangle& expose_area);
	//! Catch some mouse events to select objects (handles) in the workarea
	bool event_vfunc(GdkEvent* event);
};

}; // END of namespace studio

/* === E N D =============================================================== */

#endif
