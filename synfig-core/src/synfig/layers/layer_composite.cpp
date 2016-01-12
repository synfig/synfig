/* === S Y N F I G ========================================================= */
/*!	\file layer_composite.cpp
**	\brief Template File
**
**	$Id$
**
**	\legal
**	Copyright (c) 2002-2005 Robert B. Quattlebaum Jr., Adrian Bentley
**	Copyright (c) 2008 Chris Moore
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

#include <synfig/general.h>
#include <synfig/localization.h>
#include <synfig/context.h>
#include <synfig/time.h>
#include <synfig/color.h>
#include <synfig/surface.h>
#include <synfig/renddesc.h>
#include <synfig/target.h>
#include <synfig/render.h>
#include <synfig/paramdesc.h>
#include <synfig/cairo_renddesc.h>

#include "layer_composite.h"

#include "layer_pastecanvas.h"
#include "layer_bitmap.h"
#include <synfig/rendering/common/task/taskblend.h>
#include <synfig/rendering/common/task/tasklayer.h>

#endif

/* === U S I N G =========================================================== */

using namespace std;
using namespace etl;
using namespace synfig;

/* === M A C R O S ========================================================= */

/* === G L O B A L S ======================================================= */

/* === P R O C E D U R E S ================================================= */

/* === M E T H O D S ======================================================= */
Layer_Composite::Layer_Composite(Real a, Color::BlendMethod bm):
		param_amount		(a),
		param_blend_method	((int)Color::BlendMethod(bm)),
		converted_blend_	(false),
		transparent_color_	(false)
	{
		SET_INTERPOLATION_DEFAULTS();
		SET_STATIC_DEFAULTS();
	}

bool
Layer_Composite::accelerated_render(Context context,Surface *surface,int quality, const RendDesc &renddesc, ProgressCallback *cb)  const
{
	RENDER_TRANSFORMED_IF_NEED(__FILE__, __LINE__)

	Real amount(param_amount.get(Real()));
	if(!amount)
		return context.accelerated_render(surface,quality,renddesc,cb);

	CanvasBase image;

	SuperCallback stageone(cb,0,50000,100000);
	SuperCallback stagetwo(cb,50000,100000,100000);

	Layer_Bitmap::Handle surfacelayer(new class Layer_Bitmap());

	Context iter;

	for(iter=context;*iter;iter++)
		image.push_back(*iter);

	image.push_front(surfacelayer.get());

	// We want to go ahead and schedule any other
	// layers...
//	while(dynamic_cast<Layer_Composite*>(context->get()))
//	while(context->get() &&
//		&context->get()->AcceleratedRender==
//		&Layer_Composite::AcceleratedRender)
//		image.push_back(*(context++));

	image.push_back(0);	// Alpha black

	// Render the backdrop on the surface layer's surface.
	if(!context.accelerated_render(&surfacelayer->surface,quality,renddesc,&stageone))
		return false;
	// Sets up the interpolation of the context (now the surface layer is the first one)
	// depending on the quality
	if(quality<=4)surfacelayer->set_param("c", 3);else
	if(quality<=5)surfacelayer->set_param("c", 2);
	else if(quality<=6)surfacelayer->set_param("c", 1);
	else surfacelayer->set_param("c",0);
	surfacelayer->set_param("tl",renddesc.get_tl());
	surfacelayer->set_param("br",renddesc.get_br());
	// Sets the blend method to straight. See below
	surfacelayer->set_blend_method(Color::BLEND_STRAIGHT);
	// Push this layer on the image. The blending result is only this layer
	// adn the surface layer. The rest of the context is ignored by the straight
	// blend method of surface layer
	image.push_front(const_cast<synfig::Layer_Composite*>(this));

	// Set up a surface target
	Target_Scanline::Handle target(surface_target_scanline(surface));

	if(!target)
	{
		if(cb)cb->error(_("Unable to create surface target"));
		return false;
	}

	RendDesc desc(renddesc);

	target->set_rend_desc(&desc);

	// Render the scene
	return render(Context(image.begin(),context),target,desc,&stagetwo);
	//return render_threaded(Context(image.begin()),target,desc,&stagetwo,2);
}


