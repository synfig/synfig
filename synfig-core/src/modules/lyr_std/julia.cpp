/* === S Y N F I G ========================================================= */
/*!	\file julia.cpp
**	\brief Implementation of the "Julia Set" layer
**
**	\legal
**	Copyright (c) 2002-2005 Robert B. Quattlebaum Jr., Adrian Bentley
**	Copyright (c) 2007 Chris Moore
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
**
** ========================================================================= */

/* === H E A D E R S ======================================================= */

#ifdef USING_PCH
#	include "pch.h"
#else
#ifdef HAVE_CONFIG_H
#	include <config.h>
#endif

#include "julia.h"

#include <synfig/localization.h>

#include <synfig/context.h>
#include <synfig/rendering/common/task/taskdistort.h>
#include <synfig/rendering/software/task/tasksw.h>

#endif

using namespace synfig;
using namespace modules;
using namespace lyr_std;

/* === M A C R O S ========================================================= */

#define LOG_OF_2		0.69314718055994528623

/* === G L O B A L S ======================================================= */

SYNFIG_LAYER_INIT(Julia);
SYNFIG_LAYER_SET_NAME(Julia,"julia");
SYNFIG_LAYER_SET_LOCAL_NAME(Julia,N_("Julia Set"));
SYNFIG_LAYER_SET_CATEGORY(Julia,N_("Fractals"));
SYNFIG_LAYER_SET_VERSION(Julia,"0.3");

/* === P R O C E D U R E S ================================================= */

/* === M E T H O D S ======================================================= */

class TaskJulia
	: public rendering::TaskDistort
{
public:
	typedef etl::handle<TaskJulia> Handle;
	static Token token;
	Token::Handle get_token() const override { return token.handle(); }

	Color icolor;
	Color ocolor;
	Angle color_shift;
	int iterations;
	Point seed;
	bool distort_inside;
	bool shade_inside;
	bool solid_inside;
	bool invert_inside;
	bool color_inside;
	bool distort_outside;
	bool shade_outside;
	bool solid_outside;
	bool invert_outside;
	bool color_outside;

	bool color_cycle;
	bool smooth_outside;
	bool broken;

	ColorReal squared_bailout;

	Rect
	compute_required_source_rect(const Rect& source_rect, const Matrix& /*inv_matrix*/) const override
	{
		if (!distort_inside && ! distort_outside) {
			if (color_inside && color_outside)
				return Rect();
			else
				return source_rect;
		}

		if (distort_outside && !solid_outside) {
			const Point c = seed;
			Rect r;
			for (Point::value_type y = source_rect.miny; y < source_rect.maxy; y += get_units_per_pixel()[1]) {
				for (Point::value_type x = source_rect.minx; x < source_rect.maxx; x += get_units_per_pixel()[0]) {
					Point z {x, y};
					for (int i=0; i<iterations; i++) {
						// Perform complex multiplication
						Real zr_hold = z[0];
						z[0] = z[0]*z[0]-z[1]*z[1] + c[0];
						z[1] = zr_hold*z[1]*2 + c[1];

						// Use "broken" algorithm, if requested (looks weird)
						if (broken)
							z[0] += z[1];

						if (z.mag_squared() > squared_bailout) {
							break;
						}
					}
					r.minx = std::min(r.minx, z[0]);
					r.miny = std::min(r.miny, z[1]);
					r.maxx = std::max(r.maxx, z[0]);
					r.maxy = std::max(r.maxy, z[1]);
				}
			}
			return r;
		}

		if (solid_outside)
			return source_rect;

		Vector expansion(8, 8);
		Rect rect(source_rect.get_min() - expansion, source_rect.get_max() + expansion);
		Rect::value_type diff = rect.get_width() - rect.get_height();
		if (approximate_greater(diff, 0.)) {
			rect.expand_y(diff);
		} else if (approximate_less(diff, 0.)) {
			rect.expand_x(-diff);
		}
		return rect;
	}

};

