/* === S Y N F I G ========================================================= */
/*!	\file savecanvas.h
**	\brief writeme
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

#ifndef __SYNFIG_SAVECANVAS_H
#define __SYNFIG_SAVECANVAS_H

/* === H E A D E R S ======================================================= */

#include "string.h"
#include "canvas.h"

/* === M A C R O S ========================================================= */

/* === T Y P E D E F S ===================================================== */

/* === C L A S S E S & S T R U C T S ======================================= */

namespace synfig {

/* === E X T E R N S ======================================================= */

typedef enum
{
	FILE_VERSION_0_61_08,
	FILE_VERSION_0_61_07,
} FileVersion;

//!	Saves a canvas to \a filename
/*!	\return	\c true on success, \c false on error. */
bool save_canvas(const String &filename, Canvas::ConstHandle canvas);

//! Stores a Canvas in a string in XML format
String canvas_to_string(Canvas::ConstHandle canvas);

void set_file_version(FileVersion version);
FileVersion get_file_version();

}; // END of namespace synfig

/* === E N D =============================================================== */

#endif
