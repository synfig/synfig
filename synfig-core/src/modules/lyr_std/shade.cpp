/* === S Y N F I G ========================================================= */
/*!	\file shade.cpp
**	\brief Implementation of the "Shade" layer
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

#include <ETL/misc>

#include "shade.h"

#include <synfig/localization.h>
#include <synfig/general.h>

#include <synfig/string.h>
#include <synfig/time.h>
#include <synfig/context.h>
#include <synfig/paramdesc.h>
#include <synfig/renddesc.h>
#include <synfig/surface.h>
#include <synfig/value.h>
#include <synfig/valuenode.h>
#include <synfig/segment.h>
#include <synfig/cairo_renddesc.h>

#include <synfig/rendering/primitive/transformationaffine.h>

#include <synfig/rendering/common/task/taskblur.h>
#include <synfig/rendering/common/task/tasktransformation.h>
#include <synfig/rendering/common/task/taskpixelprocessor.h>

#endif

using namespace std;
using namespace etl;
using namespace synfig;
using namespace modules;
using namespace lyr_std;

/*#define TYPE_BOX			0
#define TYPE_FASTGUASSIAN	1
#define TYPE_CROSS			2
#define TYPE_GAUSSIAN		3
#define TYPE_DISC			4
*/

/* -- G L O B A L S --------------------------------------------------------- */

SYNFIG_LAYER_INIT(Layer_Shade);
SYNFIG_LAYER_SET_NAME(Layer_Shade,"shade");
SYNFIG_LAYER_SET_LOCAL_NAME(Layer_Shade,N_("Shade"));
SYNFIG_LAYER_SET_CATEGORY(Layer_Shade,N_("Stylize"));
SYNFIG_LAYER_SET_VERSION(Layer_Shade,"0.2");

/* -- F U N C T I O N S ----------------------------------------------------- */

static inline void clamp_size(Vector &v)
{
	if(v[0]<0.0)v[0]=0.0;
	if(v[1]<0.0)v[1]=0.0;
}

Layer_Shade::Layer_Shade():
	Layer_CompositeFork(0.75,Color::BLEND_BEHIND),
	param_size(ValueBase(Vector(0.1,0.1))),
	param_type(ValueBase(int(Blur::FASTGAUSSIAN))),
	param_color(ValueBase(Color::black())),
	param_origin(ValueBase(Vector(0.2,-0.2))),
	param_invert(ValueBase(false))
{
	SET_INTERPOLATION_DEFAULTS();
	SET_STATIC_DEFAULTS();
}

bool
Layer_Shade::set_param(const String &param, const ValueBase &value)
{
	IMPORT_VALUE_PLUS(param_size,
		{
			Vector size=param_size.get(Vector());
			clamp_size(size);
			param_size.set(size);
		}
		);
	IMPORT_VALUE(param_type);
	IMPORT_VALUE_PLUS(param_color,
		{
			Color color=param_color.get(Color());
			if (color.get_a() == 0)
			{
				if (converted_blend_)
				{
					set_blend_method(Color::BLEND_ALPHA_OVER);
					color.set_a(1);
					param_color.set(color);
				}
				else
					transparent_color_ = true;
			}
		}
		);
	IMPORT_VALUE(param_origin);
	IMPORT_VALUE(param_invert);

	if(param=="offset")
		return set_param("origin", value);

	return Layer_Composite::set_param(param,value);
}

ValueBase
Layer_Shade::get_param(const String &param)const
{
	EXPORT_VALUE(param_size);
	EXPORT_VALUE(param_type);
	EXPORT_VALUE(param_color);
	EXPORT_VALUE(param_origin);
	EXPORT_VALUE(param_invert);

	EXPORT_NAME();
	EXPORT_VERSION();

	return Layer_Composite::get_param(param);
}

Color
Layer_Shade::get_color(Context context, const Point &pos)const
{
	Vector size=param_size.get(Vector());
	int type=param_type.get(int());
	Color color=param_color.get(Color());
	Vector origin=param_origin.get(Vector());
	bool invert=param_invert.get(bool());
	
	Point blurpos = Blur(size,type)(pos);

	if(get_amount()==0.0)
		return context.get_color(pos);

	Color shade(color);

	if(!invert)
		shade.set_a(context.get_color(blurpos-origin).get_a());
	else
		shade.set_a(1.0f-context.get_color(blurpos-origin).get_a());

	return Color::blend(shade,context.get_color(pos),get_amount(),get_blend_method());
}

Layer::Vocab
Layer_Shade::get_param_vocab(void)const
{
	Layer::Vocab ret(Layer_Composite::get_param_vocab());

	ret.push_back(ParamDesc("color")
		.set_local_name(_("Color"))
	);
	ret.push_back(ParamDesc("origin")
		.set_local_name(_("Origin"))
	);
	ret.push_back(ParamDesc("size")
		.set_local_name(_("Size"))
		.set_description(_("Size of the shade"))
		.set_is_distance()
		.set_origin("origin")
	);
	ret.push_back(ParamDesc("type")
		.set_local_name(_("Type"))
		.set_description(_("Type of blur to use"))
		.set_hint("enum")
		.add_enum_value(Blur::BOX,"box",_("Box Blur"))
		.add_enum_value(Blur::FASTGAUSSIAN,"fastgaussian",_("Fast Gaussian Blur"))
		.add_enum_value(Blur::CROSS,"cross",_("Cross-Hatch Blur"))
		.add_enum_value(Blur::GAUSSIAN,"gaussian",_("Gaussian Blur"))
		.add_enum_value(Blur::DISC,"disc",_("Disc Blur"))
	);

	ret.push_back(ParamDesc("invert")
		.set_local_name(_("Invert"))
	);

	return ret;
}

Rect
Layer_Shade::get_full_bounding_rect(Context context)const
{
	Vector size=param_size.get(Vector());
	Vector origin=param_origin.get(Vector());
	bool invert=param_invert.get(bool());

	if(is_disabled())
		return context.get_full_bounding_rect();

	if(invert)
		return Rect::full_plane();

	Rect under(context.get_full_bounding_rect());

	if(Color::is_onto(get_blend_method()))
		return under;

	Rect bounds((under+origin).expand_x(size[0]).expand_y(size[1]));

	if(is_solid_color())
		return bounds;

	return bounds|under;
}

rendering::Task::Handle
Layer_Shade::build_composite_fork_task_vfunc(ContextParams /* context_params */, rendering::Task::Handle sub_task)const
{
	Vector size = param_size.get(Vector());
	rendering::Blur::Type type = (rendering::Blur::Type)param_type.get(int());
	Color color = param_color.get(Color());
	Vector origin = param_origin.get(Vector());
	bool invert = param_invert.get(bool());

	if (!sub_task)
		return sub_task;

	rendering::TaskBlur::Handle task_blur(new rendering::TaskBlur());
	task_blur->blur.size = size;
	task_blur->blur.type = type;
	if (sub_task)
		task_blur->sub_task() = sub_task->clone_recursive();

	ColorMatrix matrix;
	matrix *= ColorMatrix().set_replace_color(color);
	if (invert)
		matrix *= ColorMatrix().set_invert_alpha();

	rendering::TaskPixelColorMatrix::Handle task_colormatrix(new rendering::TaskPixelColorMatrix());
	task_colormatrix->matrix = matrix;
	task_colormatrix->sub_task() = task_blur;

	rendering::TaskTransformationAffine::Handle task_transformation(new rendering::TaskTransformationAffine());
	task_transformation->transformation->matrix.set_translate(origin);
	task_transformation->sub_task() = task_colormatrix;

	return task_transformation;
}
