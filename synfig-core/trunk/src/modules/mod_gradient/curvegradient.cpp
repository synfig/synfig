/*! ========================================================================
** Synfig
** Template File
** $Id: curvegradient.cpp,v 1.2 2005/01/13 06:48:39 darco Exp $
**
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

#include <synfig/string.h>
#include <synfig/time.h>
#include <synfig/context.h>
#include <synfig/paramdesc.h>
#include <synfig/renddesc.h>
#include <synfig/surface.h>
#include <synfig/value.h>
#include <synfig/valuenode.h>
#include <ETL/bezier>
#include <ETL/hermite>
#include <ETL/calculus>

#endif

/* === M A C R O S ========================================================= */

/* === G L O B A L S ======================================================= */

SYNFIG_LAYER_INIT(CurveGradient);
SYNFIG_LAYER_SET_NAME(CurveGradient,"curve_gradient");
SYNFIG_LAYER_SET_LOCAL_NAME(CurveGradient,_("Curve Gradient"));
SYNFIG_LAYER_SET_CATEGORY(CurveGradient,_("Gradients"));
SYNFIG_LAYER_SET_VERSION(CurveGradient,"0.0");
SYNFIG_LAYER_SET_CVS_ID(CurveGradient,"$Id: curvegradient.cpp,v 1.2 2005/01/13 06:48:39 darco Exp $");

/* === P R O C E D U R E S ================================================= */

inline float calculate_distance(const synfig::BLinePoint& a,const synfig::BLinePoint& b)
{
#if 1
	const Point& c1(a.get_vertex());
	const Point c2(a.get_vertex()+a.get_tangent2()/3);
	const Point c3(b.get_vertex()-b.get_tangent1()/3);
	const Point& c4(b.get_vertex());
	return (c1-c2).mag()+(c2-c3).mag()+(c3-c4).mag();
#else
#endif
}

inline float calculate_distance(const std::vector<synfig::BLinePoint>& bline)
{
	std::vector<synfig::BLinePoint>::const_iterator iter,next,ret;
	std::vector<synfig::BLinePoint>::const_iterator end(bline.end());

	float dist(0);

	next=bline.begin();

	//if(loop)
	//	iter=--bline.end();
	//else
		iter=next++;

	for(;next!=end;iter=next++)
	{
		// Setup the curve
		etl::hermite<Vector> curve(
			iter->get_vertex(),
			next->get_vertex(),
			iter->get_tangent2(),
			next->get_tangent1()
		);

//		dist+=calculate_distance(*iter,*next);
		dist+=curve.length();
	}

	return dist;
}

std::vector<synfig::BLinePoint>::const_iterator
find_closest(const std::vector<synfig::BLinePoint>& bline,const Point& p,bool loop=false,float *bline_dist_ret=0)
{
	std::vector<synfig::BLinePoint>::const_iterator iter,next,ret;
	std::vector<synfig::BLinePoint>::const_iterator end(bline.end());

	ret=bline.end();
	float dist(100000000000.0);

	next=bline.begin();

	float best_bline_dist(0);
	float best_bline_len(0);
	float total_bline_dist(0);
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
			next->get_tangent1()
		);

		/*
		const float t(curve.find_closest(p,6,0.01,0.99));
		bp=curve(t);if((bp-p).mag_squared()<dist) { ret=iter; dist=(bp-p).mag_squared(); ret_t=t; }
		*/

		float thisdist(0);
		float len(0);
		if(bline_dist_ret)
		{
			//len=calculate_distance(*iter,*next);
			len=curve.length();
		}

#define POINT_CHECK(x) bp=curve(x);	thisdist=(bp-p).mag_squared(); if(thisdist<dist) { ret=iter; dist=thisdist; best_bline_dist=total_bline_dist; best_bline_len=len; best_curve=curve; }

		POINT_CHECK(0.0001);
		POINT_CHECK((1.0/6.0));
		POINT_CHECK((2.0/6.0));
		POINT_CHECK((3.0/6.0));
		POINT_CHECK((4.0/6.0));
		POINT_CHECK((5.0/6.0));
		POINT_CHECK(0.9999);

		total_bline_dist+=len;
	}

	if(bline_dist_ret)
	{
		*bline_dist_ret=best_bline_dist+best_curve.find_distance(0,best_curve.find_closest(p));
//		*bline_dist_ret=best_bline_dist+best_curve.find_closest(p)*best_bline_len;
	}

	return ret;
}

/* === M E T H O D S ======================================================= */

inline void
CurveGradient::sync()
{
	diff=(p2-p1);
	const Real mag(diff.inv_mag());
	diff*=mag*mag;

	curve_length_=calculate_distance(bline);
}


CurveGradient::CurveGradient():
	p1(1,1),
	p2(-1,-1),
	offset(0,0),
	width(0.25),
	gradient(Color::black(), Color::white()),
	loop(false),
	zigzag(false),
	perpendicular(false)
{
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

	sync();
}

