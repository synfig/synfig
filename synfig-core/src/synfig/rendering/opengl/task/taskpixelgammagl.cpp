/* === S Y N F I G ========================================================= */
/*!	\file synfig/rendering/software/task/taskpixelgammagl.cpp
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

class TaskPixelGammaGL: public TaskPixelGamma, public TaskGL
{
public:
	typedef etl::handle<TaskPixelGammaGL> Handle;
	static Token token;
	virtual Token::Handle get_token() const { return token.handle(); }

	static inline ColorReal clamp_positive(const ColorReal &x)
	{
		const ColorReal max = ColorReal(1.0)/real_low_precision<ColorReal>();
		return synfig::clamp(x, real_low_precision<ColorReal>(), max);
	}

	virtual bool run(RunParams&) const
	{
		if(!is_valid()) return true;

		LockWrite ldst(this);
		if(!ldst) return false;

		gl::Context::Lock lock(env().get_or_create_context());

		glEnable(GL_SCISSOR_TEST);

		glViewport(0, 0, ldst->get_width(), ldst->get_height());
		glScissor(target_rect.minx, target_rect.miny, target_rect.get_width(), target_rect.get_height());

		gl::Framebuffer& framebuffer = ldst->get_framebuffer();

        const Task::Handle& sub = sub_task();

		RectInt r = target_rect;
		if(sub && sub->is_valid())
		{
			VectorInt oa = TaskList::calc_target_offset(*this, *sub);
			RectInt ra = sub->target_rect - oa;

			if(ra.is_valid())
			{
				rect_set_intersect(ra, ra, r);

				// blit sub_task_a in the intersection
				LockRead lsrc(sub);
				if(!lsrc) {
					return false;
				}

				gl::Framebuffer& src = lsrc.cast_handle()->get_framebuffer();
				src.use_read(0);

				framebuffer.use_write();

				glScissor(ra.minx, ra.miny, ra.get_width(), ra.get_height());

                gl::Programs::Program shader = env().get_or_create_context().get_program("blit_gamma");
                shader.use();
                shader.set_1i("tex", 0);
                shader.set_2i("offset", oa);
                shader.set_3f("gamma",
                        clamp_positive(gamma.get_r()),
                        clamp_positive(gamma.get_g()),
                        clamp_positive(gamma.get_b()));

				gl::Plane plane;
				plane.render();

				src.unuse();
			}
		}

		glDisable(GL_SCISSOR_TEST);
		return true;
	}
};


Task::Token TaskPixelGammaGL::token(
	DescReal<TaskPixelGammaGL, TaskPixelGamma>("TaskPixelGammaGL") );

} // end of anonimous namespace

/* === E N T R Y P O I N T ================================================= */
