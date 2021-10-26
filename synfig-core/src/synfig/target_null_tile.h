/* === S Y N F I G ========================================================= */
/*!	\file target_null_tile.h
**	\brief Template Header
**
**	\legal
**	Copyright (c) 2002-2005 Robert B. Quattlebaum Jr., Adrian Bentley
**
**	This file is part of Synfig.
**
**	Synfig is free software: you can redistribute it and/or modify
**	it under the terms of the GNU General Public License as published by
**	the Free Software Foundation, either version 2 of the License, or
**	(at your option) any later version.
**
**	Synfig is distributed in the hope that it will be useful,
**	but WITHOUT ANY WARRANTY; without even the implied warranty of
**	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
**	GNU General Public License for more details.
**
**	You should have received a copy of the GNU General Public License
**	along with Synfig.  If not, see <https://www.gnu.org/licenses/>.
**	\endlegal
*/
/* ========================================================================= */

/* === S T A R T =========================================================== */

#ifndef __SYNFIG_TARGET_NULL_TILE_H
#define __SYNFIG_TARGET_NULL_TILE_H

/* === H E A D E R S ======================================================= */

#include "target_tile.h"

/* === M A C R O S ========================================================= */

/* === T Y P E D E F S ===================================================== */

/* === C L A S S E S & S T R U C T S ======================================= */

namespace synfig {

/*!	\class Target_Null_Tile
**	\brief A target which renders to nothing using tiles. Useful for benchmarks and other tests.
**	\todo writeme
*/
class Target_Null_Tile : public Target_Tile
{
	Target_Null_Tile() { }

public:

	~Target_Null_Tile() {  }
	virtual bool add_tile(const synfig::Surface &/*surface*/, int /*x*/, int /*y*/) { return true; }

	virtual bool start_frame(ProgressCallback */*cb*/=NULL)
		{ return true; }

	virtual void end_frame() { return; }

	static Target* create(const char */*filename*/=0) { return new Target_Null_Tile(); }
}; // END of class Target_Null_Tile

}; // END of namespace synfig

/* === E N D =============================================================== */

#endif
