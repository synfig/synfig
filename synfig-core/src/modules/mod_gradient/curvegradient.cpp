/* === S Y N F I G ========================================================= */
/*!	\file curvegradient.cpp
**	\brief Implementation of the "Curve Gradient" layer
**
**	$Id$
**
**	\legal
**	Copyright (c) 2002-2005 Robert B. Quattlebaum Jr., Adrian Bentley
**	Copyright (c) 2007-2008 Chris Moore
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

#include "curvegradient.h"

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
#include <ETL/bezier>
#include <ETL/hermite>
#include <ETL/calculus>

#endif

/* === M A C R O S ========================================================= */

#define FAKE_TANGENT_STEP 0.000001

/* === G L O B A L S ======================================================= */

SYNFIG_LAYER_INIT(CurveGradient);
SYNFIG_LAYER_SET_NAME(CurveGradient,"curve_gradient");
SYNFIG_LAYER_SET_LOCAL_NAME(CurveGradient,N_("Curve Gradient"));
SYNFIG_LAYER_SET_CATEGORY(CurveGradient,N_("Gradients"));
SYNFIG_LAYER_SET_VERSION(CurveGradient,"0.0");
SYNFIG_LAYER_SET_CVS_ID(CurveGradient,"$Id$");

/* === P R O C E D U R E S ================================================= */

inline Real calculate_distance(const synfig::BLinePoint& a,const synfig::BLinePoint& b)
{
	const Point& c1(a.get_vertex());
	const Point c2(a.get_vertex()+a.get_tangent2()/3);
	const Point c3(b.get_vertex()-b.get_tangent1()/3);
	const Point& c4(b.get_vertex());
	return (c1-c2).mag()+(c2-c3).mag()+(c3-c4).mag();
}

inline Real calculate_distance(const std::vector<synfig::BLinePoint>& bline, bool bline_loop)
{
	std::vector<synfig::BLinePoint>::const_iterator iter,next/*,ret*/;
	std::vector<synfig::BLinePoint>::const_iterator end(bline.end());

	Real dist(0);

	if (bline.empty()) return dist;

	next=bline.begin();

	if(bline_loop)
		iter=--bline.end();
	else
		iter=next++;

	for(;next!=end;iter=next++)
	{
		// Setup the curve
		etl::hermite<Vector> curve(
			iter->get_vertex(),
			next->get_vertex(),
			iter->get_tangent2(),
			next->get_tangent1());

//		dist+=calculate_distance(*iter,*next);
		dist+=curve.length();
	}

	return dist;
}

std::vector<synfig::BLinePoint>::const_iterator
find_closest(bool fast, const std::vector<synfig::BLinePoint>& bline,const Point& p, Real& t, bool loop=false, Real *bline_dist_ret=0)
{
	std::vector<synfig::BLinePoint>::const_iterator iter,next,ret;
	std::vector<synfig::BLinePoint>::const_iterator end(bline.end());

	ret=bline.end();
	Real dist(100000000000.0);

	next=bline.begin();

	Real best_bline_dist(0);
	//Real best_bline_len(0);
	Real total_bline_dist(0);
	Real best_pos(0);
	etl::hermite<Vector> best_curve;

	if(loop)
		iter=--bline.end();
	else
		iter=next++;

	Point bp;

	for(;next!=end;iter=next++)
	{
		// Setup the curve
		etl::hermite<Vector> curve(
			iter->get_vertex(),
			next->get_vertex(),
			iter->get_tangent2(),
			next->get_tangent1());

		/*
		const float t(curve.find_closest(p,6,0.01,0.99));
		bp=curve(t);if((bp-p).mag_squared()<dist) { ret=iter; dist=(bp-p).mag_squared(); ret_t=t; }
		*/

		Real thisdist(0);
		Real len(0);
		if(bline_dist_ret)
		{
			//len=calculate_distance(*iter,*next);
			len=curve.length();
		}

		if (fast)
		{
#define POINT_CHECK(x) bp=curve(x);	thisdist=(bp-p).mag_squared(); if(thisdist<dist) { ret=iter; dist=thisdist; best_bline_dist=total_bline_dist; /* best_bline_len=len; */ best_curve=curve; }
			POINT_CHECK(0.0001);
			POINT_CHECK((1.0/6.0));
			POINT_CHECK((2.0/6.0));
			POINT_CHECK((3.0/6.0));
			POINT_CHECK((4.0/6.0));
			POINT_CHECK((5.0/6.0));
			POINT_CHECK(0.9999);
		}
		else
		{
			Real pos = curve.find_closest(fast, p);
			thisdist=(curve(pos)-p).mag_squared();
			if(thisdist<dist)
			{
				ret=iter;
				dist=thisdist;
				best_bline_dist=total_bline_dist;
				//best_bline_len=len;
				best_curve=curve;
				best_pos = pos;
			}
		}

		total_bline_dist+=len;
	}

	t = best_pos;

	if(bline_dist_ret)
	{
		//! \todo is this a redundant call to find_closest()?
		// note bline_dist_ret is null except when 'perpendicular' is true
		*bline_dist_ret=best_bline_dist+best_curve.find_distance(0,best_curve.find_closest(fast, p));
//		*bline_dist_ret=best_bline_dist+best_curve.find_closest(fast, p)*best_bline_len;
	}

	return ret;
}