/////
bool
Layer_Composite::accelerated_cairorender(Context context,cairo_t *cr, int quality, const RendDesc &renddesc_, ProgressCallback *cb)  const
{
	RendDesc renddesc(renddesc_);
	Real amount(param_amount.get(Real()));

	if(!amount)
		return context.accelerated_cairorender(cr,quality,renddesc,cb);

	// Untransform the render desc
	if(!cairo_renddesc_untransform(cr, renddesc))
		return false;
	const Real pw(renddesc.get_pw()),ph(renddesc.get_ph());
	const Point tl(renddesc.get_tl());
	const int w(renddesc.get_w());
	const int h(renddesc.get_h());
	
	CanvasBase image;
	
	SuperCallback stageone(cb,0,50000,100000);
	SuperCallback stagetwo(cb,50000,100000,100000);
	
	Layer_Bitmap::Handle surfacelayer(new class Layer_Bitmap());
	
	Context iter;
	
	for(iter=context;*iter;iter++)
		image.push_back(*iter);
	
	// Add one Bitmap Layer on top
	image.push_front(surfacelayer.get());
	
	image.push_back(0);	// and Alpha black at end
	
	// Render the backdrop on the surface layer's surface.
	cairo_surface_t* cs=cairo_image_surface_create(CAIRO_FORMAT_ARGB32, w, h);
	cairo_surface_t* result=cairo_image_surface_create(CAIRO_FORMAT_ARGB32, w, h);
	cairo_t* cr_cs=cairo_create(cs);
	cairo_scale(cr_cs, 1/pw, 1/ph);
	cairo_translate(cr_cs, -tl[0], -tl[1]);
	if(!context.accelerated_cairorender(cr_cs,quality,renddesc,&stageone))
	{
		cairo_surface_destroy(cs);
		cairo_destroy(cr_cs);
		return false;
	}
	cairo_destroy(cr_cs);
	surfacelayer->set_cairo_surface(cs);
	cairo_surface_destroy(cs);
	// Sets up the interpolation of the context (now the surface layer is the first one)
	// depending on the quality
	if(quality<=4)surfacelayer->set_param("c", 3);else
		if(quality<=5)surfacelayer->set_param("c", 2);
		else if(quality<=6)surfacelayer->set_param("c", 1);
		else surfacelayer->set_param("c",0);
	surfacelayer->set_param("tl",renddesc.get_tl());
	surfacelayer->set_param("br",renddesc.get_br());
	// Sets the blend method to straight. See below
	surfacelayer->set_blend_method(Color::BLEND_STRAIGHT);
	surfacelayer->set_render_method(context, CAIRO);
	// Push this layer on the image. The blending result is only this layer
	// and the surface layer. The rest of the context is ignored by the straight
	// blend method of surface layer
	image.push_front(const_cast<synfig::Layer_Composite*>(this));
	
	// Render the scene
	if(!cairorender(Context(image.begin(),context),result,renddesc,&stagetwo))
	{
		cairo_surface_destroy(result);
		return false;
	}
	// Put the result on the cairo context
	cairo_save(cr);
	cairo_translate(cr, tl[0], tl[1]);
	cairo_scale(cr, pw, ph);
	cairo_set_source_surface(cr, result, 0, 0);
	cairo_set_operator(cr, CAIRO_OPERATOR_SOURCE);
	cairo_paint(cr);
	cairo_restore(cr);

	cairo_surface_destroy(result);
	// Mark our progress as finished
	if(cb && !cb->amount_complete(10000,10000))
		return false;
	return true;
}


Rect
Layer_Composite::get_full_bounding_rect(Context context)const
{
	if(is_disabled() || Color::is_onto(get_blend_method()))
		return context.get_full_bounding_rect();

	return context.get_full_bounding_rect()|get_bounding_rect();
}

Layer::Vocab
Layer_Composite::get_param_vocab()const
{
	//! First fills the returning vocabulary with the ancestor class
	Layer::Vocab ret(Layer::get_param_vocab());
	//! Now inserts the two parameters that this layer knows.
	ret.push_back(ParamDesc(param_amount,"amount")
		.set_local_name(_("Amount"))
		.set_description(_("Alpha channel of the layer"))
	);
	ret.push_back(ParamDesc(param_blend_method,"blend_method")
		.set_local_name(_("Blend Method"))
		.set_description(_("The blending method used to composite on the layers below"))
		.set_static(true)
	);

	return ret;
}

bool
Layer_Composite::set_param(const String & param, const ValueBase &value)
{
	IMPORT_VALUE(param_amount)
	IMPORT_VALUE_PLUS(param_blend_method,
		Color::BlendMethod blend_method = static_cast<Color::BlendMethod>(value.get(int()));
		if (blend_method < 0 || blend_method >= Color::BLEND_END)
		{
			warning("illegal value (%d) for blend_method - using Composite instead", blend_method);
			param_blend_method.set((int)Color::BLEND_COMPOSITE);
			return false;
		}

		if (blend_method == Color::BLEND_STRAIGHT && !reads_context())
		{
			Canvas::Handle canvas(get_canvas());
			if (canvas)
			{
				String version(canvas->get_version());

				if (version == "0.1" || version == "0.2")
				{
					if (dynamic_cast<Layer_PasteCanvas*>(this) != NULL)
						warning("loaded a version %s canvas with a 'Straight' blended PasteCanvas (%s) - check it renders OK",
								version.c_str(), get_non_empty_description().c_str());
					else
					{
						param_blend_method.set(int(Color::BLEND_COMPOSITE));
						converted_blend_ = true;

						// if this layer has a transparent color, go back and set the color again
						// now that we know we are converting the blend method as well.  that will
						// make the color non-transparent, and change the blend method to alpha over
						if (transparent_color_)
							set_param("color", get_param("color"));
					}
				}
			}
		}
		);

	return Layer::set_param(param,value);
}

ValueBase
Layer_Composite::get_param(const String & param)const
{

	EXPORT_VALUE(param_amount)
	EXPORT_VALUE(param_blend_method)
	//! If it is unknown then call the ancestor's get param member
	//! to see if it can handle that parameter's string.
	return Layer::get_param(param);
}

rendering::Task::Handle
Layer_Composite::build_composite_task_vfunc(ContextParams /*context_params*/)const
{
	rendering::TaskLayer::Handle task = new rendering::TaskLayer();
	task->layer = clone(NULL);
	return task;
}

rendering::Task::Handle
Layer_Composite::build_rendering_task_vfunc(Context context)const
{
	rendering::TaskBlend::Handle task_blend(new rendering::TaskBlend());
	task_blend->amount = get_amount() * Context::z_depth_visibility(context.get_params(), *this);
	task_blend->blend_method = get_blend_method();
	task_blend->sub_task_a() = context.build_rendering_task();
	task_blend->sub_task_b() = build_composite_task_vfunc(context.get_params());
	return task_blend;
}