class TaskJuliaSW
	: public TaskJulia, public rendering::TaskSW
{
public:
	typedef etl::handle<TaskJuliaSW> Handle;
	static Token token;
	Token::Handle get_token() const override { return token.handle(); }

	TaskJuliaSW()
	{
	}

	Color
	get_color(const Point& point, const synfig::Surface& context) const
	{
		Real
			cr, ci,
			zr, zi;

		ColorReal mag(0);

		Color ret;

		cr=seed[0];
		ci=seed[1];
		zr=point[0];
		zi=point[1];

		const struct SideInfo
		{
			Color color;
			bool is_solid;
			bool must_distort;
			bool must_invert;
			bool must_color;
			bool must_shade;
		} side_info[2] = {
			{icolor, solid_inside, distort_inside, invert_inside, color_inside, shade_inside},
			{ocolor, solid_outside, distort_outside, invert_outside, color_outside, shade_outside},
		};
		ColorReal shade_factor;
		ColorReal depth;

		int side_idx = 0; // inside

		for (int i=0; i<iterations; i++) {
			// Perform complex multiplication
			Real zr_hold = zr;
			zr = zr*zr-zi*zi + cr;
			zi = zr_hold*zi*2 + ci;

			// Use "broken" algorithm, if requested (looks weird)
			if(broken) zr += zi;

			// Calculate Magnitude
			mag = zr*zr + zi*zi;

			if (mag > squared_bailout) {
				side_idx = 1; // outside
				depth = static_cast<ColorReal>(i);
				if (smooth_outside) {
					// Darco's original mandelbrot smoothing algo
					// depth=((Point::value_type)i+(2.0-sqrt(mag))/PI);

					// Linas Vepstas algo (Better than darco's)
					// See (http://linas.org/art-gallery/escape/smooth.html)
					depth -= log(log(sqrt(mag))) / LOG_OF_2;

					// Clamp
					if (depth<0) depth=0;
				}
				shade_factor = depth/static_cast<ColorReal>(iterations);
				break;
			}
		}

		if (side_idx == 0)
			shade_factor = mag;

		const auto& info = side_info[side_idx];
		if (info.is_solid)
			ret = info.color;
		else {
			Point q = info.must_distort ? Point(zr,zi) : point;

			auto pixel_offset = -required_source_rect.get_min().multiply_coords(sub_tasks[0]->get_pixels_per_unit());
			q = q.multiply_coords(sub_tasks[0]->get_pixels_per_unit()) + pixel_offset;
			if (q[0] >= context.get_w() || q[0] < 0 || q[1] >= context.get_h() || q[1] < 0)
				ret = Color::magenta();
			else
				ret = context.cubic_sample(q[0], q[1]);
		}

		if (info.must_invert)
			ret = ~ret;

		if (info.must_color)
			ret = ret.set_uv(zr,zi).clamped_negative();

		if (side_idx == 1 && color_cycle) // outside
			ret = ret.rotate_uv(color_shift*depth).clamped_negative();

		if (info.must_shade)
			ret += (info.color - ret) * shade_factor;
		return ret;
	}

	bool run(Task::RunParams& /*params*/) const override
	{
		if (!sub_task())
			return true;
		if (!is_valid())
			return true;

		const Vector ppu = get_pixels_per_unit();

		rendering::TaskSW::LockWrite la(this);
		if (!la)
			return false;

		rendering::TaskSW::LockRead lb(sub_task());
		if (!lb)
			return false;
		// const Vector ppub = sub_task()->get_pixels_per_unit();
		const int tw = target_rect.get_width();
		const synfig::Surface& b = lb->get_surface();

		synfig::Surface::pen pen(la->get_surface().get_pen(target_rect.minx, target_rect.miny));

		Matrix bounds_transformation;
		bounds_transformation.m00 = ppu[0];
		bounds_transformation.m11 = ppu[1];
		bounds_transformation.m20 = target_rect.minx - ppu[0]*source_rect.minx;
		bounds_transformation.m21 = target_rect.miny - ppu[1]*source_rect.miny;

		Matrix inv_matrix = bounds_transformation.get_inverted();

		Vector dx = inv_matrix.axis_x();
		Vector dy = inv_matrix.axis_y() - dx*(Real)tw;
		Vector p = inv_matrix.get_transformed( Vector((Real)target_rect.minx, (Real)target_rect.miny) );

		for (int iy = target_rect.miny; iy < target_rect.maxy; ++iy, p += dy, pen.inc_y(), pen.dec_x(tw)) {
			for (int ix = target_rect.minx; ix < target_rect.maxx; ++ix, p += dx, pen.inc_x()) {
				pen.put_value(get_color(p, b));
			}
		}

		return true;
	}
};