/* === M E T H O D S ======================================================= */

inline void
CurveGradient::sync()
{
	std::vector<synfig::BLinePoint> bline(param_bline.get_list_of(BLinePoint()));
	curve_length_=calculate_distance(bline, bline_loop);
}

void
CurveGradient::compile()
{
	compiled_gradient.set(
		param_gradient.get(Gradient()),
		param_loop.get(bool()),
		param_zigzag.get(bool()) );
}


CurveGradient::CurveGradient():
	Layer_Composite(1.0,Color::BLEND_COMPOSITE),
	param_origin(ValueBase(Point(0,0))),
	param_width(ValueBase(Real(0.25))),
	param_bline(ValueBase(std::vector<synfig::BLinePoint>())),
	param_gradient(Gradient(Color::black(), Color::white())),
	param_loop(ValueBase(false)),
	param_zigzag(ValueBase(false)),
	param_perpendicular(ValueBase(false)),
	param_fast(ValueBase(true))
{
	std::vector<synfig::BLinePoint> bline;
	bline.push_back(BLinePoint());
	bline.push_back(BLinePoint());
	bline.push_back(BLinePoint());
	bline[0].set_vertex(Point(0,1));
	bline[1].set_vertex(Point(0,-1));
	bline[2].set_vertex(Point(1,0));
	bline[0].set_tangent(bline[1].get_vertex()-bline[2].get_vertex()*0.5f);
	bline[1].set_tangent(bline[2].get_vertex()-bline[0].get_vertex()*0.5f);
	bline[2].set_tangent(bline[0].get_vertex()-bline[1].get_vertex()*0.5f);
	bline[0].set_width(1.0f);
	bline[1].set_width(1.0f);
	bline[2].set_width(1.0f);
	bline_loop=true;
	param_bline.set_list_of(bline);

	sync();

	SET_INTERPOLATION_DEFAULTS();
	SET_STATIC_DEFAULTS();
}

