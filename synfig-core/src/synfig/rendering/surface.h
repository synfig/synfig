/* === S Y N F I G ========================================================= */
/*!	\file synfig/rendering/surface.h
**	\brief Surface Header
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

#ifndef __SYNFIG_RENDERING_SURFACE_H
#define __SYNFIG_RENDERING_SURFACE_H

/* === H E A D E R S ======================================================= */

#include "renderer.h"

/* === M A C R O S ========================================================= */

/* === T Y P E D E F S ===================================================== */

/* === C L A S S E S & S T R U C T S ======================================= */

namespace synfig
{
namespace rendering
{

class Renderer;
class Transformation;
class Blending;
class Primitive;

class Surface: public Renderer::DependentObject
{
public:
	typedef etl::handle<Surface> Handle;

protected:
	virtual void assign_vfunc(int width, int height) = 0;
	virtual void assign_vfunc(const etl::handle<Surface> &surface) = 0;
	virtual int get_width_vfunc() const = 0;
	virtual int get_height_vfunc() const = 0;
	virtual void get_pixels_vfunc(Color *buffer) const = 0;

public:
	void assign(int width, int height);
	void assign(const etl::handle<Surface> &surface);

	int get_width() const;
	int get_height() const;
	void get_pixels(Color *buffer) const;
};

} /* end namespace rendering */
} /* end namespace synfig */

/* -- E N D ----------------------------------------------------------------- */

#endif
