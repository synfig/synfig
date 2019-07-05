/* === S Y N F I G ========================================================= */
/*!	\file layer_motionblur.cpp
**	\brief Implementation of the "Motion Blur" layer
**
**	$Id$
**
**	\legal
**	Copyright (c) 2002-2005 Robert B. Quattlebaum Jr., Adrian Bentley
**	Copyright (c) 2007, 2008 Chris Moore
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

#include "layer_motionblur.h"

#include <synfig/general.h>
#include <synfig/localization.h>

#include <synfig/canvas.h>
#include <synfig/context.h>
#include <synfig/paramdesc.h>
#include <synfig/renddesc.h>
#include <synfig/string.h>
#include <synfig/surface.h>
#include <synfig/time.h>
#include <synfig/value.h>
#include <synfig/valuenode.h>

#include <synfig/rendering/common/task/taskblend.h>

#endif

/* === U S I N G =========================================================== */

using namespace synfig;
using namespace etl;
using namespace std;

/* === G L O B A L S ======================================================= */

SYNFIG_LAYER_INIT(Layer_MotionBlur);
SYNFIG_LAYER_SET_NAME(Layer_MotionBlur,"MotionBlur"); // todo: use motion_blur
SYNFIG_LAYER_SET_LOCAL_NAME(Layer_MotionBlur,N_("Motion Blur"));
SYNFIG_LAYER_SET_CATEGORY(Layer_MotionBlur,N_("Blurs"));
SYNFIG_LAYER_SET_VERSION(Layer_MotionBlur,"0.1");
SYNFIG_LAYER_SET_CVS_ID(Layer_MotionBlur,"$Id$");

/* === M E M B E R S ======================================================= */

Layer_MotionBlur::Layer_MotionBlur():
	Layer_CompositeFork     (1.0,Color::BLEND_STRAIGHT),
	param_aperture          (ValueBase(Time(1.0))),
	param_subsamples_factor (ValueBase(Real(1.0))),
	param_subsampling_type  (ValueBase(int(SUBSAMPLING_HYPERBOLIC))),
	param_subsample_start   (ValueBase(Real(0.0))),
	param_subsample_end     (ValueBase(Real(1.0)))
{

}

bool
Layer_MotionBlur::set_param(const String &param, const ValueBase &value)
{

	IMPORT_VALUE(param_aperture);
	IMPORT_VALUE(param_subsamples_factor);
	IMPORT_VALUE(param_subsampling_type);
	IMPORT_VALUE(param_subsample_start);
	IMPORT_VALUE(param_subsample_end);
	return Layer_Composite::set_param(param,value);
}

ValueBase
Layer_MotionBlur::get_param(const String &param)const
{
	EXPORT_VALUE(param_aperture);
	EXPORT_VALUE(param_subsamples_factor);
	EXPORT_VALUE(param_subsampling_type);
	EXPORT_VALUE(param_subsample_start);
	EXPORT_VALUE(param_subsample_end);

	EXPORT_NAME();
	EXPORT_VERSION();

	return Layer_Composite::get_param(param);
}

Color
Layer_MotionBlur::get_color(Context context, const Point &pos)const
{
/*	if(aperture)
	{
		Time time(get_time_mark());
		time+=(Vector::value_type)( (signed)(RAND_MAX/2)-(signed)rand() )/(Vector::value_type)(RAND_MAX) *aperture -aperture*0.5;
		context.set_time(time, pos);
	}
*/
	return context.get_color(pos);
}

Layer::Vocab
Layer_MotionBlur::get_param_vocab()const
{
	Layer::Vocab ret;
	//ret=Layer_Composite::get_param_vocab();

	ret.push_back(ParamDesc("aperture")
		.set_local_name(_("Aperture"))
		.set_description(_("Shutter Time"))
	);

	ret.push_back(ParamDesc("subsamples_factor")
		.set_local_name(_("Subsamples Factor"))
		.set_description(_("Multiplies The Number Of Subsamples Rendered"))
	);

	ret.push_back(ParamDesc("subsampling_type")
		.set_local_name(_("Subsampling Type"))
		.set_description(_("Curve Type For Weighting Subsamples"))
		.set_hint("enum")
		.add_enum_value(SUBSAMPLING_CONSTANT,"constant",_("Constant"))
		.add_enum_value(SUBSAMPLING_LINEAR,"linear",_("Linear"))
		.add_enum_value(SUBSAMPLING_HYPERBOLIC,"hyperbolic",_("Hyperbolic"))
	);

	ret.push_back(ParamDesc("subsample_start")
		.set_local_name(_("Subsample Start Amount"))
		.set_description(_("Relative Amount Of The First Subsample, For Linear Weighting"))
	);

	ret.push_back(ParamDesc("subsample_end")
		.set_local_name(_("Subsample End Amount"))
		.set_description(_("Relative Amount Of The Last Subsample, For Linear Weighting"))
	);

	return ret;
}

