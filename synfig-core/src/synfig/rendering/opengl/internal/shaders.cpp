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

bool
gl::Shaders::initialize()
{
	map["basic.vs"] = load_shader("basic.vs");
	assert(map["basic.vs"].valid);

	map["solid.fs"] = load_shader("solid.fs");
	assert(map["solid.fs"].valid);

	map["colormatrix.fs"] = load_shader("colormatrix.fs");
	assert(map["colormatrix.fs"].valid);

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

bool
gl::Programs::initialize(const Shaders& shaders)
{
	assert(shaders.is_valid());

	map["solid"] = create_program({ shaders.get_shader("basic.vs"), shaders.get_shader("solid.fs") });
	assert(map["solid"].valid);

	map["colormatrix"] = create_program({ shaders.get_shader("basic.vs"), shaders.get_shader("colormatrix.fs") });
	assert(map["colormatrix"].valid);

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
