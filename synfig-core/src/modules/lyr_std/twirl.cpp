/* === S Y N F I G ========================================================= */
/*!	\file twirl.cpp
**	\brief Implementation of the "Twirl" layer
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
#include <synfig/time.h>
#include <synfig/context.h>
#include <synfig/paramdesc.h>
#include <synfig/renddesc.h>
#include <synfig/value.h>
#include <synfig/transform.h>
#include "twirl.h"

#endif

/* === U S I N G =========================================================== */

using namespace etl;
using namespace synfig;
using namespace modules;
using namespace lyr_std;

/* === G L O B A L S ======================================================= */

SYNFIG_LAYER_INIT(Twirl);
SYNFIG_LAYER_SET_NAME(Twirl,"twirl");
SYNFIG_LAYER_SET_LOCAL_NAME(Twirl,N_("Twirl"));
SYNFIG_LAYER_SET_CATEGORY(Twirl,N_("Distortions"));
SYNFIG_LAYER_SET_VERSION(Twirl,"0.1");

/* === P R O C E D U R E S ================================================= */

/* === M E T H O D S ======================================================= */

/* === E N T R Y P O I N T ================================================= */

Twirl::Twirl():
	Layer_CompositeFork(1.0,Color::BLEND_STRAIGHT),
	param_center(ValueBase(Point(0,0))),
	param_radius(ValueBase(Real(1.0))),
	param_rotations(ValueBase(Angle::zero())),
	param_distort_inside(ValueBase(true)),
	param_distort_outside(ValueBase(false))
{
	SET_INTERPOLATION_DEFAULTS();
	SET_STATIC_DEFAULTS();
}

bool
Twirl::set_param(const String & param, const ValueBase &value)
{
	IMPORT_VALUE(param_center);
	IMPORT_VALUE(param_radius);
	IMPORT_VALUE(param_rotations);
	IMPORT_VALUE(param_distort_inside);
	IMPORT_VALUE(param_distort_outside);

	return Layer_Composite::set_param(param,value);
}

ValueBase
Twirl::get_param(const String &param)const
{
	EXPORT_VALUE(param_center);
	EXPORT_VALUE(param_radius);
	EXPORT_VALUE(param_rotations);
	EXPORT_VALUE(param_distort_inside);
	EXPORT_VALUE(param_distort_outside);

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
		.set_description(_("Center of the circle"))
		.set_is_distance()
	);

	ret.push_back(ParamDesc("radius")
		.set_local_name(_("Radius"))
		.set_description(_("Radius of the circle"))
		.set_is_distance()
		.set_origin("center")
	);

	ret.push_back(ParamDesc("rotations")
		.set_local_name(_("Rotations"))
		.set_description(_("Number of rotations of the twirl effect"))
		.set_origin("center")
	);

	ret.push_back(ParamDesc("distort_inside")
		.set_local_name(_("Distort Inside"))
		.set_description(_("When checked, distorts inside the circle"))
	);

	ret.push_back(ParamDesc("distort_outside")
		.set_local_name(_("Distort Outside"))
		.set_description(_("When checked, distorts outside the circle"))
	);

	return ret;
}

Point
Twirl::distort(const Point &pos,bool reverse)const
{
	Point center=param_center.get(Point());
	Real radius=param_radius.get(Real());
	Angle rotations=param_rotations.get(Angle());
	bool distort_inside=param_distort_inside.get(bool());
	bool distort_outside=param_distort_outside.get(bool());
	
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

Layer::Handle
Twirl::hit_check(Context context, const Point &pos)const
{
	return context.hit_check(distort(pos));
}

Color
Twirl::get_color(Context context, const Point &pos)const
{
	return context.get_color(distort(pos));
}


class lyr_std::Twirl_Trans : public Transform
{
	etl::handle<const Twirl> layer;
public:
	Twirl_Trans(const Twirl* x):Transform(x->get_guid()),layer(x) { }

	Vector perform(const Vector& x)const
	{
		return layer->distort(x,true);
	}

	Vector unperform(const Vector& x)const
	{
		return layer->distort(x,false);
	}

	String get_string()const
	{
		return "twirl";
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

RendDesc
Twirl::get_sub_renddesc_vfunc(const RendDesc &renddesc) const
{
	RendDesc desc(renddesc);
	Real pw = desc.get_pw();
	Real ph = desc.get_ph();
	desc.set_tl(Vector(-10.0, -10.0));
	desc.set_br(Vector( 10.0,  10.0));
	desc.set_wh(
		(int)approximate_ceil(fabs((desc.get_br()[0] - desc.get_tl()[0])/pw)),
		(int)approximate_ceil(fabs((desc.get_br()[1] - desc.get_tl()[1])/ph)) );
	return desc;
}

rendering::Task::Handle
Twirl::build_rendering_task_vfunc(Context context) const
	{ return Layer::build_rendering_task_vfunc(context); }