rendering::Task::Token TaskJulia::token(
	DescAbstract<TaskJulia>("Julia") );
rendering::Task::Token TaskJuliaSW::token(
	DescReal<TaskJuliaSW, TaskJulia>("JuliaSW") );



Julia::Julia():
param_color_shift(ValueBase(Angle::deg(0)))
{
	param_icolor=ValueBase(Color::black());
	param_ocolor=ValueBase(Color::black());
	param_iterations=ValueBase(int(32));
	param_color_shift=ValueBase(Angle::deg(0));
	
	param_distort_inside=ValueBase(true);
	param_distort_outside=ValueBase(true);
	param_shade_inside=ValueBase(true);
	param_shade_outside=ValueBase(true);
	param_solid_inside=ValueBase(false);
	param_solid_outside=ValueBase(false);
	param_invert_inside=ValueBase(false);
	param_invert_outside=ValueBase(false);
	param_color_inside=ValueBase(true);
	param_color_outside=ValueBase(false);
	param_color_cycle=ValueBase(false);
	param_smooth_outside=ValueBase(true);
	param_broken=ValueBase(false);
	param_seed=ValueBase(Point(0,0));

	param_bailout=ValueBase(Real(2));
	squared_bailout = 4;

	SET_INTERPOLATION_DEFAULTS();
	SET_STATIC_DEFAULTS();
}

bool
Julia::set_param(const String & param, const ValueBase &value)
{

	IMPORT_VALUE(param_icolor);
	IMPORT_VALUE(param_ocolor);
	IMPORT_VALUE(param_color_shift);
	IMPORT_VALUE(param_seed);

	IMPORT_VALUE(param_distort_inside);
	IMPORT_VALUE(param_distort_outside);
	IMPORT_VALUE(param_shade_inside);
	IMPORT_VALUE(param_shade_outside);
	IMPORT_VALUE(param_solid_inside);
	IMPORT_VALUE(param_solid_outside);
	IMPORT_VALUE(param_invert_inside);
	IMPORT_VALUE(param_invert_outside);
	IMPORT_VALUE(param_color_inside);
	IMPORT_VALUE(param_color_outside);

	IMPORT_VALUE(param_color_cycle);
	IMPORT_VALUE(param_smooth_outside);
	IMPORT_VALUE(param_broken);

	IMPORT_VALUE_PLUS(param_iterations, {
		int iterations = value.get(int());
		iterations = synfig::clamp(iterations, 0, 500000);
		param_iterations.set(iterations);
		return true;
	});
	IMPORT_VALUE_PLUS(param_bailout, {
		Real bailout=param_bailout.get(Real());
		squared_bailout = bailout * bailout;
		return true;
	});

	return false;
}

