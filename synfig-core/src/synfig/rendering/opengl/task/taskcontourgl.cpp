/* === S Y N F I G ========================================================= */
/*!	\file synfig/rendering/opengl/task/taskcontourgl.cpp
**	\brief TaskContourGL
**
**	$Id$
**
**	\legal
**	......... ... 2015-2018 Ivan Mahonin
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

#include "taskcontourgl.h"

#include "../internal/environment.h"
#include "../surfacegl.h"

#endif

using namespace synfig;
using namespace rendering;

/* === M A C R O S ========================================================= */

/* === G L O B A L S ======================================================= */

/* === P R O C E D U R E S ================================================= */

/* === M E T H O D S ======================================================= */


Task::Token TaskContourGL::token(
	DescReal<TaskContourGL, TaskContour>("ContourGL") );


void
TaskContourGL::render_polygon(
	const std::vector<Vector> &polygon,
	const Rect &bounds,
	bool invert,
	bool antialias,
	Contour::WindingStyle winding_style,
	const Color &color )
{
	if (polygon.empty()) return;
	gl::Environment &e = gl::Environment::get_instance();

	if (antialias) e.antialiasing.multisample_begin(false);

	glClearColor(color.get_r(), color.get_g(), color.get_b(), 0.f);
	glClear(GL_COLOR_BUFFER_BIT);
	glClearColor(0.f, 0.f, 0.f, 0.f);

	gl::Buffers::BufferLock quad_buf = e.buffers.get_default_quad_buffer();
	gl::Buffers::VertexArrayLock quad_va = e.buffers.get_vertex_array();

	gl::Buffers::BufferLock polygon_buf = e.buffers.get_array_buffer(polygon);
	gl::Buffers::VertexArrayLock polygon_va = e.buffers.get_vertex_array();

	bool even_odd = winding_style == Contour::WINDING_EVEN_ODD;

	GLint vp[4] = { };
	glGetIntegerv(GL_VIEWPORT, vp);
	int x0 = (int)floor(vp[0] + (bounds.minx + 1.0)*0.5*vp[2]);
	int x1 = (int)ceil (vp[0] + (bounds.maxx + 1.0)*0.5*vp[2]);
	int y0 = (int)floor(vp[1] + (bounds.miny + 1.0)*0.5*vp[3]);
	int y1 = (int)ceil (vp[1] + (bounds.maxy + 1.0)*0.5*vp[3]);
	glScissor(x0, y0, x1-x0, y1-y0);

	glEnable(GL_SCISSOR_TEST);
	glEnable(GL_STENCIL_TEST);

	// render mask

	glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
	glClear(GL_STENCIL_BUFFER_BIT);
	glStencilFunc(GL_ALWAYS, 0, 0);
	if (even_odd) {
		glStencilOp(GL_INCR_WRAP, GL_INCR_WRAP, GL_INCR_WRAP);
	} else {
		glStencilOpSeparate(GL_FRONT, GL_INCR_WRAP, GL_INCR_WRAP, GL_INCR_WRAP);
		glStencilOpSeparate(GL_BACK, GL_DECR_WRAP, GL_DECR_WRAP, GL_DECR_WRAP);
	}

	glBindVertexArray(polygon_va.get_id());
	glEnableVertexAttribArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, polygon_buf.get_id());
	glVertexAttribPointer(0, 2, GL_DOUBLE, GL_TRUE, 0, polygon_buf.get_pointer());
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	e.shaders.simple();
	glDrawArrays(GL_TRIANGLE_FAN, 0, polygon.size());

	glDisableVertexAttribArray(0);
	glBindVertexArray(0);

	// fill mask

	glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);
	if (!even_odd && !invert)
		glStencilFunc(GL_NOTEQUAL, 0, -1);
	if (!even_odd &&  invert)
		glStencilFunc(GL_EQUAL, 0, -1);
	if ( even_odd && !invert)
		glStencilFunc(GL_EQUAL, 1, 1);
	if ( even_odd &&  invert)
		glStencilFunc(GL_EQUAL, 0, 1);

	glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_TRUE);

	glBindVertexArray(quad_va.get_id());
	glEnableVertexAttribArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, quad_buf.get_id());
	glVertexAttribPointer(0, 2, GL_DOUBLE, GL_TRUE, 0, quad_buf.get_pointer());
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	e.shaders.color(color);
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

	glDisableVertexAttribArray(0);
	glBindVertexArray(0);

	glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
	glDisable(GL_STENCIL_TEST);
	glDisable(GL_SCISSOR_TEST);

	if (antialias) e.antialiasing.multisample_end();
}

