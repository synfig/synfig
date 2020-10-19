/* === S Y N F I G ========================================================= */
/*!	\file synfig/rendering/opengl/internal/sampler.h
**	\brief Sampler Header
**
**	$Id$
**
**	\legal
**	......... ... 2015 Ivan Mahonin
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

#ifndef __SYNFIG_RENDERING_GL_SAMPLER_H
#define __SYNFIG_RENDERING_GL_SAMPLER_H

/* === H E A D E R S ======================================================= */

#include <list>
#include <vector>

#include <synfig/color.h>

#include "context.h"

/* === M A C R O S ========================================================= */

/* === T Y P E D E F S ===================================================== */

/* === C L A S S E S & S T R U C T S ======================================= */

namespace synfig
{
namespace rendering
{
namespace gl
{

class Samplers
{
public:
	Context &context;

private:
	GLuint nearest;
	GLuint linear;

public:
	Samplers(Context &context);
	~Samplers();

	GLuint get_nearest() { return nearest; }
	GLuint get_linear() { return linear; }

	// use with Shaders::antialiased_textured_rect
	GLuint get_interpolation(Color::Interpolation interpolation);
};

}; /* end namespace gl */
}; /* end namespace rendering */
}; /* end namespace synfig */

/* -- E N D ----------------------------------------------------------------- */

#endif
