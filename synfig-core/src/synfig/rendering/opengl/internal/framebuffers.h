/* === S Y N F I G ========================================================= */
/*!	\file synfig/rendering/opengl/internal/framebuffers.h
**	\brief Framebuffers Header
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

#ifndef __SYNFIG_RENDERING_GL_FRAMEBUFFERS_H
#define __SYNFIG_RENDERING_GL_FRAMEBUFFERS_H

/* === H E A D E R S ======================================================= */

#include <cassert>

#include <list>

#include "context.h"
#ifdef WITH_OPENCL
#include "clcontext.h"
#endif
#include "buffers.h"
#include "shaders.h"
#include "antialiasing.h"

/* === M A C R O S ========================================================= */

/* === T Y P E D E F S ===================================================== */

/* === C L A S S E S & S T R U C T S ======================================= */

namespace synfig
{
namespace rendering
{
namespace gl
{

class Framebuffers
{
public:
	class Renderbuffer {
	public:
		GLuint id;
		GLenum internalformat;
		int width;
		int height;
		int locks;
	};

	class RenderbufferLock {
	private:
		friend class Framebuffers;
		Renderbuffer *renderbuffer;

		void set(Renderbuffer *renderbuffer) {
			if (this->renderbuffer) --this->renderbuffer->locks;
			this->renderbuffer = renderbuffer;
			if (this->renderbuffer) ++this->renderbuffer->locks;
		}

		RenderbufferLock(Renderbuffer *renderbuffer): renderbuffer()
			{ set(renderbuffer); }
	public:
		RenderbufferLock(): renderbuffer()
			{ }
		RenderbufferLock(const RenderbufferLock &other): renderbuffer()
			{ *this = other; }
		~RenderbufferLock()
			{ set(NULL); }
		RenderbufferLock& operator = (const RenderbufferLock &other)
			{ set(other.renderbuffer); return *this; }
		GLuint get_id() const
			{ return renderbuffer ? renderbuffer->id : 0; }
		GLenum get_internalformat() const
			{ return renderbuffer ? renderbuffer->internalformat : 0; }
		int get_width() const
			{ return renderbuffer ? renderbuffer->width : 0; }
		int get_height() const
			{ return renderbuffer ? renderbuffer->height : 0; }
	};

	class Framebuffer {
	public:
		GLuint id;
		int locks;
		Framebuffer(): id(), locks() { }
	};

	class FramebufferLock {
	private:
		friend class Framebuffers;
		Framebuffer *framebuffer;

		void set(Framebuffer *framebuffer) {
			if (this->framebuffer) --this->framebuffer->locks;
			this->framebuffer = framebuffer;
			if (this->framebuffer) ++this->framebuffer->locks;
		}

		FramebufferLock(Framebuffer *framebuffer): framebuffer()
			{ set(framebuffer); }
	public:
		FramebufferLock(): framebuffer()
			{ }
		FramebufferLock(const FramebufferLock &other): framebuffer()
			{ *this = other; }
		~FramebufferLock()
			{ set(NULL); }
		FramebufferLock& operator = (const FramebufferLock &other)
			{ set(other.framebuffer); return *this; }
		GLuint get_id() const
			{ return framebuffer ? framebuffer->id : 0; }
	};

public:
	Context &context;

private:
	std::list<Renderbuffer> renderbuffers;
	std::list<Framebuffer> framebuffers;

public:
	Framebuffers(Context &context);
	~Framebuffers();

	RenderbufferLock get_renderbuffer(GLenum internalformat, int width, int height);
	FramebufferLock get_framebuffer();

	void check(const char *s = "");
};

}; /* end namespace gl */
}; /* end namespace rendering */
}; /* end namespace synfig */

/* -- E N D ----------------------------------------------------------------- */

#endif
