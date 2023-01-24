/* === S Y N F I G ========================================================= */
/*!	\file synfig/rendering/opengl/internal/samplers.cpp
**	\brief Samplers
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

/* === H E A D E R S ======================================================= */

#ifdef USING_PCH
#	include "pch.h"
#else
#ifdef HAVE_CONFIG_H
#	include <config.h>
#endif

#include "samplers.h"

#endif

using namespace synfig;
using namespace rendering;

/* === M A C R O S ========================================================= */

/* === G L O B A L S ======================================================= */

/* === P R O C E D U R E S ================================================= */

/* === M E T H O D S ======================================================= */

gl::Samplers::Samplers(Context &context):
	context(context),
	nearest(),
	linear()
{
	Context::Lock lock(context);
	glGenSamplers(1, &nearest);
	glSamplerParameteri(nearest, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glSamplerParameteri(nearest, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	//glSamplerParameteri(nearest, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	//glSamplerParameteri(nearest, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	context.check();

	glGenSamplers(1, &linear);
	glSamplerParameteri(linear, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glSamplerParameteri(linear, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	//glSamplerParameteri(linear, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	//glSamplerParameteri(linear, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	context.check();
}

gl::Samplers::~Samplers()
{
	Context::Lock lock(context);
	glDeleteSamplers(1, &linear);
	glDeleteSamplers(1, &nearest);
}

GLuint
gl::Samplers::get_interpolation(Color::Interpolation interpolation)
{
	switch (interpolation) {
		case Color::INTERPOLATION_LINEAR:
			return get_linear();
			break;
		default:
			break;
	}
	return get_nearest();
}

/* === E N T R Y P O I N T ================================================= */
