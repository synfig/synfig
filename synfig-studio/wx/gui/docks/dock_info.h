/* === S Y N F I G ========================================================= */
/*!	\file docks/dock_info.h
**	\brief Info Dock Header
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

#ifndef __SYNFIG_DOCK_INFO_H
#define __SYNFIG_DOCK_INFO_H

/* === H E A D E R S ======================================================= */
#include "docks/dock_canvasspecific.h"
#include "sigc++/signal.h"

#include "widgets/widget_distance.h"

/* === M A C R O S ========================================================= */

/* === T Y P E D E F S ===================================================== */

/* === C L A S S E S & S T R U C T S ======================================= */

namespace studio {

class Dock_Info : public Dock_CanvasSpecific
{
	Gtk::Label  r,g,b,a;
	Gtk::Label	x,y;

	sigc::connection mousecon;

	void on_mouse_move();

public:
	Dock_Info();
	~Dock_Info();

	virtual void changed_canvas_view_vfunc(etl::loose_handle<CanvasView> canvas_view);
};

}; // END of namespace studio

/* === E N D =============================================================== */

#endif
