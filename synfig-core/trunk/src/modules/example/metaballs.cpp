/* === S I N F G =========================================================== */
/*!	\file metaballs.cpp
**	\brief Implements metaballs
**
**	$Id: metaballs.cpp,v 1.1.1.1 2005/01/04 01:23:09 darco Exp $
**
**	\legal
**	Copyright (c) 2002 Robert B. Quattlebaum Jr.
**
**	This software and associated documentation
**	are CONFIDENTIAL and PROPRIETARY property of
**	the above-mentioned copyright holder.
**
**	You may not copy, print, publish, or in any
**	other way distribute this software without
**	a prior written agreement with
**	the copyright holder.
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

#include <sinfg/string.h>
#include <sinfg/time.h>
#include <sinfg/context.h>
#include <sinfg/paramdesc.h>
#include <sinfg/renddesc.h>
#include <sinfg/surface.h>
#include <sinfg/value.h>
#include <sinfg/valuenode.h>
#include <ETL/pen>

#include "metaballs.h"

#endif

/* === U S I N G =========================================================== */

using namespace etl;
using namespace std;
using namespace sinfg;

/* === G L O B A L S ======================================================= */

SINFG_LAYER_INIT(Metaballs);
SINFG_LAYER_SET_NAME(Metaballs,"metaballs");
SINFG_LAYER_SET_LOCAL_NAME(Metaballs,_("Metaballs"));
SINFG_LAYER_SET_CATEGORY(Metaballs,_("Default"));
SINFG_LAYER_SET_VERSION(Metaballs,"0.1");
SINFG_LAYER_SET_CVS_ID(Metaballs,"$Id: metaballs.cpp,v 1.1.1.1 2005/01/04 01:23:09 darco Exp $");

/* === P R O C E D U R E S ================================================= */

/* === M E T H O D S ======================================================= */

/* === E N T R Y P O I N T ================================================= */

Metaballs::Metaballs():
	Layer_Composite(1.0,Color::BLEND_STRAIGHT),
	color(Color::black())
{
}
	
bool
Metaballs::set_param(const String & param, const ValueBase &value)
{
	if(	param=="centers" && value.same_as(centers))
	{
		centers = value;
		return true;
	}
	
	if(	param=="weights" && value.same_as(weights))
	{
		weights = value;
		return true;
	}
	
	if(	param=="radii" && value.same_as(radii))
	{
		radii = value;
		return true;
	}
	
	IMPORT(color);
	IMPORT(threshold);
	
	return Layer_Composite::set_param(param,value);
}

ValueBase
Metaballs::get_param(const String &param)const
{
	EXPORT(color);
	
	EXPORT(radii);
	EXPORT(weights);	
	EXPORT(centers);
	EXPORT(threshold);

	EXPORT_NAME();
	EXPORT_VERSION();
		
	return Layer_Composite::get_param(param);	
}

Layer::Vocab
Metaballs::get_param_vocab()const
{
	Layer::Vocab ret(Layer_Composite::get_param_vocab());
	
	ret.push_back(ParamDesc("color")
		.set_local_name(_("Color"))
	);

	ret.push_back(ParamDesc("centers")
		.set_local_name(_("Points"))
	);
	
	ret.push_back(ParamDesc("radii")
		.set_local_name(_("Radii"))
	);
	
	ret.push_back(ParamDesc("weights")
		.set_local_name(_("Weights"))
	);
	
	ret.push_back(ParamDesc("threshold")
		.set_local_name(_("Threshold"))
	);
	
	return ret;
}

static inline Real densityfunc(const sinfg::Point &p, const sinfg::Point &c, Real R)
{
	const Real dx = p[0] - c[0];
	const Real dy = p[1] - c[1];
	
	const Real n = (1 - (dx*dx + dy*dy)/(R*R));
	return (n*n*n);

	/*
	f(d) = (1 - d^2)^3
	f'(d) = -6d * (1 - d^2)^2
	
	could use this too...
	f(d) = (1 - d^2)^2
	f'(d) = -6d * (1 - d^2)
	*/
}

Real
Metaballs::totaldensity(const Point &pos) const
{
	Real density = 0;
	
	//sum up weighted functions
	for(unsigned int i=0;i<centers.size();i++)
	{
		density += weights[i] * densityfunc(pos,centers[i], radii[i]);
	}
	
	return density;
}

Color
Metaballs::get_color(Context context, const Point &pos)const
{
	Real dens = totaldensity(pos);
	
	if(dens >= threshold) 
		return color;
	else
		return context.get_color(pos);
}

bool
Metaballs::accelerated_render(Context context,Surface *surface,int quality, const RendDesc &renddesc, ProgressCallback *cb)const
{	
	// Width and Height of a pixel
	const Point br(renddesc.get_br()), 
				tl(renddesc.get_tl());
	
	const int 	w = renddesc.get_w(), 
				h = renddesc.get_h();
	
	Real	pw = renddesc.get_pw();
	Real	ph = renddesc.get_ph();
		
	SuperCallback supercb(cb,0,9000,10000);
	
	if(!context.accelerated_render(surface,quality,renddesc,&supercb))
	{
		if(cb)cb->error(strprintf(__FILE__"%d: Accelerated Renderer Failure",__LINE__));
		return false;
	}
		
	Point pos(tl[0],tl[1]);
	
	Real	dens;
	
	if(!context.accelerated_render(surface,quality,renddesc,&supercb))
	{
		if(cb)cb->error(strprintf(__FILE__"%d: Accelerated Renderer Failure",__LINE__));
		return false;
	}
	
	for(int y = 0; y < h; y++, pos[1] += ph)
	{
		pos[0] = tl[0];
		for(int x = 0; x < w; x++, pos[0] += pw)
		{
			dens = totaldensity(pos);
			
			if(dens >= threshold)
			{
				(*surface)[y][x] = Color::blend(color,(*surface)[y][x],get_amount(),get_blend_method());
			}
		}		
	}

	// Mark our progress as finished
	if(cb && !cb->amount_complete(10000,10000))
		return false;

	return true;
}
