/* === S Y N F I G ========================================================= */
/*!	\file supersample.cpp
**	\brief Implementation of the "Super Sample" layer
**
**	$Id$
**
**	\legal
**	Copyright (c) 2002-2005 Robert B. Quattlebaum Jr., Adrian Bentley
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

#include "supersample.h"
#include <synfig/string.h>
#include <synfig/time.h>
#include <synfig/context.h>
#include <synfig/paramdesc.h>
#include <synfig/renddesc.h>
#include <synfig/surface.h>
#include <synfig/value.h>
#include <synfig/valuenode.h>
#include <synfig/cairo_renddesc.h>

#include <synfig/target.h>
#include <synfig/target_scanline.h>
#include <synfig/render.h>

#endif

/* === M A C R O S ========================================================= */

/* === G L O B A L S ======================================================= */

SYNFIG_LAYER_INIT(SuperSample);
SYNFIG_LAYER_SET_NAME(SuperSample,"super_sample");
SYNFIG_LAYER_SET_LOCAL_NAME(SuperSample,N_("Super Sample"));
SYNFIG_LAYER_SET_CATEGORY(SuperSample,N_("Other"));
SYNFIG_LAYER_SET_VERSION(SuperSample,"0.1");
SYNFIG_LAYER_SET_CVS_ID(SuperSample,"$Id$");

/* === P R O C E D U R E S ================================================= */

/* === M E T H O D S ======================================================= */

SuperSample::SuperSample():
param_width(ValueBase(int(2))),
param_height(ValueBase(int(2)))
{
	param_scanline=ValueBase(false);
	param_alpha_aware=ValueBase(true);
	SET_INTERPOLATION_DEFAULTS();
	SET_STATIC_DEFAULTS();

}

bool
SuperSample::set_param(const String & param, const ValueBase &value)
{
	IMPORT_VALUE_PLUS(param_width,
		{
			int width=param_width.get(int());
			if(value.get(int()) < 1) width = 1;
			else width=value.get(int());
			param_width.set(width);
			return true;
		}
		);
	IMPORT_VALUE_PLUS(param_height,
		{
			int height=param_height.get(int());
			if(value.get(int()) < 1) height = 1;
			else height=value.get(int());
			param_height.set(height);
			return true;
		}
		);
	IMPORT_VALUE(param_scanline);
	IMPORT_VALUE(param_alpha_aware);

	return false;
}

ValueBase
SuperSample::get_param(const String& param)const
{
	EXPORT_VALUE(param_width);
	EXPORT_VALUE(param_height);
    EXPORT_VALUE(param_scanline);
    EXPORT_VALUE(param_alpha_aware);

	EXPORT_NAME();
	EXPORT_VERSION();

	return ValueBase();
}

bool
SuperSample::accelerated_render(Context context,Surface *surface,int quality, const RendDesc &renddesc, ProgressCallback *cb)const
{
	RENDER_TRANSFORMED_IF_NEED(__FILE__, __LINE__)

	int width=param_width.get(int());
	int height=param_height.get(int());
	bool scanline=param_scanline.get(bool());
	bool alpha_aware=param_alpha_aware.get(bool());
	
	// don't bother supersampling if our quality is too low.
	if(quality>=10)
		return context.accelerated_render(surface,quality,renddesc,cb);

	RendDesc desc(renddesc);

	SuperCallback subcb(cb,1,9000,10000);
	SuperCallback stagetwo(cb,9000,10000,10000);

	desc.clear_flags();
	desc.set_wh(desc.get_w()*width,desc.get_h()*height);

	Surface tempsurface;

	// Render the scene
	if(scanline)
	{
		handle<Target_Scanline> target=surface_target_scanline(&tempsurface);
		if(!target)
		{
			if(cb)cb->error(_("Unable to create SurfaceTarget"));
			return false;
		}
		target->set_rend_desc(&desc);

		if(!render(context.get_previous(),target,desc,&subcb))
		{
			if(cb)cb->error(strprintf(__FILE__"%d: Scanline Renderer Failure",__LINE__));
			return false;
		}
	}
	else
		if(!context.accelerated_render(&tempsurface,quality,desc,cb))
		{
			//if(cb)cb->error(strprintf(__FILE__"%d: Accelerated Renderer Failure",__LINE__));
			return false;
		}

	surface->set_wh(renddesc.get_w(),renddesc.get_h());

	Surface::pen pen(surface->begin());
	Surface::pen temp_pen(tempsurface.begin());

	if(cb && !cb->amount_complete(9001,10000)) return false;

	if(alpha_aware)
	{
		int x,y,u,v;
		float sum;
		Color pool;
		for(y=0;y<surface->get_h();y++,pen.inc_y(),pen.dec_x(x),temp_pen.inc_y(height),temp_pen.dec_x(x*width))
		{
			for(x=0;x<surface->get_w();x++,pen.inc_x(),temp_pen.inc_x(width))
			{
				pool=Color(0,0,0,0);
				sum=0;

				for(v=0;v<height;v++,temp_pen.inc_y(),temp_pen.dec_x(u))
					for(u=0;u<width;u++,temp_pen.inc_x())
					{
						pool+=temp_pen.get_value()*temp_pen.get_value().get_a();
						sum+=temp_pen.get_value().get_a();
					}
				temp_pen.dec_y(v);

				if(sum)
				{
					pool/=sum;
					pool.set_a(sum/float(width*height));
					pen.put_value(pool);
				}
				else
					pen.put_value(Color::alpha());
			}
			if((y&31)==0 && cb)
			{
				if(!stagetwo.amount_complete(y,surface->get_h()))
					return false;
			}
		}
	}
	else
	{
		int x,y,u,v;
		Color pool;
		float multiplier=1.0f/float(width*height);
		for(y=0;y<surface->get_h();y++,pen.inc_y(),pen.dec_x(x),temp_pen.inc_y(height),temp_pen.dec_x(x*width))
		{
			for(x=0;x<surface->get_w();x++,pen.inc_x(),temp_pen.inc_x(width))
			{
				pool=Color(0,0,0,0);
				for(v=0;v<height;v++,temp_pen.inc_y(),temp_pen.dec_x(u))
					for(u=0;u<width;u++,temp_pen.inc_x())
						pool+=temp_pen.get_value();
				temp_pen.dec_y(v);
				pen.put_value(pool*multiplier);
			}
			if((y&31)==0 && cb)
			{
				if(!stagetwo.amount_complete(y,surface->get_h()))
					return false;
			}
		}
	}

	if(cb && !cb->amount_complete(10000,10000)) return false;

	return true;
}


