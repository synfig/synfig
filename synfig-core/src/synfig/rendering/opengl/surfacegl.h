/* === S Y N F I G ========================================================= */
/*!	\file synfig/rendering/opengl/surfacegl.h
**	\brief SurfaceGL Header
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

#ifndef __SYNFIG_RENDERING_SURFACEGL_H
#define __SYNFIG_RENDERING_SURFACEGL_H

/* === H E A D E R S ======================================================= */

#include "../surface.h"
#include "internal/predeclarations.h"

/* === M A C R O S ========================================================= */

/* === T Y P E D E F S ===================================================== */

/* === C L A S S E S & S T R U C T S ======================================= */

namespace synfig
{
namespace rendering
{

class SurfaceGL: public Surface
{
public:
	typedef etl::handle<SurfaceGL> Handle;
	static Token token;
	virtual Token::Handle get_token() const
		{ return token.handle(); }

private:
	gl::Identifier id;

protected:
	gl::Environment& env() const;

	virtual bool create_vfunc(int width, int height);
	virtual bool assign_vfunc(const Surface &surface);
	virtual bool reset_vfunc();
	virtual bool get_pixels_vfunc(Color *buffer) const;

public:
	SurfaceGL();
	explicit SurfaceGL(const Surface &other);
	~SurfaceGL();

	gl::Identifier get_id() const
		{ return id; }
};

} /* end namespace rendering */
} /* end namespace synfig */

/* -- E N D ----------------------------------------------------------------- */

#endif
