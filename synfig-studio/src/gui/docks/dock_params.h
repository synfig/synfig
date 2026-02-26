/* === S Y N F I G ========================================================= */
/*!	\file docks/dock_params.h
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

#ifndef __SYNFIG_STUDIO_DOCK_PARAMS_H
#define __SYNFIG_STUDIO_DOCK_PARAMS_H

/* === H E A D E R S ======================================================= */

#include <gui/docks/dock_canvasspecific.h>

/* === M A C R O S ========================================================= */

/* === T Y P E D E F S ===================================================== */

/* === C L A S S E S & S T R U C T S ======================================= */

namespace studio {

class Dock_Params : public Dock_CanvasSpecific
{
	Glib::RefPtr<Gtk::Adjustment> vadjustment;
	sigc::connection refresh_selected_param_connection;
	sigc::connection canvas_changed_connection;

protected:
	virtual void init_canvas_view_vfunc(etl::loose_handle<CanvasView> canvas_view);
	virtual void changed_canvas_view_vfunc(etl::loose_handle<CanvasView> canvas_view);

	void refresh_tree_view();

	void refresh_selected_param();

public:
	Dock_Params();
	~Dock_Params();
}; // END of Dock_Keyframes

}; // END of namespace studio

/* === E N D =============================================================== */

#endif