void
TaskContourGL::render_contour(
	const Contour &contour,
	const Matrix &transform_matrix,
	bool invert,
	bool antialias,
	Contour::WindingStyle winding_style,
	const Color &color,
	Real detail )
{
	GLint vp[4] = { };
	glGetIntegerv(GL_VIEWPORT, vp);
	if (!vp[2] || !vp[3]) return;

	std::vector<Vector> polygon;
	Rect full_bounds(-1.0, -1.0, 1.0, 1.0);
	Rect bounds = full_bounds;
	Vector pixel_size(2.0/(Real)vp[2], 2.0/(Real)vp[3]);

	contour.split(polygon, bounds, transform_matrix, pixel_size*detail);

	if (invert) bounds = full_bounds;
	render_polygon(polygon, bounds, invert, antialias, winding_style, color);
}

bool
TaskContourGL::run(RunParams & /* params */) const
{
	if (!is_valid())
		return true;

	// transformation

	Vector rect_size = source_rect.get_size();
	Matrix bounds_transfromation;
	bounds_transfromation.m00 = 2.0/rect_size[0];
	bounds_transfromation.m11 = 2.0/rect_size[1];
	bounds_transfromation.m20 = -1.0 - source_rect.minx*bounds_transfromation.m00;
	bounds_transfromation.m21 = -1.0 - source_rect.miny*bounds_transfromation.m11;

	Matrix matrix = bounds_transfromation * transformation->matrix;

	// apply bounds

	std::vector<Vector> polygon;
	Rect bounds(-1.0, -1.0, 1.0, 1.0);
	Vector pixel_size(
		2.0/(Real)target_rect.get_width(),
		2.0/(Real)target_rect.get_height() );
	contour->split(polygon, bounds, matrix, pixel_size*detail);

	// lock resources

	gl::Context::Lock lock(env().context);
	LockWrite la(this);
	if (!la)
		return false;

	// bind framebuffer

	gl::Framebuffers::RenderbufferLock renderbuffer = env().framebuffers.get_renderbuffer(GL_STENCIL_INDEX8, la->get_width(), la->get_height());
	gl::Framebuffers::FramebufferLock framebuffer = env().framebuffers.get_framebuffer();
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, framebuffer.get_id());
	glFramebufferRenderbuffer(GL_DRAW_FRAMEBUFFER, GL_STENCIL_ATTACHMENT, GL_RENDERBUFFER, renderbuffer.get_id());
	glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, la->get_id(), 0);
	env().framebuffers.check("TaskContourGL::run bind framebuffer");
	glViewport(
		target_rect.minx,
		target_rect.miny,
		target_rect.get_width(),
		target_rect.get_height() );
	env().context.check("TaskContourGL::run viewport");

	// render

	render_polygon(
		polygon,
		bounds,
		contour->invert,
		allow_antialias && contour->antialias,
		contour->winding_style,
		contour->color );

	// release framebuffer

	glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, 0, 0);
	glFramebufferRenderbuffer(GL_DRAW_FRAMEBUFFER, GL_STENCIL_ATTACHMENT, GL_RENDERBUFFER, 0);
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
	env().context.check("TaskContourGL::run release contour");

	return true;
}

/* === E N T R Y P O I N T ================================================= */