bool
Layer_MotionBlur::accelerated_render(Context context,Surface *surface,int quality, const RendDesc &renddesc, ProgressCallback *cb)const
{
	RENDER_TRANSFORMED_IF_NEED(__FILE__, __LINE__)

	Time aperture=param_aperture.get(Time());
	Real subsamples_factor=param_subsamples_factor.get(Real());
	SubsamplingType subsampling_type=(SubsamplingType)param_subsampling_type.get(int());
	Real subsample_start=param_subsample_start.get(Real());
	Real subsample_end=param_subsample_end.get(Real());
	
	if(aperture && quality<=10)
	{
		//int x, y;
		SuperCallback subimagecb;
		int samples=1;
		switch(quality)
		{
			case 1:	// Production Quality
				samples=32;
				break;
			case 2: // Excellent Quality
				samples=24;
				break;
			case 3: // Good Quality
				samples=16;
				break;
			case 4: // Moderate Quality
				samples=12;
				break;
			case 5: // Draft Quality
				samples=7;
				break;
			case 6:
				samples=6;
				break;
			case 7:
				samples=5;
				break;
			case 8:
				samples=3;
				break;
			case 9:
				samples=2;
				break;
			case 10: // Rough Quality
            default:
				samples=1;
				break;

		}

		samples *= subsamples_factor;

		if (samples <= 1) return context.accelerated_render(surface,quality,renddesc,cb);

		// Only in modes where subsample_start/end matters...
		if(subsampling_type == SUBSAMPLING_LINEAR)
		{
			// We won't render when the scale==0, so we'll use those samples elsewhere
			if(subsample_start == 0) samples++;
			if(subsample_end == 0) samples++;
		}

		Surface tmp;
		int i;
		float scale, divisor = 0;

		surface->set_wh(renddesc.get_w(),renddesc.get_h());
		surface->clear();

		// Render subsamples from time_cur-aperture to time_cur
		for(i=0;i<samples;i++)
		{
			float pos = i/(samples-1.0);
			float ipos = 1.0-pos;
			switch(subsampling_type)
			{
				case SUBSAMPLING_LINEAR:
					scale = ipos*subsample_start + pos*subsample_end;
					break;
				case SUBSAMPLING_HYPERBOLIC:
					scale = 1.0/(samples-i);
					break;
				case SUBSAMPLING_CONSTANT:
				default:
					scale = 1.0; // Weights don't matter for constant overall subsampling.
					break;
			}
			// Don't bother rendering if scale is zero
			if(scale==0)
				continue;
			divisor += scale;
			subimagecb=SuperCallback(cb,i*(5000/samples),(i+1)*(5000/samples),5000);
			context.set_time(get_time_mark()-aperture*ipos);
			if(!context.accelerated_render(&tmp,quality,renddesc,&subimagecb))
				return false;
			for(int y=0;y<renddesc.get_h();y++)
				for(int x=0;x<renddesc.get_w();x++)
					(*surface)[y][x]+=tmp[y][x].premult_alpha()*scale;
		}
		for(int y=0;y<renddesc.get_h();y++)
			for(int x=0;x<renddesc.get_w();x++)
				(*surface)[y][x]=((*surface)[y][x]/divisor).demult_alpha();
	}
	else
		return context.accelerated_render(surface,quality,renddesc,cb);

	return true;
}


