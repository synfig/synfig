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

#include <synfig/string.h>
#include <synfig/time.h>
#include <synfig/context.h>
#include <synfig/paramdesc.h>
#include <synfig/renddesc.h>
#include <synfig/value.h>

#endif

using namespace etl;
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
SYNFIG_LAYER_SET_VERSION(Julia,"0.1");

/* === P R O C E D U R E S ================================================= */

inline void
color_neg_flip(Color &color)
{
	if(color.get_a()==0)
	{
		color=Color::alpha();
		return;
	}

	if(color.get_a()<0)
		color=-color;

	if(color.get_r()<0)
	{
		color.set_g(color.get_g()-color.get_r());
		color.set_b(color.get_b()-color.get_r());
		color.set_r(0);
	}
	if(color.get_g()<0)
	{
		color.set_r(color.get_r()-color.get_g());
		color.set_b(color.get_b()-color.get_g());
		color.set_g(0);
	}
	if(color.get_b()<0)
	{
		color.set_r(color.get_r()-color.get_b());
		color.set_g(color.get_g()-color.get_b());
		color.set_b(0);
	}
}

/* === M E T H O D S ======================================================= */

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

	param_bailout=ValueBase(Real(4));
	lp=log(log(param_bailout.get(Real())));

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

	IMPORT_VALUE_PLUS(param_iterations,
	{
		int iterations=param_iterations.get(int());
		iterations=value.get(iterations);
		if(iterations<0)
			iterations=0;
		if(iterations>500000)
			iterations=500000;
		param_iterations.set(iterations);
		return true;
	}
	);
	IMPORT_VALUE_PLUS(param_bailout,
	{
		Real bailout=param_bailout.get(Real());
		bailout=value.get(bailout);
		bailout*=bailout;
		lp=log(log(bailout));
		param_bailout.set(bailout);
		return true;
	}
	);

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

	if(param=="bailout")
	{
		// This line is needed to copy the static and interpolation options
		ValueBase ret(param_bailout);
		ret.set(sqrt(param_bailout.get(Real())));
		return ret;
	}

	EXPORT_NAME();
	EXPORT_VERSION();

	return ValueBase();
}

RendDesc
Julia::get_sub_renddesc_vfunc(const RendDesc &renddesc) const
{
	RendDesc desc(renddesc);
	desc.set_wh(512, 512);
	desc.set_tl(Vector(-5.0, -5.0));
	desc.set_br(Vector( 5.0,  5.0));
	return desc;
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

		if(mag>4)
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