inline Color
CurveGradient::color_func(const Point &point_, int quality, float supersample)const
{
	Vector tangent;
	Vector diff;
	Point p1;
	Real thickness;
	Real dist;

	float perp_dist;

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
		Point point(point_-offset);

		std::vector<synfig::BLinePoint>::const_iterator iter,next;

		// Figure out the BLinePoints we will be using,
		// Taking into account looping.
		if(perpendicular)
		{
			next=find_closest(bline,point,bline_loop,&perp_dist);
			perp_dist/=curve_length_;
		}
		else
		{
			next=find_closest(bline,point,bline_loop);
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
			if(!perpendicular)
			{
				if(quality<=6)search_iterations=7;
				else if(quality<=7)search_iterations=6;
				else if(quality<=8)search_iterations=5;
				else search_iterations=4;
			}
			else
			{
				if(quality>7)
					search_iterations=4;
			}

			// Figure out the closest point on the curve
			const float t(curve.find_closest(point,search_iterations));


			// Calculate our values
			p1=curve(t);
			tangent=deriv(t).norm();

			if(perpendicular)
			{
				tangent*=curve_length_;
				p1-=tangent*perp_dist;
				tangent=-tangent.perp();
			}
			else
			{
				thickness=(next->get_width()-iter->get_width())*t+iter->get_width();
			}
		//}
	}


	if(!perpendicular)
	{
			diff=tangent.perp()*thickness*width;
			p1-=diff*0.5;
			const Real mag(diff.inv_mag());
			supersample=supersample*mag;
			diff*=mag*mag;
			dist=((point_-offset)*diff-p1*diff);
	}
	else
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
		dist=((point_-offset)*diff-p1*diff);
		}
	}

	if(loop)
		dist-=floor(dist);

	if(zigzag)
	{
		dist*=2.0;
		supersample*=2.0;
		if(dist>1)dist=2.0-dist;
	}

	if(loop)
	{
		if(dist+supersample*0.5>1.0)
		{
			Color pool(gradient(dist,supersample*0.5).premult_alpha()*(1.0-(dist-supersample*0.5)));
			pool+=gradient((dist+supersample*0.5)-1.0,supersample*0.5).premult_alpha()*((dist+supersample*0.5)-1.0);
			return pool.demult_alpha();
		}
		if(dist-supersample*0.5<0.0)
		{
			Color pool(gradient(dist,supersample*0.5).premult_alpha()*(dist+supersample*0.5));
			pool+=gradient(1.0-(dist-supersample*0.5),supersample*0.5).premult_alpha()*(-(dist-supersample*0.5));
			return pool.demult_alpha();
		}
	}
	return gradient(dist,supersample);
}

float
CurveGradient::calc_supersample(const synfig::Point &x, float pw,float ph)const
{
//	return pw/(p2-p1).mag();
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
	if(param=="p1" && value.same_as(p1))
	{
		p1=value.get(p1);
		sync();
		return true;
	}
	if(param=="p2" && value.same_as(p2))
	{
		p2=value.get(p2);
		sync();
		return true;
	}
	//IMPORT(p1);
	//IMPORT(p2);


	IMPORT(offset);
	IMPORT(perpendicular);

	if(param=="bline" && value.get_type()==ValueBase::TYPE_LIST)
	{
		bline=value;
		bline_loop=value.get_loop();
		sync();

		return true;
	}

	IMPORT(width);
	IMPORT(gradient);
	IMPORT(loop);
	IMPORT(zigzag);
	return Layer_Composite::set_param(param,value);
}

ValueBase
CurveGradient::get_param(const String & param)const
{
	EXPORT(offset);
	EXPORT(bline);
	EXPORT(p1);
	EXPORT(p2);
	EXPORT(gradient);
	EXPORT(loop);
	EXPORT(zigzag);
	EXPORT(width);
	EXPORT(perpendicular);

	EXPORT_NAME();
	EXPORT_VERSION();

	return Layer_Composite::get_param(param);
}

Layer::Vocab
CurveGradient::get_param_vocab()const
{
	Layer::Vocab ret(Layer_Composite::get_param_vocab());

		ret.push_back(ParamDesc("offset")
		.set_local_name(_("Offset"))
	);

	ret.push_back(ParamDesc("width")
		.set_is_distance()
		.set_local_name(_("Width"))
	);

	ret.push_back(ParamDesc("bline")
		.set_local_name(_("Vertices"))
		.set_origin("offset")
		.set_scalar("width")
		.set_description(_("A list of BLine Points"))
	);


	ret.push_back(ParamDesc("gradient")
		.set_local_name(_("Gradient"))
	);
	ret.push_back(ParamDesc("loop")
		.set_local_name(_("Loop"))
	);
	ret.push_back(ParamDesc("zigzag")
		.set_local_name(_("ZigZag"))
	);
	ret.push_back(ParamDesc("perpendicular")
		.set_local_name(_("Perpendicular"))
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
