/* === S I N F G =========================================================== */
/*!	\file dock_layers.h
**	\brief Template Header
**
**	$Id: dock_layers.h,v 1.1.1.1 2005/01/07 03:34:36 darco Exp $
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

#ifndef __SINFG_STUDIO_DOCK_LAYERS_H
#define __SINFG_STUDIO_DOCK_LAYERS_H

/* === H E A D E R S ======================================================= */

#include "dockable.h"
#include <gtkmm/treeview.h>
#include "instance.h"
#include "dock_canvasspecific.h"
#include <gtkmm/actiongroup.h>
#include <list>

/* === M A C R O S ========================================================= */

/* === T Y P E D E F S ===================================================== */

/* === C L A S S E S & S T R U C T S ======================================= */

namespace Gtk { class HScale; }

namespace studio {

class LayerActionManager;

class Dock_Layers : public Dock_CanvasSpecific
{	
	Glib::RefPtr<Gtk::ActionGroup> action_group_new_layers;
	Glib::RefPtr<Gtk::ActionGroup> action_group_layer_ops;
	
	Gtk::HScale *layer_amount_hscale;

	LayerActionManager* layer_action_manager;
	
protected:
	virtual void init_canvas_view_vfunc(etl::loose_handle<CanvasView> canvas_view);
	virtual void changed_canvas_view_vfunc(etl::loose_handle<CanvasView> canvas_view);

private:

	void add_layer(sinfg::String id);
	void increase_amount();
	void decrease_amount();

public:


	Dock_Layers();
	~Dock_Layers();
}; // END of Dock_Layers

}; // END of namespace studio

/* === E N D =============================================================== */

#endif
