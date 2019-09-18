/* === S Y N F I G ========================================================= */
/*!	\file debugsurface.h
**	\brief Template Header
**
**	$Id$
**
**	\legal
**	......... ... 2015 Ivan Mahonin
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

#ifndef __SYNFIG_DEBUGSURFACE_H
#define __SYNFIG_DEBUGSURFACE_H

/* === H E A D E R S ======================================================= */

#include <ETL/handle>
#include <synfig/string.h>

/* === M A C R O S ========================================================= */

/* === T Y P E D E F S ===================================================== */

/* === C L A S S E S & S T R U C T S ======================================= */


namespace synfig { class Surface; }
namespace synfig { namespace rendering { class Surface; } }
namespace synfig { namespace rendering { class SurfaceResource; } }

namespace synfig {
namespace debug {

class DebugSurface
{
public:
	static void save_to_file(const void *buffer, int width, int height, int stride, const String &filename, bool overwrite = false);
	static void save_to_file(const Surface &surface, const String &filename, bool overwrite = false);
	static void save_to_file(const rendering::Surface &surface, const String &filename, bool overwrite = false);
	static void save_to_file(const etl::handle<rendering::Surface> &surface, const String &filename, bool overwrite = false);
	static void save_to_file(const etl::handle<rendering::SurfaceResource> &surface, const String &filename, bool overwrite = false);
};

}; // END of namespace debug
}; // END of namespace synfig

/* === E N D =============================================================== */

#endif
