/* === S I N F G =========================================================== */
/*!	\file dock_layergroups.h
**	\brief Template Header
**
**	$Id: dock_layergroups.h,v 1.1.1.1 2005/01/07 03:34:36 darco Exp $
**
**	\legal
**	Copyright (c) 2002 Robert B. Quattlebaum Jr.
**
**	This software and associated documentation
**	are CONFIDENTIAL and PROPRIETARY property of
**	the above-mentioned copyright holder.
**
**	You may not copy, print, publish, or in any
**	other way distribute this software without
**	a prior written agreement with
**	the copyright holder.
**	\endlegal
*/
/* ========================================================================= */

/* === S T A R T =========================================================== */

#ifndef __SINFG_STUDIO_DOCK_LAYERGROUPS_H
#define __SINFG_STUDIO_DOCK_LAYERGROUPS_H

/* === H E A D E R S ======================================================= */

#include "dockable.h"
#include <gtkmm/treeview.h>
#include "instance.h"
#include "dock_canvasspecific.h"
#include <gtkmm/actiongroup.h>

/* === M A C R O S ========================================================= */

/* === T Y P E D E F S ===================================================== */

/* === C L A S S E S & S T R U C T S ======================================= */

namespace Gtk { class HScale; }

namespace studio {

class GroupActionManager ;
	
class Dock_LayerGroups : public Dock_CanvasSpecific
{	
	Glib::RefPtr<Gtk::ActionGroup> action_group_group_ops;
	Glib::RefPtr<Gtk::Action> action_group_add;

	GroupActionManager *group_action_manager;
protected:
	virtual void init_canvas_view_vfunc(etl::loose_handle<CanvasView> canvas_view);
	virtual void changed_canvas_view_vfunc(etl::loose_handle<CanvasView> canvas_view);

private:

public:


	Dock_LayerGroups();
	~Dock_LayerGroups();
}; // END of Dock_LayerGroups

}; // END of namespace studio

/* === E N D =============================================================== */

#endif
