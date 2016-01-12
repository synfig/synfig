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
#include <synfig/general.h>

#include <synfig/string.h>
#include <synfig/time.h>
#include <synfig/context.h>
#include <synfig/paramdesc.h>
#include <synfig/renddesc.h>
#include <synfig/surface.h>
#include <synfig/value.h>
#include <synfig/valuenode.h>
#include <synfig/transform.h>

#include "zoom.h"

#include <synfig/rendering/common/task/tasktransformation.h>
#include <synfig/rendering/primitive/affinetransformation.h>

#endif

/* === M A C R O S ========================================================= */

/* === G L O B A L S ======================================================= */

SYNFIG_LAYER_INIT(Zoom);
SYNFIG_LAYER_SET_NAME(Zoom,"zoom");
SYNFIG_LAYER_SET_LOCAL_NAME(Zoom,N_("Scale"));
SYNFIG_LAYER_SET_CATEGORY(Zoom,N_("Transform"));
SYNFIG_LAYER_SET_VERSION(Zoom,"0.1");
SYNFIG_LAYER_SET_CVS_ID(Zoom,"$Id$");

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
	);

	return ret;
}

synfig::Layer::Handle
Zoom::hit_check(synfig::Context context, const synfig::Point &pos)const
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

class Zoom_Trans : public Transform
{
	etl::handle<const Zoom> layer;
public:
	Zoom_Trans(const Zoom* x):Transform(x->get_guid()),layer(x) { }

	synfig::Vector perform(const synfig::Vector& x)const
	{
		Vector center=layer->param_center.get(Vector());
		Real amount=layer->param_amount.get(Real());
		return (x-center)*exp(amount)+center;
	}

	synfig::Vector unperform(const synfig::Vector& x)const
	{
		Vector center=layer->param_center.get(Vector());
		Real amount=layer->param_amount.get(Real());
		return (x-center)/exp(amount)+center;
	}

	synfig::String get_string()const
	{
		return "zoom";
	}
};
etl::handle<Transform>
Zoom::get_transform()const
{
	return new Zoom_Trans(this);
}

bool
Zoom::accelerated_render(Context context,Surface *surface,int quality, const RendDesc &renddesc, ProgressCallback *cb)const
{
	Vector center=param_center.get(Vector());
	Real amount=param_amount.get(Real());

	RendDesc transformed_renddesc(renddesc);
	transformed_renddesc.clear_flags();
	transformed_renddesc.set_transformation_matrix(
		Matrix().set_translate(-center)
	  *	Matrix().set_scale(exp(amount))
	  *	Matrix().set_translate(center)
	  * renddesc.get_transformation_matrix() );

	// Render the scene
	return context.accelerated_render(surface,quality,transformed_renddesc,cb);
}


/////
bool
Zoom::accelerated_cairorender(Context context, cairo_t *cr,int quality, const RendDesc &renddesc, ProgressCallback *cb)const
{
	Vector center=param_center.get(Vector());
	Real amount=param_amount.get(Real());

	double zoomfactor=exp(amount);
	
	cairo_save(cr);
	cairo_translate(cr, center[0], center[1]);
	cairo_scale(cr, zoomfactor, zoomfactor);
	cairo_translate(cr, -center[0], -center[1]);

	if(!context.accelerated_cairorender(cr,quality,renddesc,cb))
	{
		cairo_restore(cr);
		return false;
	}
	cairo_restore(cr);
	return true;
}
/////


synfig::Rect
Zoom::get_full_bounding_rect(synfig::Context context)const
{
	Vector center=param_center.get(Vector());
	return (context.get_full_bounding_rect()-center)*exp(param_amount.get(Real()))+center;
}

rendering::Task::Handle
Zoom::build_rendering_task_vfunc(Context context)const
{
	Real amount=param_amount.get(Real());
	Point center=param_center.get(Point());

	rendering::TaskTransformation::Handle task_transformation(new rendering::TaskTransformation());
	rendering::AffineTransformation::Handle affine_transformation(new rendering::AffineTransformation());
	affine_transformation->matrix =
			Matrix().set_translate(-center)
		  * Matrix().set_scale(exp(amount))
		  * Matrix().set_translate(center);
	task_transformation->transformation = affine_transformation;
	task_transformation->sub_task() = context.build_rendering_task();
	return task_transformation;
}
