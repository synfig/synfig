/* === S I N F G =========================================================== */
/*!	\file halftone2.cpp
**	\brief blehh
**
**	$Id: halftone2.cpp,v 1.1.1.1 2005/01/04 01:23:10 darco Exp $
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

#include "halftone2.h"
#include "halftone.h"

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

using namespace sinfg;
using namespace std;
using namespace etl;

/* === G L O B A L S ======================================================= */

SINFG_LAYER_INIT(Halftone2);
SINFG_LAYER_SET_NAME(Halftone2,"halftone2");
SINFG_LAYER_SET_LOCAL_NAME(Halftone2,_("Halftone2"));
SINFG_LAYER_SET_CATEGORY(Halftone2,_("Filters"));
SINFG_LAYER_SET_VERSION(Halftone2,"0.0");
SINFG_LAYER_SET_CVS_ID(Halftone2,"$Id: halftone2.cpp,v 1.1.1.1 2005/01/04 01:23:10 darco Exp $");

/* === P R O C E D U R E S ================================================= */

/* === M E T H O D S ======================================================= */

Halftone2::Halftone2():
	color_dark(Color::black()),
	color_light(Color::white())
{
	halftone.offset=(sinfg::Point(0,0));
	halftone.size=(sinfg::Vector(0.25,0.25));
	halftone.angle=(Angle::zero());
	halftone.type=TYPE_SYMMETRIC;
	
	set_blend_method(Color::BLEND_STRAIGHT);
}

inline Color
Halftone2::color_func(const Point &point, float supersample,const Color& color)const
{
	const float amount(halftone(point,color.get_y(),supersample));
	Color halfcolor;

	if(amount<=0.0f)
		halfcolor=color_dark;
	else if(amount>=1.0f)
		halfcolor=color_light;
	else
		halfcolor=Color::blend(color_light,color_dark,amount,Color::BLEND_STRAIGHT);		
	
	halfcolor.set_a(color.get_a());

	return halfcolor;
}

inline float
Halftone2::calc_supersample(const sinfg::Point &x, float pw,float ph)const
{
	return abs(pw/(halftone.size).mag());
}

sinfg::Layer::Handle
Halftone2::hit_check(sinfg::Context context, const sinfg::Point &point)const
{
	return const_cast<Halftone2*>(this);
}

bool
Halftone2::set_param(const String & param, const ValueBase &value)
{
	IMPORT(color_dark);
	IMPORT(color_light);

	IMPORT_AS(halftone.size,"size");
	IMPORT_AS(halftone.type,"type");
	IMPORT_AS(halftone.angle,"angle");
	IMPORT_AS(halftone.offset,"offset");
	
	return Layer_Composite::set_param(param,value);	
}

ValueBase
Halftone2::get_param(const String & param)const
{
	EXPORT_AS(halftone.size,"size");
	EXPORT_AS(halftone.type,"type");
	EXPORT_AS(halftone.angle,"angle");
	EXPORT_AS(halftone.offset,"offset");

	EXPORT(color_dark);
	EXPORT(color_light);
	
	EXPORT_NAME();
	EXPORT_VERSION();
		
	return Layer_Composite::get_param(param);	
}

Layer::Vocab
Halftone2::get_param_vocab()const
{
	Layer::Vocab ret(Layer_Composite::get_param_vocab());
	
	ret.push_back(ParamDesc("offset")
		.set_local_name(_("Mask Offset"))
		.set_is_distance()
	);
	ret.push_back(ParamDesc("angle")
		.set_local_name(_("Mask Angle"))
		.set_origin("offset")
	);
	ret.push_back(ParamDesc("size")
		.set_local_name(_("Mask Size"))
		.set_is_distance()
		.set_origin("offset")
	);
	ret.push_back(ParamDesc("color_light")
		.set_local_name(_("Light Color"))
	);
	ret.push_back(ParamDesc("color_dark")
		.set_local_name(_("Dark Color"))
	);
	ret.push_back(ParamDesc("type")
		.set_local_name(_("Type"))
		.set_hint("enum")
		.add_enum_value(TYPE_SYMMETRIC,"symmetric",_("Symmetric"))
		.add_enum_value(TYPE_LIGHTONDARK,"lightondark",_("Light On Dark"))
		//.add_enum_value(TYPE_DARKONLIGHT,"darkonlight",_("Dark on Light"))
		.add_enum_value(TYPE_DIAMOND,"diamond",_("Diamond"))
		.add_enum_value(TYPE_STRIPE,"stripe",_("Stripe"))
	);
	
	return ret;
}

Color
Halftone2::get_color(Context context, const Point &point)const
{
	const Color undercolor(context.get_color(point));
	const Color color(color_func(point,0,undercolor));

	if(get_amount()==1.0 && get_blend_method()==Color::BLEND_STRAIGHT)
		return color;
	else
		return Color::blend(color,undercolor,get_amount(),get_blend_method());
}

bool
Halftone2::accelerated_render(Context context,Surface *surface,int quality, const RendDesc &renddesc, ProgressCallback *cb)const
{
	SuperCallback supercb(cb,0,9500,10000);

	if(!context.accelerated_render(surface,quality,renddesc,&supercb))
		return false;
	if(get_amount()==0)
		return true;
		
	const Real pw(renddesc.get_pw()),ph(renddesc.get_ph());
	const Point tl(renddesc.get_tl());
	const int w(surface->get_w());
	const int h(surface->get_h());
	const float supersample_size(abs(pw/(halftone.size).mag()));

	Surface::pen pen(surface->begin());
	Point pos;
	int x,y;
	
	if(is_solid_color())
	{
		for(y=0,pos[1]=tl[1];y<h;y++,pen.inc_y(),pen.dec_x(x),pos[1]+=ph)
			for(x=0,pos[0]=tl[0];x<w;x++,pen.inc_x(),pos[0]+=pw)
				pen.put_value(
					color_func(
						pos,
						supersample_size,
						pen.get_value()
					)
				);
	}
	else
	{
		for(y=0,pos[1]=tl[1];y<h;y++,pen.inc_y(),pen.dec_x(x),pos[1]+=ph)
			for(x=0,pos[0]=tl[0];x<w;x++,pen.inc_x(),pos[0]+=pw)
				pen.put_value(
					Color::blend(
				 		color_func(
							pos,
							supersample_size,
							pen.get_value()
						),
						pen.get_value(),
						get_amount(),
						get_blend_method()
					)
				);
	}
	
	// Mark our progress as finished
	if(cb && !cb->amount_complete(10000,10000))
		return false;

	return true;
}
