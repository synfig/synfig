/* === S Y N F I G ========================================================= */
/*!	\file mandelbrot.cpp
**	\brief Implementation of the "Mandelbrot Set" layer
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

#include "mandelbrot.h"

#include <synfig/localization.h>
#include <synfig/general.h>

#include <synfig/string.h>
#include <synfig/time.h>
#include <synfig/context.h>
#include <synfig/paramdesc.h>
#include <synfig/renddesc.h>
#include <synfig/surface.h>
#include <synfig/value.h>
#include <synfig/valuenode.h>

#endif

using namespace std;
using namespace etl;
using namespace synfig;
using namespace modules;
using namespace lyr_std;

/* === M A C R O S ========================================================= */

#define LOG_OF_2		0.69314718055994528623

/* === G L O B A L S ======================================================= */

SYNFIG_LAYER_INIT(Mandelbrot);
SYNFIG_LAYER_SET_NAME(Mandelbrot,"mandelbrot");
SYNFIG_LAYER_SET_LOCAL_NAME(Mandelbrot,N_("Mandelbrot Set"));
SYNFIG_LAYER_SET_CATEGORY(Mandelbrot,N_("Fractals"));
SYNFIG_LAYER_SET_VERSION(Mandelbrot,"0.2");

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

Mandelbrot::Mandelbrot():
	param_gradient_inside(ValueBase(Gradient(Color::alpha(),Color::black()))),
	param_gradient_offset_inside(ValueBase(Real(0.0))),
	param_gradient_loop_inside(ValueBase(true)),
	param_gradient_outside(ValueBase(Gradient(Color::alpha(),Color::black()))),
	param_gradient_offset_outside(ValueBase(Real(0.0))),
	param_gradient_scale_outside(ValueBase(Real(1.0)))
{
	param_iterations=ValueBase(int(32));

	param_distort_inside=ValueBase(true);
	param_distort_outside=ValueBase(true);
	param_solid_inside=ValueBase(false);
	param_solid_outside=ValueBase(false);
	param_invert_inside=ValueBase(false);
	param_invert_outside=ValueBase(false);
	param_shade_inside=ValueBase(true);
	param_shade_outside=ValueBase(true);

	param_smooth_outside=ValueBase(true);
	param_broken=ValueBase(false);

	param_bailout=ValueBase(Real(4));
	lp=log(log(param_bailout.get(Real())));

	SET_INTERPOLATION_DEFAULTS();
	SET_STATIC_DEFAULTS();
}

