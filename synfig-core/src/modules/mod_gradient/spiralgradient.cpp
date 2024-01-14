/* === S Y N F I G ========================================================= */
/*!	\file spiralgradient.cpp
**	\brief Implementation of the "Spiral Gradient" layer
**
**	\legal
**	Copyright (c) 2002-2005 Robert B. Quattlebaum Jr., Adrian Bentley
**	Copyright (c) 2011 Carlos López
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

#include "spiralgradient.h"

#include <synfig/context.h>
#include <synfig/localization.h>

#include <synfig/rendering/software/task/taskpaintpixelsw.h>

#endif

/* === U S I N G =========================================================== */

using namespace synfig;

/* === G L O B A L S ======================================================= */

SYNFIG_LAYER_INIT(SpiralGradient);
SYNFIG_LAYER_SET_NAME(SpiralGradient,"spiral_gradient");
SYNFIG_LAYER_SET_LOCAL_NAME(SpiralGradient,N_("Spiral Gradient"));
SYNFIG_LAYER_SET_CATEGORY(SpiralGradient,N_("Gradients"));
SYNFIG_LAYER_SET_VERSION(SpiralGradient,"0.2");

/* === P R O C E D U R E S ================================================= */

/* === M E T H O D S ======================================================= */

class TaskSpiralGradient: public rendering::Task
{
public:
	typedef etl::handle<TaskSpiralGradient> Handle;
	SYNFIG_EXPORT static Token token;
	Token::Handle get_token() const override { return token.handle(); }

	Point center;
	Real radius;
	Angle angle;
	bool clockwise;
	CompiledGradient compiled_gradient;

	TaskSpiralGradient() : clockwise(false) { }
};


class TaskSpiralGradientSW: public TaskSpiralGradient, public rendering::TaskPaintPixelSW
{
public:
	typedef etl::handle<TaskSpiralGradientSW> Handle;
	SYNFIG_EXPORT static Token token;
	Token::Handle get_token() const override { return token.handle(); }

	mutable Real pw = 0;

	void pre_run(const Matrix3&, const Matrix3&) const override
	{
		pw = get_units_per_pixel()[0];
	}

	bool run(RunParams&) const override
	{
		return run_task();
	}

	Color get_color(const Vector& p) const override
	{
		const Point centered(p-center);
		Real supersample = (1.41421*pw/radius+(1.41421*pw/centered.mag())/(PI*2))*0.5;

		Angle a = Angle::tan(-centered[1],centered[0]).mod();
		a += angle;

		if(supersample<0.00001)supersample=0.00001;

		Real dist(centered.mag()/radius);
		if(clockwise)
			dist+=Angle::rot(a.mod()).get();
		else
			dist-=Angle::rot(a.mod()).get();

//		supersample *= 0.5;
		return compiled_gradient.average(dist - supersample, dist + supersample);
		//return compiled_gradient.color(dist);
	}
};

SYNFIG_EXPORT rendering::Task::Token TaskSpiralGradient::token(
	DescAbstract<TaskSpiralGradient>("TaskSpiralGradient") );
SYNFIG_EXPORT rendering::Task::Token TaskSpiralGradientSW::token(
	DescReal<TaskSpiralGradientSW, TaskSpiralGradient>("TaskSpiralGradientSW") );



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
	Angle a(angle);
	a += Angle::tan(-centered[1],centered[0]).mod();

	if(supersample<0.00001)supersample=0.00001;

	Real dist((pos-center).mag()/radius);
	if(clockwise)
		dist+=Angle::rot(a.mod()).get();
	else
		dist-=Angle::rot(a.mod()).get();

	supersample *= 0.5;
	return compiled_gradient.average(dist - supersample, dist + supersample);
}

synfig::Layer::Handle
SpiralGradient::hit_check(synfig::Context context, const synfig::Point &point)const
{
	bool check_myself_first;
	auto layer = basic_hit_check(context, point, check_myself_first);

	if (!check_myself_first)
		return layer;

	if(get_blend_method()==Color::BLEND_STRAIGHT && get_amount()>=0.5)
		return const_cast<SpiralGradient*>(this);
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

rendering::Task::Handle
SpiralGradient::build_composite_task_vfunc(ContextParams /*context_params*/) const
{
	TaskSpiralGradient::Handle task(new TaskSpiralGradient());
	task->center = param_center.get(Point());
	task->radius = param_radius.get(Real());
	task->angle = param_angle.get(Angle());
	task->clockwise = param_clockwise.get(bool());
	task->compiled_gradient = compiled_gradient;

	return task;
}
