/* === S Y N F I G ========================================================= */
/*!	\file synfig/rendering/opengl/internal/shaders.h
**	\brief Environment Header
**
**	$Id$
**
**	\legal
**	......... ... 2014 Ivan Mahonin
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

#ifndef __SYNFIG_RENDERING_GL_SHADERS_H
#define __SYNFIG_RENDERING_GL_SHADERS_H

/* === H E A D E R S ======================================================= */

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

class Shaders
{
public:
	Context &context;

private:
	GLuint simple_vertex_id;
	GLuint simpleProgramId;

	GLuint color_fragment_id;
	GLuint colorProgramId;
	GLint colorUniform;

	void check_shader(GLuint id, const String &src);
	void check_program(GLuint id, const String &name);

public:
	Shaders(Context &context);
	~Shaders();

	void simple();
	void color(const Color &c);
};

}; /* end namespace gl */
}; /* end namespace rendering */
}; /* end namespace synfig */

/* -- E N D ----------------------------------------------------------------- */

#endif
