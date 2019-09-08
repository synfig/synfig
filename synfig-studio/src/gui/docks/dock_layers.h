/* === S Y N F I G ========================================================= */
/*!	\file docks/dock_layers.h
**	\brief Template Header
**
**	$Id$
**
**	\legal
**	Copyright (c) 2002-2005 Robert B. Quattlebaum Jr., Adrian Bentley
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

#ifndef __SYNFIG_STUDIO_DOCK_LAYERS_H
#define __SYNFIG_STUDIO_DOCK_LAYERS_H

/* === H E A D E R S ======================================================= */

#include "docks/dockable.h"
#include <gtkmm/treeview.h>
#include "instance.h"
#include "docks/dock_canvasspecific.h"
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

	Glib::RefPtr<Gtk::Action> action_new_layer;

	Gtk::HScale *layer_amount_hscale;

	LayerActionManager* layer_action_manager;

protected:
	virtual void init_canvas_view_vfunc(etl::loose_handle<CanvasView> canvas_view);
	virtual void changed_canvas_view_vfunc(etl::loose_handle<CanvasView> canvas_view);

private:

	void add_layer(synfig::String id);
	void popup_add_layer_menu();

public:


	Dock_Layers();
	~Dock_Layers();
}; // END of Dock_Layers

}; // END of namespace studio

/* === E N D =============================================================== */

#endif