ValueBase
Julia::get_param(const String & param)const
{
	EXPORT_VALUE(param_icolor);
	EXPORT_VALUE(param_ocolor);
	EXPORT_VALUE(param_color_shift);
	EXPORT_VALUE(param_iterations);
	EXPORT_VALUE(param_seed);

	EXPORT_VALUE(param_distort_inside);
	EXPORT_VALUE(param_distort_outside);
	EXPORT_VALUE(param_shade_inside);
	EXPORT_VALUE(param_shade_outside);
	EXPORT_VALUE(param_solid_inside);
	EXPORT_VALUE(param_solid_outside);
	EXPORT_VALUE(param_invert_inside);
	EXPORT_VALUE(param_invert_outside);
	EXPORT_VALUE(param_color_inside);
	EXPORT_VALUE(param_color_outside);
	EXPORT_VALUE(param_color_cycle);
	EXPORT_VALUE(param_smooth_outside);
	EXPORT_VALUE(param_broken);
	EXPORT_VALUE(param_bailout);

	EXPORT_NAME();
	EXPORT_VERSION();

	return ValueBase();
}

Color
Julia::get_color(Context context, const Point &pos)const
{
	Color icolor=param_icolor.get(Color());
	Color ocolor=param_ocolor.get(Color());
	Angle color_shift=param_color_shift.get(Angle());
	int iterations=param_iterations.get(int());
	Point seed=param_seed.get(Point());
	bool distort_inside=param_distort_inside.get(bool());
	bool shade_inside=param_shade_inside.get(bool());
	bool solid_inside=param_solid_inside.get(bool());
	bool invert_inside=param_invert_inside.get(bool());
	bool color_inside=param_color_inside.get(bool());
	bool distort_outside=param_distort_outside.get(bool());
	bool shade_outside=param_shade_outside.get(bool());
	bool solid_outside=param_solid_outside.get(bool());
	bool invert_outside=param_invert_outside.get(bool());
	bool color_outside=param_color_outside.get(bool());

	bool color_cycle=param_color_cycle.get(bool());
	bool smooth_outside=param_smooth_outside.get(bool());
	bool broken=param_broken.get(bool());

	Real
		cr, ci,
		zr, zi,
		zr_hold;

	ColorReal
		depth, mag(0);

	Color
		ret;

	cr=seed[0];
	ci=seed[1];
	zr=pos[0];
	zi=pos[1];

	for(int i=0;i<iterations;i++)
	{
		// Perform complex multiplication
		zr_hold=zr;
		zr=zr*zr-zi*zi + cr;
		zi=zr_hold*zi*2 + ci;

		// Use "broken" algorithm, if requested (looks weird)
		if(broken)zr+=zi;

		// Calculate Magnitude
		mag=zr*zr+zi*zi;

		if(mag>squared_bailout)
		{
			if(smooth_outside)
			{
				// Darco's original mandelbrot smoothing algo
				// depth=((Point::value_type)i+(2.0-sqrt(mag))/PI);

				// Linas Vepstas algo (Better than darco's)
				// See (http://linas.org/art-gallery/escape/smooth.html)
				depth= (ColorReal)i - log(log(sqrt(mag))) / LOG_OF_2;

				// Clamp
				if(depth<0) depth=0;
			}
			else
				depth=static_cast<ColorReal>(i);

			if(solid_outside)
				ret=ocolor;
			else
				if(distort_outside)
					ret=context.get_color(Point(zr,zi));
				else
					ret=context.get_color(pos);

			if(invert_outside)
				ret=~ret;

			if(color_outside)
				ret=ret.set_uv(zr,zi).clamped_negative();

			if(color_cycle)
				ret=ret.rotate_uv(color_shift.operator*(depth)).clamped_negative();

			if(shade_outside)
			{
				ColorReal alpha=depth/static_cast<ColorReal>(iterations);
				ret=(ocolor-ret)*alpha+ret;
			}
			return ret;
		}
	}

	if(solid_inside)
		ret=icolor;
	else
		if(distort_inside)
			ret=context.get_color(Point(zr,zi));
		else
			ret=context.get_color(pos);

	if(invert_inside)
		ret=~ret;

	if(color_inside)
		ret=ret.set_uv(zr,zi).clamped_negative();

	if(shade_inside)
		ret=(icolor-ret)*mag+ret;

	return ret;
}

