/* === S Y N F I G ========================================================= */
/*!	\file mod_gradient/radialgradient.cpp
**	\brief Implementation of the "Radial Gradient" layer
**
**	\legal
**	Copyright (c) 2002-2005 Robert B. Quattlebaum Jr., Adrian Bentley
**	Copyright (c) 2007 Chris Moore
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

#include "radialgradient.h"

#include <synfig/context.h>
#include <synfig/localization.h>

#include <synfig/rendering/software/task/taskpaintpixelsw.h>

#endif

/* === U S I N G =========================================================== */

using namespace synfig;

/* === G L O B A L S ======================================================= */

SYNFIG_LAYER_INIT(RadialGradient);
SYNFIG_LAYER_SET_NAME(RadialGradient,"radial_gradient");
SYNFIG_LAYER_SET_LOCAL_NAME(RadialGradient,N_("Radial Gradient"));
SYNFIG_LAYER_SET_CATEGORY(RadialGradient,N_("Gradients"));
SYNFIG_LAYER_SET_VERSION(RadialGradient,"0.2");

/* === P R O C E D U R E S ================================================= */

/* === M E T H O D S ======================================================= */

class TaskRadialGradient: public rendering::Task
{
public:
	typedef etl::handle<TaskRadialGradient> Handle;
	SYNFIG_EXPORT static Token token;
	Token::Handle get_token() const override { return token.handle(); }

	Point center;
	Real radius;
	CompiledGradient compiled_gradient;

	TaskRadialGradient() { }
};


class TaskRadialGradientSW: public TaskRadialGradient, public rendering::TaskPaintPixelSW
{
public:
	typedef etl::handle<TaskRadialGradientSW> Handle;
	SYNFIG_EXPORT static Token token;
	Token::Handle get_token() const override { return token.handle(); }

	mutable Real supersample = 0.;

	void pre_run(const Matrix3& /*matrix*/, const Matrix3& /*inverse_matrix*/) const override
	{
		supersample = 1.2*get_units_per_pixel()[0]/radius;
		supersample *= 0.5;
	}

	bool run(RunParams&) const override
	{
		return run_task();
	}

	Color get_color(const Vector& p) const override
	{
		Real dist((p-center).mag()/radius);

		return compiled_gradient.average(dist - supersample, dist + supersample);
		//return params.gradient.color(dist);
	}
};

SYNFIG_EXPORT rendering::Task::Token TaskRadialGradient::token(
	DescAbstract<TaskRadialGradient>("TaskRadialGradient") );
SYNFIG_EXPORT rendering::Task::Token TaskRadialGradientSW::token(
	DescReal<TaskRadialGradientSW, TaskRadialGradient>("TaskRadialGradientSW") );


RadialGradient::RadialGradient():
	Layer_Composite(1.0,Color::BLEND_COMPOSITE),
	param_gradient(ValueBase(Gradient(Color::black(),Color::white()))),
	param_center(ValueBase(Point(0,0))),
	param_radius(ValueBase(Real(0.5))),
	param_loop(ValueBase(false)),
	param_zigzag(ValueBase(false))
{
	SET_INTERPOLATION_DEFAULTS();
	SET_STATIC_DEFAULTS();
}

bool
RadialGradient::set_param(const String & param, const ValueBase &value)
{
	IMPORT_VALUE_PLUS(param_gradient, compile());
	IMPORT_VALUE(param_center);
	IMPORT_VALUE(param_radius);
	IMPORT_VALUE_PLUS(param_loop, compile());
	IMPORT_VALUE_PLUS(param_zigzag, compile());

	return Layer_Composite::set_param(param,value);
}

ValueBase
RadialGradient::get_param(const String &param)const
{
	EXPORT_VALUE(param_gradient);
	EXPORT_VALUE(param_center);
	EXPORT_VALUE(param_radius);
	EXPORT_VALUE(param_loop);
	EXPORT_VALUE(param_zigzag);

	EXPORT_NAME();
	EXPORT_VERSION();

	return Layer_Composite::get_param(param);
}

Layer::Vocab
RadialGradient::get_param_vocab()const
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

	ret.push_back(ParamDesc("loop")
		.set_local_name(_("Loop"))
		.set_description(_("When checked, the gradient is looped"))
	);

	ret.push_back(ParamDesc("zigzag")
		.set_local_name(_("ZigZag"))
		.set_description(_("When checked, the gradient is symmetrical at the center"))
	);

	return ret;
}

void
RadialGradient::compile()
{
	compiled_gradient.set(
		param_gradient.get(Gradient()),
		param_loop.get(bool()),
		param_zigzag.get(bool()) );
}

inline Color
RadialGradient::color_func(const Point &point, Real supersample)const
{
	Point center = param_center.get(Point());
	Real radius = param_radius.get(Real());

	Real dist((point-center).mag()/radius);

	supersample *= 0.5;
	return compiled_gradient.average(dist - supersample, dist + supersample);
}

synfig::Layer::Handle
RadialGradient::hit_check(synfig::Context context, const synfig::Point &point)const
{
	bool check_myself_first;
	auto layer = basic_hit_check(context, point, check_myself_first);

	if (!check_myself_first)
		return layer;

	if(get_blend_method()==Color::BLEND_STRAIGHT && get_amount()>=0.5)
		return const_cast<RadialGradient*>(this);
	if((get_blend_method()==Color::BLEND_STRAIGHT || get_blend_method()==Color::BLEND_COMPOSITE) && color_func(point).get_a()>0.5)
		return const_cast<RadialGradient*>(this);
	return context.hit_check(point);
}

Color
RadialGradient::get_color(Context context, const Point &pos)const
{
	const Color color(color_func(pos));

	if(get_amount()==1.0 && get_blend_method()==Color::BLEND_STRAIGHT)
		return color;
	else
		return Color::blend(color,context.get_color(pos),get_amount(),get_blend_method());
}

rendering::Task::Handle
RadialGradient::build_composite_task_vfunc(ContextParams /*context_params*/) const
{
	TaskRadialGradient::Handle task(new TaskRadialGradient());
	task->center = param_center.get(Point());
	task->radius = param_radius.get(Real());
	task->compiled_gradient = compiled_gradient;

	return task;
}

