/*! ========================================================================
** Sinfg
** Template File
** $Id: supersample.cpp,v 1.1.1.1 2005/01/04 01:23:10 darco Exp $
**
** Copyright (c) 2002 Robert B. Quattlebaum Jr.
**
** This software and associated documentation
** are CONFIDENTIAL and PROPRIETARY property of
** the above-mentioned copyright holder.
**
** You may not copy, print, publish, or in any
** other way distribute this software without
** a prior written agreement with
** the copyright holder.
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

#include "supersample.h"
#include <sinfg/string.h>
#include <sinfg/time.h>
#include <sinfg/context.h>
#include <sinfg/paramdesc.h>
#include <sinfg/renddesc.h>
#include <sinfg/surface.h>
#include <sinfg/value.h>
#include <sinfg/valuenode.h>

#include <sinfg/target.h>
#include <sinfg/render.h>

#endif

/* === M A C R O S ========================================================= */

/* === G L O B A L S ======================================================= */

SINFG_LAYER_INIT(SuperSample);
SINFG_LAYER_SET_NAME(SuperSample,"super_sample");
SINFG_LAYER_SET_LOCAL_NAME(SuperSample,_("Super Sample"));
SINFG_LAYER_SET_CATEGORY(SuperSample,_("Other"));
SINFG_LAYER_SET_VERSION(SuperSample,"0.1");
SINFG_LAYER_SET_CVS_ID(SuperSample,"$Id: supersample.cpp,v 1.1.1.1 2005/01/04 01:23:10 darco Exp $");

/* === P R O C E D U R E S ================================================= */

/* === M E T H O D S ======================================================= */

SuperSample::SuperSample():width(2),height(2)
{
	scanline=false;
	alpha_aware=true;
}

bool
SuperSample::set_param(const String & param, const ValueBase &value)
{

	IMPORT(width);
	IMPORT(height);
	IMPORT(scanline);
	IMPORT(alpha_aware);
	
	return false;
}

ValueBase
SuperSample::get_param(const String& param)const
{
	EXPORT(width);
	EXPORT(height);
    EXPORT(scanline);
    EXPORT(alpha_aware);
	
	EXPORT_NAME();
	EXPORT_VERSION();

	return ValueBase();
}

bool
SuperSample::accelerated_render(Context context,Surface *surface,int quality, const RendDesc &renddesc, ProgressCallback *cb)const
{
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
		handle<Target> target=surface_target(&tempsurface);
		if(!target)
		{
			if(cb)cb->error(_("Unable to create SurfaceTarget"));
			return false;
		}
		target->set_rend_desc(&desc);

		if(!render(context-1,target,desc,&subcb))
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
			if(y&31==0 && cb)
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
			if(y&31==0 && cb)
			{
				if(!stagetwo.amount_complete(y,surface->get_h()))
					return false;
			}
		}
	}
	
	if(cb && !cb->amount_complete(10000,10000)) return false;
	
	return true;
}

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
	);
	
	return ret;
}

Rect
SuperSample::get_bounding_rect(Context context)const
{
	return context.get_full_bounding_rect();
}
