/* === S Y N F I G ========================================================= */
/*!	\file synfig/rendering/opengl/internal/shaders.cpp
**	\brief Environment
**
**	$Id$
**
**	\legal
**	......... ... 2015 Ivan Mahonin
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

#include <cctype>

#include <fstream>

#include <synfig/general.h>
#include <synfig/localization.h>
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
	color_uniform(),

	texture_vertex_id(),
	texture_fragment_id(),
	texture_program_id(),
	texture_uniform(),

	antialiased_textured_rect_vertex_id()
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
#error implement add_composite and alpha
	load_blend(Color::BLEND_ADD_COMPOSITE,  "add_composite");
	load_blend(Color::BLEND_ALPHA,          "alpha");
	#ifndef NDEBUG
	for(int i = 0; i < Color::BLEND_END; ++i)
		assert(blend_programs[i].id);
	#endif

	// texture
	texture_vertex_id = load_and_compile_shader(GL_VERTEX_SHADER, "texture_vertex.glsl");
	texture_fragment_id = load_and_compile_shader(GL_FRAGMENT_SHADER, "texture_fragment.glsl");
	texture_program_id = glCreateProgram();
	glAttachShader(texture_program_id, texture_vertex_id);
	glAttachShader(texture_program_id, texture_fragment_id);
	glLinkProgram(texture_program_id);
	check_program(texture_program_id, "texture");
	texture_uniform = glGetUniformLocation(texture_program_id, "sampler");

	// antialiased textured rect
	antialiased_textured_rect_vertex_id = load_and_compile_shader(GL_VERTEX_SHADER, "antialiased_textured_rect_vertex.glsl");
	load_antialiased_textured_rect(Color::INTERPOLATION_NEAREST, "nearest");
	load_antialiased_textured_rect(Color::INTERPOLATION_LINEAR, "linear");
	load_antialiased_textured_rect(Color::INTERPOLATION_COSINE, "cosine");
	load_antialiased_textured_rect(Color::INTERPOLATION_CUBIC, "cubic");
	#ifndef NDEBUG
	for(int i = 0; i < Color::INTERPOLATION_COUNT; ++i)
		assert(antialiased_textured_rect_programs[i].id);
	#endif
}

gl::Shaders::~Shaders()
{
	Context::Lock lock(context);
	glUseProgram(0);

	// texture
	glDeleteProgram(texture_program_id);
	glDeleteShader(texture_fragment_id);
	glDeleteShader(texture_vertex_id);

	// blend
	for(int i = 0; i < Color::BLEND_END; ++i)
	{
		glDeleteProgram(blend_programs[i].id);
		glDeleteShader(blend_programs[i].fragment_id);
	}

	// color
	glDeleteProgram(color_program_id);
	glDeleteShader(color_fragment_id);

	// simple
	glDeleteProgram(simple_program_id);
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
	//assert(f);
	return String( std::istreambuf_iterator<char>(f),
			       std::istreambuf_iterator<char>() );
}

GLuint
gl::Shaders::compile_shader(GLenum type, const String &src)
{
	//assert(!src.empty());
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

void
gl::Shaders::load_blend(Color::BlendMethod method, const String &name)
{
	assert(method >= 0 && method < Color::BLEND_END);

	String src = load_shader("blend_fragment.glsl");
	size_t pos = src.find("#0");
	if (pos != String::npos)
		src = src.substr(0, pos) + name + src.substr(pos + 2);

	BlendProgramInfo &i = blend_programs[method];
	i.fragment_id = compile_shader(GL_FRAGMENT_SHADER, src);
	i.id = glCreateProgram();
	glAttachShader(i.id, simple_vertex_id);
	glAttachShader(i.id, i.fragment_id);
	glLinkProgram(i.id);
	check_program(i.id, "blend_" + name);
	i.amount_uniform = glGetUniformLocation(i.id, "amount");
	i.sampler_dest_uniform = glGetUniformLocation(i.id, "sampler_dest");
	i.sampler_src_uniform = glGetUniformLocation(i.id, "sampler_src");
	context.check("gl::Shaders::load_blend");
}

void
gl::Shaders::load_antialiased_textured_rect(Color::Interpolation interpolation, const String &name)
{
	assert(interpolation >= 0 && interpolation < (int)Color::INTERPOLATION_COUNT);

	String src = load_shader("antialiased_textured_rect_fragment.glsl");
	size_t pos = src.find("#0");
	if (pos != String::npos)
		src = src.substr(0, pos) + name + src.substr(pos + 2);

	AntialiasedTexturedRectProgramInfo &i = antialiased_textured_rect_programs[interpolation];
	i.fragment_id = compile_shader(GL_FRAGMENT_SHADER, src);
	i.id = glCreateProgram();
	glAttachShader(i.id, antialiased_textured_rect_vertex_id);
	glAttachShader(i.id, i.fragment_id);
	glLinkProgram(i.id);
	check_program(i.id, "antialiased_textured_rect_" + name);
	i.sampler_uniform = glGetUniformLocation(i.id, "sampler");
	i.aascale_uniform = glGetUniformLocation(i.id, "aascale");
	context.check("gl::Shaders::load_antialiased_textured_rect");
}


void
gl::Shaders::simple()
{
	glUseProgram(simple_program_id);
	context.check("gl::Shaders::simple");
}

void
gl::Shaders::color(const Color &c)
{
	glUseProgram(color_program_id);
	glUniform4fv(color_uniform, 1, (const GLfloat*)&c);
	context.check("gl::Shaders::color");
}

void
gl::Shaders::blend(Color::BlendMethod method, Color::value_type amount)
{
	assert(method >= 0 && method < Color::BLEND_END);
	BlendProgramInfo &i = blend_programs[method];
	glUseProgram(i.id);
	glUniform1f(i.amount_uniform, amount);
	glUniform1i(i.sampler_dest_uniform, 0);
	glUniform1i(i.sampler_src_uniform, 1);
	context.check("gl::Shaders::blend");
}

void
gl::Shaders::texture()
{
	glUseProgram(texture_program_id);
	glUniform1i(texture_uniform, 0);
	context.check("gl::Shaders::texture");
}

void
gl::Shaders::antialiased_textured_rect(Color::Interpolation interpolation, const Vector &aascale)
{
	assert(interpolation >= 0 && interpolation < (int)Color::INTERPOLATION_COUNT);
	AntialiasedTexturedRectProgramInfo &i = antialiased_textured_rect_programs[interpolation];
	glUseProgram(i.id);
	glUniform1i(i.sampler_uniform, 0);
	glUniform2f(i.aascale_uniform, (float)aascale[0], (float)aascale[1]);
	context.check("gl::Shaders::antialiased_textured_rect");
}


/* === E N T R Y P O I N T ================================================= */
