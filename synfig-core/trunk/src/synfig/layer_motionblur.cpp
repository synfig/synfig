/* === S Y N F I G ========================================================= */
/*!	\file layer_motionblur.h
**	\brief Template Header
**
**	$Id: layer_motionblur.cpp,v 1.1.1.1 2005/01/04 01:23:14 darco Exp $
**
**	\legal
**	Copyright (c) 2002-2005 Robert B. Quattlebaum Jr., Adrian Bentley
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
SYNFIG_LAYER_SET_NAME(Layer_MotionBlur,"MotionBlur");
SYNFIG_LAYER_SET_LOCAL_NAME(Layer_MotionBlur,_("Motion Blur"));
SYNFIG_LAYER_SET_CATEGORY(Layer_MotionBlur,_("Blurs"));
SYNFIG_LAYER_SET_VERSION(Layer_MotionBlur,"0.1");
SYNFIG_LAYER_SET_CVS_ID(Layer_MotionBlur,"$Id: layer_motionblur.cpp,v 1.1.1.1 2005/01/04 01:23:14 darco Exp $");

/* === M E M B E R S ======================================================= */

Layer_MotionBlur::Layer_MotionBlur():
	Layer_Composite	(1.0,Color::BLEND_STRAIGHT),
	aperture		(0)
{
}
	
bool
Layer_MotionBlur::set_param(const String &param, const ValueBase &value)
{

	IMPORT(aperture);		
	return Layer_Composite::set_param(param,value);
}

ValueBase
Layer_MotionBlur::get_param(const String &param)const
{
 	EXPORT(aperture);
	
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
	
	return ret;
}

bool
Layer_MotionBlur::accelerated_render(Context context,Surface *surface,int quality, const RendDesc &renddesc, ProgressCallback *cb)const
{
	if(aperture && quality<10)
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
				samples=3;
				break;
			case 10: // Rough Quality
            default:			
				samples=1;
				break;
				
		}
	
		Surface tmp;
		int i;

		surface->set_wh(renddesc.get_w(),renddesc.get_h());
		surface->clear();
		
		for(i=0;i<samples;i++)
		{
			subimagecb=SuperCallback(cb,i*(5000/samples),(i+1)*(5000/samples),5000);
			context.set_time(time_cur+(aperture/samples)*i-aperture*0.5);
			if(!context.accelerated_render(&tmp,quality,renddesc,&subimagecb))
				return false;
			for(int y=0;y<renddesc.get_h();y++)
				for(int x=0;x<renddesc.get_w();x++)
					(*surface)[y][x]+=tmp[y][x].premult_alpha();
		}
		for(int y=0;y<renddesc.get_h();y++)
			for(int x=0;x<renddesc.get_w();x++)
				(*surface)[y][x]=((*surface)[y][x]/(float)samples).demult_alpha();
	}
	else
		return context.accelerated_render(surface,quality,renddesc,cb);
	
	return true;
}
