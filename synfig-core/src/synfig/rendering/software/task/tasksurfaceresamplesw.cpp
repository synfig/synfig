/* === S Y N F I G ========================================================= */
/*!	\file synfig/rendering/opengl/task/tasksurfaceresamplesw.cpp
**	\brief TaskSurfaceResampleSW
**
**	$Id$
**
**	\legal
**	......... ... 2015 Ivan Mahonin
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

#ifndef WIN32
#include <unistd.h>
#include <sys/types.h>
#include <signal.h>
#endif

#include <synfig/general.h>
#include <synfig/localization.h>

#include "tasksurfaceresamplesw.h"

#include "../surfacesw.h"

#endif

using namespace synfig;
using namespace rendering;

/* === M A C R O S ========================================================= */

/* === G L O B A L S ======================================================= */

/* === P R O C E D U R E S ================================================= */

/* === M E T H O D S ======================================================= */

class TaskSurfaceResampleSW::Helper
{
public:
	struct Args {
		const synfig::Surface &surface;
		const RectInt &bounds;
		float gamma_adjust;
		Vector pos, pos_dx, pos_dy;
		Vector aa0, aa0_dx, aa0_dy;
		Vector aa1, aa1_dx, aa1_dy;
		Args(const synfig::Surface &surface, const RectInt &bounds):
			surface(surface), bounds(bounds), gamma_adjust() { }
	};

	static inline Color nearest(const synfig::Surface &surface, const Vector &pos)
	{
		return surface[ std::max(std::min((int)floor(pos[1]), surface.get_h()-1), 0) ]
					  [ std::max(std::min((int)floor(pos[0]), surface.get_w()-1), 0) ];
	}

	static inline Color linear(const synfig::Surface &surface, const Vector &pos)
		{ return surface.linear_sample(pos[0] - 0.5, pos[1] - 0.5); }

	static inline Color cosine(const synfig::Surface &surface, const Vector &pos)
		{ return surface.cosine_sample(pos[0] - 0.5, pos[1] - 0.5); }

	static inline Color cubic(const synfig::Surface &surface, const Vector &pos)
		{ return surface.cubic_sample(pos[0] - 0.5, pos[1] - 0.5); }

	static inline void gamma(Color &color, float gamma_adjust)
	{
		if (gamma_adjust != 1.f)
		{
			color.set_r( powf((float)color.get_r(), gamma_adjust) );
			color.set_g( powf((float)color.get_g(), gamma_adjust) );
			color.set_b( powf((float)color.get_b(), gamma_adjust) );
		}
	}

	static inline void nogamma(Color&, float) { }

	template<typename pen, void gamma(Color&, float), Color filter(const synfig::Surface&, const Vector&)>
	static inline void fill(pen &p, Args &a)
	{
		int idx = a.bounds.maxx - a.bounds.minx;
		int idy = a.bounds.maxy - a.bounds.miny;
		for(int y = idy; y; --y)
		{
			for(int x = idx; x; --x)
			{
				Color c = filter(a.surface, a.pos);
				gamma(c, a.gamma_adjust);
				p.put_value(c);
				a.pos += a.pos_dx;
				p.inc_x();
			}
			p.dec_x(idx);
			p.inc_y();
			a.pos += a.pos_dy;
		}
	}

	template<typename pen, void gamma(Color&, float), Color filter(const synfig::Surface&, const Vector&)>
	static inline void fill_aa(pen &p, Args &a)
	{
		int idx = a.bounds.maxx - a.bounds.minx;
		int idy = a.bounds.maxy - a.bounds.miny;
		for(int y = idy; y; --y)
		{
			for(int x = idx; x; --x)
			{
				if ( a.aa0[0] > 0.0 && a.aa0[1] > 0.0
				  && a.aa1[0] > 0.0 && a.aa1[1] > 0.0 )
				{
					Color c = filter(a.surface, a.pos);
					c.set_a( c.get_a()
						   * std::min(a.aa0[0], 1.0)
						   * std::min(a.aa0[1], 1.0)
						   * std::min(a.aa1[0], 1.0)
						   * std::min(a.aa1[1], 1.0) );
					gamma(c, a.gamma_adjust);
					p.put_value(c);
				}

				a.pos += a.pos_dx;
				a.aa0 += a.aa0_dx;
				a.aa1 += a.aa1_dx;
				p.inc_x();
			}
			p.dec_x(idx);
			p.inc_y();
			a.pos += a.pos_dy;
			a.aa0 += a.aa0_dy;
			a.aa1 += a.aa1_dy;
		}
	}

