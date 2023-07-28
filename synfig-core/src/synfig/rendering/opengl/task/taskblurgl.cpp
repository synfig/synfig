/* === S Y N F I G ========================================================= */
/*!	\file synfig/rendering/software/task/taskblursw.cpp
**	\brief TaskBlurGL
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
#include "../../common/task/taskblur.h"

#include "synfig/angle.h"
#include <math.h>
#include <vector>
#endif

using namespace synfig;
using namespace rendering;

/* === M A C R O S ========================================================= */

/* === G L O B A L S ======================================================= */

/* === P R O C E D U R E S ================================================= */

/* === M E T H O D S ======================================================= */

namespace {

class TaskBlurGL:   public TaskBlur,
                    public TaskGL
{
public:
	typedef etl::handle<TaskBlurGL> Handle;
	static Token token;
	virtual Token::Handle get_token() const { return token.handle(); }

	bool blit_sub_task(gl::Framebuffer& framebuffer, const Task::Handle& sub) const
	{
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

				framebuffer.use_write(false);

				glScissor(ra.minx, ra.miny, ra.get_width(), ra.get_height());

                gl::Programs::Program shader = env().get_or_create_context().get_program("blit");
                shader.use();
                shader.set_1i("tex", 0);
                shader.set_2i("offset", oa);

				gl::Plane plane;
				plane.render();

				src.unuse();
			}
		}
		return true;
	}

	virtual bool run(RunParams&) const
	{
		if(!is_valid()) return true;

		LockWrite ldst(this);
		if(!ldst) return false;

		gl::Context::Lock lock(env().get_or_create_context());
		gl::Framebuffer& framebuffer = ldst->get_framebuffer();

		glEnable(GL_SCISSOR_TEST);

        gl::Framebuffer blitBuffer;
        blitBuffer.from_pixels(ldst->get_width(), ldst->get_height(), nullptr);

		glViewport(0, 0, ldst->get_width(), ldst->get_height());

        blitBuffer.use_write();
        // first blit sub task to a new framebuffer then
        if(!blit_sub_task(blitBuffer, sub_task()))
        {
            blitBuffer.unuse();
            return false;
        }
        blitBuffer.unuse();

        // blur the framebuffer
        framebuffer.use_write();
        blitBuffer.use_read(0);

        glScissor(target_rect.minx, target_rect.miny, target_rect.get_width(), target_rect.get_height());

		const float precision(1e-10);

        if(blur.type == Blur::Type::BOX)
        {
            Vector ppu = get_pixels_per_unit();
            Vector s = blur.size.multiply_coords(ppu) * 0.25 * sqrt(PI);

            VectorInt size;
            size[0] = 1 + floor(s[0] + precision);
            size[1] = 1 + floor(s[1] + precision);

            gl::Programs::Program shader = env().get_or_create_context().get_program("box_blur");
            assert(shader.valid);
            shader.use();
            shader.set_1i("tex", 0);
            shader.set_2i("size", size);
            shader.set_4i("rect", target_rect.minx, target_rect.maxx, target_rect.miny, target_rect.maxy);

            gl::Plane plane;
            plane.render();
        }

        blitBuffer.unuse();
		framebuffer.unuse();

		glDisable(GL_SCISSOR_TEST);
		return true;
	}
};


Task::Token TaskBlurGL::token(
	DescReal<TaskBlurGL, TaskBlur>("TaskBlurGL") );
} // end of anonimous namespace

/* === E N T R Y P O I N T ================================================= */
