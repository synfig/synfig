/* === S Y N F I G ========================================================= */
/*!	\file twirl.cpp
**	\brief Implementation of the "Twirl" layer
**
**	\legal
**	Copyright (c) 2002-2005 Robert B. Quattlebaum Jr., Adrian Bentley
**	Copyright (c) 2011-2013 Carlos López
**
**	This file is part of Synfig.
**
**	Synfig is free software: you can redistribute it and/or modify
**	it under the terms of the GNU General Public License as published by
**	the Free Software Foundation, either version 2 of the License, or
**	(at your option) any later version.
**
**	Synfig is distributed in the hope that it will be useful,
**	but WITHOUT ANY WARRANTY; without even the implied warranty of
**	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
**	GNU General Public License for more details.
**
**	You should have received a copy of the GNU General Public License
**	along with Synfig.  If not, see <https://www.gnu.org/licenses/>.
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

#include <synfig/rendering/software/task/taskdistortsw.h>

#endif

/* === U S I N G =========================================================== */

using namespace synfig;
using namespace modules;
using namespace lyr_std;

/* === G L O B A L S ======================================================= */

SYNFIG_LAYER_INIT(Twirl);
SYNFIG_LAYER_SET_NAME(Twirl,"twirl");
SYNFIG_LAYER_SET_LOCAL_NAME(Twirl,N_("Twirl"));
SYNFIG_LAYER_SET_CATEGORY(Twirl,N_("Distortions"));
SYNFIG_LAYER_SET_VERSION(Twirl,"0.2");

/* === P R O C E D U R E S ================================================= */

/* === M E T H O D S ======================================================= */

Point
Twirl::Internal::transform(const Point& pos) const
{
	return transform(pos, false);
}

Point
Twirl::Internal::transform(const Point& pos, bool reverse) const
{
	Point centered(pos-center);
	Real mag(centered.mag());

	Angle a;

	if ((distort_inside || mag>radius) && (distort_outside || mag<radius))
		a = rotations*((mag-radius)/radius);
	else
		return pos;

	if (reverse)
		a = -a;

	const Real sin(Angle::sin(a).get());
	const Real cos(Angle::cos(a).get());

	Point twirled;
	twirled[0] = cos*centered[0] - sin*centered[1];
	twirled[1] = sin*centered[0] + cos*centered[1];

	return twirled + center;
}

TaskTwirl::Token::Handle
TaskTwirl::get_token() const
{
	return token.handle();
}

Rect
TaskTwirl::compute_required_source_rect(const Rect& source_rect, const Matrix& raster_to_world_transformation) const
{
	if (!internal.distort_outside) {
		Point corner{internal.radius, internal.radius};
		return source_rect | Rect(internal.center - corner, internal.center + corner);
	}

	const int tw = target_rect.get_width();
	const int th = target_rect.get_height();
	Vector dx = raster_to_world_transformation.axis_x();
	Vector dy = raster_to_world_transformation.axis_y() - dx*(Real)tw;
	Vector p = raster_to_world_transformation.get_transformed( Vector((Real)target_rect.minx, (Real)target_rect.miny) );

	Rect sub_source_rect = source_rect;

	// Check from where the boundary pixels come in source context (before transform)
	// vertical borders
	for (int iy = target_rect.miny; iy < target_rect.maxy; ++iy, p[1] += dy[1]) {
		Point tmp = internal.transform(p);
		sub_source_rect.expand(tmp);
		tmp = internal.transform(Point(p[0] + dx[0]*(Real)tw, p[1]));
		sub_source_rect.expand(tmp);
	}

	// horizontal borders
	for (int ix = target_rect.minx; ix < target_rect.maxx; ++ix, p[0] += dx[0]) {
		Point tmp = internal.transform(p);
		sub_source_rect.expand(tmp);
		tmp = internal.transform(Point(p[0], p[1] - dy[1]*(Real)th));
		sub_source_rect.expand(tmp);
	}

	return sub_source_rect;
}

Rect
TaskTwirl::calc_bounds() const
{
	if (!sub_task())
		return Rect::zero();

	if (internal.distort_outside)
		return sub_task()->get_bounds();
	if (internal.distort_inside) {
		Point corner{internal.radius, internal.radius};
		return sub_task()->get_bounds() | Rect(internal.center - corner, internal.center + corner);
	}
	return Rect::zero();
}

class TaskTwirlSW
	: public TaskTwirl, public rendering::TaskDistortSW
{
public:
	typedef etl::handle<TaskTwirlSW> Handle;
	static Token token;
	Token::Handle get_token() const override { return token.handle(); }

	Point
	point_vfunc(const Point& point) const override
	{
		return internal.transform(point);
	}

	bool run(Task::RunParams& /*params*/) const override
	{
		return run_task(*this);
	}
};

rendering::Task::Token TaskTwirl::token(
	DescAbstract<TaskTwirl>("Twirl") );
rendering::Task::Token TaskTwirlSW::token(
	DescReal<TaskTwirlSW, TaskTwirl>("TwirlSW") );

/* === E N T R Y P O I N T ================================================= */

Twirl::Twirl():
	Layer_CompositeFork(1.0,Color::BLEND_STRAIGHT),
	param_center(ValueBase(Point(0,0))),
	param_radius(ValueBase(Real(1.0))),
	param_rotations(ValueBase(Angle::zero())),
	param_distort_inside(ValueBase(true)),
	param_distort_outside(ValueBase(false)),
	param_cobra(true)
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
	IMPORT_VALUE(param_cobra);

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
	EXPORT_VALUE(param_cobra);

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

	ret.push_back(ParamDesc("cobra")
		.set_local_name(_("Cobra"))
		.set_description(_("When checked, uses Cobra renderer"))
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
{
	if (!param_cobra.get(bool()))
		return Layer::build_rendering_task_vfunc(context);

	rendering::Task::Handle task = context.build_rendering_task();

	TaskTwirl::Handle task_twirl(new TaskTwirl());
	task_twirl->internal.center = param_center.get(Point());
	task_twirl->internal.radius = param_radius.get(Real());
	task_twirl->internal.rotations = param_rotations.get(Angle());
	task_twirl->internal.distort_inside = param_distort_inside.get(bool());
	task_twirl->internal.distort_outside = param_distort_outside.get(bool());

	task_twirl->sub_task() = task;
	task = task_twirl;
	return task;
}
