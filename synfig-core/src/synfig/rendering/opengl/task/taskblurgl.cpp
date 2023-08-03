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

#include <algorithm>
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

#include "../../software/function/blur.h"

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

    void blit_framebuffer(gl::Framebuffer& src, gl::Framebuffer& dest, VectorInt offset, bool mul_alpha, bool inverse_alpha) const
    {
        glDisable(GL_SCISSOR_TEST);

        src.use_read(0);
        dest.use_write(true);

        glViewport(0, 0, dest.get_w(), dest.get_h());

        gl::Programs::Program shader = env().get_or_create_context().get_program(mul_alpha ? "blit_alpha" : "blit");
        shader.use();
        shader.set_1i("tex", 0);
        shader.set_2i("offset", offset);
        if(mul_alpha) shader.set_1i("inverse_alpha", inverse_alpha);

        gl::Plane plane;
        plane.render();

        src.unuse();
        dest.unuse();
    }

	virtual bool run(RunParams&) const
	{
		if(!is_valid()) return true;

		LockWrite ldst(this);
		if(!ldst) return false;

		gl::Context::Lock lock(env().get_or_create_context());
        LockRead lsrc(sub_task());
        if(!lsrc) {
            return false;
        }

        gl::Framebuffer destBuf = ldst->get_framebuffer();
        gl::Framebuffer srcBuf = lsrc.cast_handle()->get_framebuffer();

        gl::Plane plane;

        Vector ppu = get_pixels_per_unit();
        Vector orig_size = blur.size.multiply_coords(ppu);

        Vector s = orig_size * software::Blur::get_size_amplifier(blur.type);
        if(blur.type == Blur::FASTGAUSSIAN)
        {
            s *= 1.662155813;
        }

        VectorInt extraSize = software::Blur::get_extra_size(blur.type, orig_size);

        VectorInt size;
        size[0] = round(s[0]);
        size[1] = round(s[1]);

        VectorInt offset = TaskList::calc_target_offset(*this, *sub_task());
        offset += target_rect.get_min();

        RectInt blitRect, srcRect, destRect;
        blitRect = lsrc.rect;

        destRect = target_rect;
        rect_set_intersect(destRect, destRect, RectInt(0, 0, destBuf.get_w(), destBuf.get_h()));
        if(!destRect.valid()) return false;

        srcRect = destRect + offset;
        srcRect.minx -= extraSize[0];
        srcRect.miny -= extraSize[1];
        srcRect.maxx += extraSize[0];
        srcRect.maxy += extraSize[1];
        if(!srcRect.valid()) return false;
        rect_set_intersect(srcRect, srcRect, RectInt(0, 0, srcBuf.get_w(), srcBuf.get_h()));
        if(!srcRect.valid()) return false;

        destRect = srcRect - offset;
        destRect.minx += extraSize[0];
        destRect.miny += extraSize[1];
        destRect.maxx -= extraSize[0];
        destRect.maxy -= extraSize[1];
        if(!destRect.valid()) return false;
        if(!rect_contains(RectInt(0, 0, destBuf.get_w(), destBuf.get_h()), destRect)) return false;

        offset = srcRect.get_min() + extraSize;

        gl::Framebuffer fbos[2];
        for(int i = 0; i < 2; i++) {
            fbos[i].from_dims(srcBuf.get_w(), srcBuf.get_h());
            fbos[i].clear();
        }

        gl::Programs::Program shader;
        int count = 1;
        bool separable = true;
        bool floatSize = false;

        switch (blur.type) {
            case Blur::BOX:
                shader = env().get_or_create_context().get_program("box_blur");
                break;
            case Blur::DISC:
                separable = false;
                floatSize = true;
                shader = env().get_or_create_context().get_program("disc_blur");
                break;
            case Blur::CROSS:
                separable = false;
                shader = env().get_or_create_context().get_program("cross_blur");
                break;
            case Blur::FASTGAUSSIAN:
                count = 2;
                shader = env().get_or_create_context().get_program("box_blur");
                break;
            case Blur::GAUSSIAN:
                floatSize = true;
                shader = env().get_or_create_context().get_program("gauss_blur");
                break;
        }
        assert(shader.valid);
        shader.use();
        shader.set_1i("tex", 0);
        if(floatSize) shader.set_2f("size", s);
        else shader.set_2i("size", size);

        if(blur.type == Blur::GAUSSIAN) shader.set_2i("psize", extraSize);

		glDisable(GL_SCISSOR_TEST);

        glViewport(0, 0, srcBuf.get_w(), srcBuf.get_h());

        blit_framebuffer(srcBuf, fbos[0], VectorInt(0, 0), true, false);

        int lastBuf = 0, curBuf = 1;

        bool horizontal = !separable;
        for(int j = 0; j < separable + 1; j++)
        {
            for(int i = 0; i < count; i++)
            {
                // blit to fill the whole buffer and then blur the inner rect
                blit_framebuffer(fbos[lastBuf], fbos[curBuf], VectorInt(0, 0), false, false);

                shader.use();

                if(separable) shader.set_1i("horizontal", horizontal);

                glEnable(GL_SCISSOR_TEST);
                glScissor(srcRect.minx, srcRect.miny, srcRect.get_width(), srcRect.get_height());

                fbos[curBuf].use_write();
                fbos[lastBuf].use_read(0);

                plane.render();

                fbos[curBuf].unuse();
                fbos[lastBuf].unuse();

                glDisable(GL_SCISSOR_TEST);

                lastBuf = curBuf;
                curBuf = (curBuf + 1) % 2;
            }
            horizontal = true;
        }

        destBuf.clear();
        blit_framebuffer(fbos[lastBuf], destBuf, TaskList::calc_target_offset(*this, *sub_task()), true, true);

		return true;
	}
};


Task::Token TaskBlurGL::token(
	DescReal<TaskBlurGL, TaskBlur>("TaskBlurGL") );
} // end of anonimous namespace

/* === E N T R Y P O I N T ================================================= */
