/* === S Y N F I G ========================================================= */
/*!	\file filledrect.cpp
**	\brief Implementation of the "Rectangle" layer
**
**	$Id$
**
**	\legal
**	Copyright (c) 2002-2005 Robert B. Quattlebaum Jr., Adrian Bentley
**	Copyright (c) 2008 Chris Moore
**	Copyright (c) 2011 Carlos López
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

#include <synfig/localization.h>
#include <synfig/general.h>

#include <synfig/string.h>
#include <synfig/time.h>
#include <synfig/context.h>
#include <synfig/paramdesc.h>
#include <synfig/renddesc.h>
#include <synfig/surface.h>
#include <synfig/value.h>
#include <synfig/valuenode.h>
#include <synfig/cairo_renddesc.h>
#include <ETL/pen>

#include "filledrect.h"

#endif

/* === U S I N G =========================================================== */

using namespace etl;
using namespace std;
using namespace synfig;

/* === G L O B A L S ======================================================= */

SYNFIG_LAYER_INIT(FilledRect);
SYNFIG_LAYER_SET_NAME(FilledRect,"filled_rectangle");
SYNFIG_LAYER_SET_LOCAL_NAME(FilledRect,N_("Filled Rectangle"));
SYNFIG_LAYER_SET_CATEGORY(FilledRect,N_("Example"));
SYNFIG_LAYER_SET_VERSION(FilledRect,"0.1");
SYNFIG_LAYER_SET_CVS_ID(FilledRect,"$Id$");

/* === P R O C E D U R E S ================================================= */

/* === M E T H O D S ======================================================= */

/* === E N T R Y P O I N T ================================================= */

FilledRect::FilledRect():
	Layer_Composite(1.0,Color::BLEND_COMPOSITE),
	param_color(ValueBase(Color::black())),
	param_point1(ValueBase(Vector(0,0))),
	param_point2(ValueBase(Vector(1,1))),
	param_feather_x(ValueBase(Real(0))),
	param_feather_y(ValueBase(Real(0))),
	param_bevel(ValueBase(Real(0))),
	param_bevCircle(ValueBase(bool(false)))
{
	SET_INTERPOLATION_DEFAULTS();
	SET_STATIC_DEFAULTS();
}

bool
FilledRect::set_param(const String & param, const ValueBase &value)
{
	IMPORT_VALUE(param_color);
	IMPORT_VALUE(param_point1);
	IMPORT_VALUE(param_point2);
	IMPORT_VALUE_PLUS(param_feather_x,
		{
			Real feather_x=param_feather_x.get(Real());
			if(feather_x<0) feather_x=0;
			param_feather_x.set(feather_x);
		});
	IMPORT_VALUE_PLUS(param_feather_y,
		  {
			  Real feather_y=param_feather_y.get(Real());
			  if(feather_y<0) feather_y=0;
			  param_feather_y.set(feather_y);
		  });
	IMPORT_VALUE(param_bevel);
	IMPORT_VALUE(param_bevCircle);

	return Layer_Composite::set_param(param,value);
}

ValueBase
FilledRect::get_param(const String &param)const
{
	EXPORT_VALUE(param_color);
	EXPORT_VALUE(param_point1);
	EXPORT_VALUE(param_point2);
	EXPORT_VALUE(param_feather_x);
	EXPORT_VALUE(param_feather_y);
	EXPORT_VALUE(param_bevel);
	EXPORT_VALUE(param_bevCircle);

	EXPORT_NAME();
	EXPORT_VERSION();

	return Layer_Composite::get_param(param);
}

Layer::Vocab
FilledRect::get_param_vocab()const
{
	Layer::Vocab ret(Layer_Composite::get_param_vocab());

	ret.push_back(ParamDesc("color")
		.set_local_name(_("Color"))
		.set_description(_("Fill color of the layer"))
	);

	ret.push_back(ParamDesc("point1")
		.set_local_name(_("Point 1"))
		.set_description(_("First corner of the rectangle"))
		.set_box("point2")
	);

	ret.push_back(ParamDesc("point2")
		.set_local_name(_("Point 2"))
		.set_description(_("Second corner of the rectangle"))
	);

	ret.push_back(ParamDesc("feather_x")
		.set_local_name(_("Feather X"))
		.set_is_distance()
	);

	ret.push_back(ParamDesc("feather_y")
		.set_local_name(_("Feather Y"))
		.set_is_distance()
	);

	ret.push_back(ParamDesc("bevel")
		.set_local_name(_("Bevel"))
		.set_description(_("Use Bevel for the corners"))
		.set_is_distance()
	);

	ret.push_back(ParamDesc("bevCircle")
		.set_local_name(_("Keep Bevel Circular"))
		.set_description(_("When checked the bevel is circular"))
	);

	return ret;
}

