/*! ========================================================================
** Sinfg
** Template File
** $Id: mandelbrot.cpp,v 1.1.1.1 2005/01/04 01:23:10 darco Exp $
**
** Copyright (c) 2002 Robert B. Quattlebaum Jr.
**
** This software and associated documentation
** are CONFIDENTIAL and PROPRIETARY property of
** the above-mentioned copyright holder.
**
** You may not copy, print, publish, or in any
** other way distribute this software without
** a prior written agreement with
** the copyright holder.
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

#include <sinfg/string.h>
#include <sinfg/time.h>
#include <sinfg/context.h>
#include <sinfg/paramdesc.h>
#include <sinfg/renddesc.h>
#include <sinfg/surface.h>
#include <sinfg/value.h>
#include <sinfg/valuenode.h>

#endif

/* === M A C R O S ========================================================= */

#define LOG_OF_2		0.69314718055994528623

/* === G L O B A L S ======================================================= */

SINFG_LAYER_INIT(Mandelbrot);
SINFG_LAYER_SET_NAME(Mandelbrot,"mandelbrot");
SINFG_LAYER_SET_LOCAL_NAME(Mandelbrot,_("Mandelbrot Set"));
SINFG_LAYER_SET_CATEGORY(Mandelbrot,_("Fractals"));
SINFG_LAYER_SET_VERSION(Mandelbrot,"0.2");
SINFG_LAYER_SET_CVS_ID(Mandelbrot,"$Id: mandelbrot.cpp,v 1.1.1.1 2005/01/04 01:23:10 darco Exp $");

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
	gradient_offset_inside(0.0),
	gradient_offset_outside(0.0),
	gradient_loop_inside(true),
	gradient_scale_outside(1.0),
	gradient_inside(Color::alpha(),Color::black()),
	gradient_outside(Color::alpha(),Color::black())
{
	iterations=32;
//	color_shift=Angle::deg(0);

	distort_inside=true;
	distort_outside=true;
	solid_inside=false;
	solid_outside=false;
	invert_inside=false;
	invert_outside=false;
	shade_inside=true;
	shade_outside=true;

	smooth_outside=true;
	broken=false;
	
	bailout=4;
	lp=log(log(bailout));
}
	
bool
Mandelbrot::set_param(const String & param, const ValueBase &value)
{

//	IMPORT(color_shift);

	IMPORT(gradient_offset_inside);
	IMPORT(gradient_offset_outside);
	IMPORT(gradient_loop_inside);
	IMPORT(gradient_scale_outside);
	
	IMPORT(distort_inside);
	IMPORT(distort_outside);
	IMPORT(solid_inside);
	IMPORT(solid_outside);
	IMPORT(invert_inside);
	IMPORT(invert_outside);
	IMPORT(shade_inside);
	IMPORT(shade_outside);

	IMPORT(smooth_outside);
	IMPORT(broken);

	IMPORT(gradient_inside);
	IMPORT(gradient_outside);
	
	if(param=="iterations" && value.same_as(iterations))
	{
		iterations=value.get(iterations);
		if(iterations<0)
			iterations=0;
		if(iterations>500000)
			iterations=500000;
		return true;
	}
	if(param=="bailout" && value.same_as(bailout))
	{
		bailout=value.get(bailout);
		bailout*=bailout;
		lp=log(log(bailout));
		return true;
	}

	return false;
}

ValueBase
Mandelbrot::get_param(const String & param)const
{
//	EXPORT(icolor);
//	EXPORT(ocolor);
//	EXPORT(color_shift);
	EXPORT(iterations);

	EXPORT(gradient_offset_inside);
	EXPORT(gradient_offset_outside);
	EXPORT(gradient_loop_inside);
	EXPORT(gradient_scale_outside);

	EXPORT(distort_inside);
	EXPORT(distort_outside);
	EXPORT(solid_inside);
	EXPORT(solid_outside);
	EXPORT(invert_inside);
	EXPORT(invert_outside);
	EXPORT(shade_inside);
	EXPORT(shade_outside);
	EXPORT(smooth_outside);
	EXPORT(broken);

	EXPORT(gradient_inside);
	EXPORT(gradient_outside);
	
	if(param=="bailout")
		return sqrt(bailout);

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

Color
Mandelbrot::get_color(Context context, const Point &pos)const
{
	Real
		cr, ci,
		zr, zi,
		zr_hold;
	
	ColorReal
		depth, mag;
	
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
