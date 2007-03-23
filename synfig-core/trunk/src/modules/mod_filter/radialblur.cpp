/* === S Y N F I G ========================================================= */
/*!	\file radialblur.cpp
**	\brief Template Header
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

#include "radialblur.h"
#include <synfig/string.h>
#include <synfig/time.h>
#include <synfig/context.h>
#include <synfig/paramdesc.h>
#include <synfig/renddesc.h>
#include <synfig/surface.h>
#include <synfig/value.h>
#include <synfig/valuenode.h>
#include <synfig/transform.h>
#include <ETL/misc>

#endif

/* === M A C R O S ========================================================= */

/* === G L O B A L S ======================================================= */

SYNFIG_LAYER_INIT(RadialBlur);
SYNFIG_LAYER_SET_NAME(RadialBlur,"radial_blur");
SYNFIG_LAYER_SET_LOCAL_NAME(RadialBlur,_("Radial Blur"));
SYNFIG_LAYER_SET_CATEGORY(RadialBlur,_("Blurs"));
SYNFIG_LAYER_SET_VERSION(RadialBlur,"0.1");
SYNFIG_LAYER_SET_CVS_ID(RadialBlur,"$Id$");

/* === P R O C E D U R E S ================================================= */

/* === M E T H O D S ======================================================= */

/* === E N T R Y P O I N T ================================================= */

RadialBlur::RadialBlur():
	origin	(0,0),
	size	(0.2),
	fade_out(false)
{
}

RadialBlur::~RadialBlur()
{
}

bool
RadialBlur::set_param(const String & param, const ValueBase &value)
{
	IMPORT(origin);
	IMPORT(size);
	IMPORT(fade_out);

	return Layer_Composite::set_param(param,value);
}

ValueBase
RadialBlur::get_param(const String &param)const
{
	EXPORT(origin);
	EXPORT(size);
	EXPORT(fade_out);

	EXPORT_NAME();
	EXPORT_VERSION();

	return Layer_Composite::get_param(param);
}

Layer::Vocab
RadialBlur::get_param_vocab()const
{
	Layer::Vocab ret(Layer_Composite::get_param_vocab());

	ret.push_back(ParamDesc("origin")
		.set_local_name(_("Origin"))
		.set_description(_("Point where you want the origin to be"))
	);

	ret.push_back(ParamDesc("size")
		.set_local_name(_("Size"))
		.set_description(_("Size of blur"))
		.set_origin("origin")
	);

	ret.push_back(ParamDesc("fade_out")
		.set_local_name(_("Fade Out"))
	);

	return ret;
}

Color
RadialBlur::get_color(Context context, const Point &p)const
{
	//! \writeme
	return context.get_color(p);
}

bool
RadialBlur::accelerated_render(Context context,Surface *surface,int quality, const RendDesc &renddesc, ProgressCallback *cb)const
{
	if(cb && !cb->amount_complete(0,10000))
		return false;

	Surface tmp_surface;

	if(!context.accelerated_render(surface,quality,renddesc,cb))
		return false;

	tmp_surface=*surface;

	int x,y;

	const Point tl(renddesc.get_tl());
	Point pos;
	const int w(surface->get_w());
	const int h(surface->get_h());
	const Real pw(renddesc.get_pw()),ph(renddesc.get_ph());

	Surface::alpha_pen apen(surface->begin());

	apen.set_alpha(get_amount());
	apen.set_blend_method(get_blend_method());

	int steps(5);

	if(quality>=9)steps=20;
	else if(quality>=5)steps=30;
	else if(quality>=4)steps=60;
	else if(quality>=3)steps=100;
	else steps=120;

	Surface::value_prep_type cooker;

	for(y=0,pos[1]=tl[1];y<h;y++,apen.inc_y(),apen.dec_x(x),pos[1]+=ph)
		for(x=0,pos[0]=tl[0];x<w;x++,apen.inc_x(),pos[0]+=pw)
		{
			Point
				begin(pos-tl),
				end((pos-origin)*(1.0f-size)+origin-tl);
			begin[0]/=pw;begin[1]/=ph;
			end[0]/=pw;end[1]/=ph;

			Color pool(Color::alpha());
			int poolsize(0);

			int x0(round_to_int(begin[0])),
				y0(round_to_int(begin[1])),
				x1(round_to_int(end[0])),
				y1(round_to_int(end[1]));

			int i;
			int steep = 1;
			int sx, sy;  /* step positive or negative (1 or -1) */
			int dx, dy;  /* delta (difference in X and Y between points) */
			int e;
			int w(tmp_surface.get_w()),h(tmp_surface.get_h());

			dx = abs(x1 - x0);
			sx = ((x1 - x0) > 0) ? 1 : -1;
			dy = abs(y1 - y0);
			sy = ((y1 - y0) > 0) ? 1 : -1;
			if (dy > dx)
			{
				steep = 0;
				swap(x0, y0);
				swap(dx, dy);
				swap(sx, sy);
				swap(w,h);
			}
			e = (dy << 1) - dx;
			for (i = 0; i < dx; i++)
			{
				if(y0>=0 && x0>=0 && y0<h && x0<w)
				{
					if(fade_out)
					{
						if (steep)
							pool+=cooker.cook(tmp_surface[y0][x0])*(i-dx);
						else
							pool+=cooker.cook(tmp_surface[x0][y0])*(i-dx);
						poolsize+=(i-dx);
					}
					else
					{
						if (steep)
							pool+=cooker.cook(tmp_surface[y0][x0]);
						else
							pool+=cooker.cook(tmp_surface[x0][y0]);
						poolsize+=1;
					}
				}

				while (e >= 0)
				{
					y0 += sy;
					e -= (dx << 1);
				}
				x0 += sx;
				e += (dy << 1);
			}
			if(poolsize)
			{
				pool/=poolsize;
				apen.put_value(cooker.uncook(pool));
			}
/*
			Point begin,end;
			begin=pos;
			end=(pos-origin)*(1.0f-size)+origin;

			Color pool(Color::alpha());
			float f,poolsize(0);
			int i;
			int steps(steps*size);
			for(f=0,i=0;i<steps;i++,f+=1.0f/(steps-1))
			{
				Point loc((end-begin)*f+begin-tl);
				loc[0]/=pw;loc[1]/=ph;

				if(fade_out)
					pool+=tmp_surface.linear_sample(loc[0],loc[1])*(i-steps),poolsize+=(i-steps);
				else
					pool+=tmp_surface.linear_sample(loc[0],loc[1]),poolsize+=1;
			}
			pool/=poolsize;
			apen.put_value(pool);
*/
		}


	if(cb && !cb->amount_complete(10000,10000)) return false;

	return true;
}
