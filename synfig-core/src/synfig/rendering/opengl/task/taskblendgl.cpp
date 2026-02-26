/* === S Y N F I G ========================================================= */
/*!	\file synfig/rendering/software/task/taskblendsw.cpp
**	\brief TaskBlendGL
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

#include "../../common/task/taskblend.h"

#endif

using namespace synfig;
using namespace rendering;

/* === M A C R O S ========================================================= */

/* === G L O B A L S ======================================================= */

/* === P R O C E D U R E S ================================================= */

/* === M E T H O D S ======================================================= */

namespace {

class TaskBlendGL: public TaskBlend,
                   public TaskGL
{
public:
	typedef etl::handle<TaskBlendGL> Handle;
	static Token token;
	virtual Token::Handle get_token() const { return token.handle(); }

	bool blit_sub_task(gl::Framebuffer& framebuffer, const Task::Handle& sub, bool use_blend, bool use_b = false) const
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

				if(!use_blend)
				{
					gl::Programs::Program shader = env().get_or_create_context().get_program("blit");
					shader.use();
					shader.set_1i("tex", 0);
					shader.set_2i("offset", oa);
				} else
				{
					gl::Programs::Program shader = env().get_or_create_context().get_blend_program(blend_method);
					shader.use();
					shader.set_1f("amount", amount);
                    if(use_b)
                    {
                        shader.set_1i("use_a", 0);
                        shader.set_1i("use_b", 1);
                        shader.set_1i("sampler_b", 0);
                        shader.set_2i("offset_b", oa);
                    } else
                    {
                        shader.set_1i("use_a", 1);
                        shader.set_1i("use_b", 0);
                        shader.set_1i("sampler_a", 0);
                        shader.set_2i("offset_a", oa);
                    }
				}

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

		glViewport(0, 0, ldst->get_width(), ldst->get_height());
		glScissor(target_rect.minx, target_rect.miny, target_rect.get_width(), target_rect.get_height());

		framebuffer.use_write();
        if(Color::is_straight(blend_method))
        {
            if(!blit_sub_task(framebuffer, sub_task_a(), true, true))
            {
                framebuffer.unuse();
                return false;
            }
        } else {
            if(!blit_sub_task(framebuffer, sub_task_a(), false))
            {
                framebuffer.unuse();
                return false;
            }
        }

		if(!blit_sub_task(framebuffer, sub_task_b(), true))
		{
			framebuffer.unuse();
			return false;
		}

		if(sub_task_a() && sub_task_a()->is_valid() &&
			sub_task_b() && sub_task_b()->is_valid())
		{
			VectorInt oa = TaskList::calc_target_offset(*this, *sub_task_a());
			RectInt ra = sub_task_a()->target_rect - oa;

			VectorInt ob = TaskList::calc_target_offset(*this, *sub_task_b());
			RectInt rb = sub_task_b()->target_rect - ob;

			if(ra.is_valid() && rb.is_valid())
			{
				RectInt r = target_rect;
				rect_set_intersect(r, r, ra);
				rect_set_intersect(r, r, rb);

                if(r.is_valid())
                {
                    LockRead lsrc_a(sub_task_a()), lsrc_b(sub_task_b());
                    if(!lsrc_a || !lsrc_b) {
                        framebuffer.unuse();
                        return false;
                    }

                    framebuffer.use_write(false);

                    glScissor(r.minx, r.miny, r.get_width(), r.get_height());

                    gl::Framebuffer& src_a = lsrc_a.cast_handle()->get_framebuffer();
                    src_a.use_read(0);

                    gl::Framebuffer& src_b = lsrc_b.cast_handle()->get_framebuffer();
                    src_b.use_read(1);

                    gl::Programs::Program shader = env().get_or_create_context().get_blend_program(blend_method);
                    shader.use();
                    shader.set_1f("amount", amount);
                    shader.set_1i("use_a", 1);
                    shader.set_1i("use_b", 1);
                    shader.set_1i("sampler_a", 1);
                    shader.set_2i("offset_a", ob);
                    shader.set_1i("sampler_b", 0);
                    shader.set_2i("offset_b", oa);

                    gl::Plane plane;
                    plane.render();

                    src_a.unuse();
                    src_b.unuse();
                }
			}
		}
		glDisable(GL_SCISSOR_TEST);

		framebuffer.unuse();

		return true;
	}
};


Task::Token TaskBlendGL::token(
	DescReal<TaskBlendGL, TaskBlend>("TaskBlendGL") );
} // end of anonimous namespace

/* === E N T R Y P O I N T ================================================= */
