/* === S I N F G =========================================================== */
/*!	\file mod_palette.h
**	\brief Template Header
**
**	$Id: mod_palette.h,v 1.1.1.1 2005/01/07 03:34:37 darco Exp $
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

#ifndef __SINFG_MOD_PALETTE_H
#define __SINFG_MOD_PALETTE_H

/* === H E A D E R S ======================================================= */

#include <ETL/handle>
#include "../module.h"

/* === M A C R O S ========================================================= */

/* === T Y P E D E F S ===================================================== */

/* === C L A S S E S & S T R U C T S ======================================= */

namespace studio {

class Dock_PalEdit;
class Dock_PalBrowse;
	
class ModPalette : public Module
{
	friend class Dock_PalEdit;
	friend class Dock_PalBrowse;
		
	Dock_PalEdit*	dock_pal_edit;
	Dock_PalBrowse*	dock_pal_browse;
	
protected:
	virtual bool start_vfunc();
	virtual bool stop_vfunc();

public:
	virtual ~ModPalette() { stop(); }
};
	
}; // END of namespace studio

/* === E N D =============================================================== */

#endif
