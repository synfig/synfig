/* === S Y N F I G ========================================================= */
/*!	\file radialblur.cpp
**	\brief Implementation of the "Radial Blur" layer
**
**	$Id$
**
**	\legal
**	Copyright (c) 2002-2005 Robert B. Quattlebaum Jr., Adrian Bentley
**	Copyright (c) 2007, 2008 Chris Moore
**
**	This file is part of Synfig.
**
**	Synfig is free software: you can redistribute it and/or modify
**	it under the terms of the GNU General Public License as published by
**	the Free Software Foundation, either version 2 of the License, or
**	(at your option) any later version.
**
**	Synfig is distributed in the hope that it will be useful,
**	but WITHOUT ANY WARRANTY; without even the implied warranty of
**	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
**	GNU General Public License for more details.
**
**	You should have received a copy of the GNU General Public License
**	along with Synfig.  If not, see <https://www.gnu.org/licenses/>.
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

#include "radialblur.h"
#include <synfig/string.h>
#include <synfig/context.h>
#include <synfig/paramdesc.h>
#include <synfig/renddesc.h>
#include <synfig/surface.h>
#include <synfig/value.h>
#include <synfig/transform.h>
#include <ETL/misc>

#endif

/* === M A C R O S ========================================================= */

/* === G L O B A L S ======================================================= */

SYNFIG_LAYER_INIT(RadialBlur);
SYNFIG_LAYER_SET_NAME(RadialBlur,"radial_blur");
SYNFIG_LAYER_SET_LOCAL_NAME(RadialBlur,N_("Radial Blur"));
SYNFIG_LAYER_SET_CATEGORY(RadialBlur,N_("Blurs"));
SYNFIG_LAYER_SET_VERSION(RadialBlur,"0.1");

/* === P R O C E D U R E S ================================================= */

/* === M E T H O D S ======================================================= */

/* === E N T R Y P O I N T ================================================= */

RadialBlur::RadialBlur():
	Layer_CompositeFork(1.0,Color::BLEND_STRAIGHT),
	param_origin (ValueBase(Vector(0,0))),
	param_size(ValueBase(Real(0.2))),
	param_fade_out(ValueBase(false))
{
	SET_INTERPOLATION_DEFAULTS();
	SET_STATIC_DEFAULTS();
}

RadialBlur::~RadialBlur()
{
}

bool
RadialBlur::set_param(const String & param, const ValueBase &value)
{
	IMPORT_VALUE(param_origin);
	IMPORT_VALUE(param_size);
	IMPORT_VALUE(param_fade_out);

	return Layer_Composite::set_param(param,value);
}

