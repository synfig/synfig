/* === S Y N F I G ========================================================= */
/*!	\file synfig/rendering/opengl/task/taskblendgl.cpp
**	\brief TaskBlendGL
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

#include "taskblendgl.h"

#include "../internal/environment.h"
#include "../surfacegl.h"

#endif

using namespace synfig;
using namespace rendering;

/* === M A C R O S ========================================================= */

/* === G L O B A L S ======================================================= */

/* === P R O C E D U R E S ================================================= */

/* === M E T H O D S ======================================================= */

bool
TaskBlendGL::run(RunParams & /* params */) const
{
	gl::Context::Lock(env().context);

	SurfaceGL::Handle a =
		SurfaceGL::Handle::cast_dynamic(sub_task_a()->target_surface);
	SurfaceGL::Handle b =
		SurfaceGL::Handle::cast_dynamic(sub_task_b()->target_surface);
	SurfaceGL::Handle target =
		SurfaceGL::Handle::cast_dynamic(target_surface);

	gl::Framebuffers::FramebufferLock framebuffer = env().framebuffers.get_framebuffer();
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, framebuffer.get_id());
	glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, target->get_id(), 0);
	glViewport(0, 0, target->get_width(), target->get_height());
	env().context.check();

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, a->get_id());
	glBindSampler(0, env().misc.get_sampler_nearest());
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, b->get_id());
	glBindSampler(1, env().misc.get_sampler_nearest());
	env().context.check();

	gl::Buffers::BufferLock quad_buf = env().buffers.get_default_quad_buffer();
	gl::Buffers::VertexArrayLock quad_va = env().buffers.get_vertex_array();
	env().context.check();

	glBindVertexArray(quad_va.get_id());
	glEnableVertexAttribArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, quad_buf.get_id());
	glVertexAttribPointer(0, 2, GL_DOUBLE, GL_TRUE, 0, quad_buf.get_pointer());
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	env().context.check();

	env().shaders.blend(blend_method, amount);
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
	env().context.check();

	glDisableVertexAttribArray(0);
	glBindVertexArray(0);
	env().context.check();

	glActiveTexture(GL_TEXTURE1);
	glBindSampler(1, 0);
	glBindTexture(GL_TEXTURE_2D, 0);
	glActiveTexture(GL_TEXTURE0);
	glBindSampler(0, 0);
	glBindTexture(GL_TEXTURE_2D, 0);
	env().context.check();

	glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, 0, 0);
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
	env().context.check();

	return true;
}

/* === E N T R Y P O I N T ================================================= */
