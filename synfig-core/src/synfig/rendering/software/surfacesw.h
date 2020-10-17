/* === S Y N F I G ========================================================= */
/*!	\file synfig/rendering/software/surfacesw.h
**	\brief SurfaceSW Header
**
**	$Id$
**
**	\legal
**	......... ... 2015-2018 Ivan Mahonin
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

#ifndef __SYNFIG_RENDERING_SURFACESW_H
#define __SYNFIG_RENDERING_SURFACESW_H

/* === H E A D E R S ======================================================= */

#include <synfig/surface.h>
#include <synfig/synfig_export.h>

#include "../surface.h"

/* === M A C R O S ========================================================= */

/* === T Y P E D E F S ===================================================== */

/* === C L A S S E S & S T R U C T S ======================================= */

namespace synfig
{
namespace rendering
{

class SurfaceSW: public Surface
{
public:
	typedef etl::handle<SurfaceSW> Handle;
	SYNFIG_EXPORT static Token token;
	virtual Token::Handle get_token() const
		{ return token.handle(); }

private:
	bool own_surface;
	synfig::Surface *surface;

protected:
	virtual bool create_vfunc(int width, int height);
	virtual bool assign_vfunc(const Surface &surface);
	virtual bool clear_vfunc();
	virtual bool reset_vfunc();
	virtual const Color* get_pixels_pointer_vfunc() const;

public:
	SurfaceSW();
	explicit SurfaceSW(synfig::Surface &surface, bool own_surface);
	~SurfaceSW();

	void set_surface(synfig::Surface &surface, bool own_surface = false);

	const synfig::Surface& get_surface() const
		{ return *surface; }
	synfig::Surface& get_surface()
		{ return *surface; }
	bool is_own_surface() const
		{ return own_surface; }

	void reset_surface();
};

} /* end namespace rendering */
} /* end namespace synfig */

/* -- E N D ----------------------------------------------------------------- */

#endif