ValueBase
RadialBlur::get_param(const String &param)const
{
	EXPORT_VALUE(param_origin);
	EXPORT_VALUE(param_size);
	EXPORT_VALUE(param_fade_out);

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
		.set_description(_("Origin of the blur"))
		.set_is_distance()
	);

	ret.push_back(ParamDesc("size")
		.set_local_name(_("Size"))
		.set_description(_("Size of the blur"))
		.set_origin("origin")
		.set_is_distance()
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
	RENDER_TRANSFORMED_IF_NEED(__FILE__, __LINE__)

	Vector origin=param_origin.get(Vector());
	Real size=param_size.get(Real());
	bool fade_out=param_fade_out.get(bool());
	
	// don't do anything at quality 10
	if (quality == 10)
		return context.accelerated_render(surface,quality,renddesc,cb);

	if(cb && !cb->amount_complete(0,10000))
		return false;

	Surface tmp_surface;
	const Point tl(renddesc.get_tl()), br(renddesc.get_br());
	const int w(renddesc.get_w()), h(renddesc.get_h());
	const Real pw(renddesc.get_pw()),ph(renddesc.get_ph());

	Rect rect(tl, br);
	Point pos;

	// find how far towards the origin of the blur we are going to
	// wander for each of the 4 corners of our tile, expanding the
	// render description for each of them if necessary
	int x, y;
	for(y=0,pos[1]=tl[1];y<h;y+=(h-1),pos[1]+=ph*(h-1))
		for(x=0,pos[0]=tl[0];x<w;x+=(w-1),pos[0]+=pw*(w-1))
			rect.expand((pos-origin)*(1.0f-size) + origin);

	Vector stl = rect.get_min();
	Vector sbr = rect.get_max();
	if (br[0] < tl[0]) std::swap(stl[0], sbr[0]);
	if (br[1] < tl[1]) std::swap(stl[1], sbr[1]);

	// round out to the nearest pixel
	Point tmp_surface_tl = Point(tl[0] - pw*(int((tl[0]-stl[0])/pw+1-1e-6)),
								 tl[1] - ph*(int((tl[1]-stl[1])/ph+1-1e-6)));
	Point tmp_surface_br = Point(br[0] + pw*(int((sbr[0]-br[0])/pw+2-1e-6)),
								 br[1] + ph*(int((sbr[1]-br[1])/ph+2-1e-6)));

	// round to nearest integer width and height (should be very
	// nearly whole numbers already, but don't want to round 5.99999
	// down to 5)
	int tmp_surface_width = int((tmp_surface_br[0]-tmp_surface_tl[0])/pw + 0.5);
	int tmp_surface_height = int((tmp_surface_br[1]-tmp_surface_tl[1])/ph + 0.5);

	RendDesc desc(renddesc);
	desc.clear_flags();
	desc.set_wh(tmp_surface_width,tmp_surface_height);
	desc.set_tl(tmp_surface_tl);
	desc.set_br(tmp_surface_br);

	// render the layers beneath us
	if(!context.accelerated_render(&tmp_surface,quality,desc,cb))
		return false;

	// copy the part of the layers beneath us that corresponds to this tile
	surface->set_wh(w, h);
	Surface::pen pen(surface->get_pen(0, 0));
	tmp_surface.blit_to(pen,
						int((tl[0] - tmp_surface_tl[0])/pw + 0.5),
						int((tl[1] - tmp_surface_tl[1])/ph + 0.5),
						w, h);

	Surface::alpha_pen apen(surface->begin());

	apen.set_alpha(get_amount());
	apen.set_blend_method(get_blend_method());

/*
	int steps(5);

	if(quality>=9)steps=20;
	else if(quality>=5)steps=30;
	else if(quality>=4)steps=60;
	else if(quality>=3)steps=100;
	else steps=120;
*/

	Surface::value_prep_type cooker;

	// loop through the pixels
	for(y=0,pos[1]=tl[1];y<h;y++,apen.inc_y(),apen.dec_x(x),pos[1]+=ph)
		for(x=0,pos[0]=tl[0];x<w;x++,apen.inc_x(),pos[0]+=pw)
		{
			Point
				begin(pos-tmp_surface_tl),
				end((pos-origin)*(1.0f-size) + origin-tmp_surface_tl);
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
			int w(tmp_surface_width), h(tmp_surface_height);

			dx = std::abs(x1 - x0);
			sx = ((x1 - x0) > 0) ? 1 : -1;
			dy = std::abs(y1 - y0);
			sy = ((y1 - y0) > 0) ? 1 : -1;
			if (dy > dx)
			{
				steep = 0;
				std::swap(x0, y0);
				std::swap(dx, dy);
				std::swap(sx, sy);
				std::swap(w,h);
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
				} else
					printf("%s:%d unexpected %d >= %d or %d >= %d?\n", __FILE__, __LINE__, x0, w, y0, h);

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

// #define DRAW_TILE_OUTLINES
#ifdef DRAW_TILE_OUTLINES
	// draw red lines to show tiles
	{
		int x, y;
		if (w != 0 && h != 0) {
			Surface::alpha_pen apen(surface->begin());
			apen.set_alpha(get_amount());
			apen.set_blend_method(get_blend_method());
			apen.set_value(Color(1, 0, 0, .1));
			for (x = 0; x < w; x++) { apen.put_value(); apen.inc_x(); } apen.dec_x(w);
			for (y = 0; y < h; y++) { apen.put_value(); apen.inc_y(); } apen.dec_y(h);
		}
	}
#endif // DRAW_TILE_OUTLINES

	return true;
}


rendering::Task::Handle
RadialBlur::build_rendering_task_vfunc(Context context) const
	{ return Layer::build_rendering_task_vfunc(context); }
