/* === S I N F G =========================================================== */
/*!	\file dock_params.h
**	\brief Template Header
**
**	$Id: dock_params.h,v 1.1.1.1 2005/01/07 03:34:36 darco Exp $
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

#ifndef __SINFG_STUDIO_DOCK_PARAMS_H
#define __SINFG_STUDIO_DOCK_PARAMS_H

/* === H E A D E R S ======================================================= */

#include "dockable.h"
#include <gtkmm/treeview.h>
#include "instance.h"
#include "dock_canvasspecific.h"

/* === M A C R O S ========================================================= */

/* === T Y P E D E F S ===================================================== */

/* === C L A S S E S & S T R U C T S ======================================= */

namespace studio {

class Dock_Params : public Dock_CanvasSpecific
{
	Glib::RefPtr<Gtk::ActionGroup> action_group;
	
protected:
	virtual void init_canvas_view_vfunc(etl::loose_handle<CanvasView> canvas_view);
	virtual void changed_canvas_view_vfunc(etl::loose_handle<CanvasView> canvas_view);

	void refresh_selected_param();

public:


	Dock_Params();
	~Dock_Params();
}; // END of Dock_Keyframes

}; // END of namespace studio

/* === E N D =============================================================== */

#endif