	template<typename pen, void gamma(Color&, float), Color filter(const synfig::Surface&, const Vector&)>
	static inline void fill(
		bool antialiasing,
		pen &p, Args &a )
	{
		if (antialiasing)
			fill_aa<pen, gamma, filter>(p, a);
		else
			fill<pen, gamma, filter>(p, a);
	}

	template<typename pen, void gamma(Color&, float)>
	static inline void fill(
		Color::Interpolation interpolation,
		bool antialiasing,
		pen &p, Args &a )
	{
		switch(interpolation)
		{
		case Color::INTERPOLATION_LINEAR:
			fill<pen, gamma, linear>(antialiasing, p, a); break;
		case Color::INTERPOLATION_COSINE:
			fill<pen, gamma, cosine>(antialiasing, p, a); break;
		case Color::INTERPOLATION_CUBIC:
			fill<pen, gamma, cubic>(antialiasing, p, a); break;
		default:
			fill<pen, gamma, nearest>(antialiasing, p, a); break;
		}
	}

	template<typename pen>
	static inline void fill(
		Color::value_type gamma_adjust,
		Color::Interpolation interpolation,
		bool antialiasing,
		pen &p, Args &a )
	{
		if (fabsf(gamma_adjust - 1.f) < 1e-6)
		{
			fill<pen, nogamma>(interpolation, antialiasing, p, a);
		}
		else
		{
			a.gamma_adjust = gamma_adjust;
			fill<pen, gamma>(interpolation, antialiasing, p, a);
		}
	}
};

