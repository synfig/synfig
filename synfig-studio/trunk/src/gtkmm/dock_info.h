/* === S Y N F I G ========================================================= */
/*!	\file dock_info.h
**	\brief Info Dock Header
**
**	$Id: dock_info.h,v 1.1.1.1 2005/01/07 03:34:36 darco Exp $
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

#ifndef __SYNFIG_DOCK_INFO_H
#define __SYNFIG_DOCK_INFO_H

/* === H E A D E R S ======================================================= */
#include "dock_canvasspecific.h"
#include "sigc++/signal.h"

#include "widget_distance.h"

/* === M A C R O S ========================================================= */

/* === T Y P E D E F S ===================================================== */

/* === C L A S S E S & S T R U C T S ======================================= */

namespace studio {

class Dock_Info : public Dock_CanvasSpecific
{
	//bool			valid;
	//synfig::Point	pos;
	
	Gtk::Label  r,g,b,a;
	Gtk::Label	x,y;
	
	SigC::Connection mousecon;	
	
	void on_mouse_move();	
		
public:
	Dock_Info();
	~Dock_Info();

	virtual void changed_canvas_view_vfunc(etl::loose_handle<CanvasView> canvas_view);
};
	
}; // END of namespace studio

/* === E N D =============================================================== */

#endif
