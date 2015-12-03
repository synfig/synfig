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

#include <synfig/color.h>
#include <synfig/vector.h>

#include "resource.h"

/* === M A C R O S ========================================================= */

/* === T Y P E D E F S ===================================================== */

/* === C L A S S E S & S T R U C T S ======================================= */

namespace synfig
{
namespace rendering
{

class Surface: public Resource
{
public:
	typedef etl::handle<Surface> Handle;

private:
	int width;
	int height;
	bool created;

protected:
	void mark_as_created(bool create = true);
	virtual bool create_vfunc() = 0;
	virtual bool assign_vfunc(const Surface &surface) = 0;
	virtual void destroy_vfunc() = 0;
	virtual bool get_pixels_vfunc(Color *buffer) const = 0;

public:
	// TODO: move to Resource?
	bool is_temporary;

	Surface();
	virtual ~Surface();

	void set_size(int width, int height);

	bool create();
	bool assign(const Color *buffer);
	bool assign(const Color *buffer, int width, int height);
	bool assign(const Surface &surface);
	bool assign(const Handle &surface);
	void destroy();

	bool empty() const;
	int get_width() const { return width; }
	int get_height() const { return height; }
	int get_pixels_count() const { return get_width()*get_height(); }
	size_t get_buffer_size() const;
	bool is_created() const { return created; }
	bool get_pixels(Color *buffer) const;

	void set_size(const VectorInt &x)
		{ set_size(x[0], x[1]); }
	VectorInt get_size() const
		{ return VectorInt(get_width(), get_height()); }
};

} /* end namespace rendering */
} /* end namespace synfig */

/* -- E N D ----------------------------------------------------------------- */

#endif