synfig::Layer::Handle
FilledRect::hit_check(synfig::Context context, const synfig::Point &point)const
{
	Color 	clr;
	Real 	amt;

	if (!get_color(point,clr,amt))
		return context.hit_check(point);

	synfig::Layer::Handle tmp;

	if (get_blend_method()==Color::BLEND_BEHIND && (tmp=context.hit_check(point)))
		return tmp;

	if (Color::is_onto(get_blend_method()) && !(context.hit_check(point)))
		return 0;

	return const_cast<FilledRect*>(this);
}

bool
FilledRect::get_color(const Point &pos, Color &out, Real &outamount) const
{
	Color color=param_color.get(Color());
	Point point1=param_point1.get(Point());
	Point point2=param_point2.get(Point());
	Real feather_x=param_feather_x.get(Real());
	Real feather_y=param_feather_y.get(Real());
	Real bevel=param_bevel.get(Real());
	bool bevCircle=param_bevCircle.get(bool());
	
	Point p[2] = {point1,point2};
	Real swap;

	if(p[0][0] > p[1][0])
	{
		swap = p[0][0];
		p[0][0] = p[1][0];
		p[1][0] = swap;
	}

	if(p[0][1] > p[1][1])
	{
		swap = p[0][1];
		p[0][1] = p[1][1];
		p[1][1] = swap;
	}

	/*
	p[0][0] -= feather_x;
	p[1][0] += feather_x;
	p[0][1] -= feather_y;
	p[1][1] += feather_y;*/
	const Real	w = p[1][0] - p[0][0];
	const Real	h = p[1][1] - p[0][1];

	if(pos[0] >= p[0][0] && pos[0] <= p[1][0] && pos[1] >= p[0][1] && pos[1] <= p[1][1])
	{
		Real value = 1;

		if(feather_x > 0)
		{
			Real xdist = pos[0] - p[0][0];
			xdist = min(xdist,p[1][0]-pos[0]);

			if(xdist < feather_x)
			{
				value = xdist/feather_x;
			}
		}

		if(feather_y > 0)
		{
			Real ydist = pos[1]-p[0][1];
			ydist = min(ydist,p[1][1]-pos[1]);

			if(ydist < feather_y)
			{
				value = min(value,(ydist/feather_y));
			}
		}

		//if we're beveled then check with ellipse code...
		if(bevel > 0)
		{
			const Real bev = (bevel > 1) ? 1 : bevel;
			const Real bevx = bevCircle ? min(w*bev/2,h*bev/2) : w*bev/2;
			const Real bevy = bevCircle ? min(w*bev/2,h*bev/2) : h*bev/2;;

			Vector v(0,0);
			bool	in = false;

			//based on which quarter it is in (and because it's x/y symmetric) get a positive vector (x/y)
			if(pos[0] < p[0][0] + bevx)
			{
				 if(pos[1] < p[0][1] + bevy)
				 {
					 v[0] = p[0][0] + bevx - pos[0];
					 v[1] = p[0][1] + bevy - pos[1];
					 in = true;
				 }else if(pos[1] > p[1][1] - bevy)
				 {
					 v[0] = p[0][0] + bevx - pos[0];
					 v[1] = pos[1] - (p[1][1] - bevy);
					 in = true;
				 }
			}
			else if(pos[0] > p[1][0] - bevx)
			{
				if(pos[1] < p[0][1] + bevy)
				 {
					 v[0] = pos[0] - (p[1][0] - bevx);
					 v[1] = p[0][1] + bevy - pos[1];
					 in = true;
				 }else if(pos[1] > p[1][1] - bevy)
				 {
					 v[0] = pos[0] - (p[1][0] - bevx);
					 v[1] = pos[1] - (p[1][1] - bevy);
					 in = true;
				 }
			}

			//if it's inside a bevelled block
			if(in)
			{
				const Vector scale(bevx,bevy);

				Vector vc(v[0]/scale[0],v[1]/scale[1]);
				Real	d = vc.mag();

				//if it's inside the ellipse
				if(d < 1)
				{
					Real val = atan2(vc[1],vc[0]);
					val /= (PI/2); //< will always be (0,pi/2) because both components are positive

					Real fthx=1,fthy=1;

					//change d into distance away from edge
					d = 1 - d;

					if(feather_x > 0)
					{
						if(scale[0] < feather_x)
						{
							fthy = scale[0]/feather_x;
						}

						if(d*scale[0] < feather_x)
						{
							fthx = d*scale[0]/feather_x;
						}
					}

					if(feather_y > 0)
					{
						if(scale[1] < feather_y)
						{
							fthx = min(fthx,scale[1]/feather_y);
						}

						if(d*scale[1] < feather_y)
						{
							fthy = min(fthy,d*scale[1]/feather_y);
						}
					}

					//interpolate
					outamount = min(value,((1-val)*fthx + val*fthy)) * get_amount();
					out = color;
					return true;

				}else return false;
			}
		}

		outamount = value * get_amount();
		out = color;

		return true;
	}else
		return false;
}

