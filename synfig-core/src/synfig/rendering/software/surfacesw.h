/* === S Y N F I G ========================================================= */
/*!	\file synfig/rendering/surfacesoftware.h
**	\brief SurfaceSoftware Header
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

#ifndef __SYNFIG_RENDERING_SURFACESOFTWARE_H
#define __SYNFIG_RENDERING_SURFACESOFTWARE_H

/* === H E A D E R S ======================================================= */

#include "../surface.h"
#include "surface.h"

/* === M A C R O S ========================================================= */

/* === T Y P E D E F S ===================================================== */

/* === C L A S S E S & S T R U C T S ======================================= */

namespace synfig
{
namespace rendering
{

class SurfaceSoftware: public Surface, public synfig::Surface
{
public:
	typedef etl::handle<SurfaceSoftware> Handle;

protected:
	virtual void assign_size_vfunc(int width, int height);
	virtual void assign_surface_vfunc(const rendering::Surface::Handle &surface);
	virtual void get_size_vfunc(int &out_width, int &out_height) const;
	virtual void get_pixels_vfunc(Color *buffer) const;
};

} /* end namespace rendering */
} /* end namespace synfig */

/* -- E N D ----------------------------------------------------------------- */

#endif
