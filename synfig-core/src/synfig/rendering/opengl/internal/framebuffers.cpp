/* === S Y N F I G ========================================================= */
/*!	\file synfig/rendering/opengl/internal/framebuffers.cpp
**	\brief Framebuffers
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

#include "framebuffers.h"

#endif

using namespace synfig;
using namespace rendering;

/* === M A C R O S ========================================================= */

/* === G L O B A L S ======================================================= */

/* === P R O C E D U R E S ================================================= */

/* === M E T H O D S ======================================================= */

gl::Framebuffers::Framebuffers(Context &context):
	context(context)
{ }

gl::Framebuffers::~Framebuffers()
{
	Context::Lock lock(context);

	// delete framebuffers
	for(std::list<Framebuffer>::iterator i = framebuffers.begin(); i != framebuffers.end(); ++i)
	{
		assert(!i->locks);
		glDeleteFramebuffers(1, &i->id);
	}

	// delete renderbuffers
	for(std::list<Renderbuffer>::iterator i = renderbuffers.begin(); i != renderbuffers.end(); ++i)
	{
		assert(!i->locks);
		glDeleteRenderbuffers(1, &i->id);
	}
}

gl::Framebuffers::RenderbufferLock
gl::Framebuffers::get_renderbuffer(GLenum internalformat, int width, int height)
{
	for(std::list<Renderbuffer>::iterator i = renderbuffers.begin(); i != renderbuffers.end(); ++i)
		if ( !i->locks
		  && i->internalformat == internalformat
		  && i->width >= width
		  && i->height >= height )
			return RenderbufferLock(&*i);

	Context::Lock lock(context);
	renderbuffers.push_back(Renderbuffer());
	Renderbuffer *i = &renderbuffers.back();
	i->internalformat = internalformat;
	i->width = width;
	i->height = height;
	glGenRenderbuffers(1, &i->id);
	glBindRenderbuffer(GL_RENDERBUFFER, i->id);
	glRenderbufferStorage(GL_RENDERBUFFER, i->internalformat, i->width, i->height);
	glBindRenderbuffer(GL_RENDERBUFFER, 0);
	return RenderbufferLock(i);
}

gl::Framebuffers::FramebufferLock
gl::Framebuffers::get_framebuffer()
{
	for(std::list<Framebuffer>::iterator i = framebuffers.begin(); i != framebuffers.end(); ++i)
		if (!i->locks)
			return FramebufferLock(&*i);

	Context::Lock lock(context);
	framebuffers.push_back(Framebuffer());
	Framebuffer *i = &framebuffers.back();
	glGenFramebuffers(1, &i->id);
	return FramebufferLock(i);
}

void
gl::Framebuffers::check(const char *s) {
	context.check(s);

	GLuint id = 0;
	glGetIntegerv(GL_DRAW_FRAMEBUFFER_BINDING, (GLint*)&id);
	if (id)
	{
		GLenum status = glCheckFramebufferStatus(GL_DRAW_FRAMEBUFFER);
		if (status != GL_FRAMEBUFFER_COMPLETE)
			warning("%s GL wrong draw framebuffer status: 0x%x %s id %d", s, status, context.get_enum_string(status), id);
	}

	id = 0;
	glGetIntegerv(GL_READ_FRAMEBUFFER_BINDING, (GLint*)&id);
	if (id)
	{
		GLenum status = glCheckFramebufferStatus(GL_READ_FRAMEBUFFER);
		if (status != GL_FRAMEBUFFER_COMPLETE)
			warning("%s GL wrong read framebuffer status: 0x%x %s id %d", s, status, context.get_enum_string(status), id);
	}
}

/* === E N T R Y P O I N T ================================================= */
