/* === S I N F G =========================================================== */
/*!	\file template.h
**	\brief Template Header
**
**	$Id: renderer_canvas.h,v 1.1.1.1 2005/01/07 03:34:36 darco Exp $
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

#ifndef __SINFG_RENDERER_CANVAS_H
#define __SINFG_RENDERER_CANVAS_H

/* === H E A D E R S ======================================================= */

#include "workarearenderer.h"
#include <vector>

/* === M A C R O S ========================================================= */

/* === T Y P E D E F S ===================================================== */

/* === C L A S S E S & S T R U C T S ======================================= */

namespace studio {

class Renderer_Canvas : public studio::WorkAreaRenderer
{
	
public:
	~Renderer_Canvas();

	std::vector< std::pair<Glib::RefPtr<Gdk::Pixbuf>,int> >& get_tile_book();

	bool get_full_frame()const;
	
	int get_refreshes()const;
	bool get_canceled()const;
	bool get_queued()const;
	bool get_rendering()const;

	void render_vfunc(const Glib::RefPtr<Gdk::Drawable>& drawable,const Gdk::Rectangle& expose_area	);
};

}; // END of namespace studio

/* === E N D =============================================================== */

#endif
