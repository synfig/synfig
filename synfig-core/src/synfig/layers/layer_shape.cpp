/* === S Y N F I G ========================================================= */
/*!	\file layer_shape.cpp
**	\brief Implementation of the "Shape" layer
**
**	$Id$
**
**	\legal
**	Copyright (c) 2002-2005 Robert B. Quattlebaum Jr., Adrian Bentley
**	Copyright (c) 2007, 2008 Chris Moore
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

#include <cfloat>

#include <vector>

#include "layer_shape.h"

#include <synfig/general.h>
#include <synfig/localization.h>

#include <synfig/blur.h>
#include <synfig/context.h>
#include <synfig/curve_helper.h>
#include <synfig/paramdesc.h>
#include <synfig/renddesc.h>
#include <synfig/string.h>
#include <synfig/surface.h>
#include <synfig/time.h>
#include <synfig/value.h>
#include <synfig/valuenode.h>

#include <synfig/rendering/primitive/intersector.h>
#include <synfig/rendering/common/task/taskblend.h>
#include <synfig/rendering/common/task/taskblur.h>
#include <synfig/rendering/common/task/taskcontour.h>
#include <synfig/rendering/software/function/contour.h>
#include <synfig/rendering/software/function/blur.h>

#endif

/* === U S I N G =========================================================== */

using namespace synfig;
using namespace std;
using namespace etl;

/* === G L O B A L S ======================================================= */

SYNFIG_LAYER_INIT(Layer_Shape);
SYNFIG_LAYER_SET_NAME(Layer_Shape,"shape");
SYNFIG_LAYER_SET_LOCAL_NAME(Layer_Shape,N_("Shape"));
SYNFIG_LAYER_SET_CATEGORY(Layer_Shape,N_("Internal"));
SYNFIG_LAYER_SET_VERSION(Layer_Shape,"0.1");
SYNFIG_LAYER_SET_CVS_ID(Layer_Shape,"$Id$");

/* === C L A S S E S ======================================================= */

/* === M E T H O D S ======================================================= */

Layer_Shape::Layer_Shape(const Real &a, const Color::BlendMethod m):
	Layer_Composite      (a, m),
	param_color          (Color::black()),
	param_origin         (Vector(0,0)),
	param_invert         (bool(false)),
	param_antialias      (bool(true)),
	param_blurtype       (int(Blur::FASTGAUSSIAN)),
	param_feather        (Real(0.0)),
	param_winding_style	 (int(rendering::Contour::WINDING_NON_ZERO)),
	contour				 (new rendering::Contour)
{ }

Layer_Shape::~Layer_Shape()
	{ }

void
Layer_Shape::clear()
	{ contour->clear(); }

bool
Layer_Shape::set_shape_param(const String &/* param */, const synfig::ValueBase &/* value */)
	{ return false; }

bool
Layer_Shape::set_param(const String & param, const ValueBase &value)
{
	if (set_shape_param(param, value))
		{ force_sync(); return true; }

	IMPORT_VALUE_PLUS(param_color,
	{
		Color color=param_color.get(Color());
		if (color.get_a() == 0)
		{
			if (converted_blend_)
			{
				set_blend_method(Color::BLEND_ALPHA_OVER);
				color.set_a(1);
			}
			else
			transparent_color_ = true;
		}
		param_color.set(color);
	}
	);
	IMPORT_VALUE(param_origin);
	IMPORT_VALUE(param_invert);
	IMPORT_VALUE(param_antialias);
	IMPORT_VALUE_PLUS(param_feather,
	{
		Real feather=param_feather.get(Real());
		if(feather<0)
		{
			feather=0;
			param_feather.set(feather);
		}
		set_feather(Vector(feather, feather));
	}
	);

	IMPORT_VALUE(param_blurtype);
	IMPORT_VALUE(param_winding_style);

	if(param=="offset" && param_origin.get_type() == value.get_type())
	{
		param_origin=value;
		return true;
	}
	return Layer_Composite::set_param(param,value);
}

ValueBase
Layer_Shape::get_param(const String &param)const
{
	EXPORT_VALUE(param_color);
	EXPORT_VALUE(param_origin);
	EXPORT_VALUE(param_invert);
	EXPORT_VALUE(param_antialias);
	EXPORT_VALUE(param_feather);
	EXPORT_VALUE(param_blurtype);
	EXPORT_VALUE(param_winding_style);

	EXPORT_NAME();
	EXPORT_VERSION();

	return Layer_Composite::get_param(param);
}

