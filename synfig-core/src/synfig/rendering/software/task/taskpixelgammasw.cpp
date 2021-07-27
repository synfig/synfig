/* === S Y N F I G ========================================================= */
/*!	\file synfig/rendering/software/task/taskpixelgammasw.cpp
**	\brief TaskPixelGammaSW
**
**	$Id$
**
**	\legal
**	......... ... 2016-2018 Ivan Mahonin
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

#include <synfig/debug/debugsurface.h>
#include <synfig/general.h>

#include "../../common/task/taskpixelprocessor.h"
#include "tasksw.h"

#endif

using namespace synfig;
using namespace rendering;

/* === M A C R O S ========================================================= */

/* === G L O B A L S ======================================================= */

/* === P R O C E D U R E S ================================================= */

/* === M E T H O D S ======================================================= */

namespace {

class TaskPixelGammaSW: public TaskPixelGamma, public TaskSW
{
public:
	typedef std::shared_ptr<TaskPixelGammaSW> Handle;
	static Token token;
	virtual Token::Handle get_token() const { return token.handle(); }

private:
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
			ColorReal gamma[3];
			struct {
				ColorReal gamma_r, gamma_g, gamma_b;
			};
		};

		Params():
			dst(), dst_stride(),
			src(), src_stride(),
			width(), height(),
			gamma_r(1.0), gamma_g(1.0), gamma_b(1.0)
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
			ColorReal gamma_b
		):
			dst((ColorReal*)dst), dst_stride(dst_stride),
			src((const ColorReal*)src), src_stride(src_stride),
			width(width), height(height),
			gamma_r(gamma_r), gamma_g(gamma_g), gamma_b(gamma_b)
		{ }
	};

	static inline ColorReal clamp(const ColorReal &x)
	{
		const ColorReal max = ColorReal(1.0)/real_low_precision<ColorReal>();
		return std::max(-max, std::min(max, x));
	}

	static inline ColorReal clamp_positive(const ColorReal &x)
	{
		const ColorReal max = ColorReal(1.0)/real_low_precision<ColorReal>();
		return std::max(real_low_precision<ColorReal>(), std::min(max, x));
	}

	static inline void func_none(ColorReal&, const ColorReal&, const ColorReal&) { }
	static inline void func_copy(ColorReal &dst, const ColorReal &src, const ColorReal&)
		{ dst = src; }
	static inline void func_one(ColorReal &dst, const ColorReal &, const ColorReal &)
		{ dst = ColorReal(1.0); }
	static inline void func_pow(ColorReal &dst, const ColorReal &src, const ColorReal &gamma)
		{ dst = clamp(src < 0 ? -pow(-src, gamma) : pow(src, gamma)); }

	template<Func fr, Func fg, Func fb>
	static void process_rgb(const Params &p) {
		if (p.src == p.dst)
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
					dst[3] = src[3];
				}
			}
		}
	}

	template<Func fr, Func fg>
	static void process_rg(const Params &p) {
		if ( approximate_equal_lp(p.gamma_b, ColorReal(0.0))) process_rgb<fr, fg, func_one >(p); else
		if (!approximate_equal_lp(p.gamma_b, ColorReal(1.0))) process_rgb<fr, fg, func_pow >(p); else
		if (p.src == p.dst)                                   process_rgb<fr, fg, func_none>(p); else
				                                              process_rgb<fr, fg, func_copy>(p);
	}

	template<Func fr>
	static void process_r(const Params &p) {
		if ( approximate_equal_lp(p.gamma_g, ColorReal(0.0))) process_rg<fr, func_one >(p); else
		if (!approximate_equal_lp(p.gamma_g, ColorReal(1.0))) process_rg<fr, func_pow >(p); else
		if (p.src == p.dst)                                   process_rg<fr, func_none>(p); else
				                                              process_rg<fr, func_copy>(p);
	}

	static void process(const Params &p) {
		if ( approximate_equal_lp(p.gamma_r, ColorReal(0.0))) process_r<func_one >(p); else
		if (!approximate_equal_lp(p.gamma_r, ColorReal(1.0))) process_r<func_pow >(p); else
		if (p.src == p.dst)                                   process_r<func_none>(p); else
				                                              process_r<func_copy>(p);
	}

public:
	virtual bool run(RunParams&) const {
		if (!is_valid() || !sub_task() || !sub_task()->is_valid())
			return true;

		RectInt rd = target_rect;
		VectorInt offset = get_offset();
		RectInt rs = sub_task()->target_rect + rd.get_min() + offset;
		rect_set_intersect(rs, rs, rd);
		if (rs.is_valid())
		{
			LockWrite ldst(this);
			if (!ldst) return false;
			LockRead lsrc(sub_task());
			if (!lsrc) return false;

			synfig::Surface &dst = ldst->get_surface();
			const synfig::Surface &src = lsrc->get_surface();

			process(Params(
				&dst[rs.miny][rs.minx],
				dst.get_pitch()/sizeof(Color),
				&src[rs.miny - rd.miny - offset[1]][rs.minx - rd.minx - offset[0]],
				src.get_pitch()/sizeof(Color),
				rs.get_width(),
				rs.get_height(),
				clamp_positive(gamma.get_r()),
				clamp_positive(gamma.get_g()),
				clamp_positive(gamma.get_b()) ));
		}

		return true;
	}
};


Task::Token TaskPixelGammaSW::token(
	DescReal<TaskPixelGammaSW, TaskPixelGamma>("PixelGammaSW") );

} // end of anonimous namespace

/* === E N T R Y P O I N T ================================================= */