inline Color
CurveGradient::color_func(const Point &point_, int quality, Real supersample)const
{
	Point origin=param_origin.get(Point());
	Real width=param_width.get(Real());
	std::vector<synfig::BLinePoint> bline(param_bline.get_list_of(BLinePoint()));
	bool loop=param_loop.get(bool());
	bool perpendicular=param_perpendicular.get(bool());
	bool fast=param_fast.get(bool());

	Vector tangent;
	Vector diff;
	Point p1;
	Real thickness;
	Real dist;

	Real perp_dist = 0;
	bool edge_case = false;

	if(bline.size()==0)
		return Color::alpha();
	else if(bline.size()==1)
	{
		tangent=bline.front().get_tangent1();
		p1=bline.front().get_vertex();
		thickness=bline.front().get_width();
	}
	else
	{
		Real t;
		Point point(point_-origin);

		std::vector<synfig::BLinePoint>::const_iterator iter,next;

		// Figure out the BLinePoints we will be using,
		// Taking into account looping.
		if(perpendicular)
		{
			next=find_closest(fast,bline,point,t,bline_loop,&perp_dist);
			perp_dist/=curve_length_;
		}
		else					// not perpendicular
		{
			next=find_closest(fast,bline,point,t,bline_loop);
		}

		iter=next++;
		if(next==bline.end()) next=bline.begin();

		// Setup the curve
		etl::hermite<Vector> curve(
			iter->get_vertex(),
			next->get_vertex(),
			iter->get_tangent2(),
			next->get_tangent1()
			);

		// Setup the derivative function
		etl::derivative<etl::hermite<Vector> > deriv(curve);

		int search_iterations(7);

		/*if(quality==0)search_iterations=8;
		  else if(quality<=2)search_iterations=10;
		  else if(quality<=4)search_iterations=8;
		*/
		if(perpendicular)
		{
			if(quality>7)
				search_iterations=4;
		}
		else					// not perpendicular
		{
			if(quality<=6)search_iterations=7;
			else if(quality<=7)search_iterations=6;
			else if(quality<=8)search_iterations=5;
			else search_iterations=4;
		}

		// Figure out the closest point on the curve
		if (fast)
			t = curve.find_closest(fast, point,search_iterations);

		// Calculate our values
		p1=curve(t);			 // the closest point on the curve
		tangent=deriv(t);		 // the tangent at that point

		// if the point we're nearest to is at either end of the
		// bline, our distance from the curve is the distance from the
		// point on the curve.  we need to know which side of the
		// curve we're on, so find the average of the two tangents at
		// this point
		if (t<0.00001 || t>0.99999)
		{
			bool zero_tangent = (tangent[0] == 0 && tangent[1] == 0);

			if (t<0.5)
			{
				if (iter->get_split_tangent_angle() || iter->get_split_tangent_radius() || zero_tangent)
				{
					// fake the current tangent if we need to
					if (zero_tangent) tangent = curve(FAKE_TANGENT_STEP) - curve(0);

					// calculate the other tangent
					Vector other_tangent(iter->get_tangent1());
					if (other_tangent[0] == 0 && other_tangent[1] == 0)
					{
						// find the previous blinepoint
						std::vector<synfig::BLinePoint>::const_iterator prev;
						if (iter != bline.begin()) (prev = iter)--;
						else if (loop) (prev = bline.end())--;
						else prev = iter;

						etl::hermite<Vector> other_curve(prev->get_vertex(), iter->get_vertex(), prev->get_tangent2(), iter->get_tangent1());
						other_tangent = other_curve(1) - other_curve(1-FAKE_TANGENT_STEP);
					}

					// normalise and sum the two tangents
					tangent=(other_tangent.norm()+tangent.norm());
					edge_case=true;
				}
			}
			else
			{
				if (next->get_split_tangent_angle() || next->get_split_tangent_radius() || zero_tangent)
				{
					// fake the current tangent if we need to
					if (zero_tangent) tangent = curve(1) - curve(1-FAKE_TANGENT_STEP);

					// calculate the other tangent
					Vector other_tangent(next->get_tangent2());
					if (other_tangent[0] == 0 && other_tangent[1] == 0)
					{
						// find the next blinepoint
						std::vector<synfig::BLinePoint>::const_iterator next2(next);
						if (++next2 == bline.end())
						{
							if (loop) next2 = bline.begin();
							else next2 = next;
						}

						etl::hermite<Vector> other_curve(next->get_vertex(), next2->get_vertex(), next->get_tangent2(), next2->get_tangent1());
						other_tangent = other_curve(FAKE_TANGENT_STEP) - other_curve(0);
					}

					// normalise and sum the two tangents
					tangent=(other_tangent.norm()+tangent.norm());
					edge_case=true;
				}
			}
		}
		tangent = tangent.norm();

		if(perpendicular)
		{
			tangent*=curve_length_;
			p1-=tangent*perp_dist;
			tangent=-tangent.perp();
		}
		else					// not perpendicular
			// the width of the bline at the closest point on the curve
			thickness=(next->get_width()-iter->get_width())*t+iter->get_width();
	}

	if (perpendicular && bline.size() > 1)
	{
		if(quality>7)
		{
			dist=perp_dist;
/*			diff=tangent.perp();
			const Real mag(diff.inv_mag());
			supersample=supersample*mag;
*/
			supersample=0;
		}
		else
		{
			diff=tangent.perp();
			//p1-=diff*0.5;
			const Real mag(diff.inv_mag());
			supersample=supersample*mag;
			diff*=mag*mag;
			dist=(point_-origin - p1)*diff;
		}
	}
	else						// not perpendicular
	{
		if (edge_case)
		{
			diff=(p1-(point_-origin));
			if(diff*tangent.perp()<0) diff=-diff;
			diff=diff.norm()*thickness*width;
		}
		else
			diff=tangent.perp()*thickness*width;

		p1-=diff*0.5;
		const Real mag(diff.inv_mag());
		supersample=supersample*mag;
		diff*=mag*mag;
		dist=(point_-origin - p1)*diff;
	}

	supersample *= 0.5;
	return compiled_gradient.average(dist - supersample, dist + supersample);
}

