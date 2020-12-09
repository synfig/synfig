/* === S Y N F I G ========================================================= */
/*!	\file docks/dock_palbrowse.h
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

#ifndef __SYNFIG_STUDIO_DOCK_PAL_BROWSE_H
#define __SYNFIG_STUDIO_DOCK_PAL_BROWSE_H

/* === H E A D E R S ======================================================= */

#include <gui/docks/dockable.h>

/* === M A C R O S ========================================================= */

/* === T Y P E D E F S ===================================================== */

/* === C L A S S E S & S T R U C T S ======================================= */

namespace synfigapp {
class CanvasInterface;
};

namespace studio {

/*

The palette browser was intended to be a way to manage and select a single
palette from a set of palettes that you could save to files. The palette
editor was for editing individual palettes. Unfortunately the palette
browser was never implemented.

*/

class Dock_PalBrowse : public Dockable
{
public:
	Dock_PalBrowse();
	~Dock_PalBrowse();
}; // END of Dock_PalBrowse

}; // END of namespace studio

/* === E N D =============================================================== */

#endif
