/* === S Y N F I G ========================================================= */
/*!	\file xorpattern.cpp
**	\brief Implementation of the "XOR Pattern" layer
**
**	\legal
**	Copyright (c) 2002-2005 Robert B. Quattlebaum Jr., Adrian Bentley
**	Copyright (c) 2007-2008 Chris Moore
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

#include "xorpattern.h"

#include <synfig/localization.h>

#include <synfig/string.h>
#include <synfig/time.h>
#include <synfig/context.h>
#include <synfig/paramdesc.h>
#include <synfig/renddesc.h>
#include <synfig/value.h>

#include <synfig/rendering/software/task/taskpaintpixelsw.h>

#endif

using namespace synfig;
using namespace modules;
using namespace lyr_std;

/* === M A C R O S ========================================================= */

/* === G L O B A L S ======================================================= */

SYNFIG_LAYER_INIT(XORPattern);
SYNFIG_LAYER_SET_NAME(XORPattern,"xor_pattern");
SYNFIG_LAYER_SET_LOCAL_NAME(XORPattern,N_("XOR Pattern"));
SYNFIG_LAYER_SET_CATEGORY(XORPattern,N_("Other"));
SYNFIG_LAYER_SET_VERSION(XORPattern,"0.2");

/* === P R O C E D U R E S ================================================= */

/* === M E T H O D S ======================================================= */

class TaskXORPattern: public rendering::Task, public rendering::TaskInterfaceTransformation
{
public:
	typedef etl::handle<TaskXORPattern> Handle;
	SYNFIG_EXPORT static Token token;
	Token::Handle get_token() const override { return token.handle(); }

	Vector origin;
	Vector size;

	rendering::Transformation::Handle get_transformation() const override {
		return transformation.handle();
	}

private:
	rendering::Holder<rendering::TransformationAffine> transformation;
};

class TaskXORPatternSW: public TaskXORPattern, public rendering::TaskPaintPixelSW
{
public:
	typedef etl::handle<TaskXORPatternSW> Handle;
	SYNFIG_EXPORT static Token token;
	Token::Handle get_token() const override { return token.handle(); }

	bool run(RunParams&) const override
	{
		return run_task();
	}

	Color get_color(const Vector& point) const override
	{
		const unsigned int a = floor((point[0] - origin[0]) / size[0]);
		const unsigned int b = floor((point[1] - origin[1]) / size[1]);
		unsigned char rindex = (a^b);
		unsigned char gindex = (a^(~b)) * 4;
		unsigned char bindex = ~(a^b) * 2;

		return
			Color(
				(Color::value_type)rindex / (Color::value_type)255.0,
				(Color::value_type)gindex / (Color::value_type)255.0,
				(Color::value_type)bindex / (Color::value_type)255.0,
				1.0);
	}
};

SYNFIG_EXPORT rendering::Task::Token TaskXORPattern::token(
	DescAbstract<TaskXORPattern>("TaskXORPattern") );
SYNFIG_EXPORT rendering::Task::Token TaskXORPatternSW::token(
	DescReal<TaskXORPatternSW, TaskXORPattern>("TaskXORPatternSW") );

XORPattern::XORPattern():
	Layer_Composite(1.0,Color::BLEND_COMPOSITE),
	param_origin(ValueBase(Vector(0.125,0.125))),
	param_size(ValueBase(Vector(0.25,0.25)))
{
	SET_INTERPOLATION_DEFAULTS();
	SET_STATIC_DEFAULTS();
}

bool
XORPattern::set_param(const String & param, const ValueBase &value)
{
	IMPORT_VALUE(param_origin);
	IMPORT_VALUE(param_size);

	if(param=="pos")
		return set_param("origin", value);

	return Layer_Composite::set_param(param,value);
}

ValueBase
XORPattern::get_param(const String & param)const
{
	EXPORT_VALUE(param_origin);
	EXPORT_VALUE(param_size);

	EXPORT_NAME();
	EXPORT_VERSION();

	return Layer_Composite::get_param(param);
}

Color
XORPattern::get_color(Context context, const Point &point)const
{
	Point origin=param_origin.get(Point());
	Point size=param_size.get(Point());
	
	if(get_amount()==0.0)
		return context.get_color(point);

	unsigned int a=(unsigned int)floor((point[0]-origin[0])/size[0]), b=(unsigned int)floor((point[1]-origin[1])/size[1]);
	unsigned char rindex=(a^b);
	unsigned char gindex=(a^(~b))*4;
	unsigned char bindex=~(a^b)*2;

	Color color((Color::value_type)rindex/(Color::value_type)255.0,
				(Color::value_type)gindex/(Color::value_type)255.0,
				(Color::value_type)bindex/(Color::value_type)255.0,
				1.0);

	if(get_amount() == 1 && get_blend_method() == Color::BLEND_STRAIGHT)
		return color;
	else
		return Color::blend(color,context.get_color(point),get_amount(),get_blend_method());

}

Layer::Vocab
XORPattern::get_param_vocab()const
{
	Layer::Vocab ret(Layer_Composite::get_param_vocab());

	ret.push_back(ParamDesc("origin")
		.set_local_name(_("Origin"))
		.set_description(_("Center of the pattern"))
		.set_is_distance()
	);
	ret.push_back(ParamDesc("size")
		.set_local_name(_("Size"))
		.set_description(_("Size of the pattern"))
		.set_origin("origin")
		.set_is_distance()
	);

	return ret;
}

Layer::Handle
XORPattern::hit_check(Context context, const Point &getpos)const
{
	bool check_myself_first;
	auto layer = basic_hit_check(context, getpos, check_myself_first);

	if (!check_myself_first)
		return layer;

	return const_cast<XORPattern*>(this);
}

rendering::Task::Handle
XORPattern::build_composite_task_vfunc(ContextParams /*context_params*/) const
{
	TaskXORPattern::Handle task(new TaskXORPattern());
	task->origin = param_origin.get(Vector());
	task->size = param_size.get(Vector());

	return task;
}
