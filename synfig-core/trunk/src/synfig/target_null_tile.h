/* === S I N F G =========================================================== */
/*!	\file target_null_tile.h
**	\brief Template Header
**
**	$Id: target_null_tile.h,v 1.1.1.1 2005/01/04 01:23:15 darco Exp $
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

#ifndef __SINFG_TARGET_NULL_TILE_H
#define __SINFG_TARGET_NULL_TILE_H

/* === H E A D E R S ======================================================= */

#include "target_tile.h"
#include "general.h"

/* === M A C R O S ========================================================= */

/* === T Y P E D E F S ===================================================== */

/* === C L A S S E S & S T R U C T S ======================================= */

namespace sinfg {

/*!	\class Target_Null_Tile
**	\brief A target which renders to nothing using tiles. Useful for benchmarks and other tests.
**	\todo writeme
*/
class Target_Null_Tile : public Target_Tile
{
	Target_Null_Tile() { }
	
public:

	~Target_Null_Tile() {  } 
	virtual bool add_tile(const sinfg::Surface &surface, int x, int y) { return true; }

	virtual bool start_frame(ProgressCallback *cb=NULL)
		{ return true; }

	virtual void end_frame() { return; }
	
	static Target* create(const char *filename=0) { return new Target_Null_Tile(); }
}; // END of class Target_Null_Tile

}; // END of namespace sinfg

/* === E N D =============================================================== */

#endif
