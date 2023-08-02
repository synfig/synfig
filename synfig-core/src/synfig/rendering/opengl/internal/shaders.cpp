/* === S Y N F I G ========================================================= */
/*!	\file synfig/rendering/opengl/internal/shaders.cpp
**	\brief Shaders
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

/* === H E A D E R S ======================================================= */

#ifdef USING_PCH
#	include "pch.h"
#else
#ifdef HAVE_CONFIG_H
#	include <config.h>
#endif

#include "shaders.h"

#include "synfig/general.h"
#include "synfig/main.h"

#include <fstream>
#include <cassert>

#endif

using namespace synfig;
using namespace rendering;

using Shader = gl::Shaders::Shader;
using Program = gl::Programs::Program;

/* === M A C R O S ========================================================= */

/* === G L O B A L S ======================================================= */

/* === P R O C E D U R E S ================================================= */

/* === M E T H O D S ======================================================= */

// loads and compiles shader, file extension used to determine shader type

// SHADER

Shader
compile_shader(GLenum type, const std::string& content)
{
	Shader s;
	s.valid = false;

	s.id = glCreateShader(type);

	const GLchar* source = content.c_str();
	glShaderSource(s.id, 1, &source, 0);

	glCompileShader(s.id);

	GLint isCompiled = 0;
	glGetShaderiv(s.id, GL_COMPILE_STATUS, &isCompiled);
	if(isCompiled == GL_FALSE)
	{
		GLint maxLength = 0;
		glGetShaderiv(s.id, GL_INFO_LOG_LENGTH, &maxLength);

		std::vector<GLchar> infoLog(maxLength);
		glGetShaderInfoLog(s.id, maxLength, &maxLength, &infoLog[0]);
		
		error("Opengl shader error: %s", infoLog.data());

		glDeleteShader(s.id);

		return s;
	}

	s.valid = true;
	return s;
}

std::string
get_shader_path(const std::string& file)
{
	return Main::get_instance().bin_path + "/shaders/" + file;
}

Shader
load_shader(const std::string& file)
{
	const std::string ext = file.substr(file.find('.'));

	String path = get_shader_path(file);

	std::ifstream f(path.c_str());
	assert(f.good());

	std::string src = String( std::istreambuf_iterator<char>(f),
			       std::istreambuf_iterator<char>() );

	if(ext == ".vs") return compile_shader(GL_VERTEX_SHADER, src);
	else if(ext == ".fs") return compile_shader(GL_FRAGMENT_SHADER, src);

	return { 0, false };
}

void
delete_shader(Shader s)
{
	assert(s.valid);
	glDeleteShader(s.id);
}

void
gl::Shaders::load_blend(Color::BlendMethod method, const String &name)
{
	std::ifstream f(get_shader_path("blend.fs"));
	assert(f.good());

	std::string src = String( std::istreambuf_iterator<char>(f),
			       std::istreambuf_iterator<char>() );

	size_t pos = src.find("#0");
	if (pos != String::npos)
		src = src.substr(0, pos) + name + src.substr(pos + 2);

	blend_shaders[method] = compile_shader(GL_FRAGMENT_SHADER, src);
}

bool
gl::Shaders::initialize()
{
	map["basic.vs"] = load_shader("basic.vs");
	assert(map["basic.vs"].valid);

	map["solid.fs"] = load_shader("solid.fs");
	assert(map["solid.fs"].valid);

	map["colormatrix.fs"] = load_shader("colormatrix.fs");
	assert(map["colormatrix.fs"].valid);

	map["blit.fs"] = load_shader("blit.fs");
	assert(map["blit.fs"].valid);

	map["box_blur.fs"] = load_shader("blurs/box_blur.fs");
	assert(map["box_blur.fs"].valid);

	map["cross_blur.fs"] = load_shader("blurs/cross_blur.fs");
	assert(map["cross_blur.fs"].valid);

	map["disc_blur.fs"] = load_shader("blurs/disc_blur.fs");
	assert(map["disc_blur.fs"].valid);

	// blend
	load_blend(Color::BLEND_COMPOSITE,      "composite");
	load_blend(Color::BLEND_STRAIGHT,       "straight");
	load_blend(Color::BLEND_ONTO,           "onto");
	load_blend(Color::BLEND_STRAIGHT_ONTO,  "straightonto");
	load_blend(Color::BLEND_BEHIND,         "behind");
	load_blend(Color::BLEND_SCREEN,         "screen");
	load_blend(Color::BLEND_OVERLAY,        "overlay");
	load_blend(Color::BLEND_HARD_LIGHT,     "hardlight");
	load_blend(Color::BLEND_MULTIPLY,       "multiply");
	load_blend(Color::BLEND_DIVIDE,         "divide");
	load_blend(Color::BLEND_ADD,            "add");
	load_blend(Color::BLEND_SUBTRACT,       "subtract");
	load_blend(Color::BLEND_DIFFERENCE,     "difference");
	load_blend(Color::BLEND_BRIGHTEN,       "brighten");
	load_blend(Color::BLEND_DARKEN,         "darken");
	load_blend(Color::BLEND_COLOR,          "color");
	load_blend(Color::BLEND_HUE,            "hue");
	load_blend(Color::BLEND_SATURATION,     "saturation");
	load_blend(Color::BLEND_LUMINANCE,      "luminance");
	load_blend(Color::BLEND_ALPHA_OVER,     "alphaover");
	load_blend(Color::BLEND_ALPHA_BRIGHTEN, "alphabrighten");
	load_blend(Color::BLEND_ALPHA_DARKEN,   "alphadarken");
	load_blend(Color::BLEND_ADD_COMPOSITE,  "add_composite");
	load_blend(Color::BLEND_ALPHA,          "alpha");


	valid = true;
	return true;
}

