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
#include <synfig/rendering/software/surfacesw.h>

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
		.set_local_name(_("Opacity"))
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
	return new rendering::TaskLayer();
	//rendering::TaskLayer::Handle task = new rendering::TaskLayer();
	//// TODO: This is not thread-safe
	//task->layer = const_cast<Layer_Composite*>(this);//clone(NULL);
	//return task;
}

rendering::Task::Handle
Layer_Composite::build_rendering_task_vfunc(Context context)const
{
	rendering::Task::Handle sub_task = build_composite_task_vfunc(context.get_params());
	if (sub_task.type_is<rendering::TaskLayer>())
		return Layer::build_rendering_task_vfunc(context);

	rendering::TaskBlend::Handle task_blend(new rendering::TaskBlend());
	task_blend->amount = get_amount() * Context::z_depth_visibility(context.get_params(), *this);
	task_blend->blend_method = get_blend_method();
	task_blend->sub_task_a() = context.build_rendering_task();
	task_blend->sub_task_b() = sub_task;
	return task_blend;
}