////
bool
SuperSample::accelerated_cairorender(Context context, cairo_t *cr, int quality, const RendDesc &renddesc, ProgressCallback *cb)const
{
	int width=param_width.get(int());
	int height=param_height.get(int());

	// don't bother supersampling if our quality is too low.
	if(quality>=10 || (width==1 && height==1))
		return context.accelerated_cairorender(cr,quality,renddesc,cb);
	
	RendDesc desc(renddesc);
	// Untransform the render desc
	if(!cairo_renddesc_untransform(cr, desc))
		return false;

	//grab values before expand
	const double pw=desc.get_pw();
	const double ph=desc.get_ph();
	const double tlx=desc.get_tl()[0];
	const double tly=desc.get_tl()[1];
	
	// Expand the renddesc
	desc.clear_flags();
	desc.set_wh(desc.get_w()*width,desc.get_h()*height);
	
	// New expanded desc values
	const int ww=desc.get_w();
	const int wh=desc.get_h();
	const double wtlx=desc.get_tl()[0];
	const double wtly=desc.get_tl()[1];
	const double wpw=desc.get_pw();
	const double wph=desc.get_ph();
	
	cairo_surface_t* tempsurface=cairo_surface_create_similar(cairo_get_target(cr), CAIRO_CONTENT_COLOR_ALPHA, ww, wh);
	cairo_t* tempcr=cairo_create(tempsurface);
	cairo_scale(tempcr, 1/wpw, 1/wph);
	cairo_translate(tempcr, -wtlx, -wtly);
	// Render the scene
	if(!context.accelerated_cairorender(tempcr,quality,desc,cb))
	{
		if(cb)cb->error(strprintf(__FILE__"%d: Accelerated Renderer Failure",__LINE__));
		return false;
	}
	cairo_destroy(tempcr);
	// Calculate the scales values
	float scalex=1.0/width;
	float scaley=1.0/height;
	// Calculate the cairo filter based on quality
	cairo_filter_t filter;
	cairo_antialias_t anti;
	switch(quality)
	{
		case 1:
		case 2:
		case 3:
		case 4:
		case 5:	// Best
			filter=CAIRO_FILTER_BEST;
			anti=CAIRO_ANTIALIAS_BEST;
			break;
		case 6:
		case 7:
		case 8:	// Good
			filter=CAIRO_FILTER_GOOD;
			anti=CAIRO_ANTIALIAS_GOOD;
			break;
		case 9:	// Fast
		default:
			filter=CAIRO_FILTER_FAST;
			anti=CAIRO_ANTIALIAS_FAST;
			break;
	}
	cairo_save(cr);

	cairo_translate(cr, tlx, tly);
	cairo_scale(cr, pw, ph);

	cairo_scale(cr, scalex, scaley);
	cairo_set_source_surface(cr, tempsurface, 0,0);
	cairo_set_operator(cr, CAIRO_OPERATOR_SOURCE);
	cairo_pattern_set_filter(cairo_get_source(cr), filter);
	cairo_set_antialias(cr, anti);
	cairo_paint(cr);
	
	cairo_restore(cr);
	
	cairo_surface_destroy(tempsurface);
	
	return true;
}

////

Layer::Vocab
SuperSample::get_param_vocab(void)const
{
	Layer::Vocab ret;

	ret.push_back(ParamDesc("width")
		.set_local_name(_("Width"))
		.set_description(_("Width of sample area (In pixels)"))
	);
	ret.push_back(ParamDesc("height")
		.set_local_name(_("Height"))
		.set_description(_("Height of sample area (In pixels)"))
	);
	ret.push_back(ParamDesc("scanline")
		.set_local_name(_("Use Parametric"))
		.set_description(_("Use the Parametric Renderer"))
	);
	ret.push_back(ParamDesc("alpha_aware")
		.set_local_name(_("Be Alpha Safe"))
		.set_description(_("Avoid alpha artifacts when checked"))
	);

	return ret;
}

Rect
SuperSample::get_bounding_rect(Context context)const
{
	return context.get_full_bounding_rect();
}
