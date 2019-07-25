/* === S Y N F I G ========================================================= */
/*!	\file docks/dock_keyframes.h
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

#ifndef __SYNFIG_STUDIO_DOCK_KEYFRAMES_H
#define __SYNFIG_STUDIO_DOCK_KEYFRAMES_H

/* === H E A D E R S ======================================================= */

#include "docks/dockable.h"
#include "docks/dock_canvasspecific.h"
#include <gtkmm/treeview.h>
#include "instance.h"
#include <gtkmm/actiongroup.h>

/* === M A C R O S ========================================================= */

/* === T Y P E D E F S ===================================================== */

/* === C L A S S E S & S T R U C T S ======================================= */

namespace studio {

class KeyframeTreeStore;
class KeyframeTree;

class KeyframeActionManager;

/*! \class Dock_Keyframes
**	\brief A Dockable dialog who hold keyframe list
*/
class Dock_Keyframes : public Dock_CanvasSpecific
{
private:
	//The manager of keyframes actions
	KeyframeActionManager* keyframe_action_manager;

	void show_keyframe_properties();
	void keyframe_toggle();
	void keyframe_description_set();

	//animation render description change signal handler
	void refresh_rend_desc();

protected:
	virtual void init_canvas_view_vfunc(etl::loose_handle<CanvasView> canvas_view);
	virtual void changed_canvas_view_vfunc(etl::loose_handle<CanvasView> canvas_view);

public:
	Dock_Keyframes();
	~Dock_Keyframes();
}; // END of Dock_Keyframes

}; // END of namespace studio

/* === E N D =============================================================== */

#endif
