/* === S Y N F I G ========================================================= */
/*!	\file docks/dock_metadata.h
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

#ifndef __SYNFIG_STUDIO_DOCK_METADATA_H
#define __SYNFIG_STUDIO_DOCK_METADATA_H

/* === H E A D E R S ======================================================= */

#include <gui/docks/dock_canvasspecific.h>

/* === M A C R O S ========================================================= */

/* === T Y P E D E F S ===================================================== */

/* === C L A S S E S & S T R U C T S ======================================= */

namespace studio {

class CanvasView;
class Instance;

class Dock_MetaData : public Dock_CanvasSpecific
{
	Glib::RefPtr<Gtk::ActionGroup> action_group;

	void on_add_pressed();
	void on_delete_pressed();

protected:

	virtual void init_canvas_view_vfunc(std::shared_ptr<CanvasView> canvas_view);
	virtual void changed_canvas_view_vfunc(std::shared_ptr<CanvasView> canvas_view);

public:

	Dock_MetaData();
	~Dock_MetaData();
}; // END of Dock_MetaData

}; // END of namespace studio

/* === E N D =============================================================== */

#endif