Color
FilledRect::get_color(Context context, const Point &pos)const
{
	Color 	clr;
	Real 	amt;

	if(get_color(pos,clr,amt))
	{
		if(amt==1.0 && get_blend_method()==Color::BLEND_STRAIGHT)
			return clr;
		else
			return Color::blend(clr,context.get_color(pos),amt,get_blend_method());
	}
	else
		return context.get_color(pos);
}

bool
FilledRect::accelerated_render(Context context,Surface *surface,int quality, const RendDesc &renddesc, ProgressCallback *cb)const
{
	RENDER_TRANSFORMED_IF_NEED(__FILE__, __LINE__)

	Point point1=param_point1.get(Point());
	Point point2=param_point2.get(Point());

	// Width and Height of a pixel
	const Point br(renddesc.get_br()), tl(renddesc.get_tl());
	const int w = renddesc.get_w(), h = renddesc.get_h();

	Real	wpp = (br[0]-tl[0])/w;
	Real	hpp = (br[1]-tl[1])/h;
	//const Real	xneg = wpp<0?-1:1;
	//const Real	yneg = hpp<0?-1:1;

	//the bounds of the rectangle
	Point p[2] = {point1,point2};

	if((p[0][0] > p[1][0]) ^ (wpp < 0))
	{
		swap(p[0][0],p[1][0]);
	}

	if((p[0][1] > p[1][1]) ^ (hpp < 0))
	{
		swap(p[0][1],p[1][1]);
	}

	/*p[0][0] -= xneg*feather_x;
	p[1][0] += xneg*feather_x;
	p[0][1] -= yneg*feather_y;
	p[1][1] += yneg*feather_y;*/

	//the integer coordinates
	int y_start = (int)((p[0][1] - tl[1])/hpp +.5); 	//round start up
	int x_start = (int)((p[0][0] - tl[0])/wpp +.5);
	int y_end = (int)((p[1][1] - tl[1])/hpp +.5);	//and ends up
	int x_end =	(int)((p[1][0] - tl[0])/wpp +.5);

	y_start = max(0,y_start);
	x_start = max(0,x_start);
	y_end = min(h,y_end);
	x_end = min(w,x_end);

	SuperCallback supercb(cb,0,9000,10000);

	if(y_start >= h || x_start > w	|| x_end < 0 || y_end < 0)
	{
		if(!context.accelerated_render(surface,quality,renddesc,&supercb))
		{
			if(cb)cb->error(strprintf(__FILE__"%d: Accelerated Renderer Failure",__LINE__));
			return false;
		}

		return true;
	}

	Real xf_start = tl[0] + x_start*wpp;
	Point pos(xf_start,tl[1] + y_start*hpp);

	Color 	clr = Color::black();
	Real	amt;

	if(!context.accelerated_render(surface,quality,renddesc,&supercb))
	{
		if(cb)cb->error(strprintf(__FILE__"%d: Accelerated Renderer Failure",__LINE__));
		return false;
	}

	for(int y = y_start; y < y_end; y++, pos[1] += hpp)
	{
		pos[0] = xf_start;
		for(int x = x_start; x < x_end; x++, pos[0] += wpp)
		{
			if(get_color(pos,clr,amt))
			{
				if(amt==1.0 && get_blend_method()==Color::BLEND_STRAIGHT)
					(*surface)[y][x] = clr;
				else
					(*surface)[y][x] = Color::blend(clr,(*surface)[y][x],amt,get_blend_method());
			}
		}
	}

	return true;

#if	0
	//faster version but much more complex
	//the floating point
	Real y1,y2,y3;
	Real x1,x2,x3;
	Real dx1,dx2; //reversed in 3rd y section, not used in 2nd

	//the transparent
	Real fx = 0, fy = 0;
	Real dfx,dfy;

	//Get the slopes of the lines we need to worry about
	if(feather_x)
	{
		dfx = 1/(fxsize);

		if(fxsize*2 > xfw)
		{
			x1 = xfw/2;
			x2 = 0;
			x3 = xfw;
		}else
		{
			x1 = fxsize;
			x2 = xfw - fxsize;
			x3 = xfw;
		}
	}else
	{
		fx = 1;
		dfx = 0;
		x1=0;
		x2=xfw;
		x3=0;
	}

	if(feather_y)
	{
		dfy = 1/(fysize);

		if(fysize*2 > yfh)
		{
			y1 = yfh/2;
			y2 = 0;
			y3 = yfh;
		}else
		{
			y1 = fysize;
			y2 = yfh - fysize;
			y3 = yfh;
		}

		dx1 = ph*feather_x/feather_y;
		dx2 = -2*dx1;

	}else
	{
		fy = 1;
		dfy = 0;
		y1=0;
		y2=yfh;
		y3=0;
	}

	fy = yf*dfy;

	int x,y;
	Real value = 0;
	SuperCallback supercb(cb,0,9000,10000);
	Surface::pen	p;

	Real tx1 = 0,tx2 =
	for(y = y_start;yf < y1; y++,yf++,fy+=dfy, tx1+=dx1)
	{
		fx = xf*dfx;

		p = surface->get_pen(x_start,y);
		for(; xf < x1; xf++,p.inc_x(), fx+=dfx)
		{
			//we are in the x portion... use fx
			value = fx*get_amount();
			if(value==1.0 && get_blend_method()==Color::BLEND_STRAIGHT)
				p.put_value(color);
			else
				p.put_value(Color::blend(color,p.get_value(),value,get_blend_method()));
		}

		for(;xf < x2; xf++,p.inc_x())
		{
			//we are now in y... use fy
			value = fy*get_amount();
			if(value==1.0 && get_blend_method()==Color::BLEND_STRAIGHT)
				p.put_value(color);
			else
				p.put_value(Color::blend(color,p.get_value(),value,get_blend_method()));
		}

		fx = xfw?(xfw - xf)/xfw:1;
		for(;xf < x3 && ; xf++,p.inc_x(), fx-=dfx)
		{
			//we are in the x portion... use fx
			value = max(0,fx*get_amount());
			if(value==1.0 && get_blend_method()==Color::BLEND_STRAIGHT)
				p.put_value(color);
			else
				p.put_value(Color::blend(color,p.get_value(),value,get_blend_method()));
		}
	}

	x1 =
	for(;fy < 1.0; y++,yf++,fy+=dfy)
	{
		fx = xf*dfx;

		p = surface->get_pen(x_start,y);
		for(; xf < x1; xf++,p.inc_x(), fx+=dfx)
		{
			//we are in the x portion... use fx
			value = fx*get_amount();
			if(value==1.0 && get_blend_method()==Color::BLEND_STRAIGHT)
				p.put_value(color);
			else
				p.put_value(Color::blend(color,p.get_value(),value,get_blend_method()));
		}

		for(;xf < x2; xf++,p.inc_x())
		{
			//we are now in y... use fy
			value = fy*get_amount();
			if(value==1.0 && get_blend_method()==Color::BLEND_STRAIGHT)
				p.put_value(color);
			else
				p.put_value(Color::blend(color,p.get_value(),value,get_blend_method()));
		}

		fx = xfw?(xfw - xf)/xfw:1;
		for(;xf < x3 && ; xf++,p.inc_x(), fx-=dfx)
		{
			//we are in the x portion... use fx
			value = max(0,fx*get_amount());
			if(value==1.0 && get_blend_method()==Color::BLEND_STRAIGHT)
				p.put_value(color);
			else
				p.put_value(Color::blend(color,p.get_value(),value,get_blend_method()));
		}
	}

	for()
	{
		for(int x = x_start; x < x_end; x++)
		{

		}
	}

#endif

	// Mark our progress as finished
	if(cb && !cb->amount_complete(10000,10000))
		return false;

	return true;
}

