/* === S Y N F I G ========================================================= */
/*!	\file savecanvas.h
**	\brief Implementation for the Synfig Save Canvas to file
**
**	$Id$
**
**	\legal
**	Copyright (c) 2002-2005 Robert B. Quattlebaum Jr., Adrian Bentley
**	Copyright (c) 2008 Chris Moore
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

#ifndef __SYNFIG_SAVECANVAS_H
#define __SYNFIG_SAVECANVAS_H

/* === H E A D E R S ======================================================= */

#include <list>
#include "string.h"
#include "canvas.h"
#include "releases.h"
#include "layer.h"

/* === M A C R O S ========================================================= */

/* === T Y P E D E F S ===================================================== */

/* === C L A S S E S & S T R U C T S ======================================= */

namespace synfig {

/* === E X T E R N S ======================================================= */

typedef bool (*save_canvas_external_file_callback_t)(void *user_data, Layer::ConstHandle layer, const std::string &param_name, std::string &filename);


//!	Saves a canvas to \a filename
/*!	\return	\c true on success, \c false on error. */
bool save_canvas(const FileSystem::Identifier &identifier, Canvas::ConstHandle canvas, bool safe = true);

//! Stores a Canvas in a string in XML format
/*! \return The string with the XML canvas definition */
String canvas_to_string(Canvas::ConstHandle canvas);

void set_save_canvas_external_file_callback(save_canvas_external_file_callback_t callback, void *user_data);

void set_file_version(ReleaseVersion version);
ReleaseVersion get_file_version();

}; // END of namespace synfig

/* === E N D =============================================================== */

#endif