bool
Layer_MotionBlur::accelerated_cairorender(Context context, cairo_t *cr ,int quality, const RendDesc &renddesc, ProgressCallback *cb)const
{
	Time aperture=param_aperture.get(Time());
	Real subsamples_factor=param_subsamples_factor.get(Real());
	SubsamplingType subsampling_type=param_subsampling_type.get(SUBSAMPLING_LINEAR);
	Real subsample_start=param_subsample_start.get(Real());
	Real subsample_end=param_subsample_end.get(Real());

	if(aperture && quality<=10)
	{
		//int x, y;
		SuperCallback subimagecb;
		int samples=1;
		switch(quality)
		{
			case 1:	// Production Quality
				samples=32;
				break;
			case 2: // Excellent Quality
				samples=24;
				break;
			case 3: // Good Quality
				samples=16;
				break;
			case 4: // Moderate Quality
				samples=12;
				break;
			case 5: // Draft Quality
				samples=7;
				break;
			case 6:
				samples=6;
				break;
			case 7:
				samples=5;
				break;
			case 8:
				samples=3;
				break;
			case 9:
				samples=2;
				break;
			case 10: // Rough Quality
            default:
				samples=1;
				break;
				
		}
		
		samples *= subsamples_factor;
		
		if (samples <= 1) return context.accelerated_cairorender(cr,quality,renddesc,cb);
		
		// Only in modes where subsample_start/end matters...
		if(subsampling_type == SUBSAMPLING_LINEAR)
		{
			// We won't render when the scale==0, so we'll use those samples elsewhere
			if(subsample_start == 0) samples++;
			if(subsample_end == 0) samples++;
		}
		// We need to clear the given surface since it maybe not clean.
//		cairo_set_operator(cr, CAIRO_OPERATOR_CLEAR);
//		cairo_paint(cr);
		
		int i;
		float scale, divisor = 0;
		
		// Precalculate the divisor
		// this way we can use directly the values from the cairo surface
		// and don't need to premultiply before sum and demultiply after divide
		// Also it avoids a final for loop on the image pixels to divide.
		for(i=0;i<samples;i++)
		{
			float pos = i/(samples-1.0);
			float ipos = 1.0-pos;
			switch(subsampling_type)
			{
				case SUBSAMPLING_LINEAR:
					scale = ipos*subsample_start + pos*subsample_end;
					break;
				case SUBSAMPLING_HYPERBOLIC:
					scale = 1.0/(samples-i);
					break;
				case SUBSAMPLING_CONSTANT:
				default:
					scale = 1.0; // Weights don't matter for constant overall subsampling.
					break;
			}
			// Don't bother rendering if scale is zero
			if(scale==0)
				continue;
			divisor += scale;
		}
		
		// Render subsamples from time_cur-aperture to time_cur
		for(i=0;i<samples;i++)
		{
			float pos = i/(samples-1.0);
			float ipos = 1.0-pos;
			switch(subsampling_type)
			{
				case SUBSAMPLING_LINEAR:
					scale = ipos*subsample_start + pos*subsample_end;
					break;
				case SUBSAMPLING_HYPERBOLIC:
					scale = 1.0/(samples-i);
					break;
				case SUBSAMPLING_CONSTANT:
				default:
					scale = 1.0; // Weights don't matter for constant overall subsampling.
					break;
			}
			// Don't bother rendering if scale is zero
			if(scale==0)
				continue;
			subimagecb=SuperCallback(cb,i*(5000/samples),(i+1)*(5000/samples),5000);
			context.set_time(get_time_mark()-aperture*ipos);
			cairo_push_group(cr);
			if(!context.accelerated_cairorender(cr,quality,renddesc,&subimagecb))
			{
				cairo_pop_group(cr);
				return false;
			}
			cairo_pop_group_to_source(cr);
			float s=scale/divisor;
			cairo_set_operator(cr, CAIRO_OPERATOR_ADD);
			cairo_paint_with_alpha(cr, s);
		}
	}
	else
		return context.accelerated_cairorender(cr,quality,renddesc,cb);
	
	return true;
}

rendering::Task::Handle
Layer_MotionBlur::build_rendering_task_vfunc(Context context) const
{
	const Real precision = 1e-8;

	Time aperture = param_aperture.get(Time());
	Real subsamples_factor = param_subsamples_factor.get(Real());
	SubsamplingType subsampling_type = (SubsamplingType)param_subsampling_type.get(int());
	Real subsample_start = param_subsample_start.get(Real());
	Real subsample_end = param_subsample_end.get(Real());

	int samples = (int)round(12.0 * fabs(subsamples_factor));
	if (samples <= 1)
		return context.build_rendering_task();

	// Only in modes where subsample_start/end matters...
	if (subsampling_type == SUBSAMPLING_LINEAR)
	{
		// We won't render when the scale==0, so we'll use those samples elsewhere
		if (fabs(subsample_start) < precision) ++samples;
		if (fabs(subsample_end) < precision) ++samples;
	}

	vector<Real> scales(samples, 0.0);
	Real sum = 0.0;
	for(int i = 0; i < samples; i++)
	{
		Real pos = (Real)i/(Real)(samples - 1);
		Real ipos = 1.0 - pos;
		Real scale = 0.0;
		switch(subsampling_type)
		{
			case SUBSAMPLING_LINEAR:
				scale = ipos*subsample_start + pos*subsample_end;
				break;
			case SUBSAMPLING_HYPERBOLIC:
				scale = 1.0/(samples - i);
				break;
			case SUBSAMPLING_CONSTANT:
			default:
				scale = 1.0; // Weights don't matter for constant overall subsampling.
				break;
		}
		scales[i] = scale;
		sum += scale;
	}

	Real k = 1.0/sum;
	rendering::Task::Handle task;
	for(int i = 0; i < samples; i++)
	{
		if (fabs(scales[i]*k) < 1e-8)
			continue;

		Real pos = (Real)i/(Real)(samples - 1);
		Real ipos = 1.0 - pos;
		context.set_time(get_time_mark() - aperture*ipos);

		rendering::TaskBlend::Handle task_blend(new rendering::TaskBlend());
		task_blend->amount = scales[i]*k;
		task_blend->blend_method = Color::BLEND_ADD_COMPOSITE;
		task_blend->sub_task_a() = task;
		task_blend->sub_task_b() = context.build_rendering_task();
		task = task_blend;
	}

	return task;
}
