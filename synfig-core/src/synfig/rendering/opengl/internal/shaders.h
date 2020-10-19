/* === S Y N F I G ========================================================= */
/*!	\file synfig/rendering/opengl/internal/shaders.h
**	\brief Environment Header
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

#ifndef __SYNFIG_RENDERING_GL_SHADERS_H
#define __SYNFIG_RENDERING_GL_SHADERS_H

/* === H E A D E R S ======================================================= */

#include <cstring>

#include <synfig/color.h>
#include <synfig/vector.h>

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

	struct BlendProgramInfo {
		GLuint id;
		GLuint fragment_id;
		GLuint amount_uniform;
		GLuint sampler_dest_uniform;
		GLuint sampler_src_uniform;
		BlendProgramInfo()
			{ memset(this, 0, sizeof(*this)); }
	};

	struct AntialiasedTexturedRectProgramInfo {
		GLuint id;
		GLuint fragment_id;
		GLuint sampler_uniform;
		GLuint aascale_uniform;
		AntialiasedTexturedRectProgramInfo()
			{ memset(this, 0, sizeof(*this)); }
	};

private:
	GLuint simple_vertex_id;
	GLuint simple_program_id;

	GLuint color_fragment_id;
	GLuint color_program_id;
	GLint color_uniform;

	BlendProgramInfo blend_programs[Color::BLEND_END];

	GLuint texture_vertex_id;
	GLuint texture_fragment_id;
	GLuint texture_program_id;
	GLuint texture_uniform;

	GLuint antialiased_textured_rect_vertex_id;
	AntialiasedTexturedRectProgramInfo antialiased_textured_rect_programs[Color::INTERPOLATION_COUNT];

	String get_shader_path();
	String get_shader_path(const String &filename);
	String load_shader(const String &filename);
	GLuint compile_shader(GLenum type, const String &src);
	GLuint load_and_compile_shader(GLenum type, const String &filename);
	void check_shader(GLuint id, const String &src);
	void check_program(GLuint id, const String &name);

	void load_blend(Color::BlendMethod method, const String &name);
	void load_antialiased_textured_rect(Color::Interpolation interpolation, const String &name);

public:
	Shaders(Context &context);
	~Shaders();

	void simple();
	void color(const Color &c);
	void blend(Color::BlendMethod method, Color::value_type amount);
	void texture();
	void antialiased_textured_rect(Color::Interpolation interpolation, const Vector &aascale);
};

}; /* end namespace gl */
}; /* end namespace rendering */
}; /* end namespace synfig */

/* -- E N D ----------------------------------------------------------------- */

#endif
