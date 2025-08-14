/* === S Y N F I G ========================================================= */
/*!	\file synfig/rendering/opengl/surfacegl.h
**	\brief SurfaceGL Header
**
**	\legal
**	......... ... 2015-2018 Ivan Mahonin
**	......... ... 2023 Bharat Sahlot
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

#ifndef __SYNFIG_RENDERING_SURFACEGL_H
#define __SYNFIG_RENDERING_SURFACEGL_H

/* === H E A D E R S ======================================================= */

#include "../surface.h"
#include "internal/framebuffer.h"
#include <memory>

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
	gl::Framebuffer framebuffer;

protected:
	virtual bool create_vfunc(int width, int height);
	virtual bool assign_vfunc(const Surface &surface);
	virtual bool clear_vfunc();
	virtual bool reset_vfunc();
	virtual const Color* get_pixels_pointer_vfunc() const;

public:
	SurfaceGL();
	explicit SurfaceGL(const Surface &other);
	~SurfaceGL();

	gl::Framebuffer& get_framebuffer();
};

} /* end namespace rendering */
} /* end namespace synfig */

/* -- E N D ----------------------------------------------------------------- */

#endif
