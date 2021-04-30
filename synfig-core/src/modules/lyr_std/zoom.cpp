/* === S Y N F I G ========================================================= */
/*!	\file zoom.cpp
**	\brief Implementation of the "Zoom" layer, aka "Scale"
**
**	$Id$
**
**	\legal
**	Copyright (c) 2002-2005 Robert B. Quattlebaum Jr., Adrian Bentley
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

#include <synfig/localization.h>

#include <synfig/string.h>
#include <synfig/context.h>
#include <synfig/paramdesc.h>
#include <synfig/value.h>
#include <synfig/transform.h>

#include "zoom.h"

#include <synfig/rendering/common/task/tasktransformation.h>
#include <synfig/rendering/primitive/transformationaffine.h>

#endif

using namespace std;
using namespace etl;
using namespace synfig;
using namespace modules;
using namespace lyr_std;

/* === M A C R O S ========================================================= */

/* === G L O B A L S ======================================================= */

SYNFIG_LAYER_INIT(Zoom);
SYNFIG_LAYER_SET_NAME(Zoom,"zoom");
SYNFIG_LAYER_SET_LOCAL_NAME(Zoom,N_("Scale"));
SYNFIG_LAYER_SET_CATEGORY(Zoom,N_("Transform"));
SYNFIG_LAYER_SET_VERSION(Zoom,"0.1");

/* === P R O C E D U R E S ================================================= */

/* === M E T H O D S ======================================================= */

/* === E N T R Y P O I N T ================================================= */

Zoom::Zoom():
	param_center(ValueBase(Vector(0,0))),
	param_amount(ValueBase(Real(0)))
{
	SET_INTERPOLATION_DEFAULTS();
	SET_STATIC_DEFAULTS();
}

bool
Zoom::set_param(const String & param, const ValueBase &value)
{

	IMPORT_VALUE(param_center);
	IMPORT_VALUE(param_amount);

	return false;
}

ValueBase
Zoom::get_param(const String &param)const
{
	EXPORT_VALUE(param_center);
	EXPORT_VALUE(param_amount);

	EXPORT_NAME();
	EXPORT_VERSION();

	return ValueBase();
}

Layer::Vocab
Zoom::get_param_vocab()const
{
	Layer::Vocab ret;

	ret.push_back(ParamDesc("amount")
		.set_local_name(_("Amount"))
		.set_description(_("Amount to scale to"))
		.set_origin("center")
		.set_exponential()
	);

	ret.push_back(ParamDesc("center")
		.set_local_name(_("Origin"))
		.set_description(_("Point to scale from"))
		.set_is_distance()
	);

	return ret;
}

Layer::Handle
Zoom::hit_check(Context context, const Point &pos)const
{
	Vector center=param_center.get(Vector());
	return context.hit_check((pos-center)/exp(param_amount.get(Real()))+center);
}

Color
Zoom::get_color(Context context, const Point &pos)const
{
	Vector center=param_center.get(Vector());
	return context.get_color((pos-center)/exp(param_amount.get(Real()))+center);
}

class lyr_std::Zoom_Trans : public Transform
{
	etl::handle<const Zoom> layer;
public:
	Zoom_Trans(const Zoom* x):Transform(x->get_guid()),layer(x) { }

	Vector perform(const Vector& x)const
	{
		Vector center=layer->param_center.get(Vector());
		Real amount=layer->param_amount.get(Real());
		return (x-center)*exp(amount)+center;
	}

	Vector unperform(const Vector& x)const
	{
		Vector center=layer->param_center.get(Vector());
		Real amount=layer->param_amount.get(Real());
		return (x-center)/exp(amount)+center;
	}

	String get_string()const
	{
		return "zoom";
	}
};

etl::handle<Transform>
Zoom::get_transform()const
{
	return new Zoom_Trans(this);
}

Rect
Zoom::get_full_bounding_rect(Context context)const
{
	Vector center=param_center.get(Vector());
	return (context.get_full_bounding_rect()-center)*exp(param_amount.get(Real()))+center;
}

rendering::Task::Handle
Zoom::build_rendering_task_vfunc(Context context)const
{
	Real amount=param_amount.get(Real());
	Point center=param_center.get(Point());

	rendering::TaskTransformationAffine::Handle task_transformation(new rendering::TaskTransformationAffine());
	task_transformation->transformation->matrix =
			Matrix().set_translate(center)
		  * Matrix().set_scale(exp(amount))
		  * Matrix().set_translate(-center);
	task_transformation->sub_task() = context.build_rendering_task();
	return task_transformation;
}
