/* === S Y N F I G ========================================================= */
/*!	\file halftone2.cpp
**	\brief Implementation of the "Halftone 2" layer
**
**	\legal
**	Copyright (c) 2002-2005 Robert B. Quattlebaum Jr., Adrian Bentley
**	Copyright (c) 2007-2008 Chris Moore
**	Copyright (c) 2012-2013 Carlos López
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

#include "halftone2.h"
#include "halftone.h"

#include <synfig/localization.h>

#include <synfig/string.h>
#include <synfig/context.h>
#include <synfig/paramdesc.h>
#include <synfig/rendering/common/task/taskpixelprocessor.h>
#include <synfig/rendering/software/task/taskpaintpixelsw.h>
#include <synfig/rendering/software/task/tasksw.h>

#endif

/* === M A C R O S ========================================================= */

using namespace synfig;

/* === G L O B A L S ======================================================= */

SYNFIG_LAYER_INIT(Halftone2);
SYNFIG_LAYER_SET_NAME(Halftone2,"halftone2");
SYNFIG_LAYER_SET_LOCAL_NAME(Halftone2,N_("Halftone 2"));
SYNFIG_LAYER_SET_CATEGORY(Halftone2,N_("Filters"));
SYNFIG_LAYER_SET_VERSION(Halftone2,"0.0");

/* === P R O C E D U R E S ================================================= */

class TaskHalfTone2: public rendering::TaskPixelProcessor
{
public:
	typedef etl::handle<TaskHalfTone2> Handle;
	SYNFIG_EXPORT static Token token;
	Token::Handle get_token() const override { return token.handle(); }

	Halftone halftone;
	Color color_dark;
	Color color_light;
};


class TaskHalfTone2SW: public TaskHalfTone2, public rendering::TaskFilterPixelSW
{
public:
	typedef etl::handle<TaskHalfTone2SW> Handle;
	SYNFIG_EXPORT static Token token;
	Token::Handle get_token() const override { return token.handle(); }

	Color get_color(const Vector& p, const Color& c) const override
	{
		const float supersample_size(1/std::fabs(get_pixels_per_unit()[0]*(halftone.param_size.get(Vector())).mag()));


		const float amount(halftone(p, c.get_y(), supersample_size));
		Color halfcolor;

		if (amount <= 0.0f)
			halfcolor = color_dark;
		else if (amount >= 1.0f)
			halfcolor = color_light;
		else
			halfcolor = Color::blend(color_light, color_dark, amount, Color::BLEND_STRAIGHT);

		halfcolor.set_a(c.get_a());

		return halfcolor;
	}

	bool run(RunParams&) const override {
		return run_task();
	}
};

SYNFIG_EXPORT rendering::Task::Token TaskHalfTone2::token(
	DescAbstract<TaskHalfTone2>("HalfTone2") );
SYNFIG_EXPORT rendering::Task::Token TaskHalfTone2SW::token(
	DescReal<TaskHalfTone2SW, TaskHalfTone2>("HalfTone2SW") );

/* === M E T H O D S ======================================================= */

Halftone2::Halftone2():
	Layer_CompositeFork(1.0,Color::BLEND_STRAIGHT),
	param_color_dark(ValueBase(Color::black())),
	param_color_light(ValueBase(Color::white()))
{
	halftone.param_origin=ValueBase(synfig::Point(0,0));
	halftone.param_size=ValueBase(synfig::Vector(0.25,0.25));
	halftone.param_angle=ValueBase(Angle::zero());
	halftone.param_type=ValueBase(int(TYPE_SYMMETRIC));

	SET_INTERPOLATION_DEFAULTS();
	SET_STATIC_DEFAULTS();
}

inline Color
Halftone2::color_func(const Point &point, float supersample,const Color& color)const
{
	Color color_dark=param_color_dark.get(Color());
	Color color_light=param_color_light.get(Color());

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

synfig::Layer::Handle
Halftone2::hit_check(synfig::Context /*context*/, const synfig::Point &/*point*/)const
{
	return const_cast<Halftone2*>(this);
}

bool
Halftone2::set_param(const String & param, const ValueBase &value)
{
	IMPORT_VALUE(param_color_dark);
	IMPORT_VALUE(param_color_light);

	HALFTONE2_IMPORT_VALUE(halftone.param_size);
	HALFTONE2_IMPORT_VALUE(halftone.param_type);
	HALFTONE2_IMPORT_VALUE(halftone.param_angle);
	HALFTONE2_IMPORT_VALUE(halftone.param_origin);

	if(param=="offset")
		return set_param("origin", value);

	return Layer_Composite::set_param(param,value);
}

ValueBase
Halftone2::get_param(const String & param)const
{
	EXPORT_VALUE(param_color_dark);
	EXPORT_VALUE(param_color_light);

	HALFTONE2_EXPORT_VALUE(halftone.param_size);
	HALFTONE2_EXPORT_VALUE(halftone.param_type);
	HALFTONE2_EXPORT_VALUE(halftone.param_angle);
	HALFTONE2_EXPORT_VALUE(halftone.param_origin);

	EXPORT_NAME();
	EXPORT_VERSION();

	return Layer_Composite::get_param(param);
}

Layer::Vocab
Halftone2::get_param_vocab()const
{
	Layer::Vocab ret(Layer_Composite::get_param_vocab());

	ret.push_back(ParamDesc("origin")
		.set_local_name(_("Mask Origin"))
		.set_is_distance()
	);
	ret.push_back(ParamDesc("angle")
		.set_local_name(_("Mask Angle"))
		.set_origin("origin")
	);
	ret.push_back(ParamDesc("size")
		.set_local_name(_("Mask Size"))
		.set_is_distance()
		.set_origin("origin")
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
		.set_static(true)
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

rendering::Task::Handle
Halftone2::build_composite_fork_task_vfunc(ContextParams /* context_params */, rendering::Task::Handle sub_task) const
{
	if (!sub_task)
		return sub_task;

	TaskHalfTone2::Handle task_halftone2(new TaskHalfTone2());
	task_halftone2->color_dark = param_color_dark.get(Color());
	task_halftone2->color_light = param_color_light.get(Color());
	task_halftone2->halftone = halftone;
	task_halftone2->sub_task() = sub_task;//->clone_recursive();
	return task_halftone2;
}
