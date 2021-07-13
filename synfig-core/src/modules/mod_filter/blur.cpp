/* === S Y N F I G ========================================================= */
/*!	\file mod_filter/blur.cpp
**	\brief Implementation of the "Blur" layer
**
**	$Id$
**
**	\legal
**	Copyright (c) 2002-2005 Robert B. Quattlebaum Jr., Adrian Bentley
**	Copyright (c) 2008 Chris Moore
**	Copyright (c) 2012-2013 Carlos López
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

#include <cstring>

#include "blur.h"

#include <synfig/localization.h>

#include <synfig/string.h>
#include <synfig/context.h>
#include <synfig/paramdesc.h>
#include <synfig/value.h>
#include <synfig/segment.h>

#include <synfig/rendering/common/task/taskblend.h>
#include <synfig/rendering/common/task/taskblur.h>
#include <synfig/rendering/software/function/blur.h>

#endif

using namespace synfig;
using namespace etl;

/*#define TYPE_BOX			0
#define TYPE_FASTGUASSIAN	1
#define TYPE_FASTGAUSSIAN	1
#define TYPE_CROSS			2
#define TYPE_GUASSIAN		3
#define TYPE_GAUSSIAN		3
#define TYPE_DISC			4
*/

/* -- G L O B A L S --------------------------------------------------------- */

SYNFIG_LAYER_INIT(Blur_Layer);
SYNFIG_LAYER_SET_NAME(Blur_Layer,"blur");
SYNFIG_LAYER_SET_LOCAL_NAME(Blur_Layer,N_("Blur"));
SYNFIG_LAYER_SET_CATEGORY(Blur_Layer,N_("Blurs"));
SYNFIG_LAYER_SET_VERSION(Blur_Layer,"0.3");

/* -- F U N C T I O N S ----------------------------------------------------- */

inline void clamp(synfig::Vector &v)
{
	if(v[0]<0.0)v[0]=0.0;
	if(v[1]<0.0)v[1]=0.0;
}

Blur_Layer::Blur_Layer():
	Layer_CompositeFork(1.0,Color::BLEND_STRAIGHT),
	param_size(ValueBase(Point(0.1,0.1))),
	param_type(ValueBase(int(Blur::FASTGAUSSIAN)))
{
	SET_INTERPOLATION_DEFAULTS();
	SET_STATIC_DEFAULTS();
}

bool
Blur_Layer::set_param(const String &param, const ValueBase &value)
{
	IMPORT_VALUE_PLUS(param_size,
		{
			synfig::Point size=param_size.get(Point());
			clamp(size);
			param_size.set(size);
		});
		
	IMPORT_VALUE(param_type);

	return Layer_Composite::set_param(param,value);
}

ValueBase
Blur_Layer::get_param(const String &param)const
{
	EXPORT_VALUE(param_size);
	EXPORT_VALUE(param_type);

	EXPORT_NAME();
	EXPORT_VERSION();

	return Layer_Composite::get_param(param);
}

Color
Blur_Layer::get_color(Context context, const Point &pos)const
{
	synfig::Point size=param_size.get(Point());
	int type=param_type.get(int());
  	size *= rendering::software::Blur::get_size_amplifier((rendering::Blur::Type)type)
  	      * ::Blur::get_size_amplifier(type);
	
	Point blurpos = Blur(size,type)(pos);

	if(get_amount()==1.0 && get_blend_method()==Color::BLEND_STRAIGHT)
		return context.get_color(blurpos);

	if(get_amount()==0.0)
		return context.get_color(pos);

	return Color::blend(context.get_color(blurpos),context.get_color(pos),get_amount(),get_blend_method());
}

Layer::Vocab
Blur_Layer::get_param_vocab(void)const
{
	Layer::Vocab ret(Layer_Composite::get_param_vocab());

	ret.push_back(ParamDesc("size")
		.set_local_name(_("Size"))
		.set_description(_("Size of the blur"))
		.set_is_distance()
	);
	ret.push_back(ParamDesc("type")
		.set_local_name(_("Type"))
		.set_description(_("Type of blur to use"))
		.set_hint("enum")
		.set_static(true)
		.add_enum_value(Blur::BOX,"box",_("Box Blur"))
		.add_enum_value(Blur::FASTGAUSSIAN,"fastgaussian",_("Fast Gaussian Blur"))
		.add_enum_value(Blur::CROSS,"cross",_("Cross-Hatch Blur"))
		.add_enum_value(Blur::GAUSSIAN,"gaussian",_("Gaussian Blur"))
		.add_enum_value(Blur::DISC,"disc",_("Disc Blur"))
	);

	return ret;
}

Rect
Blur_Layer::get_full_bounding_rect(Context context)const
{
	synfig::Point size=param_size.get(Point());
	int type=param_type.get(int());
  	size *= rendering::software::Blur::get_size_amplifier((rendering::Blur::Type)type)
  	      * ::Blur::get_size_amplifier(type);

	if(is_disabled() || Color::is_onto(get_blend_method()))
		return context.get_full_bounding_rect();

	Rect bounds(context.get_full_bounding_rect().expand_x(size[0]).expand_y(size[1]));

	return bounds;
}

rendering::Task::Handle
Blur_Layer::build_composite_fork_task_vfunc(ContextParams /* context_params */, rendering::Task::Handle sub_task)const
{
	Vector size = param_size.get(Point());
	rendering::Blur::Type type = (rendering::Blur::Type)param_type.get(int());

	rendering::TaskBlur::Handle task_blur(new rendering::TaskBlur());
	task_blur->blur.size = size;
	task_blur->blur.type = type;
	task_blur->sub_task() = sub_task ? sub_task->clone_recursive() : rendering::Task::Handle();

	return task_blur;
}
