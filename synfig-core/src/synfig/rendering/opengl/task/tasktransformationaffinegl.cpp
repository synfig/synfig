/* === S Y N F I G ========================================================= */
/*!	\file synfig/rendering/software/task/tasktransformationaffinegl.cpp
**	\brief TaskTransformationAffineGL
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

#include "synfig/rendering/primitive/transformation.h"
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

#include "../../common/task/tasktransformation.h"

#include <math.h>
#include <vector>
#include <algorithm>
#endif

using namespace synfig;
using namespace rendering;

/* === M A C R O S ========================================================= */

/* === G L O B A L S ======================================================= */

/* === P R O C E D U R E S ================================================= */

/* === M E T H O D S ======================================================= */

namespace {

class TaskTransformationAffineGL: public TaskTransformationAffine,
    public TaskGL
{
public:
	typedef etl::handle<TaskTransformationAffineGL> Handle;
	static Token token;
	virtual Token::Handle get_token() const { return token.handle(); }

    bool downscale(
            gl::Framebuffer& dest,
            RectInt destRect,
            gl::Framebuffer& src,
            RectInt srcRect ) const
    {
        return false;
    }

    bool resample(
            gl::Framebuffer& dest, 
            const RectInt& dest_bounds,
            gl::Framebuffer& src,
            const RectInt& src_bounds,
            const Matrix& transformation ) const
    {
        return false;
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

		Vector src_upp = sub_task()->get_units_per_pixel();
		Matrix src_pixels_to_units;
		src_pixels_to_units.m00 = src_upp[0];
		src_pixels_to_units.m11 = src_upp[1];
		src_pixels_to_units.m20 = sub_task()->source_rect.minx - src_upp[0]*sub_task()->target_rect.minx;
		src_pixels_to_units.m21 = sub_task()->source_rect.miny - src_upp[1]*sub_task()->target_rect.miny;

		Vector dst_ppu = get_pixels_per_unit();
		Matrix dst_units_to_pixels;
		dst_units_to_pixels.m00 = dst_ppu[0];
		dst_units_to_pixels.m11 = dst_ppu[1];
		dst_units_to_pixels.m20 = target_rect.minx - dst_ppu[0]*source_rect.minx;
		dst_units_to_pixels.m21 = target_rect.miny - dst_ppu[1]*source_rect.miny;

		Matrix matrix = dst_units_to_pixels * transformation->matrix * src_pixels_to_units;

        RectInt dest_bounds = target_rect;
        RectInt src_bounds = sub_task()->target_rect;

        gl::Framebuffer destBuf = ldst->get_framebuffer();
        gl::Framebuffer srcBuf = lsrc.cast_handle()->get_framebuffer();

        if(interpolation != Color::INTERPOLATION_NEAREST)
        {
            synfig::rendering::Transformation::Bounds bounds =
                TransformationAffine( matrix.get_inverted() )
                .transform_bounds( Rect(0.0, 0.0, 1.0, 1.0), Vector(1.0, 1.0) );
            bounds.resolution *= 1.20;

            int sw = src_bounds.get_width();
            int sh = src_bounds.get_height();
            int w = synfig::clamp((int)ceil((Real)sw * bounds.resolution[0]), 1, sw);
            int h = synfig::clamp((int)ceil((Real)sh * bounds.resolution[1]), 1, sh);

            if (w < sw || h < sh) {
                gl::Framebuffer new_src;
                new_src.from_dims(w, h);
                downscale(new_src, RectInt(0, 0, w, h), srcBuf, src_bounds);

                srcBuf = new_src;

                Matrix new_transformation = matrix
                    * Matrix().set_translate(src_bounds.minx, src_bounds.miny)
                    * Matrix().set_scale((Real)sw/(Real)w, (Real)sh/(Real)h);

                src_bounds = RectInt(0, 0, w, h);
                // TODO: srfBuf mew
                matrix = new_transformation;
            }
        }

        resample(destBuf, dest_bounds, srcBuf, src_bounds, matrix);

		return true;
	}
};


Task::Token TaskTransformationAffineGL::token(
	DescReal<TaskTransformationAffineGL, TaskTransformationAffine>("TaskTransformationAffineGL") );
} // end of anonimous namespace

/* === E N T R Y P O I N T ================================================= */
