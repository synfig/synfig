/* === S Y N F I G ========================================================= */
/*!	\file lineargradient.cpp
**	\brief Implementation of the "Linear Gradient" layer
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
**
** ========================================================================= */

/* === H E A D E R S ======================================================= */

#ifdef USING_PCH
#	include "pch.h"
#else
#ifdef HAVE_CONFIG_H
#	include <config.h>
#endif

#include "lineargradient.h"

#include <synfig/context.h>
#include <synfig/localization.h>

#include <synfig/rendering/software/task/taskpaintpixelsw.h>

#endif

/* === M A C R O S ========================================================= */

/* === G L O B A L S ======================================================= */

SYNFIG_LAYER_INIT(LinearGradient);
SYNFIG_LAYER_SET_NAME(LinearGradient,"linear_gradient");
SYNFIG_LAYER_SET_LOCAL_NAME(LinearGradient,N_("Linear Gradient"));
SYNFIG_LAYER_SET_CATEGORY(LinearGradient,N_("Gradients"));
SYNFIG_LAYER_SET_VERSION(LinearGradient,"0.1");

/* === P R O C E D U R E S ================================================= */

/* === M E T H O D S ======================================================= */

class TaskLinearGradient: public rendering::Task
{
public:
	typedef etl::handle<TaskLinearGradient> Handle;
	SYNFIG_EXPORT static Token token;
	Token::Handle get_token() const override { return token.handle(); }

	LinearGradient::Params params;

	TaskLinearGradient() { }
};


class TaskLinearGradientSW: public TaskLinearGradient, public rendering::TaskPaintPixelSW
{
public:
	typedef etl::handle<TaskLinearGradientSW> Handle;
	SYNFIG_EXPORT static Token token;
	Token::Handle get_token() const override { return token.handle(); }

	mutable Real supersample = 0.;

	void pre_run(const Matrix3& /*matrix*/, const Matrix3& /*inverse_matrix*/) const override
	{
		supersample = get_units_per_pixel()[0]*.1;
	}

	bool run(RunParams&) const override
	{
		return run_task();
	}

	Color get_color(const Vector& p) const override
	{
		Real dist((p - params.p1)*params.diff);
		return params.gradient.average(dist - supersample, dist + supersample);
		//return params.gradient.color(dist);
	}
};

SYNFIG_EXPORT rendering::Task::Token TaskLinearGradient::token(
	DescAbstract<TaskLinearGradient>("TaskLinearGradient") );
SYNFIG_EXPORT rendering::Task::Token TaskLinearGradientSW::token(
	DescReal<TaskLinearGradientSW, TaskLinearGradient>("TaskLinearGradientSW") );

inline void
LinearGradient::Params::calc_diff()
{
	diff=(p2-p1);
	Real mag_squared = diff.mag_squared();
	if (mag_squared > 0.0) diff /= mag_squared;
}


LinearGradient::LinearGradient():
	Layer_Composite(1.0,Color::BLEND_COMPOSITE),
	param_p1(ValueBase(Point(1,1))),
	param_p2(ValueBase(Point(-1,-1))),
	param_gradient(ValueBase(Gradient(Color::black(), Color::white()))),
	param_loop(ValueBase(false)),
	param_zigzag(ValueBase(false))
{
	SET_INTERPOLATION_DEFAULTS();
	SET_STATIC_DEFAULTS();
}

inline void
LinearGradient::fill_params(Params &params)const
{
	params.p1=param_p1.get(Point());
	params.p2=param_p2.get(Point());
	params.loop=param_loop.get(bool());
	params.zigzag=param_zigzag.get(bool());
	params.gradient.set(param_gradient.get(Gradient()), params.loop, params.zigzag);
	params.calc_diff();
}

inline Color
LinearGradient::color_func(const Params &params, const Point &point, synfig::Real supersample)const
{
	Real dist(point*params.diff - params.p1*params.diff);
	supersample *= 0.5;
	return params.gradient.average(dist - supersample, dist + supersample);
}

synfig::Layer::Handle
LinearGradient::hit_check(synfig::Context context, const synfig::Point &point)const
{
	bool check_myself_first;
	auto layer = basic_hit_check(context, point, check_myself_first);

	if (!check_myself_first)
		return layer;

	if(get_blend_method()==Color::BLEND_STRAIGHT && get_amount()>=0.5)
		return const_cast<LinearGradient*>(this);

	Params params;
	fill_params(params);

	if((get_blend_method()==Color::BLEND_STRAIGHT || get_blend_method()==Color::BLEND_COMPOSITE) && color_func(params, point).get_a()>0.5)
		return const_cast<LinearGradient*>(this);
	return context.hit_check(point);
}

bool
LinearGradient::set_param(const String & param, const ValueBase &value)
{
	IMPORT_VALUE(param_p1);
	IMPORT_VALUE(param_p2);
	IMPORT_VALUE(param_gradient);
	IMPORT_VALUE(param_loop);
	IMPORT_VALUE(param_zigzag);
	return Layer_Composite::set_param(param,value);
}

ValueBase
LinearGradient::get_param(const String & param)const
{
	EXPORT_VALUE(param_p1);
	EXPORT_VALUE(param_p2);
	EXPORT_VALUE(param_gradient);
	EXPORT_VALUE(param_loop);
	EXPORT_VALUE(param_zigzag);

	EXPORT_NAME();
	EXPORT_VERSION();

	return Layer_Composite::get_param(param);
}

Layer::Vocab
LinearGradient::get_param_vocab()const
{
	Layer::Vocab ret(Layer_Composite::get_param_vocab());

	ret.push_back(ParamDesc("p1")
		.set_local_name(_("Point 1"))
		.set_connect("p2")
		.set_description(_("Start point of the gradient"))
		.set_is_distance()
	);
	ret.push_back(ParamDesc("p2")
		.set_local_name(_("Point 2"))
		.set_description(_("End point of the gradient"))
		.set_is_distance()
	);
	ret.push_back(ParamDesc("gradient")
		.set_local_name(_("Gradient"))
		.set_description(_("Gradient to apply"))
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

Color
LinearGradient::get_color(Context context, const Point &point)const
{
	Params params;
	fill_params(params);

	const Color color(color_func(params, point));

	if(get_amount()==1.0 && get_blend_method()==Color::BLEND_STRAIGHT)
		return color;
	else
		return Color::blend(color,context.get_color(point),get_amount(),get_blend_method());
}

rendering::Task::Handle
LinearGradient::build_composite_task_vfunc(ContextParams /*context_params*/) const
{
	TaskLinearGradient::Handle task(new TaskLinearGradient());
	Params params;
	fill_params(params);
	task->params = params;

	return task;
}
