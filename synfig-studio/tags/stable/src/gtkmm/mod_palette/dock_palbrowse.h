/* === S I N F G =========================================================== */
/*!	\file dialog_palette.h
**	\brief Template Header
**
**	$Id: dock_palbrowse.h,v 1.1.1.1 2005/01/07 03:34:37 darco Exp $
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

#ifndef __SINFG_STUDIO_DOCK_PAL_BROWSE_H
#define __SINFG_STUDIO_DOCK_PAL_BROWSE_H

/* === H E A D E R S ======================================================= */

#include "../dockable.h"
#include <sinfg/palette.h>

/* === M A C R O S ========================================================= */

/* === T Y P E D E F S ===================================================== */

/* === C L A S S E S & S T R U C T S ======================================= */

namespace sinfgapp {
class CanvasInterface;
};

namespace studio {

class Dock_PalBrowse : public Dockable
{
public:
	Dock_PalBrowse();
	~Dock_PalBrowse();
}; // END of Dock_PalBrowse

}; // END of namespace studio

/* === E N D =============================================================== */

#endif