bool
Mandelbrot::set_param(const String & param, const ValueBase &value)
{

	IMPORT_VALUE(param_gradient_offset_inside);
	IMPORT_VALUE(param_gradient_offset_outside);
	IMPORT_VALUE(param_gradient_loop_inside);
	IMPORT_VALUE(param_gradient_scale_outside);

	IMPORT_VALUE(param_distort_inside);
	IMPORT_VALUE(param_distort_outside);
	IMPORT_VALUE(param_solid_inside);
	IMPORT_VALUE(param_solid_outside);
	IMPORT_VALUE(param_invert_inside);
	IMPORT_VALUE(param_invert_outside);
	IMPORT_VALUE(param_shade_inside);
	IMPORT_VALUE(param_shade_outside);

	IMPORT_VALUE(param_smooth_outside);
	IMPORT_VALUE(param_broken);

	IMPORT_VALUE(param_gradient_inside);
	IMPORT_VALUE(param_gradient_outside);

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
Mandelbrot::get_param(const String & param)const
{
	EXPORT_VALUE(param_iterations);

	EXPORT_VALUE(param_gradient_offset_inside);
	EXPORT_VALUE(param_gradient_offset_outside);
	EXPORT_VALUE(param_gradient_loop_inside);
	EXPORT_VALUE(param_gradient_scale_outside);

	EXPORT_VALUE(param_distort_inside);
	EXPORT_VALUE(param_distort_outside);
	EXPORT_VALUE(param_solid_inside);
	EXPORT_VALUE(param_solid_outside);
	EXPORT_VALUE(param_invert_inside);
	EXPORT_VALUE(param_invert_outside);
	EXPORT_VALUE(param_shade_inside);
	EXPORT_VALUE(param_shade_outside);
	EXPORT_VALUE(param_smooth_outside);
	EXPORT_VALUE(param_broken);

	EXPORT_VALUE(param_gradient_inside);
	EXPORT_VALUE(param_gradient_outside);
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

Layer::Vocab
Mandelbrot::get_param_vocab()const
{
	Layer::Vocab ret;


	ret.push_back(ParamDesc("iterations")
		.set_local_name(_("Iterations"))
	);
	ret.push_back(ParamDesc("bailout")
		.set_local_name(_("Bailout ValueBase"))
	);

	ret.push_back(ParamDesc("broken")
		.set_local_name(_("Break Set"))
		.set_description(_("Modify equation to achieve interesting results"))
	);


	ret.push_back(ParamDesc("distort_inside")
		.set_local_name(_("Distort Inside"))
		.set_group(_("Inside"))
	);
	ret.push_back(ParamDesc("shade_inside")
		.set_local_name(_("Shade Inside"))
		.set_group(_("Inside"))
	);
	ret.push_back(ParamDesc("solid_inside")
		.set_local_name(_("Solid Inside"))
		.set_group(_("Inside"))
	);
	ret.push_back(ParamDesc("invert_inside")
		.set_local_name(_("Invert Inside"))
		.set_group(_("Inside"))
	);
	ret.push_back(ParamDesc("gradient_inside")
		.set_local_name(_("Gradient Inside"))
		.set_group(_("Inside"))
	);
	ret.push_back(ParamDesc("gradient_offset_inside")
		.set_local_name(_("Offset Inside"))
		.set_group(_("Inside"))
	);
	ret.push_back(ParamDesc("gradient_loop_inside")
		.set_local_name(_("Loop Inside"))
		.set_group(_("Inside"))
	);

	ret.push_back(ParamDesc("distort_outside")
		.set_local_name(_("Distort Outside"))
		.set_group(_("Outside"))
	);
	ret.push_back(ParamDesc("shade_outside")
		.set_local_name(_("Shade Outside"))
		.set_group(_("Outside"))
	);
	ret.push_back(ParamDesc("solid_outside")
		.set_local_name(_("Solid Outside"))
		.set_group(_("Outside"))
	);
	ret.push_back(ParamDesc("invert_outside")
		.set_local_name(_("Invert Outside"))
		.set_group(_("Outside"))
	);
	ret.push_back(ParamDesc("gradient_outside")
		.set_local_name(_("Gradient outside"))
		.set_group(_("Outside"))
	);
	ret.push_back(ParamDesc("smooth_outside")
		.set_local_name(_("Smooth Outside"))
		.set_description(_("Smooth the coloration outside the set"))
		.set_group(_("Outside"))
	);
	ret.push_back(ParamDesc("gradient_offset_outside")
		.set_local_name(_("Offset Outside"))
		.set_group(_("Outside"))
	);
	ret.push_back(ParamDesc("gradient_scale_outside")
		.set_local_name(_("Scale Outside"))
		.set_group(_("Outside"))
	);

	return ret;
}

RendDesc
Mandelbrot::get_sub_renddesc_vfunc(const RendDesc &renddesc) const
{
	RendDesc desc(renddesc);
	desc.set_wh(512, 512);
	desc.set_tl(Vector(-5.0, -5.0));
	desc.set_br(Vector( 5.0,  5.0));
	return desc;
}

Color
Mandelbrot::get_color(Context context, const Point &pos)const
{
	int iterations=param_iterations.get(int());
	Real bailout=param_bailout.get(Real());
	bool broken=param_broken.get(bool());

	bool distort_inside=param_distort_inside.get(bool());
	bool shade_inside=param_shade_inside.get(bool());
	bool solid_inside=param_solid_inside.get(bool());
	bool invert_inside=param_invert_inside.get(bool());
	Gradient gradient_inside=param_gradient_inside.get(Gradient());
	Real gradient_offset_inside=param_gradient_offset_inside.get(Real());
	bool gradient_loop_inside=param_gradient_loop_inside.get(bool());

	bool distort_outside=param_distort_outside.get(bool());
	bool shade_outside=param_shade_outside.get(bool());
	bool solid_outside=param_solid_outside.get(bool());
	bool invert_outside=param_invert_outside.get(bool());
	Gradient gradient_outside=param_gradient_outside.get(Gradient());
	bool smooth_outside=param_smooth_outside.get(bool());
	Real gradient_offset_outside=param_gradient_offset_outside.get(Real());
	Real gradient_scale_outside=param_gradient_scale_outside.get(Real());
	
	Real
		cr, ci,
		zr, zi,
		zr_hold;

	ColorReal
		depth, mag(0);

	Color
		ret;


	zr=zi=0;
	cr=pos[0];
	ci=pos[1];

	for(int i=0;i<iterations;i++)
	{
		// Perform complex multiplication
		zr_hold=zr;
		zr=zr*zr-zi*zi + cr;
		if(broken)zr+=zi; // Use "broken" algorithm, if requested (looks weird)
		zi=zr_hold*zi*2 + ci;


		// Calculate Magnitude
		mag=zr*zr+zi*zi;

		if(mag>bailout)
		{
			if(smooth_outside)
			{
				// Darco's original mandelbrot smoothing algo
				// depth=((Point::value_type)i+(2.0-sqrt(mag))/PI);

				// Linas Vepstas algo (Better than darco's)
				// See (http://linas.org/art-gallery/escape/smooth.html)
				depth= (ColorReal)i + LOG_OF_2*lp - log(log(sqrt(mag))) / LOG_OF_2;

				// Clamp
				if(depth<0) depth=0;
			}
			else
				depth=static_cast<ColorReal>(i);

			ColorReal amount(depth/static_cast<ColorReal>(iterations));
			amount=amount*gradient_scale_outside+gradient_offset_outside;
			amount-=floor(amount);

			if(solid_outside)
				ret=gradient_outside(amount);
			else
			{
				if(distort_outside)
					ret=context.get_color(Point(pos[0]+zr,pos[1]+zi));
				else
					ret=context.get_color(pos);

				if(invert_outside)
					ret=~ret;

				if(shade_outside)
					ret=Color::blend(gradient_outside(amount), ret, 1.0);
			}


			return ret;
		}
	}

	ColorReal amount(abs(mag+gradient_offset_inside));
	if(gradient_loop_inside)
		amount-=floor(amount);

	if(solid_inside)
		ret=gradient_inside(amount);
	else
	{
		if(distort_inside)
			ret=context.get_color(Point(pos[0]+zr,pos[1]+zi));
		else
			ret=context.get_color(pos);

		if(invert_inside)
			ret=~ret;

		if(shade_inside)
			ret=Color::blend(gradient_inside(amount), ret, 1.0);
	}

	return ret;
}