Layer::Vocab
Julia::get_param_vocab()const
{
	Layer::Vocab ret;

	ret.push_back(ParamDesc("icolor")
		.set_local_name(_("Inside Color"))
		.set_description(_("Color of the Set"))
	);
	ret.push_back(ParamDesc("ocolor")
		.set_local_name(_("Outside Color"))
		.set_description(_("Color outside the Set"))
	);
	ret.push_back(ParamDesc("color_shift")
		.set_local_name(_("Color Shift"))
	);
	ret.push_back(ParamDesc("iterations")
		.set_local_name(_("Iterations"))
	);
	ret.push_back(ParamDesc("seed")
		.set_local_name(_("Seed Point"))
	);
	ret.push_back(ParamDesc("bailout")
		.set_local_name(_("Bailout ValueBase"))
	);

	ret.push_back(ParamDesc("distort_inside")
		.set_local_name(_("Distort Inside"))
	);
	ret.push_back(ParamDesc("shade_inside")
		.set_local_name(_("Shade Inside"))
	);
	ret.push_back(ParamDesc("solid_inside")
		.set_local_name(_("Solid Inside"))
	);
	ret.push_back(ParamDesc("invert_inside")
		.set_local_name(_("Invert Inside"))
	);
	ret.push_back(ParamDesc("color_inside")
		.set_local_name(_("Color Inside"))
	);
	ret.push_back(ParamDesc("distort_outside")
		.set_local_name(_("Distort Outside"))
	);
	ret.push_back(ParamDesc("shade_outside")
		.set_local_name(_("Shade Outside"))
	);
	ret.push_back(ParamDesc("solid_outside")
		.set_local_name(_("Solid Outside"))
	);
	ret.push_back(ParamDesc("invert_outside")
		.set_local_name(_("Invert Outside"))
	);
	ret.push_back(ParamDesc("color_outside")
		.set_local_name(_("Color Outside"))
	);

	ret.push_back(ParamDesc("color_cycle")
		.set_local_name(_("Color Cycle"))
	);
	ret.push_back(ParamDesc("smooth_outside")
		.set_local_name(_("Smooth Outside"))
		.set_description(_("When checked, smoothes the coloration outside the set"))
	);
	ret.push_back(ParamDesc("broken")
		.set_local_name(_("Break Set"))
		.set_description(_("Modify equation to achieve interesting results"))
	);


	return ret;
}

rendering::Task::Handle
Julia::build_rendering_task_vfunc(Context context) const
{
	rendering::Task::Handle task = context.build_rendering_task();

	TaskJulia::Handle task_julia(new TaskJulia());
	task_julia->icolor = param_icolor.get(Color());
	task_julia->ocolor = param_ocolor.get(Color());
	task_julia->color_shift = param_color_shift.get(Angle());
	task_julia->iterations = param_iterations.get(int());
	task_julia->seed = param_seed.get(Point());
	task_julia->distort_inside = param_distort_inside.get(bool());
	task_julia->shade_inside = param_shade_inside.get(bool());
	task_julia->solid_inside = param_solid_inside.get(bool());
	task_julia->invert_inside = param_invert_inside.get(bool());
	task_julia->color_inside = param_color_inside.get(bool());
	task_julia->distort_outside = param_distort_outside.get(bool());
	task_julia->shade_outside = param_shade_outside.get(bool());
	task_julia->solid_outside = param_solid_outside.get(bool());
	task_julia->invert_outside = param_invert_outside.get(bool());
	task_julia->color_outside = param_color_outside.get(bool());

	task_julia->color_cycle = param_color_cycle.get(bool());
	task_julia->smooth_outside = param_smooth_outside.get(bool());
	task_julia->broken = param_broken.get(bool());
	task_julia->squared_bailout = squared_bailout;

	task_julia->sub_task() = task;

	task = task_julia;

	return task;
}
