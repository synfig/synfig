/* === S Y N F I G ========================================================= */
/*!	\file synfig/rendering/opengl/task/tasktransfromationaffinegl.cpp
**	\brief TaskTransformationAffineGL
**
**	$Id$
**
**	\legal
**	......... ... 2015-2018 Ivan Mahonin
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

#include <synfig/general.h>
#include <synfig/localization.h>

#include "../../common/task/tasktransformation.h"
#include "taskgl.h"
#include "../internal/environment.h"

#endif

using namespace synfig;
using namespace rendering;

/* === M A C R O S ========================================================= */

/* === G L O B A L S ======================================================= */

/* === P R O C E D U R E S ================================================= */

/* === M E T H O D S ======================================================= */

namespace {

class TaskTransformationAffineGL: public TaskTransformationAffine, public TaskGL
{
public:
	typedef std::shared_ptr<TaskTransformationAffineGL> Handle;
	static Token token;
	virtual Token::Handle get_token() const { return token.handle(); }

	virtual bool run(RunParams &params) const {
		// TODO: remove antialiasing

		if (!is_valid() || !sub_task() || !sub_task()->is_valid())
			return true;

		gl::Context::Lock lock(env().context);

		Vector rect_size = source_rect.get_size();
		Matrix bounds_transfromation;
		bounds_transfromation.m00 = approximate_equal(rect_size[0], 0.0) ? 0.0 : 2.0/rect_size[0];
		bounds_transfromation.m11 = approximate_equal(rect_size[1], 0.0) ? 0.0 : 2.0/rect_size[1];
		bounds_transfromation.m20 = -1.0 - source_rect.minx * bounds_transfromation.m00;
		bounds_transfromation.m21 = -1.0 - source_rect.miny * bounds_transfromation.m11;

		Matrix matrix = bounds_transfromation * transformation->matrix;

		// prepare arrays
		Vector k(target_surface->get_width(), target_surface->get_height());
		Vector d( matrix.axis_x().multiply_coords(k).norm().divide_coords(k).mag() / matrix.axis_x().mag(),
				  matrix.axis_y().multiply_coords(k).norm().divide_coords(k).mag() / matrix.axis_y().mag() );
		d *= 4.0;
		Vector coords[4][3];
		for(int i = 0; i < 4; ++i)
		{
			coords[i][2] = Vector(i%2 ? 1.0 : -1.0, i/2 ? 1.0 : -1.0).multiply_coords(Vector(1.0, 1.0) + d);
			coords[i][0] = matrix.get_transformed(coords[i][2]*0.5 + Vector(0.5, 0.5));
			coords[i][1] = (coords[i][2] + Vector(1.0, 1.0))*0.5;
		}
		Vector aascale = d.one_divide_coords();

		LockWrite ldst(this);
		if (!ldst)
			return false;

		gl::Framebuffers::FramebufferLock framebuffer = env().framebuffers.get_framebuffer();
		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, framebuffer.get_id());
		glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, ldst->get_id(), 0);
		glViewport(
			target_rect.minx,
			target_rect.miny,
			target_rect.get_width(),
			target_rect.get_height() );
		env().context.check();

		LockRead lsrc(sub_task());
		if (!lsrc)
			return false;

		glBindTexture(GL_TEXTURE_2D, lsrc->get_id());
		glBindSampler(0, env().samplers.get_interpolation(interpolation));
		env().context.check();

		gl::Buffers::BufferLock buf = env().buffers.get_array_buffer(coords);
		gl::Buffers::VertexArrayLock va = env().buffers.get_vertex_array();
		env().context.check();

		glBindVertexArray(va.get_id());
		glBindBuffer(GL_ARRAY_BUFFER, buf.get_id());
		glEnableVertexAttribArray(0);
		glEnableVertexAttribArray(1);
		glEnableVertexAttribArray(2);
		glVertexAttribPointer(0, 2, GL_DOUBLE, GL_TRUE, sizeof(coords[0]), (const char*)buf.get_pointer() + 0*sizeof(coords[0][0]));
		glVertexAttribPointer(1, 2, GL_DOUBLE, GL_TRUE, sizeof(coords[0]), (const char*)buf.get_pointer() + 1*sizeof(coords[0][0]));
		glVertexAttribPointer(2, 2, GL_DOUBLE, GL_TRUE, sizeof(coords[0]), (const char*)buf.get_pointer() + 2*sizeof(coords[0][0]));
		glBindBuffer(GL_ARRAY_BUFFER, 0);
		env().context.check();

		env().shaders.antialiased_textured_rect(interpolation, aascale);
		glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
		env().context.check();

		glDisableVertexAttribArray(0);
		glDisableVertexAttribArray(1);
		glDisableVertexAttribArray(2);
		glBindVertexArray(0);
		env().context.check();

		glBindSampler(0, 0);
		glBindTexture(GL_TEXTURE_2D, 0);
		env().context.check();

		glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, 0, 0);
		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
		env().context.check();

		return true;
	}
};


Task::Token TaskTransformationAffineGL::token(
	DescReal< TaskTransformationAffineGL,
		      TaskTransformationAffine >
			    ("TransformationAffineGL") );

} // end of anonimous namespace

/* === E N T R Y P O I N T ================================================= */
