/* === S Y N F I G ========================================================= */
/*!	\file rotate.cpp
**	\brief Implementation of the "Rotate" layer
**
**	\legal
**	Copyright (c) 2002-2005 Robert B. Quattlebaum Jr., Adrian Bentley
**	Copyright (c) 2008 Chris Moore
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
**
** === N O T E S ===========================================================
**
** ========================================================================= */

/* === H E A D E R S ======================================================= */

#ifdef USING_PCH
#	include "pch.h"
#else
#ifdef HAVE_CONFIG_H
#	include <config.h>
#endif

#include <ETL/misc>

#include <synfig/localization.h>

#include <synfig/string.h>
#include <synfig/context.h>
#include <synfig/paramdesc.h>
#include <synfig/value.h>
#include <synfig/transform.h>

#include "rotate.h"

#include <synfig/rendering/common/task/tasktransformation.h>
#include <synfig/rendering/primitive/transformationaffine.h>

#endif

using namespace etl;
using namespace synfig;
using namespace modules;
using namespace lyr_std;

/* === M A C R O S ========================================================= */

/* === G L O B A L S ======================================================= */

SYNFIG_LAYER_INIT(Rotate);
SYNFIG_LAYER_SET_NAME(Rotate,"rotate");
SYNFIG_LAYER_SET_LOCAL_NAME(Rotate,N_("Rotate"));
SYNFIG_LAYER_SET_CATEGORY(Rotate,N_("Transform"));
SYNFIG_LAYER_SET_VERSION(Rotate,"0.1");

/* === P R O C E D U R E S ================================================= */

/* === M E T H O D S ======================================================= */

/* === E N T R Y P O I N T ================================================= */

Rotate::Rotate():
	param_origin (ValueBase(Vector(0,0))),
	param_amount (ValueBase(Angle::deg(0))),
	sin_val	(0),
	cos_val	(1)
{
	SET_INTERPOLATION_DEFAULTS();
	SET_STATIC_DEFAULTS();
}

Rotate::~Rotate()
{
}

bool
Rotate::set_param(const String & param, const ValueBase &value)
{
	IMPORT_VALUE(param_origin);

	IMPORT_VALUE_PLUS(param_amount,
	{
		Angle amount=value.get(Angle());
		sin_val=Angle::sin(amount).get();
		cos_val=Angle::cos(amount).get();
		param_amount.set(amount);
		return true;
	}
	);

	return false;
}

ValueBase
Rotate::get_param(const String &param)const
{
	EXPORT_VALUE(param_origin);
	EXPORT_VALUE(param_amount);

	EXPORT_NAME();
	EXPORT_VERSION();

	return ValueBase();
}

Layer::Vocab
Rotate::get_param_vocab()const
{
	Layer::Vocab ret;

	ret.push_back(ParamDesc("origin")
		.set_local_name(_("Origin"))
		.set_description(_("Origin of the rotation"))
		.set_is_distance()
	);

	ret.push_back(ParamDesc("amount")
		.set_local_name(_("Amount"))
		.set_description(_("Amount of rotation"))
		.set_origin("origin")
	);

	return ret;
}

class lyr_std::Rotate_Trans : public Transform
{
	etl::handle<const Rotate> layer;
public:
	Rotate_Trans(const Rotate* x):Transform(x->get_guid()),layer(x) { }

	Vector perform(const Vector& x)const
	{
		Vector origin=layer->param_origin.get(Vector());
		Point pos(x-origin);
		return Point(layer->cos_val*pos[0]-layer->sin_val*pos[1],layer->sin_val*pos[0]+layer->cos_val*pos[1])+origin;
	}

	Vector unperform(const Vector& x)const
	{
		Vector origin=layer->param_origin.get(Vector());
		Point pos(x-origin);
		return Point(layer->cos_val*pos[0]+layer->sin_val*pos[1],-layer->sin_val*pos[0]+layer->cos_val*pos[1])+origin;
	}

	String get_string()const
	{
		return "rotate";
	}
};
etl::handle<Transform>
Rotate::get_transform()const
{
	return new Rotate_Trans(this);
}

Layer::Handle
Rotate::hit_check(Context context, const Point &p)const
{
	Vector origin=param_origin.get(Vector());
	Point pos(p-origin);
	Point newpos(cos_val*pos[0]+sin_val*pos[1],-sin_val*pos[0]+cos_val*pos[1]);
	newpos+=origin;
	return context.hit_check(newpos);
}

Color
Rotate::get_color(Context context, const Point &p)const
{
	Vector origin=param_origin.get(Vector());
	Point pos(p-origin);
	Point newpos(cos_val*pos[0]+sin_val*pos[1],-sin_val*pos[0]+cos_val*pos[1]);
	newpos+=origin;
	return context.get_color(newpos);
}

Rect
Rotate::get_full_bounding_rect(Context context)const
{
	Rect under(context.get_full_bounding_rect());
	return get_transform()->perform(under);
}

rendering::Task::Handle
Rotate::build_rendering_task_vfunc(Context context)const
{
	Vector origin=param_origin.get(Vector());
	Angle amount=param_amount.get(Angle());

	rendering::TaskTransformationAffine::Handle task_transformation(new rendering::TaskTransformationAffine());
	task_transformation->transformation->matrix =
			Matrix().set_translate(origin)
		  * Matrix().set_rotate(amount)
		  * Matrix().set_translate(-origin);
	task_transformation->sub_task() = context.build_rendering_task();
	return task_transformation;
}

