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

#include <synfig/general.h>

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
	simpleProgramId(),
	color_fragment_id(),
	colorProgramId(),
	colorUniform()
{
	Context::Lock lock(context);
	const char *lines = NULL;

	// simple
	String simpleVertexSource =
		"in vec2 position;\n"
		"void main() { gl_Position = vec4(position, 0.0, 1.0); }\n";

	simple_vertex_id = glCreateShader(GL_VERTEX_SHADER);
	lines = simpleVertexSource.c_str();
	glShaderSource(simple_vertex_id, 1, &lines, NULL);
	glCompileShader(simple_vertex_id);
	check_shader(simple_vertex_id, simpleVertexSource);

	simpleProgramId = glCreateProgram();
	glAttachShader(simpleProgramId, simple_vertex_id);
	glBindAttribLocation(simpleProgramId, 0, "position");
	glLinkProgram(simpleProgramId);
	check_program(simpleProgramId, "simple");

	// color
	String colorFragmentSource =
		"uniform vec4 color;\n"
		//"out vec4 colorOut;\n"
		"void main() { gl_FragColor = color; }\n";

	color_fragment_id = glCreateShader(GL_FRAGMENT_SHADER);
	lines = colorFragmentSource.c_str();
	glShaderSource(color_fragment_id, 1, &lines, NULL);
	glCompileShader(color_fragment_id);
	check_shader(color_fragment_id, colorFragmentSource);

	colorProgramId = glCreateProgram();
	glAttachShader(colorProgramId, simple_vertex_id);
	glAttachShader(colorProgramId, color_fragment_id);
	glBindAttribLocation(colorProgramId, 0, "position");
	//glBindFragDataLocation(color_program_id, 0, "colorOut");
	glLinkProgram(colorProgramId);
	check_program(colorProgramId, "color");
	colorUniform = glGetUniformLocation(colorProgramId, "color");
}

gl::Shaders::~Shaders()
{
	Context::Lock lock(context);
	glUseProgram(0);
	glDeleteProgram(colorProgramId);
	glDeleteProgram(simpleProgramId);
	glDeleteShader(color_fragment_id);
	glDeleteShader(simple_vertex_id);
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
	glUseProgram(simpleProgramId);
}

void gl::Shaders::color(const Color &c)
{
	glUseProgram(colorProgramId);
	glUniform4fv(colorUniform, 1, (const GLfloat*)&c);
}



/* === E N T R Y P O I N T ================================================= */
