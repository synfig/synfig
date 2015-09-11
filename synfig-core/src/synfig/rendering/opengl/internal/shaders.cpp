/* === S Y N F I G ========================================================= */
/*!	\file synfig/rendering/opengl/internal/shaders.cpp
**	\brief Environment
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

/* === H E A D E R S ======================================================= */

#ifdef USING_PCH
#	include "pch.h"
#else
#ifdef HAVE_CONFIG_H
#	include <config.h>
#endif

#ifndef WIN32
#include <unistd.h>
#include <sys/types.h>
#include <signal.h>
#endif

#include <fstream>

#include <synfig/general.h>
#include <synfig/main.h>

#include "shaders.h"

#endif

using namespace synfig;
using namespace rendering;

/* === M A C R O S ========================================================= */

/* === G L O B A L S ======================================================= */

/* === P R O C E D U R E S ================================================= */

/* === M E T H O D S ======================================================= */

gl::Shaders::Shaders(Context &context):
	context(context),
	simple_vertex_id(),
	simple_program_id(),
	color_fragment_id(),
	color_program_id(),
	color_uniform()
{
	Context::Lock lock(context);

	// simple
	simple_vertex_id = load_and_compile_shader(GL_VERTEX_SHADER, "simple_vertex.glsl");
	simple_program_id = glCreateProgram();
	glAttachShader(simple_program_id, simple_vertex_id);
	glLinkProgram(simple_program_id);
	check_program(simple_program_id, "simple");

	// color
	color_fragment_id = load_and_compile_shader(GL_FRAGMENT_SHADER, "color_fragment.glsl");
	color_program_id = glCreateProgram();
	glAttachShader(color_program_id, simple_vertex_id);
	glAttachShader(color_program_id, color_fragment_id);
	glLinkProgram(color_program_id);
	check_program(color_program_id, "color");
	color_uniform = glGetUniformLocation(color_program_id, "color");
}

gl::Shaders::~Shaders()
{
	Context::Lock lock(context);
	glUseProgram(0);
	glDeleteProgram(color_program_id);
	glDeleteProgram(simple_program_id);
	glDeleteShader(color_fragment_id);
	glDeleteShader(simple_vertex_id);
}

String
gl::Shaders::get_shader_path()
{
	return Main::get_instance().lib_synfig_path
		 + ETL_DIRECTORY_SEPARATOR
		 + "glsl";
}

String
gl::Shaders::get_shader_path(const String &filename)
{
	return get_shader_path()
		 + ETL_DIRECTORY_SEPARATOR
		 + filename;
}

String
gl::Shaders::load_shader(const String &filename)
{
	String path = get_shader_path(filename);
	std::ifstream f(path.c_str());
	assert(f);
	return String( std::istreambuf_iterator<char>(f),
			       std::istreambuf_iterator<char>() );
}

GLuint
gl::Shaders::compile_shader(GLenum type, const String &src)
{
	assert(!src.empty());
	GLuint id = glCreateShader(type);
	const char *lines = src.c_str();
	glShaderSource(id, 1, &lines, NULL);
	glCompileShader(id);
	check_shader(id, src);
	return id;
}

GLuint
gl::Shaders::load_and_compile_shader(GLenum type, const String &filename)
{
	return compile_shader(type, load_shader(filename));
}

void
gl::Shaders::check_shader(GLuint id, const String &src)
{
	GLint compileStatus = 0;
	glGetShaderiv(id, GL_COMPILE_STATUS, &compileStatus);
	if (!compileStatus) {
		GLint log_length = 0;
		glGetShaderiv(id, GL_INFO_LOG_LENGTH, &log_length);
		String log;
		log.resize(log_length);
		glGetShaderInfoLog(id, log.size(), &log_length, &log[0]);
		log.resize(log_length);
		warning( String()
			   + "~~~~~~~~~~~~~~~~~~~~~~~~~~~\n"
			   + "cannot compile shader:\n"
			   + "~~~~~~source~~~~~~~~~~~~~~~\n"
			   + src + "\n"
			   + "~~~~~~log~~~~~~~~~~~~~~~~~~\n"
			   + log + "\n"
			   + "~~~~~~~~~~~~~~~~~~~~~~~~~~~" );
	}
}

void
gl::Shaders::check_program(GLuint id, const String &name)
{
 	GLint linkStatus = 0;
	glGetProgramiv(id, GL_LINK_STATUS, &linkStatus);
	if (!linkStatus) {
		GLint log_length = 0;
		glGetProgramiv(id, GL_INFO_LOG_LENGTH, &log_length);
		std::string log;
		log.resize(log_length);
		glGetProgramInfoLog(id, log.size(), &log_length, &log[0]);
		warning( String()
			   + "~~~~~~~~~~~~~~~~~~~~~~~~~~~\n"
			   + "cannot link program " + name +  ":\n"
			   + "~~~~~~name~~~~~~~~~~~~~~~~~\n"
			   + name + "\n"
			   + "~~~~~~log~~~~~~~~~~~~~~~~~~\n"
			   + log + "\n"
			   + "~~~~~~~~~~~~~~~~~~~~~~~~~~~" );
	}

	glValidateProgram(id);
	GLint validateStatus = 0;
	glGetProgramiv(id, GL_VALIDATE_STATUS, &validateStatus);
	if (!validateStatus) {
		GLint log_length = 0;
		glGetProgramiv(id, GL_INFO_LOG_LENGTH, &log_length);
		String log;
		log.resize(log_length);
		glGetProgramInfoLog(id, log.size(), &log_length, &log[0]);
		warning( String()
			   + "~~~~~~~~~~~~~~~~~~~~~~~~~~~\n"
			   + "program not validated " + name +  ":\n"
			   + "~~~~~~name~~~~~~~~~~~~~~~~~\n"
			   + name + "\n"
			   + "~~~~~~log~~~~~~~~~~~~~~~~~~\n"
			   + log + "\n"
			   + "~~~~~~~~~~~~~~~~~~~~~~~~~~~" );
	}
}

void gl::Shaders::simple()
{
	glUseProgram(simple_program_id);
}

void gl::Shaders::color(const Color &c)
{
	glUseProgram(color_program_id);
	glUniform4fv(color_uniform, 1, (const GLfloat*)&c);
}



/* === E N T R Y P O I N T ================================================= */
