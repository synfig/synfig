/* === S Y N F I G ========================================================= */
/*!	\file target_null.h
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
	Color *buffer;

	Target_Null():buffer(nullptr) { }

public:

	~Target_Null() { delete[] buffer; }

	virtual bool start_frame(ProgressCallback */*cb*/=NULL) {
	    if (buffer) delete[] buffer;
	    buffer = new Color[desc.get_w()*sizeof(Color)];
	    return true;
	}

	virtual void end_frame() {
	    if (buffer) delete[] buffer;
	    buffer = nullptr;
	    return;
	}

	virtual Color * start_scanline(int /*scanline*/) { return buffer; }

	virtual bool end_scanline() { return true; }

	static Target* create(const char */*filename*/=0) { return new Target_Null(); }
}; // END of class Target_Null

}; // END of namespace synfig

/* === E N D =============================================================== */

#endif
