/* === S Y N F I G ========================================================= */
/*!	\file spiralgradient.cpp
**	\brief Template Header
**
**	$Id: spiralgradient.cpp,v 1.1.1.1 2005/01/04 01:23:11 darco Exp $
**
**	\legal
**	Copyright (c) 2002-2005 Robert B. Quattlebaum Jr., Adrian Bentley
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

#include <synfig/string.h>
#include <synfig/time.h>
#include <synfig/context.h>
#include <synfig/paramdesc.h>
#include <synfig/renddesc.h>
#include <synfig/surface.h>
#include <synfig/value.h>
#include <synfig/valuenode.h>

#include "spiralgradient.h"

#endif

/* === U S I N G =========================================================== */

using namespace etl;
using namespace std;
using namespace synfig;

/* === G L O B A L S ======================================================= */

SYNFIG_LAYER_INIT(SpiralGradient);
SYNFIG_LAYER_SET_NAME(SpiralGradient,"spiral_gradient");
SYNFIG_LAYER_SET_LOCAL_NAME(SpiralGradient,_("Spiral Gradient"));
SYNFIG_LAYER_SET_CATEGORY(SpiralGradient,_("Gradients"));
SYNFIG_LAYER_SET_VERSION(SpiralGradient,"0.1");
SYNFIG_LAYER_SET_CVS_ID(SpiralGradient,"$Id: spiralgradient.cpp,v 1.1.1.1 2005/01/04 01:23:11 darco Exp $");

/* === P R O C E D U R E S ================================================= */

/* === M E T H O D S ======================================================= */

/* === E N T R Y P O I N T ================================================= */

SpiralGradient::SpiralGradient():
	Layer_Composite(1.0,Color::BLEND_STRAIGHT),
	gradient(Color::black(),Color::white()),
	center(0,0),
	radius(0.5),
	angle(Angle::zero()),
	clockwise(false)
{
}

bool
SpiralGradient::set_param(const String & param, const ValueBase &value)
{
	IMPORT(gradient);
	IMPORT(center);
	IMPORT(radius);
	IMPORT(angle);
	IMPORT(clockwise);

	return Layer_Composite::set_param(param,value);
}

ValueBase
SpiralGradient::get_param(const String &param)const
{
	EXPORT(gradient);
	EXPORT(center);
	EXPORT(radius);
	EXPORT(angle);
	EXPORT(clockwise);

	EXPORT_NAME();
	EXPORT_VERSION();

	return Layer_Composite::get_param(param);
}

Layer::Vocab
SpiralGradient::get_param_vocab()const
{
	Layer::Vocab ret(Layer_Composite::get_param_vocab());

	ret.push_back(ParamDesc("gradient")
		.set_local_name(_("Gradient"))
	);

	ret.push_back(ParamDesc("center")
		.set_local_name(_("Center"))
	);

	ret.push_back(ParamDesc("radius")
		.set_local_name(_("Radius"))
		.set_description(_("This is the radius of the circle"))
		.set_is_distance()
		.set_origin("center")
	);

	ret.push_back(ParamDesc("angle")
		.set_local_name(_("Angle"))
		.set_origin("center")
	);

	ret.push_back(ParamDesc("clockwise")
		.set_local_name(_("Clockwise"))
	);

	return ret;
}

inline Color
SpiralGradient::color_func(const Point &pos, float supersample)const
{
	const Point centered(pos-center);
	Angle a;
	a=Angle::tan(-centered[1],centered[0]).mod();
	a=a+angle;

	if(supersample<0.00001)supersample=0.00001;

	Real dist((pos-center).mag()/radius);
	if(clockwise)
		dist+=Angle::rot(a.mod()).get();
	else
		dist-=Angle::rot(a.mod()).get();

	dist-=floor(dist);
	if(dist+supersample*0.5>1.0)
	{
		Color pool(gradient(dist,supersample*0.5).premult_alpha()*(1.0-(dist-supersample*0.5)));
		pool+=gradient((dist+supersample*0.5)-1.0,supersample*0.5).premult_alpha()*((dist+supersample*0.5)-1.0);
		return pool.demult_alpha();
	}
	if(dist-supersample*0.5<0.0)
	{
		Color pool(gradient(dist,supersample*0.5).premult_alpha()*(dist+supersample*0.5));
		pool+=gradient(1.0-(dist-supersample*0.5),supersample*0.5).premult_alpha()*(-(dist-supersample*0.5));
		return pool.demult_alpha();
	}

	return gradient(dist,supersample);
}

float
SpiralGradient::calc_supersample(const synfig::Point &x, float pw,float ph)const
{
	return (1.41421*pw/radius+(1.41421*pw/Point(x-center).mag())/(PI*2))*0.5;
}

synfig::Layer::Handle
SpiralGradient::hit_check(synfig::Context context, const synfig::Point &point)const
{
	if(get_blend_method()==Color::BLEND_STRAIGHT && get_amount()>=0.5)
		return const_cast<SpiralGradient*>(this);
	if(get_amount()==0.0)
		return context.hit_check(point);
	if((get_blend_method()==Color::BLEND_STRAIGHT || get_blend_method()==Color::BLEND_COMPOSITE) && color_func(point).get_a()>0.5)
		return const_cast<SpiralGradient*>(this);
	return context.hit_check(point);
}

Color
SpiralGradient::get_color(Context context, const Point &pos)const
{
	const Color color(color_func(pos));

	if(get_amount()==1.0 && get_blend_method()==Color::BLEND_STRAIGHT)
		return color;
	else
		return Color::blend(color,context.get_color(pos),get_amount(),get_blend_method());
}

bool
SpiralGradient::accelerated_render(Context context,Surface *surface,int quality, const RendDesc &renddesc, ProgressCallback *cb)const
{
	SuperCallback supercb(cb,0,9500,10000);

	if(get_amount()==1.0 && get_blend_method()==Color::BLEND_STRAIGHT)
	{
		surface->set_wh(renddesc.get_w(),renddesc.get_h());
	}
	else
	{
		if(!context.accelerated_render(surface,quality,renddesc,&supercb))
			return false;
		if(get_amount()==0)
			return true;
	}


	int x,y;

	Surface::pen pen(surface->begin());
	const Real pw(renddesc.get_pw()),ph(renddesc.get_ph());
	Point pos;
	Point tl(renddesc.get_tl());
	const int w(surface->get_w());
	const int h(surface->get_h());

	if(get_amount()==1.0 && get_blend_method()==Color::BLEND_STRAIGHT)
	{
		for(y=0,pos[1]=tl[1];y<h;y++,pen.inc_y(),pen.dec_x(x),pos[1]+=ph)
			for(x=0,pos[0]=tl[0];x<w;x++,pen.inc_x(),pos[0]+=pw)
				pen.put_value(color_func(pos,calc_supersample(pos,pw,ph)));
	}
	else
	{
		for(y=0,pos[1]=tl[1];y<h;y++,pen.inc_y(),pen.dec_x(x),pos[1]+=ph)
			for(x=0,pos[0]=tl[0];x<w;x++,pen.inc_x(),pos[0]+=pw)
				pen.put_value(Color::blend(color_func(pos,calc_supersample(pos,pw,ph)),pen.get_value(),get_amount(),get_blend_method()));
	}

	// Mark our progress as finished
	if(cb && !cb->amount_complete(10000,10000))
		return false;

	return true;
}