bool
TaskSurfaceResampleSW::run(RunParams & /* params */) const
{
	const Real precision = 1e-10;

	const synfig::Surface &a =
		SurfaceSW::Handle::cast_dynamic(sub_task()->target_surface)->get_surface();
	synfig::Surface &target =
		SurfaceSW::Handle::cast_dynamic(target_surface)->get_surface();

	if (valid_target() && sub_task()->valid_target())
	{
		// transformation matrix

		Matrix src_pixels_to_units;
		src_pixels_to_units.m00 = sub_task()->get_units_per_pixel()[0];
		src_pixels_to_units.m11 = sub_task()->get_units_per_pixel()[1];
		src_pixels_to_units.m20 = -sub_task()->get_target_offset()[0]*src_pixels_to_units.m00 + sub_task()->get_source_rect_lt()[0];
		src_pixels_to_units.m21 = -sub_task()->get_target_offset()[1]*src_pixels_to_units.m11 + sub_task()->get_source_rect_lt()[1];

		Matrix dest_units_to_pixels;
		dest_units_to_pixels.m00 = get_pixels_per_unit()[0];
		dest_units_to_pixels.m11 = get_pixels_per_unit()[1];
		dest_units_to_pixels.m20 = -get_source_rect_lt()[0]*dest_units_to_pixels.m00 + get_target_offset()[0];
		dest_units_to_pixels.m21 = -get_source_rect_lt()[1]*dest_units_to_pixels.m11 + get_target_offset()[1];

		Matrix matrix = src_pixels_to_units * transformation * dest_units_to_pixels;

		// bounds

		RectInt sub_target = sub_task()->get_target_rect();
		Vector corners[] = {
			matrix.get_transformed(Vector( Real(sub_target.minx), Real(sub_target.miny) )),
			matrix.get_transformed(Vector( Real(sub_target.maxx), Real(sub_target.miny) )),
			matrix.get_transformed(Vector( Real(sub_target.minx), Real(sub_target.maxy) )),
			matrix.get_transformed(Vector( Real(sub_target.maxx), Real(sub_target.maxy) )) };

		Rect boundsf(   corners[0] );
		boundsf.expand( corners[1] );
		boundsf.expand( corners[2] );
		boundsf.expand( corners[3] );

		RectInt bounds( (int)floor(boundsf.minx + precision) - 1,
						(int)floor(boundsf.miny + precision) - 1,
						(int)ceil (boundsf.maxx - precision) + 1,
						(int)ceil (boundsf.maxy - precision) + 1 );
		etl::set_intersect(bounds, bounds, get_target_rect());

		// texture matrices

		if (bounds.valid())
		{
			Matrix inv_matrix = matrix;
			inv_matrix.invert();

			Helper::Args args(a, bounds);

			Vector start((Real)bounds.minx, (Real)bounds.miny);
			Vector dx(1.0, 0.0);
			Vector dy((Real)(bounds.minx - bounds.maxx), 1.0);

			args.pos    = inv_matrix.get_transformed( start );
			args.pos_dx = inv_matrix.get_transformed( dx, false );
			args.pos_dy = inv_matrix.get_transformed( dy, false );

			bool aa = antialiasing;

			if (aa)
			{
				Vector sub_corners[] = {
					inv_matrix.get_transformed(Vector( Real(bounds.minx), Real(bounds.miny) )),
					inv_matrix.get_transformed(Vector( Real(bounds.maxx), Real(bounds.miny) )),
					inv_matrix.get_transformed(Vector( Real(bounds.minx), Real(bounds.maxy) )),
					inv_matrix.get_transformed(Vector( Real(bounds.maxx), Real(bounds.maxy) )) };

				Rect sub_boundsf(   sub_corners[0] );
				sub_boundsf.expand( sub_corners[1] );
				sub_boundsf.expand( sub_corners[2] );
				sub_boundsf.expand( sub_corners[3] );

				RectInt sub_bounds( (int)floor(sub_boundsf.minx + precision) - 1,
								(int)floor(sub_boundsf.miny + precision) - 1,
								(int)ceil (sub_boundsf.maxx - precision) + 1,
								(int)ceil (sub_boundsf.maxy - precision) + 1 );

				if (etl::contains(sub_target, sub_bounds))
					aa = false;
			}

			if (aa)
			{
				Real sx = (corners[1] - corners[0]).mag()/Real(sub_target.maxx - sub_target.minx);
				Real sy = (corners[2] - corners[0]).mag()/Real(sub_target.maxy - sub_target.miny);

				Matrix aa0_matrix = inv_matrix
								  * Matrix().set_scale(sx, sy)
								  * Matrix().set_translate(0.5, 0.5);
				Matrix aa1_matrix = inv_matrix
								  * Matrix().set_translate(
										-Real(sub_target.maxx - sub_target.minx),
										-Real(sub_target.maxy - sub_target.miny) )
								  * Matrix().set_scale(-sx, -sy)
								  * Matrix().set_translate(0.5, 0.5);

				args.aa0    = aa0_matrix.get_transformed( start );
				args.aa0_dx = aa0_matrix.get_transformed( dx, false );
				args.aa0_dy = aa0_matrix.get_transformed( dy, false );

				args.aa1    = aa1_matrix.get_transformed( start );
				args.aa1_dx = aa1_matrix.get_transformed( dx, false );
				args.aa1_dy = aa1_matrix.get_transformed( dy, false );
			}

			if (blend)
			{
				synfig::Surface::alpha_pen p(target.get_pen(bounds.minx, bounds.miny));
				p.set_blend_method(blend_method);
				Helper::fill(gamma, interpolation, aa, p, args);
			}
			else
			{
				synfig::Surface::pen p(target.get_pen(bounds.minx, bounds.miny));
				Helper::fill(gamma, interpolation, aa, p, args);
			}
		}
	}

	return true;
}

/* === E N T R Y P O I N T ================================================= */
