/* === S Y N F I G ========================================================= */
/*!	\file target_null.h
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

#ifndef __SYNFIG_TARGET_NULL_H
#define __SYNFIG_TARGET_NULL_H

/* === H E A D E R S ======================================================= */

#include "target_scanline.h"

/* === M A C R O S ========================================================= */

/* === T Y P E D E F S ===================================================== */

/* === C L A S S E S & S T R U C T S ======================================= */

namespace synfig {

/*!	\class Target_Null
**	\brief A target which renders to nothing. Useful for benchmarks and other tests.
**	\todo writeme
*/
class Target_Null : public Target_Scanline
{
	std::vector<Color> buffer;

	Target_Null() { }

public:

	~Target_Null() { }

	bool start_frame(ProgressCallback* /*cb*/ = nullptr) override {
		buffer.resize(desc.get_w());
	    return true;
	}

	void end_frame() override {
		buffer.clear();
	    return;
	}

	Color* start_scanline(int /*scanline*/) override { return buffer.data(); }

	bool end_scanline() override { return true; }

	static Target* create(const synfig::filesystem::Path& /*filename*/, const synfig::TargetParam&) { return new Target_Null(); }
}; // END of class Target_Null

}; // END of namespace synfig

/* === E N D =============================================================== */

#endif
