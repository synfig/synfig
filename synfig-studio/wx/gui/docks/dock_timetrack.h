/* === S Y N F I G ========================================================= */
/*!	\file docks/dock_timetrack.h
**	\brief Template Header
**
**	$Id$
**
**	\legal
**	Copyright (c) 2002-2005 Robert B. Quattlebaum Jr., Adrian Bentley
**  Copyright (c) 2009, 2010 Carlos López
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

#ifndef __SYNFIG_STUDIO_DOCK_TIMETRACK_H
#define __SYNFIG_STUDIO_DOCK_TIMETRACK_H

/* === H E A D E R S ======================================================= */

#include "docks/dockable.h"
#include <gtkmm/treeview.h>
#include <gtkmm/scrollbar.h>
#include "instance.h"
#include "docks/dock_canvasspecific.h"

/* === M A C R O S ========================================================= */

/* === T Y P E D E F S ===================================================== */

/* === C L A S S E S & S T R U C T S ======================================= */

namespace studio {
class Widget_Timeslider;
class Widget_Keyframe_List;

class Dock_Timetrack : public Dock_CanvasSpecific
{
	Gtk::HScrollbar* hscrollbar_;
	Gtk::VScrollbar* vscrollbar_;
	Widget_Timeslider* widget_timeslider_;
	Widget_Keyframe_List* widget_kf_list_;
	Gtk::Table* table_;
	Gtk::TreeView *mimic_tree_view;

protected:
	virtual void init_canvas_view_vfunc(etl::loose_handle<CanvasView> canvas_view);
	virtual void changed_canvas_view_vfunc(etl::loose_handle<CanvasView> canvas_view);

	void refresh_selected_param();
	//animation render description change signal handler
	void refresh_rend_desc();

public:


	Dock_Timetrack();
	~Dock_Timetrack();

private:
	//! Signal handler for studio::LayerTree::signal_param_tree_header_height_changed
	/* \see studio::LayerTree::signal_param_tree_header_height_changed */
	void on_update_header_height( int header_height);
}; // END of Dock_Timetrack

}; // END of namespace studio

/* === E N D =============================================================== */

#endif
