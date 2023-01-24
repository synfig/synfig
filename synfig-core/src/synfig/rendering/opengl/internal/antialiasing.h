/* === S Y N F I G ========================================================= */
/*!	\file synfig/rendering/opengl/internal/antialiasing.h
**	\brief Antialiasing Header
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

#ifndef __SYNFIG_RENDERING_GL_ANTIALIASING_H
#define __SYNFIG_RENDERING_GL_ANTIALIASING_H

/* === H E A D E R S ======================================================= */

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

class Antialiasing
{
public:
	Context &context;

private:
	bool allowed;

	GLint multisample_max_width;
	GLint multisample_max_height;
	GLuint multisample_texture_id;
	GLuint multisample_renderbuffer_id;
	GLuint multisample_framebuffer_id;

	GLint multisample_viewport[4];
	GLint multisample_orig_viewport[4];
	GLuint multisample_orig_draw_framebuffer_id;

public:
	Antialiasing(Context &context);
	~Antialiasing();

	bool is_allowed() const { return allowed; }
	void multisample_begin(bool clear = true);
	void multisample_end();
};

}; /* end namespace gl */
}; /* end namespace rendering */
}; /* end namespace synfig */

/* -- E N D ----------------------------------------------------------------- */

#endif
