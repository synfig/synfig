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

#include "string.h"
#include "layer_motionblur.h"
#include "time.h"
#include "context.h"
#include "paramdesc.h"
#include "renddesc.h"
#include "surface.h"
#include "value.h"
#include "valuenode.h"
#include "canvas.h"

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
	Layer_Composite		(1.0,Color::BLEND_STRAIGHT),
	aperture			(0),
	subsamples_factor	(1.0),
	subsampling_type	(SUBSAMPLING_HYPERBOLIC),
	subsample_start		(0.0),
	subsample_end		(1.0)
{
	Layer::Vocab voc(get_param_vocab());
	Layer::fill_static(voc);
}

bool
Layer_MotionBlur::set_param(const String &param, const ValueBase &value)
{

	IMPORT(aperture);
	IMPORT(subsamples_factor);
	IMPORT(subsampling_type);
	IMPORT(subsample_start);
	IMPORT(subsample_end);
	return Layer_Composite::set_param(param,value);
}

ValueBase
Layer_MotionBlur::get_param(const String &param)const
{
	EXPORT(aperture);
	EXPORT(subsamples_factor);
	EXPORT(subsampling_type);
	EXPORT(subsample_start);
	EXPORT(subsample_end);

	EXPORT_NAME();
	EXPORT_VERSION();

	return Layer_Composite::get_param(param);
}

void
Layer_MotionBlur::set_time(Context context, Time time)const
{
	context.set_time(time);
	time_cur=time;
}

void
Layer_MotionBlur::set_time(Context context, Time time, const Point &pos)const
{
	context.set_time(time,pos);
	time_cur=time;
}

Color
Layer_MotionBlur::get_color(Context context, const Point &pos)const
{
/*	if(aperture)
	{
		Time time(time_cur);
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
			context.set_time(time_cur-aperture*ipos);
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
Layer_MotionBlur::accelerated_cairorender(Context context, cairo_surface_t *surface,int quality, const RendDesc &renddesc, ProgressCallback *cb)const
{
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
		
		if (samples <= 1) return context.accelerated_cairorender(surface,quality,renddesc,cb);
		
		// Only in modes where subsample_start/end matters...
		if(subsampling_type == SUBSAMPLING_LINEAR)
		{
			// We won't render when the scale==0, so we'll use those samples elsewhere
			if(subsample_start == 0) samples++;
			if(subsample_end == 0) samples++;
		}
		// We need to clear the given surface since it maybe not clean.
		cairo_t* cr=cairo_create(surface);
		cairo_set_operator(cr, CAIRO_OPERATOR_CLEAR);
		cairo_paint(cr);
		cairo_destroy(cr);
		
		cairo_surface_t* tmp;
		int i;
		float scale, divisor = 0;
		tmp=cairo_image_surface_create(CAIRO_FORMAT_ARGB32, renddesc.get_w(),renddesc.get_h());
		
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
			context.set_time(time_cur-aperture*ipos);
			cairo_t* cr=cairo_create(tmp);
			cairo_set_operator(cr, CAIRO_OPERATOR_CLEAR);
			cairo_paint(cr);
			cairo_destroy(cr);
			if(!context.accelerated_cairorender(tmp,quality,renddesc,&subimagecb))
				return false;
			CairoSurface csurface(surface);
			if(!csurface.map_cairo_image())
			{
				synfig::info("MotionBLur: map cairo image failed");
				return false;
			}
			CairoSurface ctmp(tmp);
			if(!ctmp.map_cairo_image())
			{
				synfig::info("MotionBLur: map cairo image failed");
				return false;
			}
			float s=scale/divisor;
			for(int y=0;y<renddesc.get_h();y++)
				for(int x=0;x<renddesc.get_w();x++)
					csurface[y][x]+=ctmp[y][x]*s;

			ctmp.unmap_cairo_image();
			csurface.unmap_cairo_image();
		}
	}
	else
		return context.accelerated_cairorender(surface,quality,renddesc,cb);
	
	return true;
}
