/* === S Y N F I G ========================================================= */
/*!	\file translate.cpp
**	\brief Implementation of the "Translate" layer
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
#include <synfig/canvas.h>
#include <synfig/transform.h>

#include "translate.h"

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

SYNFIG_LAYER_INIT(Translate);
SYNFIG_LAYER_SET_NAME(Translate,"translate");
SYNFIG_LAYER_SET_LOCAL_NAME(Translate,N_("Translate"));
SYNFIG_LAYER_SET_CATEGORY(Translate,N_("Transform"));
SYNFIG_LAYER_SET_VERSION(Translate,"0.1");

/* === P R O C E D U R E S ================================================= */

/* === M E T H O D S ======================================================= */

/* === E N T R Y P O I N T ================================================= */

Translate::Translate():param_origin(ValueBase(Vector(0,0)))
{
	SET_INTERPOLATION_DEFAULTS();
	SET_STATIC_DEFAULTS();
}

Translate::~Translate()
{
}

bool
Translate::set_param(const String & param, const ValueBase &value)
{
	IMPORT_VALUE(param_origin);

	return false;
}

ValueBase
Translate::get_param(const String& param)const
{
	EXPORT_VALUE(param_origin);
	EXPORT_NAME();
	EXPORT_VERSION();

	return ValueBase();
}

Layer::Vocab
Translate::get_param_vocab()const
{
	Layer::Vocab ret;

	ret.push_back(ParamDesc("origin")
		.set_local_name(_("Origin"))
		.set_description(_("Origin of the translation"))
	);

	return ret;
}

Layer::Handle
Translate::hit_check(Context context, const Point &pos)const
{
	Vector origin=param_origin.get(Vector());
	return context.hit_check(pos-origin);
}

Color
Translate::get_color(Context context, const Point &pos)const
{
	Vector origin=param_origin.get(Vector());
	return context.get_color(pos-origin);
}

class lyr_std::Translate_Trans : public Transform
{
	etl::handle<const Translate> layer;
public:
	Translate_Trans(const Translate* x):Transform(x->get_guid()),layer(x) { }

	Vector perform(const Vector& x)const
	{
		return x+layer->param_origin.get(Vector());
	}

	Vector unperform(const Vector& x)const
	{
		return x-layer->param_origin.get(Vector());
	}

	String get_string()const
	{
		return "translate";
	}
};

etl::handle<Transform>
Translate::get_transform()const
{
	return new Translate_Trans(this);
}

Rect
Translate::get_full_bounding_rect(Context context)const
{
	Vector origin=param_origin.get(Vector());
	return context.get_full_bounding_rect() + origin;
}


rendering::Task::Handle
Translate::build_rendering_task_vfunc(Context context)const
{
	rendering::TaskTransformationAffine::Handle task_transformation(new rendering::TaskTransformationAffine());
	task_transformation->transformation->matrix.set_translate(param_origin.get(Vector()));
	task_transformation->sub_task() = context.build_rendering_task();
	return task_transformation;
}
