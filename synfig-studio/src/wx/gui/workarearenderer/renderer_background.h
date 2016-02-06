/* === S Y N F I G ========================================================= */
/*!	\file renderer_background.h
 **	\brief Renderer for the background of the canvas
 **
 **	$Id$
 **
 **	\legal
 **	Copyright (c) 2002-2005 Robert B. Quattlebaum Jr., Adrian Bentley
 **	Copyright (c) 2013 Carlos López
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

#ifndef __SYNFIG_RENDERER_BACKGROUND_H
#define __SYNFIG_RENDERER_BACKGROUND_H

/* === H E A D E R S ======================================================= */

#include "workarearenderer.h"

/* === M A C R O S ========================================================= */

/* === T Y P E D E F S ===================================================== */

/* === C L A S S E S & S T R U C T S ======================================= */

namespace studio {
	
class Renderer_Background : public studio::WorkAreaRenderer
{
private:
    Cairo::RefPtr<Cairo::Surface> draw_check_pattern(int width, int height);

public:
    ~Renderer_Background();
			
    void render_vfunc(const Glib::RefPtr<Gdk::Window>& drawable,const Gdk::Rectangle& expose_area	);
	
protected:
    bool get_enabled_vfunc()const;
};
	
}; // END of namespace studio

/* === E N D =============================================================== */

#endif
