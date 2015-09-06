/* === S Y N F I G ========================================================= */
/*!	\file synfig/rendering/opengl/surfacegl.cpp
**	\brief SurfaceGL
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

#ifndef WIN32
#include <unistd.h>
#include <sys/types.h>
#include <signal.h>
#endif

#include "surfacegl.h"

#include "internal/environment.h"

#endif

using namespace synfig;
using namespace rendering;

/* === M A C R O S ========================================================= */

/* === G L O B A L S ======================================================= */

/* === P R O C E D U R E S ================================================= */

/* === M E T H O D S ======================================================= */

SurfaceGL::SurfaceGL(): id()
	{ }

SurfaceGL::~SurfaceGL()
	{ if (id) SurfaceGL::destroy_vfunc(); }

gl::Environment& SurfaceGL::env() const
	{ return gl::Environment::get_instance(); }

bool
SurfaceGL::create_vfunc()
{
	gl::Context::Lock lock(env().context);
	glGenTextures(1, &id);
	glBindTexture(GL_TEXTURE_2D, id);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, get_width(), get_height(), 0, GL_RGBA, GL_FLOAT, NULL);

	{
		// clear texture
		gl::Framebuffers::FramebufferLock framebuffer = env().framebuffers.get_framebuffer();
		glBindFramebuffer(GL_DRAW_BUFFER, framebuffer.get_id());
		glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, id, 0);
		glClear(GL_COLOR_BUFFER_BIT);
		glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, 0, 0);
		glBindFramebuffer(GL_DRAW_BUFFER, 0);
	}

	return true;
}

bool
SurfaceGL::assign_vfunc(const rendering::Surface &surface)
{
	std::vector<char> data(surface.get_buffer_size());
	surface.get_pixels((Color*)&data.front());

	gl::Context::Lock lock(env().context);
	glGenTextures(1, &id);
	glBindTexture(GL_TEXTURE_2D, id);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, get_width(), get_height(), 0, GL_RGBA, GL_FLOAT, &data.front());

	return true;
}

void
SurfaceGL::destroy_vfunc()
{
	glDeleteTextures(1, &id);
	id = 0;
}

bool
SurfaceGL::get_pixels_vfunc(Color *buffer) const
{
	gl::Framebuffers::FramebufferLock framebuffer = env().framebuffers.get_framebuffer();
	glBindFramebuffer(GL_READ_BUFFER, framebuffer.get_id());
	glFramebufferTexture2D(GL_READ_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, id, 0);
	glReadPixels(0, 0, get_width(), get_height(), GL_RGBA, GL_FLOAT, buffer);
	glFramebufferTexture2D(GL_READ_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, 0, 0);
	glBindFramebuffer(GL_READ_BUFFER, 0);
	return true;
}

/* === E N T R Y P O I N T ================================================= */