Layer::Vocab
Layer_Shape::get_param_vocab()const
{
	Layer::Vocab ret(Layer_Composite::get_param_vocab());

	ret.push_back(ParamDesc("color")
		.set_local_name(_("Color"))
		.set_description(_("Layer_Shape Color"))
	);
	ret.push_back(ParamDesc("origin")
		.set_local_name(_("Origin"))
	);
	ret.push_back(ParamDesc("invert")
		.set_local_name(_("Invert"))
	);
	ret.push_back(ParamDesc("antialias")
		.set_local_name(_("Antialiasing"))
	);
	ret.push_back(ParamDesc("feather")
		.set_local_name(_("Feather"))
		.set_is_distance()
	);
	ret.push_back(ParamDesc("blurtype")
		.set_local_name(_("Type of Feather"))
		.set_description(_("Type of feathering to use"))
		.set_hint("enum")
		.add_enum_value(Blur::BOX,"box",_("Box Blur"))
		.add_enum_value(Blur::FASTGAUSSIAN,"fastgaussian",_("Fast Gaussian Blur"))
		.add_enum_value(Blur::CROSS,"cross",_("Cross-Hatch Blur"))
		.add_enum_value(Blur::GAUSSIAN,"gaussian",_("Gaussian Blur"))
		.add_enum_value(Blur::DISC,"disc",_("Disc Blur"))
	);
	ret.push_back(ParamDesc("winding_style")
		.set_local_name(_("Winding Style"))
		.set_description(_("Winding style to use"))
		.set_hint("enum")
		.add_enum_value(rendering::Contour::WINDING_NON_ZERO, "nonzero", _("Non Zero"))
		.add_enum_value(rendering::Contour::WINDING_EVEN_ODD, "evenodd", _("Even/Odd"))
	);

	return ret;
}

synfig::Layer::Handle
Layer_Shape::hit_check(synfig::Context context, const synfig::Point &point) const
{
	sync();

	Color::BlendMethod blend_method = get_blend_method();
	Color color = param_color.get(Color());
	bool invert = param_invert.get(bool(true));
	Point origin = param_origin.get(Point());
	rendering::Contour::WindingStyle winding_style = (rendering::Contour::WindingStyle)param_winding_style.get(int());

	bool inside = false;
	if (get_amount() && blend_method != Color::BLEND_ALPHA_OVER)
		inside = contour->is_inside(point - origin, winding_style, invert);

	if (inside) {
		if (blend_method == Color::BLEND_BEHIND) {
			synfig::Layer::Handle layer = context.hit_check(point);
			if (layer) return layer;
		}
		
		if (Color::is_onto(blend_method)) {
			//if there's something in the lower layer then we're set...
			if (context.hit_check(point))
				return const_cast<Layer_Shape*>(this);
		} else
		if (blend_method == Color::BLEND_ALPHA_OVER) {
			synfig::info("layer_shape::hit_check - we've got alphaover");
			//if there's something in the lower layer then we're set...
			if (color.get_a() < 0.1 && get_amount() > .9) {
				synfig::info("layer_shape::hit_check - can see through us... so nothing");
				return Handle();
			}
		} else
			return const_cast<Layer_Shape*>(this);
	}

	return context.hit_check(point);
}

Color
Layer_Shape::get_color(Context context, const Point &p)const
{
	sync();

	Color color = param_color.get(Color());
	Point origin = param_origin.get(Point());
	bool invert = param_invert.get(bool(true));
	int blurtype = param_blurtype.get(int());
	Real feather = param_feather.get(Real());
	rendering::Contour::WindingStyle winding_style = (rendering::Contour::WindingStyle)param_winding_style.get(int());

	Point pp = p;
	if (feather)
		pp = Blur(feather,feather,blurtype)(p);

	bool inside = contour->is_inside(pp - origin, winding_style, invert);
	if (!inside)
		return Color::blend(Color::alpha(), context.get_color(pp), get_amount(), get_blend_method());

	//Ok, we're inside... bummmm ba bum buM...
	if (get_blend_method() == Color::BLEND_STRAIGHT && get_amount() == 1)
		return color;
	
	return Color::blend(color, context.get_color(p), get_amount(), get_blend_method());
}

void Layer_Shape::move_to(Real x, Real y)
	{ contour->move_to(Vector(x, y)); }
