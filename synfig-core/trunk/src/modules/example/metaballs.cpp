/* === S Y N F I G ========================================================= */
/*!	\file metaballs.cpp
**	\brief Implements metaballs
**
**	$Id$
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

#include <synfig/string.h>
#include <synfig/time.h>
#include <synfig/context.h>
#include <synfig/paramdesc.h>
#include <synfig/renddesc.h>
#include <synfig/surface.h>
#include <synfig/value.h>
#include <synfig/valuenode.h>
#include <ETL/pen>

#include "metaballs.h"

#endif

/* === U S I N G =========================================================== */

using namespace etl;
using namespace std;
using namespace synfig;

/* === G L O B A L S ======================================================= */

SYNFIG_LAYER_INIT(Metaballs);
SYNFIG_LAYER_SET_NAME(Metaballs,"metaballs");
SYNFIG_LAYER_SET_LOCAL_NAME(Metaballs,N_("Metaballs"));
SYNFIG_LAYER_SET_CATEGORY(Metaballs,N_("Default"));
SYNFIG_LAYER_SET_VERSION(Metaballs,"0.1");
SYNFIG_LAYER_SET_CVS_ID(Metaballs,"$Id$");

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
	if(	param=="centers" && value.same_type_as(centers))
	{
		centers = value;
		return true;
	}

	if(	param=="weights" && value.same_type_as(weights))
	{
		weights = value;
		return true;
	}

	if(	param=="radii" && value.same_type_as(radii))
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

static inline Real densityfunc(const synfig::Point &p, const synfig::Point &c, Real R)
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
