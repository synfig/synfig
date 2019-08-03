/* === S Y N F I G ========================================================= */
/*!	\file waypointrenderer.h
**
**	$Id$
**
**	\legal
**	Copyright (c) 2002-2005 Robert B. Quattlebaum Jr., Adrian Bentley
**	......... ... 2019 Rodolfo R. Gomes
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

#ifndef __SYNFIG_STUDIO_WAYPOINTRENDERER_H
#define __SYNFIG_STUDIO_WAYPOINTRENDERER_H

/* === H E A D E R S ======================================================= */
#include <gdkmm/rectangle.h>
#include <cairomm/context.h>

#include <synfig/node.h>
/* === M A C R O S ========================================================= */

/* === T Y P E D E F S ===================================================== */

/* === C L A S S E S & S T R U C T S ======================================= */

namespace studio {

class WaypointRenderer
{
public:
	static void
	render_time_point_to_window(
		const Cairo::RefPtr<Cairo::Context> &cr,
		const Gdk::Rectangle& area,
		const synfig::TimePoint &tp,
		bool selected );
}; // END of class WaypointRenderer

}; // END of namespace studio

/* === E N D =============================================================== */

#endif // WAYPOINTRENDERER_H
