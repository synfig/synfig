/* === S Y N F I G ========================================================= */
/*!	\file synfig/rendering/opengl/internal/context.cpp
**	\brief Context
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

/* === H E A D E R S ======================================================= */

#ifdef USING_PCH
#	include "pch.h"
#else
#ifdef HAVE_CONFIG_H
#	include <config.h>
#endif

#include <synfig/general.h>
#include <synfig/localization.h>

#include "context.h"

#endif

using namespace synfig;
using namespace rendering;

/* === M A C R O S ========================================================= */

#define GLX_CONTEXT_MAJOR_VERSION_ARB 0x2091
#define GLX_CONTEXT_MINOR_VERSION_ARB 0x2092

/* === G L O B A L S ======================================================= */

typedef GLXContext (*GLXCREATECONTEXTATTRIBSARBPROC)(Display*, GLXFBConfig, GLXContext, Bool, const int*);

/* === P R O C E D U R E S ================================================= */

/* === M E T H O D S ======================================================= */

#define ADD_ENUM(x) std::pair<GLenum, const char *>(x, #x)
std::pair<GLenum, const char*> gl::Context::enum_strings[] = {
	// errors
	ADD_ENUM(GL_INVALID_ENUM),
	ADD_ENUM(GL_INVALID_VALUE),
	ADD_ENUM(GL_INVALID_OPERATION),
	ADD_ENUM(GL_INVALID_FRAMEBUFFER_OPERATION),
	ADD_ENUM(GL_STACK_OVERFLOW),
	ADD_ENUM(GL_STACK_UNDERFLOW),
	ADD_ENUM(GL_OUT_OF_MEMORY),
	// framebuffer statuses
	ADD_ENUM(GL_FRAMEBUFFER_COMPLETE),
	ADD_ENUM(GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT),
	ADD_ENUM(GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT),
	ADD_ENUM(GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER),
	ADD_ENUM(GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER),
	ADD_ENUM(GL_FRAMEBUFFER_UNSUPPORTED),
	// end
	ADD_ENUM(GL_NONE) };



void
gl::Context::ContextInfo::make_current() const
{
	glXMakeContextCurrent(display, drawable, read_drawable, context);
	assert(*this == get_current(display));
}

gl::Context::ContextInfo
gl::Context::ContextInfo::get_current(Display *default_display)
{
	ContextInfo ci;
	ci.display = glXGetCurrentDisplay();
	if (!ci.display) ci.display = default_display;
	ci.drawable = glXGetCurrentDrawable();
	ci.read_drawable = glXGetCurrentReadDrawable();
	ci.context = glXGetCurrentContext();
	return ci;
}

gl::Context::Context():
	display(NULL),
	config(None),
	pbuffer(None),
	context(NULL)
{
	// open display (we will use default display and screen 0)
	display = XOpenDisplay(NULL);
	context_info.display = display;

	// choose config
	assert(display);
	if (display)
	{
		int config_attribs[] = {
			//GLX_DOUBLEBUFFER,      False,
			GLX_RED_SIZE,          8,
			GLX_GREEN_SIZE,        8,
			GLX_BLUE_SIZE,         8,
			GLX_ALPHA_SIZE,        8,
			//GLX_DEPTH_SIZE,        24,
			//GLX_STENCIL_SIZE,      8,
			//GLX_ACCUM_RED_SIZE,    8,
			//GLX_ACCUM_GREEN_SIZE,  8,
			//GLX_ACCUM_BLUE_SIZE,   8,
			//GLX_ACCUM_ALPHA_SIZE,  8,
			GLX_DRAWABLE_TYPE,     GLX_PBUFFER_BIT,
			None };
		int nelements = 0;
		GLXFBConfig *configs = glXChooseFBConfig(display, 0, config_attribs, &nelements);
		if (configs != NULL && nelements > 0)
			config = configs[0];
	}

	// create pbuffer
	assert(config);
	if (config)
	{
		int pbuffer_attribs[] = {
			GLX_PBUFFER_WIDTH, 256,
			GLX_PBUFFER_HEIGHT, 256,
			None };
		pbuffer = glXCreatePbuffer(display, config, pbuffer_attribs);
		context_info.drawable = pbuffer;
		context_info.read_drawable = pbuffer;
	}

	// create context
	assert(pbuffer);
	if (pbuffer)
	{
		int context_attribs[] = {
			GLX_CONTEXT_MAJOR_VERSION_ARB, 3,
			GLX_CONTEXT_MINOR_VERSION_ARB, 3,
			None };
		GLXCREATECONTEXTATTRIBSARBPROC glXCreateContextAttribsARB = (GLXCREATECONTEXTATTRIBSARBPROC) glXGetProcAddress((const GLubyte*)"glXCreateContextAttribsARB");
		context = glXCreateContextAttribsARB(display, config, NULL, True, context_attribs);
		context_info.context = context;
	}

	assert(context);
	use();
	check();
	unuse();
}

gl::Context::~Context()
{
	if (context)
		glXDestroyContext(display, context);
	context = NULL;
	context_info.context = NULL;
	if (pbuffer)
		glXDestroyPbuffer(display, pbuffer);
	pbuffer = None;
	context_info.drawable = None;
	context_info.read_drawable = None;
	config = None;
	if (display)
		XCloseDisplay(display);
	display = NULL;
	context_info.display = None;
}

bool
gl::Context::is_current() const
{
	return is_valid()
		&& context_info == ContextInfo::get_current(display);
}

void
gl::Context::use()
{
	if (is_valid()) {
		rec_mutex.lock();
		context_stack.push_back(ContextInfo::get_current(display));
		if (context_info != context_stack.back())
			context_info.make_current();
		check("gl::Context::use");
	}
}

void
gl::Context::unuse()
{
	assert(is_current());
	if (is_valid()) {
		assert(!context_stack.empty());
		check("gl::Context::unuse");
		if (context_stack.back() != context_info)
		{
			glFinish();
			context_stack.back().make_current();
		}
		context_stack.pop_back();
		rec_mutex.unlock();
	}
}

void
gl::Context::check(const char *s)
{
	if (GLenum error = glGetError())
		warning("%s GL error: 0x%x %s", s, error, get_enum_string(error));
}

const char *
gl::Context::get_enum_string(GLenum x)
{
	for(int i = 0; i < (int)sizeof(enum_strings)/(int)sizeof(enum_strings[0]); ++i)
		if (enum_strings[i].first == x)
			return enum_strings[i].second;
	return "";
}

/* === E N T R Y P O I N T ================================================= */