Real
CurveGradient::calc_supersample(const synfig::Point &/*x*/, Real pw, Real /*ph*/)const
{
	return pw;
}

synfig::Layer::Handle
CurveGradient::hit_check(synfig::Context context, const synfig::Point &point)const
{
	if(get_blend_method()==Color::BLEND_STRAIGHT && get_amount()>=0.5)
		return const_cast<CurveGradient*>(this);
	if(get_amount()==0.0)
		return context.hit_check(point);
	if((get_blend_method()==Color::BLEND_STRAIGHT || get_blend_method()==Color::BLEND_COMPOSITE|| get_blend_method()==Color::BLEND_ONTO) && color_func(point).get_a()>0.5)
		return const_cast<CurveGradient*>(this);
	return context.hit_check(point);
}

bool
CurveGradient::set_param(const String & param, const ValueBase &value)
{


	IMPORT_VALUE(param_origin);
	IMPORT_VALUE(param_width);
	if(param=="bline" && value.get_type()==type_list)
	{
		param_bline=value;
		bline_loop=value.get_loop();
		sync();
		return true;
	}
	IMPORT_VALUE_PLUS(param_gradient, compile());
	IMPORT_VALUE_PLUS(param_loop, compile());
	IMPORT_VALUE_PLUS(param_zigzag, compile());
	IMPORT_VALUE(param_perpendicular);
	IMPORT_VALUE(param_fast);

	if(param=="offset")
		return set_param("origin", value);

	return Layer_Composite::set_param(param,value);
}

ValueBase
CurveGradient::get_param(const String & param)const
{
	EXPORT_VALUE(param_origin);
	EXPORT_VALUE(param_width);
	EXPORT_VALUE(param_bline);
	EXPORT_VALUE(param_gradient);
	EXPORT_VALUE(param_loop);
	EXPORT_VALUE(param_zigzag);
	EXPORT_VALUE(param_perpendicular);
	EXPORT_VALUE(param_fast);

	EXPORT_NAME();
	EXPORT_VERSION();

	return Layer_Composite::get_param(param);
}

Layer::Vocab
CurveGradient::get_param_vocab()const
{
	Layer::Vocab ret(Layer_Composite::get_param_vocab());

	ret.push_back(ParamDesc("origin")
				  .set_local_name(_("Origin"))
				  .set_description(_("Offset for the Vertices List"))
	);
	ret.push_back(ParamDesc("width")
				  .set_is_distance()
				  .set_local_name(_("Width"))
				  .set_description(_("Global width of the gradient"))
	);
	ret.push_back(ParamDesc("bline")
				  .set_local_name(_("Vertices"))
				  .set_origin("origin")
				  .set_hint("width")
				  .set_description(_("A list of spline points"))
	);
	ret.push_back(ParamDesc("gradient")
				  .set_local_name(_("Gradient"))
				  .set_description(_("Gradient to apply"))
	);
	ret.push_back(ParamDesc("loop")
				  .set_local_name(_("Loop"))
				  .set_description(_("When checked, the gradient is looped"))
	);
	ret.push_back(ParamDesc("zigzag")
				  .set_local_name(_("ZigZag"))
				  .set_description(_("When checked, the gradient is symmetrical at the center"))
	);
	ret.push_back(ParamDesc("perpendicular")
				  .set_local_name(_("Perpendicular"))
	);
	ret.push_back(ParamDesc("fast")
				  .set_local_name(_("Fast"))
				  .set_description(_("When checked, renders quickly but with artifacts"))
	);

	return ret;
}

Color
CurveGradient::get_color(Context context, const Point &point)const
{
	const Color color(color_func(point,0));

	if(get_amount()==1.0 && get_blend_method()==Color::BLEND_STRAIGHT)
		return color;
	else
		return Color::blend(color,context.get_color(point),get_amount(),get_blend_method());
}