void
gl::Shaders::deinitialize()
{
	for(auto shader: map)
	{
		// only delete loaded shaders
		if(!shader.second.valid) continue;
		delete_shader(shader.second);
	}

	for(int method = 0; method < Color::BLEND_END; method++)
	{
		Shader& shader = blend_shaders[method];
		if(!shader.valid) continue;
		delete_shader(shader);
	}

	valid = false;
	info("Shaders deinitialized");
}

Shader
gl::Shaders::get_shader(const std::string &str) const
{
	auto itr = map.find(str);
	if(itr != map.end()) return itr->second;
	return { 0, false };
}

Shader
gl::Shaders::get_blend_shader(Color::BlendMethod method) const
{
	return blend_shaders[method];
}

// PROGRAMS

Program
create_program(std::vector<Shader> shaders)
{
	Program p;
	p.valid = false;

	p.id = glCreateProgram();

	for(Shader s: shaders)
	{
		if(!s.valid) return p;

		glAttachShader(p.id, s.id);
	}

	glLinkProgram(p.id);

	GLint isLinked = 0;
	glGetProgramiv(p.id, GL_LINK_STATUS, (int *)&isLinked);
	if (isLinked == GL_FALSE)
	{
		GLint maxLength = 0;
		glGetProgramiv(p.id, GL_INFO_LOG_LENGTH, &maxLength);

		std::vector<GLchar> infoLog(maxLength);
		glGetProgramInfoLog(p.id, maxLength, &maxLength, &infoLog[0]);

		error("Opengl Program error: %s", infoLog.data());
		
		glDeleteProgram(p.id);
		return p;
	}

	for(Shader s: shaders)
	{
		glDetachShader(p.id, s.id);
	}

	p.valid = true;
	return p;
}

Program
clone_program(const Program& p)
{
	assert(p.valid);
	return create_program(p.shaders);
}

void
delete_program(Program p)
{
	assert(p.valid);
	glDeleteProgram(p.id);
}

void
gl::Programs::load_blend(const Shaders& shaders, Color::BlendMethod method, const String &name)
{
	blend_programs[method] = create_program({ shaders.get_blend_shader(method), shaders.get_shader("basic.vs") });
	assert(blend_programs[method].valid);
}

bool
gl::Programs::initialize(const Shaders& shaders)
{
	assert(shaders.is_valid());

	map["solid"] = create_program({ shaders.get_shader("basic.vs"), shaders.get_shader("solid.fs") });
	assert(map["solid"].valid);

	map["colormatrix"] = create_program({ shaders.get_shader("basic.vs"), shaders.get_shader("colormatrix.fs") });
	assert(map["colormatrix"].valid);

	map["blit"] = create_program({ shaders.get_shader("basic.vs"), shaders.get_shader("blit.fs") });
	assert(map["blit"].valid);

	map["box_blur"] = create_program({ shaders.get_shader("basic.vs"), shaders.get_shader("box_blur.fs") });
	assert(map["box_blur"].valid);

	map["cross_blur"] = create_program({ shaders.get_shader("basic.vs"), shaders.get_shader("cross_blur.fs") });
	assert(map["cross_blur"].valid);

	map["disc_blur"] = create_program({ shaders.get_shader("basic.vs"), shaders.get_shader("disc_blur.fs") });
	assert(map["disc_blur"].valid);

	// blend
	load_blend(shaders, Color::BLEND_COMPOSITE,      "composite");
	load_blend(shaders, Color::BLEND_STRAIGHT,       "straight");
	load_blend(shaders, Color::BLEND_ONTO,           "onto");
	load_blend(shaders, Color::BLEND_STRAIGHT_ONTO,  "straightonto");
	load_blend(shaders, Color::BLEND_BEHIND,         "behind");
	load_blend(shaders, Color::BLEND_SCREEN,         "screen");
	load_blend(shaders, Color::BLEND_OVERLAY,        "overlay");
	load_blend(shaders, Color::BLEND_HARD_LIGHT,     "hardlight");
	load_blend(shaders, Color::BLEND_MULTIPLY,       "multiply");
	load_blend(shaders, Color::BLEND_DIVIDE,         "divide");
	load_blend(shaders, Color::BLEND_ADD,            "add");
	load_blend(shaders, Color::BLEND_SUBTRACT,       "subtract");
	load_blend(shaders, Color::BLEND_DIFFERENCE,     "difference");
	load_blend(shaders, Color::BLEND_BRIGHTEN,       "brighten");
	load_blend(shaders, Color::BLEND_DARKEN,         "darken");
	load_blend(shaders, Color::BLEND_COLOR,          "color");
	load_blend(shaders, Color::BLEND_HUE,            "hue");
	load_blend(shaders, Color::BLEND_SATURATION,     "saturation");
	load_blend(shaders, Color::BLEND_LUMINANCE,      "luminance");
	load_blend(shaders, Color::BLEND_ALPHA_OVER,     "alphaover");
	load_blend(shaders, Color::BLEND_ALPHA_BRIGHTEN, "alphabrighten");
	load_blend(shaders, Color::BLEND_ALPHA_DARKEN,   "alphadarken");
	load_blend(shaders, Color::BLEND_ADD_COMPOSITE,  "add_composite");
	load_blend(shaders, Color::BLEND_ALPHA,          "alpha");

	valid = true;
	return true;
}

