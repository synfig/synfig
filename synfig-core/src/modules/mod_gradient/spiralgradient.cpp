/* === S Y N F I G ========================================================= */
/*!	\file spiralgradient.cpp
**	\brief Implementation of the "Spiral Gradient" layer
**
**	$Id$
**
**	\legal
**	Copyright (c) 2002-2005 Robert B. Quattlebaum Jr., Adrian Bentley
**	Copyright (c) 2011 Carlos López
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

#include <synfig/localization.h>
#include <synfig/general.h>

#include <synfig/string.h>
#include <synfig/context.h>
#include <synfig/paramdesc.h>
#include <synfig/renddesc.h>
#include <synfig/surface.h>
#include <synfig/value.h>
#include <synfig/cairo_renddesc.h>

#include "spiralgradient.h"

#endif

/* === U S I N G =========================================================== */

using namespace etl;
using namespace std;
using namespace synfig;

/* === G L O B A L S ======================================================= */

SYNFIG_LAYER_INIT(SpiralGradient);
SYNFIG_LAYER_SET_NAME(SpiralGradient,"spiral_gradient");
SYNFIG_LAYER_SET_LOCAL_NAME(SpiralGradient,N_("Spiral Gradient"));
SYNFIG_LAYER_SET_CATEGORY(SpiralGradient,N_("Gradients"));
SYNFIG_LAYER_SET_VERSION(SpiralGradient,"0.1");

/* === P R O C E D U R E S ================================================= */

/* === M E T H O D S ======================================================= */

/* === E N T R Y P O I N T ================================================= */

SpiralGradient::SpiralGradient():
	Layer_Composite(1.0,Color::BLEND_COMPOSITE),
	param_gradient(ValueBase(Gradient(Color::black(),Color::white()))),
	param_center(ValueBase(Point(0,0))),
	param_radius(ValueBase(Real(0.5))),
	param_angle(ValueBase(Angle::zero())),
	param_clockwise(ValueBase(false))
{
	SET_INTERPOLATION_DEFAULTS();
	SET_STATIC_DEFAULTS();
}

bool
SpiralGradient::set_param(const String & param, const ValueBase &value)
{
	IMPORT_VALUE_PLUS(param_gradient, compile());
	IMPORT_VALUE(param_center);
	IMPORT_VALUE(param_radius);
	IMPORT_VALUE(param_angle);
	IMPORT_VALUE(param_clockwise);

	return Layer_Composite::set_param(param,value);
}

ValueBase
SpiralGradient::get_param(const String &param)const
{
	EXPORT_VALUE(param_gradient);
	EXPORT_VALUE(param_center);
	EXPORT_VALUE(param_radius);
	EXPORT_VALUE(param_angle);
	EXPORT_VALUE(param_clockwise);

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
		.set_description(_("Gradient to apply"))
	);

	ret.push_back(ParamDesc("center")
		.set_local_name(_("Center"))
		.set_description(_("Center of the gradient"))
		.set_is_distance()
	);

	ret.push_back(ParamDesc("radius")
		.set_local_name(_("Radius"))
		.set_description(_("Radius of the circle"))
		.set_is_distance()
		.set_origin("center")
	);

	ret.push_back(ParamDesc("angle")
		.set_local_name(_("Angle"))
		.set_description(_("Rotation of the spiral"))
		.set_origin("center")
	);

	ret.push_back(ParamDesc("clockwise")
		.set_local_name(_("Clockwise"))
		.set_description(_("When checked, the spiral turns clockwise"))
	);

	return ret;
}

void
SpiralGradient::compile()
	{ compiled_gradient.set(param_gradient.get(Gradient()), true); }

inline Color
SpiralGradient::color_func(const Point &pos, Real supersample)const
{
	Point center=param_center.get(Point());
	Real radius=param_radius.get(Real());
	Angle angle=param_angle.get(Angle());
	bool clockwise=param_clockwise.get(bool());
	
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

	supersample *= 0.5;
	return compiled_gradient.average(dist - supersample, dist + supersample);
}

Real
SpiralGradient::calc_supersample(const synfig::Point &x, Real pw, Real /*ph*/)const
{
	Point center=param_center.get(Point());
	Real radius=param_radius.get(Real());

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
	RENDER_TRANSFORMED_IF_NEED(__FILE__, __LINE__)

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

////
bool
SpiralGradient::accelerated_cairorender(Context context, cairo_t *cr,int quality, const RendDesc &renddesc_, ProgressCallback *cb)const
{
	RendDesc	renddesc(renddesc_);
	
	// Untransform the render desc
	if(!cairo_renddesc_untransform(cr, renddesc))
		return false;

	Point pos;
	const Real pw(renddesc.get_pw()),ph(renddesc.get_ph());
	const Point tl(renddesc.get_tl());
	const int w(renddesc.get_w());
	const int h(renddesc.get_h());
	
	SuperCallback supercb(cb,0,9500,10000);
	
	if(get_amount()==1.0 && get_blend_method()==Color::BLEND_STRAIGHT)
	{
		cairo_save(cr);
		cairo_set_operator(cr, CAIRO_OPERATOR_CLEAR);
		cairo_paint(cr);
		cairo_restore(cr);
	}
	else
	{
		if(!context.accelerated_cairorender(cr,quality,renddesc,&supercb))
			return false;
		if(get_amount()==0)
			return true;
	}
	
	
	int x,y;
	cairo_surface_t *surface;
	
	surface=cairo_surface_create_similar(cairo_get_target(cr), CAIRO_CONTENT_COLOR_ALPHA, w, h);
	
	CairoSurface csurface(surface);
	if(!csurface.map_cairo_image())
	{
		synfig::warning("Spiral Gradient: map cairo surface failed");
		return false;
	}
	for(y=0,pos[1]=tl[1];y<h;y++,pos[1]+=ph)
		for(x=0,pos[0]=tl[0];x<w;x++,pos[0]+=pw)
			csurface[y][x]=CairoColor(color_func(pos,calc_supersample(pos,pw,ph))).premult_alpha();
	csurface.unmap_cairo_image();
	
	// paint surface on cr
	cairo_save(cr);
	cairo_translate(cr, tl[0], tl[1]);
	cairo_scale(cr, pw, ph);
	cairo_set_source_surface(cr, surface, 0, 0);
	cairo_paint_with_alpha_operator(cr, get_amount(), get_blend_method());
	cairo_restore(cr);
	
	cairo_surface_destroy(surface);	
	// Mark our progress as finished
	if(cb && !cb->amount_complete(10000,10000))
		return false;
	
	return true;
}



