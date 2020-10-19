/* === S Y N F I G ========================================================= */
/*!	\file synfig/rendering/opengl/internal/antialiasing.cpp
**	\brief Antialiasing
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

#include <cstring>

#include <synfig/general.h>
#include <synfig/localization.h>

#include "antialiasing.h"

#endif

using namespace synfig;
using namespace rendering;

/* === M A C R O S ========================================================= */

/* === G L O B A L S ======================================================= */

/* === P R O C E D U R E S ================================================= */

/* === M E T H O D S ======================================================= */

gl::Antialiasing::Antialiasing(Context &context):
	context(context),
	allowed(false),
	multisample_max_width(),
	multisample_max_height(),
	multisample_texture_id(),
	multisample_renderbuffer_id(),
	multisample_framebuffer_id(),
	multisample_orig_draw_framebuffer_id()
{
	memset(multisample_viewport, 0, sizeof(multisample_viewport));
	memset(multisample_orig_viewport, 0, sizeof(multisample_orig_viewport));

	Context::Lock lock(context);

	// options

	multisample_max_width = 1024;
	multisample_max_height = 1024;
	int multisample_samples = 8;
	bool multisample_hdr = true;
	GLenum multisample_internal_format = multisample_hdr ? GL_RGBA16F : GL_RGBA;

	// multisample

	GLuint prev_draw_framebuffer_id = 0;
	glGetIntegerv(GL_DRAW_FRAMEBUFFER_BINDING, (GLint*)&prev_draw_framebuffer_id);

	glGenTextures(1, &multisample_texture_id);
	glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, multisample_texture_id);
	glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, multisample_samples, multisample_internal_format, multisample_max_width, multisample_max_height, GL_TRUE);
	glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, 0);

	glGenRenderbuffers(1, &multisample_renderbuffer_id);
	glBindRenderbuffer(GL_RENDERBUFFER, multisample_renderbuffer_id);
	glRenderbufferStorageMultisample(GL_RENDERBUFFER, multisample_samples, GL_STENCIL_INDEX8, multisample_max_width, multisample_max_height);
	glBindRenderbuffer(GL_RENDERBUFFER, 0);

	glGenFramebuffers(1, &multisample_framebuffer_id);
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, multisample_framebuffer_id);
	glFramebufferRenderbuffer(GL_DRAW_FRAMEBUFFER, GL_STENCIL_ATTACHMENT, GL_RENDERBUFFER, multisample_renderbuffer_id);
	glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D_MULTISAMPLE, multisample_texture_id, 0);

	GLenum multisample_status = glCheckFramebufferStatus(GL_DRAW_FRAMEBUFFER);
	if (multisample_status != GL_FRAMEBUFFER_COMPLETE)
		warning("multisample framebuffer incomplete 0x%x", multisample_status);
	else
		allowed = true;

	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, prev_draw_framebuffer_id);

	context.check();
}

gl::Antialiasing::~Antialiasing()
{
	Context::Lock lock(context);
	glDeleteFramebuffers(1, &multisample_framebuffer_id);
	glDeleteRenderbuffers(1, &multisample_renderbuffer_id);
	glDeleteTextures(1, &multisample_texture_id);
}

void
gl::Antialiasing::multisample_begin(bool clear)
{
	Context::Lock lock(context);
	if (is_allowed()) {
		glGetIntegerv(GL_VIEWPORT, multisample_orig_viewport);
		multisample_viewport[2] = std::min(multisample_orig_viewport[2], multisample_max_width);
		multisample_viewport[3] = std::min(multisample_orig_viewport[3], multisample_max_height);
		glGetIntegerv(GL_DRAW_FRAMEBUFFER_BINDING, (GLint*)&multisample_orig_draw_framebuffer_id);
		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, multisample_framebuffer_id);
		glViewport(
			multisample_viewport[0],
			multisample_viewport[1],
			multisample_viewport[2],
			multisample_viewport[3] );
	}
	if (clear) glClear(GL_COLOR_BUFFER_BIT);
}

void
gl::Antialiasing::multisample_end()
{
	Context::Lock lock(context);
	if (is_allowed()) {
		GLuint prev_read_framebuffer_id = 0;
		glGetIntegerv(GL_READ_FRAMEBUFFER_BINDING, (GLint*)&prev_read_framebuffer_id);
		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, (GLuint)multisample_orig_draw_framebuffer_id);
		glBindFramebuffer(GL_READ_FRAMEBUFFER, (GLuint)multisample_framebuffer_id);
		glBlitFramebuffer(
			multisample_viewport[0],
			multisample_viewport[1],
			multisample_viewport[2],
			multisample_viewport[3],
			multisample_orig_viewport[0],
			multisample_orig_viewport[1],
			multisample_orig_viewport[2],
			multisample_orig_viewport[3],
			GL_COLOR_BUFFER_BIT, GL_LINEAR );
		glBindFramebuffer(GL_READ_FRAMEBUFFER, prev_read_framebuffer_id);
		glViewport(
			multisample_orig_viewport[0],
			multisample_orig_viewport[1],
			multisample_orig_viewport[2],
			multisample_orig_viewport[3] );
	}
}

/* === E N T R Y P O I N T ================================================= */
