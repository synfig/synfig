/* === S I N F G =========================================================== */
/*!	\file spiralgradient.cpp
**	\brief Template Header
**
**	$Id: twirl.cpp,v 1.1.1.1 2005/01/04 01:23:10 darco Exp $
**
**	\legal
**	Copyright (c) 2002 Robert B. Quattlebaum Jr.
**
**	This software and associated documentation
**	are CONFIDENTIAL and PROPRIETARY property of
**	the above-mentioned copyright holder.
**
**	You may not copy, print, publish, or in any
**	other way distribute this software without
**	a prior written agreement with
**	the copyright holder.
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

#include <sinfg/string.h>
#include <sinfg/time.h>
#include <sinfg/context.h>
#include <sinfg/paramdesc.h>
#include <sinfg/renddesc.h>
#include <sinfg/surface.h>
#include <sinfg/value.h>
#include <sinfg/valuenode.h>
#include <sinfg/transform.h>
#include "twirl.h"

#endif

/* === U S I N G =========================================================== */

using namespace etl;
using namespace std;
using namespace sinfg;

/* === G L O B A L S ======================================================= */

SINFG_LAYER_INIT(Twirl);
SINFG_LAYER_SET_NAME(Twirl,"twirl");
SINFG_LAYER_SET_LOCAL_NAME(Twirl,_("Twirl"));
SINFG_LAYER_SET_CATEGORY(Twirl,_("Distortions"));
SINFG_LAYER_SET_VERSION(Twirl,"0.1");
SINFG_LAYER_SET_CVS_ID(Twirl,"$Id: twirl.cpp,v 1.1.1.1 2005/01/04 01:23:10 darco Exp $");

/* === P R O C E D U R E S ================================================= */

/* === M E T H O D S ======================================================= */

/* === E N T R Y P O I N T ================================================= */

Twirl::Twirl():
	Layer_Composite(1.0,Color::BLEND_STRAIGHT),
	center(0,0),
	radius(1.0),
	rotations(Angle::zero()),
	distort_inside(true),
	distort_outside(false)
{
}
	
bool
Twirl::set_param(const String & param, const ValueBase &value)
{
	IMPORT(center);
	IMPORT(radius);
	IMPORT(rotations);
	IMPORT(distort_inside);
	IMPORT(distort_outside);
	
	return Layer_Composite::set_param(param,value);
}

ValueBase
Twirl::get_param(const String &param)const
{
	EXPORT(center);
	EXPORT(radius);
	EXPORT(rotations);
	EXPORT(distort_inside);
	EXPORT(distort_outside);
	
	EXPORT_NAME();
	EXPORT_VERSION();
		
	return false;
}

Layer::Vocab
Twirl::get_param_vocab()const
{
	Layer::Vocab ret;
	
	ret.push_back(ParamDesc("center")
		.set_local_name(_("Center"))
	);
	
	ret.push_back(ParamDesc("radius")
		.set_local_name(_("Radius"))
		.set_description(_("This is the radius of the circle"))
		.set_is_distance()
		.set_origin("center")
	);

	ret.push_back(ParamDesc("rotations")
		.set_local_name(_("Rotations"))
		.set_origin("center")
	);

	ret.push_back(ParamDesc("distort_inside")
		.set_local_name(_("Distort Inside"))
	);

	ret.push_back(ParamDesc("distort_outside")
		.set_local_name(_("Distort Outside"))
	);

	return ret;
}

sinfg::Point
Twirl::distort(const sinfg::Point &pos,bool reverse)const
{
	Point centered(pos-center);
	Real mag(centered.mag());
	
	Angle a;

	if((distort_inside || mag>radius) && (distort_outside || mag<radius))
		a=rotations*((centered.mag()-radius)/radius);
	else
		return pos;
	
	if(reverse)	a=-a;
		
	const Real sin(Angle::sin(a).get());	
	const Real cos(Angle::cos(a).get());	

	Point twirled;
	twirled[0]=cos*centered[0]-sin*centered[1];
	twirled[1]=sin*centered[0]+cos*centered[1];

	return twirled+center;
}

sinfg::Layer::Handle
Twirl::hit_check(sinfg::Context context, const sinfg::Point &pos)const
{
	return context.hit_check(distort(pos));
}

Color
Twirl::get_color(Context context, const Point &pos)const
{
	return context.get_color(distort(pos));
}

class Twirl_Trans : public Transform
{
	etl::handle<const Twirl> layer;
public:
	Twirl_Trans(const Twirl* x):Transform(x->get_guid()),layer(x) { }
	
	sinfg::Vector perform(const sinfg::Vector& x)const
	{
		return layer->distort(x,true);
	}
	
	sinfg::Vector unperform(const sinfg::Vector& x)const
	{
		return layer->distort(x,false);
	}
};
etl::handle<Transform>
Twirl::get_transform()const
{
	return new Twirl_Trans(this);
}

/*
bool
Twirl::accelerated_render(Context context,Surface *surface,int quality, const RendDesc &renddesc, ProgressCallback *cb)const
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
				pen.put_value(color_func(pos));
	}
	else
	{
		for(y=0,pos[1]=tl[1];y<h;y++,pen.inc_y(),pen.dec_x(x),pos[1]+=ph)
			for(x=0,pos[0]=tl[0];x<w;x++,pen.inc_x(),pos[0]+=pw)
				pen.put_value(Color::blend(color_func(pos),pen.get_value(),get_amount(),get_blend_method()));
	}

	// Mark our progress as finished
	if(cb && !cb->amount_complete(10000,10000))
		return false;

	return true;
}
*/