void
gl::Programs::deinitialize()
{
	for(auto program: map)
	{
		delete_program(program.second);
	}

	for(int method = 0; method < Color::BLEND_END; method++)
	{
		Program& program = blend_programs[method];
		if(!program.valid) continue;
		delete_program(program);
	}
	valid = false;
}

gl::Programs
gl::Programs::clone() const
{
	gl::Programs programs = *this;

	for(auto& program: programs.map)
	{
		program.second = clone_program(program.second);
		programs.valid = programs.valid && program.second.valid;
	}

	return programs;
}

Program
gl::Programs::get_program(const std::string &str) const
{
	if(map.count(str) == 0) return {0, false};

	Program p = map.at(str);
	p.shaders = {};
	return p;
}

Program
gl::Programs::get_blend_program(Color::BlendMethod method) const
{
	return blend_programs[method];
}

// PROGRAM

void
gl::Programs::Program::set_1i(const std::string &name, int value)
{
	assert(valid);

	int loc = glGetUniformLocation(id, name.c_str());
	glUniform1i(loc, value);
}

void
gl::Programs::Program::set_1f(const std::string &name, float value)
{
	assert(valid);

	int loc = glGetUniformLocation(id, name.c_str());
	glUniform1f(loc, value);
}

void
gl::Programs::Program::set_2i(const std::string &name, VectorInt value)
{
	assert(valid);

	int loc = glGetUniformLocation(id, name.c_str());
	glUniform2i(loc, value[0], value[1]);
}

void
gl::Programs::Program::set_2f(const std::string &name, Vector value)
{
	assert(valid);

	int loc = glGetUniformLocation(id, name.c_str());
	glUniform2f(loc, value[0], value[1]);
}

void
gl::Programs::Program::set_4i(const std::string &name, int a, int b, int c, int d)
{
	assert(valid);

	int loc = glGetUniformLocation(id, name.c_str());
	glUniform4i(loc, a, b, c, d);
}

void
gl::Programs::Program::set_color(const std::string &name, Color value)
{
	assert(valid);

	int loc = glGetUniformLocation(id, name.c_str());
	glUniform4f(loc, value.get_r(), value.get_g(), value.get_b(), value.get_a());
}

void
gl::Programs::Program::set_mat5x5(const std::string &name, ColorMatrix mat)
{
	assert(valid);

	set_1f(name + ".m00", mat[0][0]); set_1f(name + ".m01", mat[0][1]);	set_1f(name + ".m02", mat[0][2]); set_1f(name + ".m03", mat[0][3]);	set_1f(name + ".m04", mat[0][4]);
	set_1f(name + ".m10", mat[1][0]); set_1f(name + ".m11", mat[1][1]);	set_1f(name + ".m12", mat[1][2]); set_1f(name + ".m13", mat[1][3]);	set_1f(name + ".m14", mat[1][4]);
	set_1f(name + ".m20", mat[2][0]); set_1f(name + ".m21", mat[2][1]);	set_1f(name + ".m22", mat[2][2]); set_1f(name + ".m23", mat[2][3]);	set_1f(name + ".m24", mat[2][4]);
	set_1f(name + ".m30", mat[3][0]); set_1f(name + ".m31", mat[3][1]);	set_1f(name + ".m32", mat[3][2]); set_1f(name + ".m33", mat[3][3]);	set_1f(name + ".m34", mat[3][4]);
	set_1f(name + ".m40", mat[4][0]); set_1f(name + ".m41", mat[4][1]);	set_1f(name + ".m42", mat[4][2]); set_1f(name + ".m43", mat[4][3]);	set_1f(name + ".m44", mat[4][4]);
}

void
gl::Programs::Program::use()
{
	assert(valid);

	glUseProgram(id);
}

/* === E N T R Y P O I N T ================================================= */
