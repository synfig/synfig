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

#include <ETL/handle>

#include <synfig/color.h>

/* === M A C R O S ========================================================= */

/* === T Y P E D E F S ===================================================== */

/* === C L A S S E S & S T R U C T S ======================================= */

namespace synfig
{
namespace rendering
{

class Surface: public etl::shared_object
{
public:
	typedef etl::handle<Surface> Handle;

private:
	int width;
	int height;
	bool created;

protected:
	virtual bool create_vfunc() = 0;
	virtual bool assign_vfunc(const Surface &surface) = 0;
	virtual void destroy_vfunc() = 0;
	virtual bool get_pixels_vfunc(Color *buffer) const = 0;

public:
	bool is_temporary;

	Surface();
	virtual ~Surface();

	void set_size(int width, int height);
	bool create();
	bool assign(const Handle &surface);
	void destroy();

	bool empty() const;
	int get_width() const { return width; }
	int get_height() const { return height; }
	bool is_created() const { return created; }
	bool get_pixels(Color *buffer) const;
};

} /* end namespace rendering */
} /* end namespace synfig */

/* -- E N D ----------------------------------------------------------------- */

#endif
