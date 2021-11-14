/* === S Y N F I G ========================================================= */
/*!	\file docks/dock_curves.h
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

#ifndef __SYNFIG_STUDIO_DOCK_CURVES_H
#define __SYNFIG_STUDIO_DOCK_CURVES_H

/* === H E A D E R S ======================================================= */

#include <gtkmm/scrollbar.h>
#include <gtkmm/table.h>

#include <gui/docks/dock_canvasspecific.h>
#include <gui/widgets/widget_canvastimeslider.h>
#include <gui/widgets/widget_keyframe_list.h>

/* === M A C R O S ========================================================= */

/* === T Y P E D E F S ===================================================== */

/* === C L A S S E S & S T R U C T S ======================================= */

namespace studio {

class Widget_Curves;

class Dock_Curves : public Dock_CanvasSpecific
{
	Gtk::Scrollbar hscrollbar_;
	Gtk::Scrollbar vscrollbar_;

	Widget_CanvasTimeslider widget_timeslider_;
	Widget_Keyframe_List widget_kf_list_;
	
	Gtk::Table* table_;
	Widget_Curves* last_widget_curves_;

protected:
	virtual void init_canvas_view_vfunc(etl::loose_handle<CanvasView> canvas_view);
	virtual void changed_canvas_view_vfunc(etl::loose_handle<CanvasView> canvas_view);

	void refresh_selected_param();

public:


	Dock_Curves();
	~Dock_Curves();

private:
	//! Signal handler for studio::LayerTree::signal_param_tree_header_height_changed
	/* \see studio::LayerTree::signal_param_tree_header_height_changed */
	void on_update_header_height(int height);

	void on_curves_waypoint_clicked(synfigapp::ValueDesc value_desc, std::set<synfig::Waypoint,std::less<synfig::UniqueID>> waypoint_set, int button);
	void on_curves_waypoint_double_clicked(synfigapp::ValueDesc value_desc, std::set<synfig::Waypoint,std::less<synfig::UniqueID>> waypoint_set, int button);
}; // END of Dock_Curves

}; // END of namespace studio

/* === E N D =============================================================== */

#endif