///////
bool
FilledRect::accelerated_cairorender(Context context, cairo_t *cr,int quality, const RendDesc &renddesc_, ProgressCallback *cb)const
{
	Point point1=param_point1.get(Point());
	Point point2=param_point2.get(Point());

	RendDesc	renddesc(renddesc_);
	
	// Untransform the render desc
	if(!cairo_renddesc_untransform(cr, renddesc))
		return false;

	// Width and Height of a pixel
	const Point br(renddesc.get_br()), tl(renddesc.get_tl());
	const int w = renddesc.get_w(), h = renddesc.get_h();
	
	const double wpp = (br[0]-tl[0])/w;
	const double hpp = (br[1]-tl[1])/h;
	
	//the bounds of the rectangle
	Point p[2] = {point1,point2};
	
	if((p[0][0] > p[1][0]) ^ (wpp < 0))
	{
		swap(p[0][0],p[1][0]);
	}
	
	if((p[0][1] > p[1][1]) ^ (hpp < 0))
	{
		swap(p[0][1],p[1][1]);
	}
	
	//the integer coordinates
	int y_start = (int)((p[0][1] - tl[1])/hpp +.5); 	//round start up
	int x_start = (int)((p[0][0] - tl[0])/wpp +.5);
	int y_end = (int)((p[1][1] - tl[1])/hpp +.5);	//and ends up
	int x_end =	(int)((p[1][0] - tl[0])/wpp +.5);
	
	y_start = max(0,y_start);
	x_start = max(0,x_start);
	y_end = min(h,y_end);
	x_end = min(w,x_end);
	
	SuperCallback supercb(cb,0,9000,10000);
	
	if(y_start >= h || x_start > w	|| x_end < 0 || y_end < 0)
	{
		if(!context.accelerated_cairorender(cr,quality,renddesc,&supercb))
		{
			if(cb)cb->error(strprintf(__FILE__"%d: Accelerated Cairo Renderer Failure",__LINE__));
			return false;
		}
		
		return true;
	}
	
	Real xf_start = tl[0] + x_start*wpp;
	Point pos(xf_start,tl[1] + y_start*hpp);
	
	Color 	clr = Color::black();
	Real	amt;

	cairo_surface_t* surface;
	
	surface=cairo_surface_create_similar(cairo_get_target(cr), CAIRO_CONTENT_COLOR_ALPHA, w, h);
	cairo_t* subcr=cairo_create(surface);
	cairo_scale(subcr, 1/wpp, 1/hpp);
	cairo_translate(subcr, -tl[0], -tl[1]);

	
	if(!context.accelerated_cairorender(subcr,quality,renddesc,&supercb))
	{
		if(cb)cb->error(strprintf(__FILE__"%d: Accelerated Cairo Renderer Failure",__LINE__));
		return false;
	}
	
	cairo_destroy(subcr);

	CairoSurface csurface(surface);
	if(!csurface.map_cairo_image())
	{
		synfig::warning("Filled Rect: map cairo surface failed");
		return false;
	}

	for(int y = y_start; y < y_end; y++, pos[1] += hpp)
	{
		pos[0] = xf_start;
		for(int x = x_start; x < x_end; x++, pos[0] += wpp)
		{
			if(get_color(pos,clr,amt))
			{
				if(amt==1.0 && get_blend_method()==Color::BLEND_STRAIGHT)
					csurface[y][x] = CairoColor(clr).premult_alpha();
				else
					csurface[y][x] = CairoColor::blend(CairoColor(clr),csurface[y][x],amt,get_blend_method()).premult_alpha();
				
			}
		}
	}
	
	csurface.unmap_cairo_image();
	// paint surface on cr
	cairo_save(cr);
	cairo_translate(cr, tl[0], tl[1]);
	cairo_scale(cr, wpp, hpp);
	cairo_set_source_surface(cr, surface, 0, 0);
	cairo_set_operator(cr, CAIRO_OPERATOR_SOURCE);
	cairo_paint(cr);
	cairo_restore(cr);

	cairo_surface_destroy(surface);
	// Mark our progress as finished
	if(cb && !cb->amount_complete(10000,10000))
		return false;
	
	return true;
}