bool
CurveGradient::accelerated_render(Context context,Surface *surface,int quality, const RendDesc &renddesc, ProgressCallback *cb)const
{
	RENDER_TRANSFORMED_IF_NEED(__FILE__, __LINE__)

	SuperCallback supercb(cb,0,9500,10000);

	if(get_amount()==1.0 && get_blend_method()==Color::BLEND_STRAIGHT)
	{
		surface->set_wh(renddesc.get_w(),renddesc.get_h());
	}
	else
	{
		if(!context.accelerated_render(surface,quality,renddesc,&supercb))
			return false;
		if(get_amount()==0)
			return true;
	}


	int x,y;

	Surface::pen pen(surface->begin());
	const Real pw(renddesc.get_pw()),ph(renddesc.get_ph());
	Point pos;
	Point tl(renddesc.get_tl());
	const int w(surface->get_w());
	const int h(surface->get_h());

	if(get_amount()==1.0 && get_blend_method()==Color::BLEND_STRAIGHT)
	{
		for(y=0,pos[1]=tl[1];y<h;y++,pen.inc_y(),pen.dec_x(x),pos[1]+=ph)
			for(x=0,pos[0]=tl[0];x<w;x++,pen.inc_x(),pos[0]+=pw)
				pen.put_value(color_func(pos,quality,calc_supersample(pos,pw,ph)));
	}
	else
	{
		for(y=0,pos[1]=tl[1];y<h;y++,pen.inc_y(),pen.dec_x(x),pos[1]+=ph)
			for(x=0,pos[0]=tl[0];x<w;x++,pen.inc_x(),pos[0]+=pw)
				pen.put_value(Color::blend(color_func(pos,quality,calc_supersample(pos,pw,ph)),pen.get_value(),get_amount(),get_blend_method()));
	}

	// Mark our progress as finished
	if(cb && !cb->amount_complete(10000,10000))
		return false;

	return true;
}

////
bool
CurveGradient::accelerated_cairorender(Context context, cairo_t *cr,int quality, const RendDesc &renddesc_, ProgressCallback *cb)const
{
	RendDesc	renddesc(renddesc_);
	
	// Untransform the render desc
	if(!cairo_renddesc_untransform(cr, renddesc))
		return false;
	
	Point pos;
	const Real pw(renddesc.get_pw()),ph(renddesc.get_ph());
	const Point tl(renddesc.get_tl());
	const int w(renddesc.get_w());
	const int h(renddesc.get_h());
	
	SuperCallback supercb(cb,0,9500,10000);
	
	if(get_amount()==1.0 && get_blend_method()==Color::BLEND_STRAIGHT)
	{
		cairo_save(cr);
		cairo_set_operator(cr, CAIRO_OPERATOR_CLEAR);
		cairo_paint(cr);
		cairo_restore(cr);
	}
	else
	{
		if(!context.accelerated_cairorender(cr,quality,renddesc,&supercb))
			return false;
		if(get_amount()==0)
			return true;
	}
	
	
	int x,y;
	cairo_surface_t *surface;
	
	surface=cairo_surface_create_similar(cairo_get_target(cr), CAIRO_CONTENT_COLOR_ALPHA, w, h);
	
	CairoSurface csurface(surface);
	if(!csurface.map_cairo_image())
	{
		synfig::warning("Curve Gradient: map cairo surface failed");
		return false;
	}
	for(y=0,pos[1]=tl[1];y<h;y++,pos[1]+=ph)
		for(x=0,pos[0]=tl[0];x<w;x++,pos[0]+=pw)
			csurface[y][x]=CairoColor(color_func(pos,calc_supersample(pos,pw,ph))).premult_alpha();
	csurface.unmap_cairo_image();
	
	// paint surface on cr
	cairo_save(cr);
	cairo_translate(cr, tl[0], tl[1]);
	cairo_scale(cr, pw, ph);
	cairo_set_source_surface(cr, surface, 0, 0);
	cairo_paint_with_alpha_operator(cr, get_amount(), get_blend_method());
	cairo_restore(cr);
	
	cairo_surface_destroy(surface);
	// Mark our progress as finished
	if(cb && !cb->amount_complete(10000,10000))
		return false;
	
	return true;

}
