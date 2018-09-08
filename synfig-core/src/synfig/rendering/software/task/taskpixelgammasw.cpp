/* === S Y N F I G ========================================================= */
/*!	\file synfig/rendering/software/task/taskpixelgammasw.cpp
**	\brief TaskPixelGammaSW
**
**	$Id$
**
**	\legal
**	......... ... 2016 Ivan Mahonin
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

#ifndef _WIN32
#include <unistd.h>
#include <sys/types.h>
#include <signal.h>
#endif

#include <synfig/debug/debugsurface.h>
#include <synfig/general.h>

#include "taskpixelgammasw.h"
#include "../surfacesw.h"
#include "../../optimizer.h"

#endif

using namespace synfig;
using namespace rendering;

/* === M A C R O S ========================================================= */

/* === G L O B A L S ======================================================= */

/* === P R O C E D U R E S ================================================= */

/* === M E T H O D S ======================================================= */


class TaskPixelGammaSW::Internal
	{
	public:
		typedef void Func(ColorReal &dst, const ColorReal &src, const ColorReal &gamma);

		struct Params
		{
			ColorReal *dst;
			int dst_stride;
			const ColorReal *src;
			int src_stride;
			int width;
			int height;

			union {
				ColorReal gamma[4];
				struct {
					ColorReal gamma_r, gamma_g, gamma_b, gamma_a;
				};
			};

			Params():
				dst(), dst_stride(),
				src(), src_stride(),
				width(), height(),
				gamma_r(1.0), gamma_g(1.0), gamma_b(1.0), gamma_a(1.0)
			{ }

			Params(
				Color *dst,
				int dst_stride,
				const Color *src,
				int src_stride,
				int width,
				int height,
				ColorReal gamma_r,
				ColorReal gamma_g,
				ColorReal gamma_b,
				ColorReal gamma_a
			):
				dst((ColorReal*)dst), dst_stride(dst_stride),
				src((const ColorReal*)src), src_stride(src_stride),
				width(width), height(height),
				gamma_r(gamma_r), gamma_g(gamma_g), gamma_b(gamma_b), gamma_a(gamma_a)
			{ }
		};

		static inline void func_none(ColorReal&, const ColorReal&, const ColorReal&) { }
		static inline void func_copy(ColorReal &dst, const ColorReal &src, const ColorReal&)
			{ dst = src; }
		static inline void func_one(ColorReal &dst, const ColorReal &, const ColorReal &)
			{ dst = ColorReal(1.0); }
		static inline void func_div(ColorReal &dst, const ColorReal &src, const ColorReal&)
			{ dst = ColorReal(1.0)/src; }
		static inline void func_pow(ColorReal &dst, const ColorReal &src, const ColorReal &gamma)
			{ dst = pow(src, gamma); }

		template<Func fr, Func fg, Func fb, Func fa>
		static void process_rgba(const Params &p) {
			bool set_one = approximate_equal_lp(p.gamma_r, ColorReal(0.0))
						&& approximate_equal_lp(p.gamma_g, ColorReal(0.0))
						&& approximate_equal_lp(p.gamma_b, ColorReal(0.0))
						&& approximate_equal_lp(p.gamma_a, ColorReal(0.0));

			if (set_one || p.src == p.dst)
			{
				assert(p.src_stride == p.dst_stride);

				int dst_dr = 4*(p.dst_stride - p.width);
				int row_size = 4*p.width;
				ColorReal *dst = p.dst;
				for(ColorReal *dst_end = dst + 4*p.dst_stride*p.height; dst != dst_end; dst += dst_dr)
				{
					for(ColorReal *dst_row_end = dst + row_size; dst != dst_row_end; dst += 4)
					{
						fr(dst[0], dst[0], p.gamma_r);
						fg(dst[1], dst[1], p.gamma_g);
						fb(dst[2], dst[2], p.gamma_b);
						fa(dst[3], dst[3], p.gamma_a);
					}
				}
			}
			else
			{
				assert(p.src + 4*p.src_stride*p.height <= p.dst || p.dst + 4*p.dst_stride*p.height <= p.src);

				int dst_dr = 4*(p.dst_stride - p.width);
				int src_dr = 4*(p.src_stride - p.width);
				int row_size = 4*p.width;
				ColorReal *dst = p.dst;
				const ColorReal *src = p.src;
				for(ColorReal *dst_end = dst + 4*p.dst_stride*p.height; dst != dst_end; dst += dst_dr, src += src_dr)
				{
					for(ColorReal *dst_row_end = dst + row_size; dst != dst_row_end; dst += 4, src += 4)
					{
						fr(dst[0], src[0], p.gamma_r);
						fg(dst[1], src[1], p.gamma_g);
						fb(dst[2], src[2], p.gamma_b);
						fa(dst[3], src[3], p.gamma_a);
					}
				}
			}
		}

		template<Func fr, Func fg, Func fb>
		static void process_rgb(const Params &p) {
			if ( approximate_equal_lp(p.gamma_a, ColorReal( 0.0))) process_rgba<fr, fg, fb, func_one >(p); else
			if ( approximate_equal_lp(p.gamma_a, ColorReal(-1.0))) process_rgba<fr, fg, fb, func_div >(p); else
			if (!approximate_equal_lp(p.gamma_a, ColorReal( 1.0))) process_rgba<fr, fg, fb, func_pow >(p); else
			if (p.src == p.dst)                                    process_rgba<fr, fg, fb, func_none>(p); else
																   process_rgba<fr, fg, fb, func_copy>(p);
		}

		template<Func fr, Func fg>
		static void process_rg(const Params &p) {
			if ( approximate_equal_lp(p.gamma_b, ColorReal( 0.0))) process_rgb<fr, fg, func_one >(p); else
			if ( approximate_equal_lp(p.gamma_b, ColorReal(-1.0))) process_rgb<fr, fg, func_div >(p); else
			if (!approximate_equal_lp(p.gamma_b, ColorReal( 1.0))) process_rgb<fr, fg, func_pow >(p); else
			if (p.src == p.dst)                                    process_rgb<fr, fg, func_none>(p); else
																   process_rgb<fr, fg, func_copy>(p);
		}

		template<Func fr>
		static void process_r(const Params &p) {
			if ( approximate_equal_lp(p.gamma_g, ColorReal( 0.0))) process_rg<fr, func_one >(p); else
			if ( approximate_equal_lp(p.gamma_g, ColorReal(-1.0))) process_rg<fr, func_div >(p); else
			if (!approximate_equal_lp(p.gamma_g, ColorReal( 1.0))) process_rg<fr, func_pow >(p); else
			if (p.src == p.dst)                                    process_rg<fr, func_none>(p); else
																   process_rg<fr, func_copy>(p);
		}

		static void process(const Params &p) {
			if ( approximate_equal_lp(p.gamma_r, ColorReal( 0.0))) process_r<func_one >(p); else
			if ( approximate_equal_lp(p.gamma_r, ColorReal(-1.0))) process_r<func_div >(p); else
			if (!approximate_equal_lp(p.gamma_r, ColorReal( 1.0))) process_r<func_pow >(p); else
			if (p.src == p.dst)                                    process_r<func_none>(p); else
																   process_r<func_copy>(p);
		}
	};


bool
TaskPixelGammaSW::run(RunParams & /* params */) const
{
	synfig::Surface &dst =
		rendering::SurfaceSW::Handle::cast_dynamic( target_surface )->get_surface();
	const synfig::Surface &src =
		rendering::SurfaceSW::Handle::cast_dynamic( sub_task()->target_surface )->get_surface();

	RectInt rd = get_target_rect();
	if (rd.valid())
	{
		VectorInt offset = get_offset();
		RectInt rs = sub_task()->get_target_rect() + rd.get_min() + offset;
		if (rs.valid())
		{
			etl::set_intersect(rs, rs, rd);
			if (rs.valid())
			{
				Internal::process(Internal::Params(
					&dst[rs.miny][rs.minx],
					dst.get_pitch()/sizeof(Color),
					&src[rs.miny - rd.miny - offset[1]][rs.minx - rd.minx - offset[0]],
					src.get_pitch()/sizeof(Color),
					rs.get_width(),
					rs.get_height(),
					1.0/gamma_r,
					1.0/gamma_g,
					1.0/gamma_b,
					1.0/gamma_a ));
			}
		}
	}

	return true;
}

/* === E N T R Y P O I N T ================================================= */
