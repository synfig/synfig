/* === S Y N F I G ========================================================= */
/*!	\file distort.cpp
**	\brief Implementation of the "Noise Distort" layer
**
**	$Id$
**
**	\legal
**	Copyright (c) 2002-2005 Robert B. Quattlebaum Jr., Adrian Bentley
**	Copyright (c) 2007 Chris Moore
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

#include "distort.h"

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
#include <time.h>

#endif

/* === M A C R O S ========================================================= */

using namespace synfig;
using namespace std;
using namespace etl;

/* === G L O B A L S ======================================================= */

SYNFIG_LAYER_INIT(NoiseDistort);
SYNFIG_LAYER_SET_NAME(NoiseDistort,"noise_distort");
SYNFIG_LAYER_SET_LOCAL_NAME(NoiseDistort,N_("Noise Distort"));
SYNFIG_LAYER_SET_CATEGORY(NoiseDistort,N_("Distortions"));
SYNFIG_LAYER_SET_VERSION(NoiseDistort,"0.0");
SYNFIG_LAYER_SET_CVS_ID(NoiseDistort,"$Id$");

/* === P R O C E D U R E S ================================================= */

/* === M E T H O D S ======================================================= */

NoiseDistort::NoiseDistort():
	Layer_CompositeFork(1.0,Color::BLEND_STRAIGHT),
	param_displacement(ValueBase(Vector(0.25,0.25))),
	param_size(ValueBase(Vector(1,1))),
	param_random(ValueBase(int(time(NULL)))),
	param_smooth(ValueBase(int(RandomNoise::SMOOTH_COSINE))),
	param_detail(ValueBase(int(4))),
	param_speed(ValueBase(Real(0))),
	param_turbulent(bool(false))
{
	SET_INTERPOLATION_DEFAULTS();
	SET_STATIC_DEFAULTS();
}

inline Point
NoiseDistort::point_func(const Point &point)const
{
	Vector displacement=param_displacement.get(Vector());
	Vector size=param_size.get(Vector());
	RandomNoise random;
	random.set_seed(param_random.get(int()));
	int smooth_=param_smooth.get(int());
	int detail=param_detail.get(int());
	Real speed=param_speed.get(Real());
	bool turbulent=param_turbulent.get(bool());
	
	float x(point[0]/size[0]*(1<<detail));
	float y(point[1]/size[1]*(1<<detail));
	
	int i;
	Time time = speed*get_time_mark();
	int temp_smooth(smooth_);
	int smooth((!speed && temp_smooth == (int)(RandomNoise::SMOOTH_SPLINE)) ? (int)(RandomNoise::SMOOTH_FAST_SPLINE) : temp_smooth);
	
	Vector vect(0,0);
	for(i=0;i<detail;i++)
	{
		vect[0]=random(RandomNoise::SmoothType(smooth),0+(detail-i)*5,x,y,time)+vect[0]*0.5;
		vect[1]=random(RandomNoise::SmoothType(smooth),1+(detail-i)*5,x,y,time)+vect[1]*0.5;
		
		if (vect[0] < -1) vect[0] = -1;
		if (vect[0] >  1) vect[0] =  1;
		
		if (vect[1] < -1) vect[1] = -1;
		if (vect[1] >  1) vect[1] =  1;
		
		if(turbulent)
		{
			vect[0]=abs(vect[0]);
			vect[1]=abs(vect[1]);
		}
		
		x/=2.0f;
		y/=2.0f;
	}
	
	if(!turbulent)
	{
		vect[0]=vect[0]/2.0f+0.5f;
		vect[1]=vect[1]/2.0f+0.5f;
	}
	vect[0]=(vect[0]-0.5f)*displacement[0];
	vect[1]=(vect[1]-0.5f)*displacement[1];
	
	return point+vect;
}

inline Color
NoiseDistort::color_func(const Point &point, float /*supersample*/,Context context)const
{
	Color ret(0,0,0,0);
	ret=context.get_color(point_func(point));
	return ret;
}

inline CairoColor
NoiseDistort::cairocolor_func(const Point &point, float /*supersample*/,Context context)const
{
	CairoColor ret(0,0,0,0);
	ret=context.get_cairocolor(point_func(point));
	return ret;
}


inline float
NoiseDistort::calc_supersample(const synfig::Point &/*x*/, float /*pw*/,float /*ph*/)const
{
	return 0.0f;
}

synfig::Layer::Handle
NoiseDistort::hit_check(synfig::Context context, const synfig::Point &point)const
{
	if(get_blend_method()==Color::BLEND_STRAIGHT && get_amount()>=0.5)
		return const_cast<NoiseDistort*>(this);
	if(get_amount()==0.0)
		return context.hit_check(point);
	if(color_func(point,0,context).get_a()>0.5)
		return const_cast<NoiseDistort*>(this);
	return synfig::Layer::Handle();
}

bool
NoiseDistort::set_param(const String & param, const ValueBase &value)
{
	IMPORT_VALUE(param_displacement);
	IMPORT_VALUE(param_size);
	IMPORT_VALUE(param_random);
	IMPORT_VALUE(param_detail);
	IMPORT_VALUE(param_smooth);
	IMPORT_VALUE(param_speed);
	IMPORT_VALUE(param_turbulent);
	if(param=="seed")
		return set_param("random", value);
	return Layer_Composite::set_param(param,value);
}

