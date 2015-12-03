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

	template<typename pen, void gamma(Color&, float)>
	static inline void fill(
		Color::Interpolation interpolation,
		pen &p, Args &a )
	{
		switch(interpolation)
		{
		case Color::INTERPOLATION_LINEAR:
			fill<pen, gamma, linear>(p, a); break;
		case Color::INTERPOLATION_COSINE:
			fill<pen, gamma, cosine>(p, a); break;
		case Color::INTERPOLATION_CUBIC:
			fill<pen, gamma, cubic>(p, a); break;
		default:
			fill<pen, gamma, nearest>(p, a); break;
		}
	}

	template<typename pen>
	static inline void fill(
		Color::value_type gamma_adjust,
		Color::Interpolation interpolation,
		pen &p, Args &a )
	{
		if (fabsf(gamma_adjust - 1.f) < 1e-6)
		{
			fill<pen, nogamma>(interpolation, p, a);
		}
		else
		{
			a.gamma_adjust = gamma_adjust;
			fill<pen, gamma>(interpolation, p, a);
		}
	}
};

bool
TaskSurfaceResampleSW::run(RunParams & /* params */) const
{
	const synfig::Surface &a =
		SurfaceSW::Handle::cast_dynamic(sub_task()->target_surface)->get_surface();
	synfig::Surface &target =
		SurfaceSW::Handle::cast_dynamic(target_surface)->get_surface();

	// transformation matrix

	Matrix bounds_transfromation;
	bounds_transfromation.m00 = get_pixels_per_unit()[0];
	bounds_transfromation.m11 = get_pixels_per_unit()[1];
	bounds_transfromation.m20 = -get_source_rect_lt()[0]*bounds_transfromation.m00 + get_target_rect().minx;
	bounds_transfromation.m21 = -get_source_rect_lt()[1]*bounds_transfromation.m11 + get_target_rect().miny;

	Matrix matrix = transformation * bounds_transfromation;

	// bounds

	Rect boundsf(   matrix.get_transformed(Vector(0.0, 0.0)) );
	boundsf.expand( matrix.get_transformed(Vector(1.0, 0.0)) );
	boundsf.expand( matrix.get_transformed(Vector(0.0, 1.0)) );
	boundsf.expand( matrix.get_transformed(Vector(1.0, 1.0)) );

	RectInt bounds( (int)floor(boundsf.minx) - 1,
			        (int)floor(boundsf.miny) - 1,
					(int)floor(boundsf.maxx) + 2,
					(int)floor(boundsf.maxy) + 2 );
	etl::set_intersect(bounds, bounds, RectInt(0, 0, target.get_w(), target.get_h()));

	// texture matrices

	if (bounds.valid())
	{
		Matrix inv_matrix = matrix;
		inv_matrix.invert();

		Matrix pos_matrix;
		pos_matrix.m00 = (crop_rb[0] - crop_lt[0])*a.get_w();
		pos_matrix.m11 = (crop_rb[1] - crop_lt[1])*a.get_h();
		pos_matrix.m20 = -crop_lt[0] * pos_matrix.m00;
		pos_matrix.m21 = -crop_lt[1] * pos_matrix.m11;

		Matrix aa0_matrix;
		aa0_matrix.m00 = matrix.get_axis_x().mag();
		aa0_matrix.m11 = matrix.get_axis_y().mag();
		aa0_matrix.m20 = 0.5;
		aa0_matrix.m21 = 0.5;

		Matrix aa1_matrix;
		aa1_matrix.m00 = -aa0_matrix.m00;
		aa1_matrix.m11 = -aa0_matrix.m11;
		aa1_matrix.m20 = 0.5 - 1.0*aa1_matrix.m00;
		aa1_matrix.m21 = 0.5 - 1.0*aa1_matrix.m11;

		pos_matrix = inv_matrix * pos_matrix;
		aa0_matrix = inv_matrix * aa0_matrix;
		aa1_matrix = inv_matrix * aa1_matrix;

		Helper::Args args(a, bounds);

		Vector start((Real)bounds.minx, (Real)bounds.miny);
		Vector dx(1.0, 0.0);
		Vector dy((Real)(bounds.minx - bounds.maxx), 1.0);

		args.pos    = pos_matrix.get_transformed( start );
		args.pos_dx = pos_matrix.get_transformed( dx, false );
		args.pos_dy = pos_matrix.get_transformed( dy, false );

		args.aa0    = aa0_matrix.get_transformed( start );
		args.aa0_dx = aa0_matrix.get_transformed( dx, false );
		args.aa0_dy = aa0_matrix.get_transformed( dy, false );

		args.aa1    = aa1_matrix.get_transformed( start );
		args.aa1_dx = aa1_matrix.get_transformed( dx, false );
		args.aa1_dy = aa1_matrix.get_transformed( dy, false );

		if (blend)
		{
			synfig::Surface::alpha_pen p(target.get_pen(bounds.minx, bounds.miny));
			p.set_blend_method(blend_method);
			Helper::fill(gamma, interpolation, p, args);
		}
		else
		{
			synfig::Surface::pen p(target.get_pen(bounds.minx, bounds.miny));
			Helper::fill(gamma, interpolation, p, args);
		}
	}

	return true;
}

/* === E N T R Y P O I N T ================================================= */
