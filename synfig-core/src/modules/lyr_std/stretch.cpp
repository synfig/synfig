/* === S Y N F I G ========================================================= */
/*!	\file stretch.cpp
**	\brief Implementation of the "Stretch" layer
**
**	$Id$
**
**	\legal
**	Copyright (c) 2002-2005 Robert B. Quattlebaum Jr., Adrian Bentley
**	Copyright (c) 2007 Chris Moore
**	Copyright (c) 2011-2013 Carlos López
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

#include "stretch.h"

#include <synfig/rendering/common/task/tasktransformation.h>
#include <synfig/rendering/primitive/affinetransformation.h>

#endif

/* === U S I N G =========================================================== */

using namespace etl;
using namespace std;
using namespace synfig;

/* === G L O B A L S ======================================================= */

SYNFIG_LAYER_INIT(Layer_Stretch);
SYNFIG_LAYER_SET_NAME(Layer_Stretch,"stretch");
SYNFIG_LAYER_SET_LOCAL_NAME(Layer_Stretch,N_("Stretch"));
SYNFIG_LAYER_SET_CATEGORY(Layer_Stretch,N_("Distortions"));
SYNFIG_LAYER_SET_VERSION(Layer_Stretch,"0.1");
SYNFIG_LAYER_SET_CVS_ID(Layer_Stretch,"$Id$");

/* === P R O C E D U R E S ================================================= */

/* === M E T H O D S ======================================================= */

/* === E N T R Y P O I N T ================================================= */

Layer_Stretch::Layer_Stretch():
	param_amount(ValueBase(Point(1,1))),
	param_center(ValueBase(Point(0,0)))
{
	SET_INTERPOLATION_DEFAULTS();
	SET_STATIC_DEFAULTS();
}


bool
Layer_Stretch::set_param(const String & param, const ValueBase &value)
{
	IMPORT_VALUE(param_amount);
	IMPORT_VALUE(param_center);

	return false;
}

ValueBase
Layer_Stretch::get_param(const String &param)const
{
	EXPORT_VALUE(param_amount);
	EXPORT_VALUE(param_center);

	EXPORT_NAME();
	EXPORT_VERSION();

	return ValueBase();
}

Layer::Vocab
Layer_Stretch::get_param_vocab()const
{
	Layer::Vocab ret;

	ret.push_back(ParamDesc("amount")
		.set_local_name(_("Amount"))
		.set_origin("center")
		.set_description(_("Size of the stretch relative to its Center"))
	);

	ret.push_back(ParamDesc("center")
		.set_local_name(_("Center"))
		.set_description(_("Where the stretch distortion is centered"))
	);

	return ret;
}

synfig::Layer::Handle
Layer_Stretch::hit_check(synfig::Context context, const synfig::Point &pos)const
{
	Vector amount=param_amount.get(Vector());
	Point center=param_center.get(Point());
	
	Point npos(pos);
	npos[0]=(npos[0]-center[0])/amount[0]+center[0];
	npos[1]=(npos[1]-center[1])/amount[1]+center[1];
	return context.hit_check(npos);
}

Color
Layer_Stretch::get_color(Context context, const Point &pos)const
{
	Vector amount=param_amount.get(Vector());
	Point center=param_center.get(Point());

	Point npos(pos);
	npos[0]=(npos[0]-center[0])/amount[0]+center[0];
	npos[1]=(npos[1]-center[1])/amount[1]+center[1];
	return context.get_color(npos);
}

class Stretch_Trans : public Transform
{
	etl::handle<const Layer_Stretch> layer;
public:
	Stretch_Trans(const Layer_Stretch* x):Transform(x->get_guid()),layer(x) { }

	synfig::Vector perform(const synfig::Vector& x)const
	{
		Vector amount=layer->param_amount.get(Vector());
		Point center=layer->param_center.get(Point());

		return Vector((x[0]-center[0])*amount[0]+center[0],
					  (x[1]-center[1])*amount[1]+center[1]);
	}

	synfig::Vector unperform(const synfig::Vector& x)const
	{
		Vector amount=layer->param_amount.get(Vector());
		Point center=layer->param_center.get(Point());

		return Vector((x[0]-center[0])/amount[0]+center[0],
					  (x[1]-center[1])/amount[1]+center[1]);
	}

	synfig::String get_string()const
	{
		return "stretch";
	}
};
etl::handle<Transform>
Layer_Stretch::get_transform()const
{
	return new Stretch_Trans(this);
}

bool
Layer_Stretch::accelerated_render(Context context,Surface *surface,int quality, const RendDesc &renddesc, ProgressCallback *cb)const
{
	Vector amount=param_amount.get(Vector());
	Point center=param_center.get(Point());

	if (amount[0] == 0 || amount[1] == 0)
	{
		surface->set_wh(renddesc.get_w(), renddesc.get_h());
		surface->clear();
		return true;
	}

	RendDesc transformed_renddesc(renddesc);
	transformed_renddesc.clear_flags();
	transformed_renddesc.set_transformation_matrix(
		Matrix().set_translate(-center)
	  *	Matrix().set_scale(amount)
	  *	Matrix().set_translate(center)
	  * renddesc.get_transformation_matrix() );

	// Render the scene
	return context.accelerated_render(surface,quality,transformed_renddesc,cb);
}



bool
Layer_Stretch::accelerated_cairorender(Context context, cairo_t *cr, int quality, const RendDesc &renddesc, ProgressCallback *cb)const
{
	Vector amount=param_amount.get(Vector());
	Point center=param_center.get(Point());

	if (amount[0] == 0 || amount[1] == 0)
	{
		cairo_set_operator(cr, CAIRO_OPERATOR_CLEAR);
		cairo_fill(cr);
		return true;
	}
	const double stx(center[0]);
	const double sty(center[1]);
	
	cairo_save(cr);
	cairo_translate(cr, stx, sty);
	cairo_scale(cr, amount[0], amount[1]);
	cairo_translate(cr, -stx, -sty);

	if(!context.accelerated_cairorender(cr,quality,renddesc,cb))
	{
		cairo_restore(cr);
		return false;
	}
	cairo_restore(cr);
	return true;
}


Rect
Layer_Stretch::get_full_bounding_rect(Context context)const
{
	Vector amount=param_amount.get(Vector());
	Point center=param_center.get(Point());

	Rect rect(context.get_full_bounding_rect());
	Point min(rect.get_min()), max(rect.get_max());

	return Rect(Point((min[0]-center[0])*amount[0]+center[0],
					  (min[1]-center[1])*amount[1]+center[1]),
				Point((max[0]-center[0])*amount[0]+center[0],
					  (max[1]-center[1])*amount[1]+center[1]));
}

rendering::Task::Handle
Layer_Stretch::build_rendering_task_vfunc(Context context)const
{
	Vector amount=param_amount.get(Vector());
	Point center=param_center.get(Point());

	rendering::TaskTransformation::Handle task_transformation(new rendering::TaskTransformation());
	rendering::AffineTransformation::Handle affine_transformation(new rendering::AffineTransformation());
	affine_transformation->matrix =
			Matrix().set_translate(-center)
		  * Matrix().set_scale(amount)
		  * Matrix().set_translate(center);
	task_transformation->transformation = affine_transformation;
	task_transformation->sub_task() = context.build_rendering_task();
	return task_transformation;
}
