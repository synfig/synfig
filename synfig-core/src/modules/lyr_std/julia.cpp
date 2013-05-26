/* === S Y N F I G ========================================================= */
/*!	\file julia.cpp
**	\brief Implementation of the "Julia Set" layer
**
**	$Id$
**
**	\legal
**	Copyright (c) 2002-2005 Robert B. Quattlebaum Jr., Adrian Bentley
**	Copyright (c) 2007 Chris Moore
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
**
** === N O T E S ===========================================================
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

#include <synfig/string.h>
#include <synfig/time.h>
#include <synfig/context.h>
#include <synfig/paramdesc.h>
#include <synfig/renddesc.h>
#include <synfig/surface.h>
#include <synfig/value.h>
#include <synfig/valuenode.h>

#endif

/* === M A C R O S ========================================================= */

#define LOG_OF_2		0.69314718055994528623

/* === G L O B A L S ======================================================= */

SYNFIG_LAYER_INIT(Julia);
SYNFIG_LAYER_SET_NAME(Julia,"julia");
SYNFIG_LAYER_SET_LOCAL_NAME(Julia,N_("Julia Set"));
SYNFIG_LAYER_SET_CATEGORY(Julia,N_("Fractals"));
SYNFIG_LAYER_SET_VERSION(Julia,"0.1");
SYNFIG_LAYER_SET_CVS_ID(Julia,"$Id$");

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

Julia::Julia():color_shift(angle::degrees(0))
{
	icolor=Color::black();
	ocolor=Color::black();
	iterations=32;
	color_shift=Angle::deg(0);

	distort_inside=true;
	distort_outside=true;
	shade_inside=true;
	shade_outside=true;
	solid_inside=false;
	solid_outside=false;
	invert_inside=false;
	invert_outside=false;
	color_inside=true;
	color_outside=false;
	color_cycle=false;
	smooth_outside=true;
	broken=false;
	seed=Point(0,0);

	bailout=4;
	lp=log(log(bailout));

}

bool
Julia::set_param(const String & param, const ValueBase &value)
{

	IMPORT(icolor);
	IMPORT(ocolor);
	IMPORT(color_shift);
	IMPORT(seed);

	IMPORT(distort_inside);
	IMPORT(distort_outside);
	IMPORT(shade_inside);
	IMPORT(shade_outside);
	IMPORT(solid_inside);
	IMPORT(solid_outside);
	IMPORT(invert_inside);
	IMPORT(invert_outside);
	IMPORT(color_inside);
	IMPORT(color_outside);

	IMPORT(color_cycle);
	IMPORT(smooth_outside);
	IMPORT(broken);

// TODO: Use IMPORT_PLUS
	if(param=="iterations" && value.same_type_as(iterations))
	{
		iterations=value.get(iterations);
		if(iterations<0)
			iterations=0;
		if(iterations>500000)
			iterations=500000;
		set_param_static(param, value.get_static());
		return true;
	}
	if(param=="bailout" && value.same_type_as(bailout))
	{
		bailout=value.get(bailout);
		bailout*=bailout;
		lp=log(log(bailout));
		set_param_static(param, value.get_static());
		return true;
	}

	return false;
}

ValueBase
Julia::get_param(const String & param)const
{
	EXPORT(icolor);
	EXPORT(ocolor);
	EXPORT(color_shift);
	EXPORT(iterations);
	EXPORT(seed);

	EXPORT(distort_inside);
	EXPORT(distort_outside);
	EXPORT(shade_inside);
	EXPORT(shade_outside);
	EXPORT(solid_inside);
	EXPORT(solid_outside);
	EXPORT(invert_inside);
	EXPORT(invert_outside);
	EXPORT(color_inside);
	EXPORT(color_outside);
	EXPORT(color_cycle);
	EXPORT(smooth_outside);
	EXPORT(broken);

	if(param=="bailout")
	{
		ValueBase ret(sqrt(bailout));
		
		return ret;
	}

	EXPORT_NAME();
	EXPORT_VERSION();

	return ValueBase();
}

Color
Julia::get_color(Context context, const Point &pos)const
{
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
		.set_description(_("Smooth the coloration outside the set"))
	);
	ret.push_back(ParamDesc("broken")
		.set_local_name(_("Break Set"))
		.set_description(_("Modify equation to achieve interesting results"))
	);


	return ret;
}
