/* === S Y N F I G ========================================================= */
/*!	\file conicalgradient.cpp
**	\brief Implementation of the "Conical Gradient" layer
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

#include "conicalgradient.h"

#include <synfig/context.h>
#include <synfig/localization.h>

#include <synfig/rendering/software/task/taskpaintpixelsw.h>

#endif

/* === U S I N G =========================================================== */

using namespace synfig;

/* === G L O B A L S ======================================================= */

SYNFIG_LAYER_INIT(ConicalGradient);
SYNFIG_LAYER_SET_NAME(ConicalGradient,"conical_gradient");
SYNFIG_LAYER_SET_LOCAL_NAME(ConicalGradient,N_("Conical Gradient"));
SYNFIG_LAYER_SET_CATEGORY(ConicalGradient,N_("Gradients"));
SYNFIG_LAYER_SET_VERSION(ConicalGradient,"0.2");

/* === P R O C E D U R E S ================================================= */

/* === M E T H O D S ======================================================= */

class TaskConicalGradient: public rendering::Task
{
public:
	typedef etl::handle<TaskConicalGradient> Handle;
	SYNFIG_EXPORT static Token token;
	Token::Handle get_token() const override { return token.handle(); }

	Point center;
	Angle angle;
	CompiledGradient compiled_gradient;

	TaskConicalGradient() { }
};


class TaskConicalGradientSW: public TaskConicalGradient, public rendering::TaskPaintPixelSW
{
public:
	typedef etl::handle<TaskConicalGradientSW> Handle;
	SYNFIG_EXPORT static Token token;
	Token::Handle get_token() const override { return token.handle(); }

	mutable Real pw = 0;
	mutable Real ph = 0;

	void pre_run(const Matrix3&, const Matrix3&) const override
	{
		pw = get_units_per_pixel()[0];
		ph = get_units_per_pixel()[1];
	}

	bool run(RunParams&) const override
	{
		return run_task();
	}

	Color get_color(const Vector& p) const override
	{
		const Point centered(p-center);
		Real supersample;
		if(std::fabs(centered[0])<std::fabs(pw*0.5) && std::fabs(centered[1])<std::fabs(ph*0.5))
			supersample = 0.5;
		else
			supersample = (pw/centered.mag())/(PI*2);

		Angle::rot a = Angle::tan(-centered[1],centered[0]).mod();
		a += angle;
		Real dist(a.mod().get());

		supersample *= 0.5;
		return compiled_gradient.average(dist - supersample, dist + supersample);
		//return compiled_gradient.color(dist);
	}
};

SYNFIG_EXPORT rendering::Task::Token TaskConicalGradient::token(
	DescAbstract<TaskConicalGradient>("TaskConicalGradient") );
SYNFIG_EXPORT rendering::Task::Token TaskConicalGradientSW::token(
	DescReal<TaskConicalGradientSW, TaskConicalGradient>("TaskConicalGradientSW") );



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

synfig::Layer::Handle
ConicalGradient::hit_check(synfig::Context context, const synfig::Point &point)const
{
	bool check_myself_first;
	auto layer = basic_hit_check(context, point, check_myself_first);

	if (!check_myself_first)
		return layer;

	if(get_blend_method()==Color::BLEND_STRAIGHT && get_amount()>=0.5)
		return const_cast<ConicalGradient*>(this);
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

rendering::Task::Handle
ConicalGradient::build_composite_task_vfunc(ContextParams /*context_params*/) const
{
	TaskConicalGradient::Handle task(new TaskConicalGradient());
	task->center = param_center.get(Point());
	task->angle = param_angle.get(Angle());
	task->compiled_gradient = compiled_gradient;

	return task;
}
