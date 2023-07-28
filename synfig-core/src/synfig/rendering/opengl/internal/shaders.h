/* === S Y N F I G ========================================================= */
/*!	\file synfig/rendering/opengl/internal/shaders.h
**	\brief Shaders Header
**
**	\legal
**	......... ... 2015 Ivan Mahonin
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

#ifndef __SYNFIG_RENDERING_GL_SHADERS_H
#define __SYNFIG_RENDERING_GL_SHADERS_H

/* === H E A D E R S ======================================================= */
#include "headers.h"
#include "synfig/color/color.h"
#include "synfig/color/colormatrix.h"
#include "synfig/vector.h"

#include <map>
#include <string>
#include <vector>

/* === M A C R O S ========================================================= */

/* === T Y P E D E F S ===================================================== */

/* === C L A S S E S & S T R U C T S ======================================= */

namespace synfig
{
namespace rendering
{
namespace gl
{

// Shaders are compiled on the main context but each context has its own programs
class Shaders
{
public:
	struct Shader
	{
		GLuint id;
		bool valid;
	};

	bool initialize();
	void deinitialize();

	Shader get_shader(const std::string& str) const;
	Shader get_blend_shader(Color::BlendMethod blend) const;

	bool is_valid() const { return valid; }

private:
	bool valid = false;

	std::map<std::string, Shader> map;
	Shader blend_shaders[Color::BLEND_END];

	void load_blend(Color::BlendMethod method, const String &name);
};

class Programs
{
public:
	class Program
	{
	public:
		GLuint id;
		bool valid;
		std::vector<Shaders::Shader> shaders;

		void use();
		void set_1i(const std::string& name, int value);
		void set_1f(const std::string& name, float value);
		void set_2i(const std::string& name, VectorInt value);
		void set_4i(const std::string &name, int a, int b, int c, int d);
		void set_color(const std::string& name, Color value);
		void set_mat5x5(const std::string& name, ColorMatrix mat);

	private:
	};

	bool initialize(const Shaders& shaders);
	void deinitialize();

	Programs clone() const;

	Program get_program(const std::string& str) const;
	Program get_blend_program(Color::BlendMethod blend) const;

	bool is_valid() const { return valid; }

private:
	bool valid = false;

	std::map<std::string, Program> map;
	Program blend_programs[Color::BLEND_END];

	void load_blend(const Shaders& shaders, Color::BlendMethod method, const String &name);
};

}; /* end namespace gl */
}; /* end namespace rendering */
}; /* end namespace synfig */

/* -- E N D ----------------------------------------------------------------- */

#endif
