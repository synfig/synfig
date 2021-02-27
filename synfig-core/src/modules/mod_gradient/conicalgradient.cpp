/* === S Y N F I G ========================================================= */
/*!	\file conicalgradient.cpp
**	\brief Implementation of the "Conical Gradient" layer
**
**	$Id$
**
**	\legal
**	Copyright (c) 2002-2005 Robert B. Quattlebaum Jr., Adrian Bentley
**	Copyright (c) 2011-2013 Carlos López
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

#include <synfig/string.h>
#include <synfig/context.h>
#include <synfig/paramdesc.h>
#include <synfig/renddesc.h>
#include <synfig/surface.h>
#include <synfig/value.h>
#include <synfig/angle.h>

#include "conicalgradient.h"

#endif

/* === U S I N G =========================================================== */

using namespace etl;
using namespace std;
using namespace synfig;

/* === G L O B A L S ======================================================= */

SYNFIG_LAYER_INIT(ConicalGradient);
SYNFIG_LAYER_SET_NAME(ConicalGradient,"conical_gradient");
SYNFIG_LAYER_SET_LOCAL_NAME(ConicalGradient,N_("Conical Gradient"));
SYNFIG_LAYER_SET_CATEGORY(ConicalGradient,N_("Gradients"));
SYNFIG_LAYER_SET_VERSION(ConicalGradient,"0.1");

/* === P R O C E D U R E S ================================================= */

/* === M E T H O D S ======================================================= */

/* === E N T R Y P O I N T ================================================= */

ConicalGradient::ConicalGradient():
	Layer_Composite(1.0,Color::BLEND_COMPOSITE),
	param_gradient(ValueBase(Gradient(Color::black(),Color::white()))),
	param_center(ValueBase(Point(0,0))),
	param_angle(ValueBase(Angle::zero())),
	param_symmetric(ValueBase(false))
{
	SET_INTERPOLATION_DEFAULTS();
	SET_STATIC_DEFAULTS();
}

bool
ConicalGradient::set_param(const String & param, const ValueBase &value)
{
	IMPORT_VALUE_PLUS(param_gradient, compile());
	IMPORT_VALUE(param_center);
	IMPORT_VALUE(param_angle);
	IMPORT_VALUE_PLUS(param_symmetric, compile());
	return Layer_Composite::set_param(param,value);
}

ValueBase
ConicalGradient::get_param(const String &param)const
{
	EXPORT_VALUE(param_gradient);
	EXPORT_VALUE(param_center);
	EXPORT_VALUE(param_angle);
	EXPORT_VALUE(param_symmetric);

	EXPORT_NAME();
	EXPORT_VERSION();

	return Layer_Composite::get_param(param);
}

Layer::Vocab
ConicalGradient::get_param_vocab()const
{
	Layer::Vocab ret(Layer_Composite::get_param_vocab());

	ret.push_back(ParamDesc("gradient")
		.set_local_name(_("Gradient"))
		.set_description(_("Gradient to apply"))
	);

	ret.push_back(ParamDesc("center")
		.set_local_name(_("Center"))
		.set_description(_("Center of the cone"))
		.set_is_distance()
	);

	ret.push_back(ParamDesc("angle")
		.set_local_name(_("Angle"))
		.set_origin("center")
		.set_description(_("Rotation of the gradient around the center"))
	);

	ret.push_back(ParamDesc("symmetric")
		.set_local_name(_("Symmetric"))
		.set_description(_("When checked, the gradient is looped"))
	);

	return ret;
}

void
ConicalGradient::compile()
{
	compiled_gradient.set(
		param_gradient.get(Gradient()),
		true,
		param_symmetric.get(bool()) );
}

inline Color
ConicalGradient::color_func(const Point &pos, Real supersample)const
{
	Point center = param_center.get(Point());
	Angle angle = param_angle.get(Angle());
	
	const Point centered(pos-center);
	Angle::rot a = Angle::tan(-centered[1],centered[0]).mod();
	a += angle;
	Real dist(a.mod().get());

	supersample *= 0.5;
	return compiled_gradient.average(dist - supersample, dist + supersample);
}

Real
ConicalGradient::calc_supersample(const synfig::Point &x, Real pw, Real ph)const
{
	Point center=param_center.get(Point());

	Point adj(x-center);
	if(abs(adj[0])<abs(pw*0.5) && abs(adj[1])<abs(ph*0.5))
		return 0.5;
	return (pw/Point(x-center).mag())/(PI*2);
}

synfig::Layer::Handle
ConicalGradient::hit_check(synfig::Context context, const synfig::Point &point)const
{
	if(get_blend_method()==Color::BLEND_STRAIGHT && get_amount()>=0.5)
		return const_cast<ConicalGradient*>(this);
	if(get_amount()==0.0)
		return context.hit_check(point);
	if((get_blend_method()==Color::BLEND_STRAIGHT || get_blend_method()==Color::BLEND_COMPOSITE) && color_func(point).get_a()>0.5)
		return const_cast<ConicalGradient*>(this);
	return context.hit_check(point);
}

Color
ConicalGradient::get_color(Context context, const Point &pos)const
{
	const Color color(color_func(pos));

	if(get_amount()==1.0 && get_blend_method()==Color::BLEND_STRAIGHT)
		return color;
	else
		return Color::blend(color,context.get_color(pos),get_amount(),get_blend_method());
}

bool
ConicalGradient::accelerated_render(Context context,Surface *surface,int quality, const RendDesc &renddesc, ProgressCallback *cb)const
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
		if(quality<9)
		{
			for(y=0,pos[1]=tl[1];y<h;y++,pen.inc_y(),pen.dec_x(x),pos[1]+=ph)
				for(x=0,pos[0]=tl[0];x<w;x++,pen.inc_x(),pos[0]+=pw)
					pen.put_value(color_func(pos,calc_supersample(pos,pw,ph)));
		}
		else
		{
			for(y=0,pos[1]=tl[1];y<h;y++,pen.inc_y(),pen.dec_x(x),pos[1]+=ph)
				for(x=0,pos[0]=tl[0];x<w;x++,pen.inc_x(),pos[0]+=pw)
					pen.put_value(color_func(pos,0));
		}
	}
	else
	{
		if(quality<9)
		{
			for(y=0,pos[1]=tl[1];y<h;y++,pen.inc_y(),pen.dec_x(x),pos[1]+=ph)
				for(x=0,pos[0]=tl[0];x<w;x++,pen.inc_x(),pos[0]+=pw)
					pen.put_value(Color::blend(color_func(pos,calc_supersample(pos,pw,ph)),pen.get_value(),get_amount(),get_blend_method()));
		}
		else
		{
			for(y=0,pos[1]=tl[1];y<h;y++,pen.inc_y(),pen.dec_x(x),pos[1]+=ph)
				for(x=0,pos[0]=tl[0];x<w;x++,pen.inc_x(),pos[0]+=pw)
					pen.put_value(Color::blend(color_func(pos,0),pen.get_value(),get_amount(),get_blend_method()));
		}
	}

	// Mark our progress as finished
	if(cb && !cb->amount_complete(10000,10000))
		return false;

	return true;
}