void Layer_Shape::close()
	{ contour->close(); }
void Layer_Shape::line_to(Real x, Real y)
	{ contour->line_to(Vector(x, y)); }
void Layer_Shape::conic_to(Real x, Real y, Real x1, Real y1)
	{ contour->conic_to(Vector(x, y), Vector(x1, y1)); }
void Layer_Shape::cubic_to(Real x, Real y, Real x1, Real y1, Real x2, Real y2)
	{ contour->cubic_to(Vector(x, y), Vector(x1, y1), Vector(x2, y2)); }
void Layer_Shape::add(const rendering::Contour::Chunk &chunk)
	{ contour->add_chunk(chunk); }
void Layer_Shape::add(const rendering::Contour::ChunkList &chunks)
	{ contour->add_chunks(chunks); }
void Layer_Shape::add_reverse(const rendering::Contour::ChunkList &chunks)
	{ contour->add_chunks_reverse(chunks); }

void
Layer_Shape::set_time_vfunc(IndependentContext context, Time time)const
{
	sync();
	Layer_Composite::set_time_vfunc(context, time);
}

void
Layer_Shape::sync(bool force) const
{
	if ( force
	  || !last_sync_time.is_equal(get_time_mark())
	  || fabs(last_sync_outline_grow - get_outline_grow_mark()) > 1e-8 )
	{
		last_sync_time = get_time_mark();
		last_sync_outline_grow = get_outline_grow_mark();
		const_cast<Layer_Shape*>(this)->sync_vfunc();
		contour->close();
	}
}

void
Layer_Shape::sync_vfunc()
	{ }

bool
Layer_Shape::render_shape(Surface *surface, bool useblend, const RendDesc &renddesc) const
{
	Point origin=param_origin.get(Point());
	Matrix translate;
	translate.set_translate(origin);
	Matrix world_to_pixels_matrix =
	    renddesc.get_world_to_pixels_matrix()
	  * renddesc.get_transformation_matrix()
	  * translate;

	rendering::software::Contour::render_contour(
		*surface,
		contour->get_chunks(),
		param_invert.get(bool(true)),
		param_antialias.get(bool(true)),
		(rendering::Contour::WindingStyle)param_winding_style.get(int()),
		world_to_pixels_matrix,
		param_color.get(Color()),
		useblend ? get_amount() : 1.0,
		useblend ? get_blend_method() : Color::BLEND_STRAIGHT );

	return true;
}

rendering::Task::Handle
Layer_Shape::build_composite_task_vfunc(ContextParams /*context_params*/)const
{
	sync();
	rendering::Task::Handle task;

	rendering::TaskContour::Handle task_contour(new rendering::TaskContour());
	// TODO: multithreading without this copying
	task_contour->transformation->matrix.set_translate( param_origin.get(Vector()) );
	task_contour->contour = new rendering::Contour();
	task_contour->contour->assign(*contour);
	task_contour->contour->color = param_color.get(Color());
	task_contour->contour->invert = param_invert.get(bool());
	task_contour->contour->antialias = param_antialias.get(bool());
	task_contour->contour->winding_style = (rendering::Contour::WindingStyle)param_winding_style.get(int());
	task = task_contour;

	rendering::Blur::Type blurtype = (rendering::Blur::Type)param_blurtype.get(int());
	Vector feather = get_feather();
	if (feather != Vector::zero())
	{
		rendering::TaskBlur::Handle task_blur(new rendering::TaskBlur());
		task_blur->blur.size = feather;
		task_blur->blur.type = blurtype;
		task_blur->sub_task() = task;
		task = task_blur;
	}

	return task;
}

Rect
Layer_Shape::get_bounding_rect()const
{
	sync();
	Point origin = param_origin.get(Point());
	bool invert = param_invert.get(bool(true));
	rendering::Blur::Type blurtype = (rendering::Blur::Type)param_blurtype.get(int());
	Vector feather = get_feather();

	Real feather_amplifier = rendering::software::Blur::get_size_amplifier(blurtype);

	if(invert)
		return Rect::full_plane();

	Rect bounds = contour->get_bounds();
	if (!bounds.is_valid())
		return Rect::zero();
	
	bounds += origin;
	bounds.expand((bounds.get_min() - bounds.get_max()).mag()*0.01);
	bounds.expand_x( fabs(feather_amplifier * feather[0]) );
	bounds.expand_x( fabs(feather_amplifier * feather[1]) );

	return bounds;
}
