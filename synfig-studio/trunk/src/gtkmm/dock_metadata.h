/* === S Y N F I G ========================================================= */
/*!	\file dock_metadata.h
**	\brief Template Header
**
**	$Id: dock_metadata.h,v 1.1.1.1 2005/01/07 03:34:36 darco Exp $
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

#ifndef __SYNFIG_STUDIO_DOCK_METADATA_H
#define __SYNFIG_STUDIO_DOCK_METADATA_H

/* === H E A D E R S ======================================================= */

#include "dockable.h"
#include <gtkmm/treeview.h>
#include "instance.h"
#include "dock_canvasspecific.h"

/* === M A C R O S ========================================================= */

/* === T Y P E D E F S ===================================================== */

/* === C L A S S E S & S T R U C T S ======================================= */

namespace studio {

class CanvasView;
class Instance;
	
class Dock_MetaData : public Dock_CanvasSpecific
{	

	Gtk::TreeView *tree_view;
	

	void on_add_pressed();
	void on_delete_pressed();

protected:
	
	virtual void init_canvas_view_vfunc(etl::loose_handle<CanvasView> canvas_view);
	virtual void changed_canvas_view_vfunc(etl::loose_handle<CanvasView> canvas_view);
	
public:


	Dock_MetaData();
	~Dock_MetaData();
}; // END of Dock_MetaData

}; // END of namespace studio

/* === E N D =============================================================== */

#endif
