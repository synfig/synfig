/* === S Y N F I G ========================================================= */
/*!	\file synfig/rendering/software/task/taskpixelcolormatrixsw.cpp
**	\brief TaskPixelColorMatrixGL
**
**	\legal
**	......... ... 2023 Bharat Sahlot
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

#include <vector>
#ifdef USING_PCH
#	include "pch.h"
#else
#ifdef HAVE_CONFIG_H
#	include <config.h>
#endif

#include "taskgl.h"

#include "synfig/general.h"
#include "../surfacegl.h"
#include "../internal/headers.h"

#include "../internal/context.h"
#include "../internal/environment.h"
#include "../internal/shaders.h"
#include "../internal/plane.h"

#include "../../common/task/taskpixelprocessor.h"

#endif

using namespace synfig;
using namespace rendering;

/* === M A C R O S ========================================================= */

/* === G L O B A L S ======================================================= */

/* === P R O C E D U R E S ================================================= */

/* === M E T H O D S ======================================================= */

namespace {

class TaskPixelColorMatrixGL: public TaskPixelColorMatrix, public TaskGL
{
public:
	typedef etl::handle<TaskPixelColorMatrixGL> Handle;
	static Token token;
	virtual Token::Handle get_token() const { return token.handle(); }

	virtual bool run(RunParams&) const
	{
		if(!is_valid()) return true;

		LockWrite ldst(this);
		if(!ldst) return false;

		gl::Context::Lock lock(env().get_or_create_context());

		if(is_constant())
		{
			gl::Framebuffer& framebuffer = ldst->get_framebuffer();

			framebuffer.use_write();
			glViewport(0, 0, ldst->get_width(), ldst->get_height());

			const Color col = matrix.get_constant();

			glClearColor(col.get_r(), col.get_g(), col.get_b(), col.get_a());
			glClear(GL_COLOR_BUFFER_BIT);

			framebuffer.unuse();
			return true;
		}

		if(sub_task() && sub_task()->is_valid())
		{
			// NOTE: for some reason this resets the current framebuffer
			LockRead lsrc(sub_task());
			if(!lsrc) {
				return false;
			}

			gl::Framebuffer& framebuffer = ldst->get_framebuffer();

			framebuffer.use_write();
			glViewport(0, 0, ldst->get_width(), ldst->get_height());

			const Color col = matrix.get_constant();

			glClearColor(col.get_r(), col.get_g(), col.get_b(), col.get_a());
			glClear(GL_COLOR_BUFFER_BIT);

			gl::Framebuffer& src = lsrc.cast_handle()->get_framebuffer();
			src.use_read(0);

			gl::Programs::Program shader = env().get_or_create_context().get_program("colormatrix");
			shader.use();
			shader.set_1i("tex", 0);
			shader.set_mat5x5("mat", matrix);

			gl::Plane plane;
			plane.render();

			src.unuse();
			framebuffer.unuse();
		}
		return true;
	}
};


Task::Token TaskPixelColorMatrixGL::token(
	DescReal<TaskPixelColorMatrixGL, TaskPixelColorMatrix>("PixelColorMatrixGL") );

} // end of anonimous namespace

/* === E N T R Y P O I N T ================================================= */