ValueBase
NoiseDistort::get_param(const String & param)const
{
	EXPORT_VALUE(param_displacement);
	EXPORT_VALUE(param_size);
	EXPORT_VALUE(param_random);
	EXPORT_VALUE(param_detail);
	EXPORT_VALUE(param_smooth);
	EXPORT_VALUE(param_speed);
	EXPORT_VALUE(param_turbulent);

	if(param=="seed")
		return get_param("random");
		
	EXPORT_NAME();
	EXPORT_VERSION();

	return Layer_Composite::get_param(param);
}

Layer::Vocab
NoiseDistort::get_param_vocab()const
{
	Layer::Vocab ret(Layer_Composite::get_param_vocab());

	ret.push_back(ParamDesc("displacement")
		.set_local_name(_("Displacement"))
		.set_description(_("How big the distortion displaces the context"))
	);

	ret.push_back(ParamDesc("size")
		.set_local_name(_("Size"))
		.set_description(_("The distance between distortions"))
	);
	ret.push_back(ParamDesc("seed")
		.set_local_name(_("RandomNoise Seed"))
		.set_description(_("Change to modify the random seed of the noise"))
	);
	ret.push_back(ParamDesc("smooth")
		.set_local_name(_("Interpolation"))
		.set_description(_("What type of interpolation to use"))
		.set_hint("enum")
		.add_enum_value(RandomNoise::SMOOTH_DEFAULT,	"nearest",	_("Nearest Neighbor"))
		.add_enum_value(RandomNoise::SMOOTH_LINEAR,	"linear",	_("Linear"))
		.add_enum_value(RandomNoise::SMOOTH_COSINE,	"cosine",	_("Cosine"))
		.add_enum_value(RandomNoise::SMOOTH_SPLINE,	"spline",	_("Spline"))
		.add_enum_value(RandomNoise::SMOOTH_CUBIC,	"cubic",	_("Cubic"))
	);
	ret.push_back(ParamDesc("detail")
		.set_local_name(_("Detail"))
		.set_description(_("Increase to obtain fine details of the noise"))
	);
	ret.push_back(ParamDesc("speed")
		.set_local_name(_("Animation Speed"))
		.set_description(_("In cycles per second"))
	);
	ret.push_back(ParamDesc("turbulent")
		.set_local_name(_("Turbulent"))
		.set_description(_("When checked produces turbulent noise"))
	);

	return ret;
}

Color
NoiseDistort::get_color(Context context, const Point &point)const
{
	const Color color(color_func(point,0,context));

	if(get_amount()==1.0 && get_blend_method()==Color::BLEND_STRAIGHT)
		return color;
	else
		return Color::blend(color,context.get_color(point),get_amount(),get_blend_method());
}

CairoColor
NoiseDistort::get_cairocolor(Context context, const Point &point)const
{
	const CairoColor color(cairocolor_func(point,0,context));
	
	if(get_amount()==1.0 && get_blend_method()==Color::BLEND_STRAIGHT)
		return color;
	else
		return CairoColor::blend(color,context.get_cairocolor(point),get_amount(),get_blend_method());
}

RendDesc
NoiseDistort::get_sub_renddesc_vfunc(const RendDesc &renddesc) const
{
	Vector displacement=param_displacement.get(Vector());

	RendDesc desc(renddesc);
	Real pw = desc.get_pw();
	Real ph = desc.get_ph();

	Rect r(renddesc.get_tl(), renddesc.get_br());
	r.expand_x(fabs(displacement[0]));
	r.expand_y(fabs(displacement[1]));

	desc.set_tl(r.get_min());
	desc.set_br(r.get_max());
	desc.set_wh(
		(int)approximate_ceil(fabs((desc.get_br()[0] - desc.get_tl()[0])/pw)),
		(int)approximate_ceil(fabs((desc.get_br()[1] - desc.get_tl()[1])/ph)) );

	return desc;
}

Rect
NoiseDistort::get_bounding_rect(Context context)const
{
	Vector displacement=param_displacement.get(Vector());

	if(is_disabled())
		return Rect::zero();

	if(Color::is_onto(get_blend_method()))
		return context.get_full_bounding_rect();

	Rect bounds(context.get_full_bounding_rect().expand_x(displacement[0]).expand_y(displacement[1]));

	return bounds;
}


/*
bool
NoiseDistort::accelerated_render(Context context,Surface *surface,int quality, const RendDesc &renddesc, ProgressCallback *cb)const
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
				pen.put_value(color_func(pos,calc_supersample(pos,pw,ph),context));
	}
	else
	{
		for(y=0,pos[1]=tl[1];y<h;y++,pen.inc_y(),pen.dec_x(x),pos[1]+=ph)
			for(x=0,pos[0]=tl[0];x<w;x++,pen.inc_x(),pos[0]+=pw)
				pen.put_value(Color::blend(color_func(pos,calc_supersample(pos,pw,ph),context),pen.get_value(),get_amount(),get_blend_method()));
	}

	// Mark our progress as finished
	if(cb && !cb->amount_complete(10000,10000))
		return false;

	return true;
}
*/

rendering::Task::Handle
NoiseDistort::build_rendering_task_vfunc(Context context) const
	{ return Layer::build_rendering_task_vfunc(context); }
