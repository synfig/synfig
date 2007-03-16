/* === S Y N F I G ========================================================= */
/*!	\file plant.cpp
**	\brief Template
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

#include <synfig/angle.h>
#include "plant.h"
#include <synfig/string.h>
#include <synfig/time.h>
#include <synfig/context.h>
#include <synfig/paramdesc.h>
#include <synfig/renddesc.h>
#include <synfig/surface.h>
#include <synfig/value.h>
#include <synfig/valuenode.h>

#include <ETL/calculus>
#include <ETL/bezier>
#include <ETL/hermite>
#include <vector>

#include <synfig/valuenode_bline.h>

#endif

using namespace etl;

/* === M A C R O S ========================================================= */

#define SAMPLES		300
#define ROUND_END_FACTOR	(4)
#define CUSP_THRESHOLD		(0.15)
#define NO_LOOP_COOKIE		synfig::Vector(84951305,7836658)
#define EPSILON				(0.000000001)
#define CUSP_TANGENT_ADJUST	(0.025)

/* === G L O B A L S ======================================================= */

SYNFIG_LAYER_INIT(Plant);
SYNFIG_LAYER_SET_NAME(Plant,"plant");
SYNFIG_LAYER_SET_LOCAL_NAME(Plant,_("Plant"));
SYNFIG_LAYER_SET_CATEGORY(Plant,_("Particle Systems"));
SYNFIG_LAYER_SET_VERSION(Plant,"0.1");
SYNFIG_LAYER_SET_CVS_ID(Plant,"$Id$");

/* === P R O C E D U R E S ================================================= */

/* === M E T H O D S ======================================================= */


Plant::Plant():
	split_angle(Angle::deg(10)),
	gravity(0,-0.1),
	velocity(0.3),
	step(0.01),
	sprouts(10)
{
	bounding_rect=Rect::zero();
	random_factor=0.2;
	random.set_seed(time(NULL));

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
	mass=(0.5);
	splits=5;
	drag=0.1;
	size=0.015;
	sync();
	size_as_alpha=false;
}

void
Plant::branch(int n,int depth,float t, float stunt_growth, synfig::Point position,synfig::Vector vel)const
{
	float next_split((1.0-t)/(splits-depth)+t/*+random_factor*random(40+depth,t*splits,0,0)/splits*/);
	for(;t<next_split;t+=step)
	{
		vel[0]+=gravity[0]*step;
		vel[1]+=gravity[1]*step;
		vel*=(1.0-(drag)*step);
		position[0]+=vel[0]*step;
		position[1]+=vel[1]*step;

		particle_list.push_back(Particle(
			position,
			gradient(t)
		));
		bounding_rect.expand(position);
	}

	if(t>=1.0-stunt_growth)return;

	synfig::Real sin_v=synfig::Angle::cos(split_angle).get();
	synfig::Real cos_v=synfig::Angle::sin(split_angle).get();

	synfig::Vector velocity1(vel[0]*sin_v-vel[1]*cos_v+random_factor*random(2,30+n+depth,t*splits,0.0f,0.0f),vel[0]*cos_v+vel[1]*sin_v+random_factor*random(2,32+n+depth,t*splits,0.0f,0.0f));
	synfig::Vector velocity2(vel[0]*sin_v+vel[1]*cos_v+random_factor*random(2,31+n+depth,t*splits,0.0f,0.0f),-vel[0]*cos_v+vel[1]*sin_v+random_factor*random(2,33+n+depth,t*splits,0.0f,0.0f));

	Plant::branch(n,depth+1,t,stunt_growth,position,velocity1);
	Plant::branch(n,depth+1,t,stunt_growth,position,velocity2);
}

void
Plant::calc_bounding_rect()const
{
	std::vector<synfig::BLinePoint>::const_iterator iter,next;

	bounding_rect=Rect::zero();

	// Bline must have at least 2 points in it
	if(bline.size()<=2)
		return;

	next=bline.begin();

	if(bline_loop)
		iter=--bline.end();
	else
		iter=next++;

	for(;next!=bline.end();iter=next++)
	{
		bounding_rect.expand(iter->get_vertex());
		bounding_rect.expand(next->get_vertex());
		bounding_rect.expand(iter->get_vertex()+iter->get_tangent2()*0.3333333333333);
		bounding_rect.expand(next->get_vertex()-next->get_tangent1()*0.3333333333333);
		bounding_rect.expand(next->get_vertex()+next->get_tangent2()*velocity);
	}
	bounding_rect.expand_x(gravity[0]);
	bounding_rect.expand_y(gravity[1]);
	bounding_rect.expand_x(size);
	bounding_rect.expand_y(size);
}

void
Plant::sync()const
{
	particle_list.clear();

	bounding_rect=Rect::zero();

	// Bline must have at least 2 points in it
	if(bline.size()<=2)
		return;

	std::vector<synfig::BLinePoint>::const_iterator iter,next;

	etl::hermite<Vector> curve;

	Real step(abs(this->step));

	int seg(0);

	next=bline.begin();

	if(bline_loop)
		iter=--bline.end();
	else
		iter=next++;

	for(;next!=bline.end();iter=next++,seg++)
	{
		curve.p1()=iter->get_vertex();
		curve.t1()=iter->get_tangent2();
		curve.p2()=next->get_vertex();
		curve.t2()=next->get_tangent1();
		curve.sync();
		etl::derivative<etl::hermite<Vector> > deriv(curve);

		Real f;
		int i(0), b(round_to_int((1.0/step)/(float)sprouts-1));
		if(b<=0)b=1;
		for(f=0.0;f<1.0;f+=step,i++)
		{
			Point point(curve(f));

			particle_list.push_back(Particle(
				point,
				gradient(0)
			));

			bounding_rect.expand(point);

			Real stunt_growth(random(2,i,f+seg,0.0f,0.0f)/2.0+0.5);
			stunt_growth*=stunt_growth;

			Vector branch_velocity(deriv(f).norm()*velocity);

			branch_velocity[0]+=random_factor*random(2,1,f*splits,0.0f,0.0f);
			branch_velocity[1]+=random_factor*random(2,2,f*splits,0.0f,0.0f);

			if(i%b==0)
				branch(
					i,
					0,
					0,	// time
					stunt_growth, // stunt growth
					point,branch_velocity
				);
		}
	}

	needs_sync_=false;
}

bool
Plant::set_param(const String & param, const ValueBase &value)
{
	if(param=="bline" && value.get_type()==ValueBase::TYPE_LIST)
	{
		bline=value;
		bline_loop=value.get_loop();
		needs_sync_=true;

		return true;
	}
	if(param=="seed" && value.same_as(int()))
	{
		random.set_seed(value.get(int()));
		needs_sync_=true;
		return true;
	}
	IMPORT_PLUS(split_angle,needs_sync_=true);
	IMPORT_PLUS(gravity,needs_sync_=true);
	IMPORT_PLUS(gradient,needs_sync_=true);
	IMPORT_PLUS(velocity,needs_sync_=true);
	IMPORT_PLUS(step,needs_sync_=true);
	IMPORT_PLUS(splits,needs_sync_=true);
	IMPORT_PLUS(sprouts,needs_sync_=true);
	IMPORT_PLUS(random_factor,needs_sync_=true);
	IMPORT_PLUS(drag,needs_sync_=true);
	IMPORT(size);
	IMPORT(size_as_alpha);

	return Layer_Composite::set_param(param,value);
}
/*
void
Plant::set_time(Context context, Time time)const
{
	if(needs_sync==true)
	{
		sync();
		needs_sync_=false;
	}
	//const_cast<Plant*>(this)->sync();
	context.set_time(time);
}

void
Plant::set_time(Context context, Time time, Vector pos)const
{
	if(needs_sync==true)
	{
		sync();
		needs_sync_=false;
	}
	//const_cast<Plant*>(this)->sync();
	context.set_time(time,pos);
}
*/
ValueBase
Plant::get_param(const String& param)const
{
	if(param=="seed")
		return random.get_seed();
	EXPORT(bline);
	EXPORT(split_angle);
	EXPORT(gravity);
	EXPORT(velocity);
	EXPORT(step);
	EXPORT(gradient);
	EXPORT(splits);
	EXPORT(sprouts);
	EXPORT(random_factor);
	EXPORT(drag);
	EXPORT(size);

	EXPORT(size_as_alpha);

	EXPORT_NAME();
	EXPORT_VERSION();

	return Layer_Composite::get_param(param);
}

Layer::Vocab
Plant::get_param_vocab()const
{
	Layer::Vocab ret(Layer_Composite::get_param_vocab());

	ret.push_back(ParamDesc("bline")
		.set_local_name(_("Vertices"))
		//.set_origin("offset")
		//.set_scalar("width")
		.set_description(_("A list of BLine Points"))
	);

	ret.push_back(ParamDesc("gradient")
		.set_local_name(_("Gradient"))
	);

	ret.push_back(ParamDesc("split_angle")
		.set_local_name(_("Split Angle"))
	);

	ret.push_back(ParamDesc("gravity")
		.set_local_name(_("Gravity"))
		.set_is_distance()
	);

	ret.push_back(ParamDesc("velocity")
		.set_local_name(_("Velocity"))
	);

	ret.push_back(ParamDesc("size")
		.set_local_name(_("Stem Size"))
		.set_is_distance()
	);

	ret.push_back(ParamDesc("size_as_alpha")
		.set_local_name(_("SizeAsAlpha"))
	);

	ret.push_back(ParamDesc("step")
		.set_local_name(_("Step"))
	);

	ret.push_back(ParamDesc("seed")
		.set_local_name(_("Seed"))
	);

	ret.push_back(ParamDesc("splits")
		.set_local_name(_("Splits"))
	);

	ret.push_back(ParamDesc("sprouts")
		.set_local_name(_("Sprouts"))
	);

	ret.push_back(ParamDesc("random_factor")
		.set_local_name(_("Random Factor"))
	);

	ret.push_back(ParamDesc("drag")
		.set_local_name(_("Drag"))
	);


	return ret;
}

bool
Plant::accelerated_render(Context context,Surface *surface,int quality, const RendDesc &renddesc, ProgressCallback *cb)const
{
	bool ret(context.accelerated_render(surface,quality,renddesc,cb));
	if(is_disabled() || !ret)
		return ret;

	Surface dest_surface;
	dest_surface.set_wh(surface->get_w(),surface->get_h());
	dest_surface.clear();

	const Point	tl(renddesc.get_tl());
	const Point br(renddesc.get_br());

	const int	w(renddesc.get_w());
	const int	h(renddesc.get_h());

	// Width and Height of a pixel
	const Real pw = (br[0] - tl[0]) / w;
	const Real ph = (br[1] - tl[1]) / h;

	if(needs_sync_==true)
		sync();

	std::vector<Particle>::reverse_iterator iter;
	const float size_factor(1);
	float radius(size_factor*size*sqrt(1.0f/(abs(pw)*abs(ph))));

	if(radius>1.0f)
	{
		radius*=1.0;
		int x1,y1,x2,y2;
		for(iter=particle_list.rbegin();iter!=particle_list.rend();++iter)
		{
			float radius(radius);
			Color color(iter->color);
			if(size_as_alpha)
			{
				radius*=color.get_a();
				color.set_a(1);
			}

			x1=ceil_to_int((iter->point[0]-tl[0])/pw-(radius*0.5));
			y1=ceil_to_int((iter->point[1]-tl[1])/ph-(radius*0.5));
			x2=x1+round_to_int(radius);
			y2=y1+round_to_int(radius);

			if(x1>=surface->get_w() || y1>=surface->get_h())
				continue;

			if(x2<0 || y2<0)
				continue;

			if(x2>=surface->get_w())
				x2=surface->get_w();
			if(y2>=surface->get_h())
				y2=surface->get_h();

			if(x1<0)
				x1=0;
			if(y1<0)
				y1=0;

			int w(min(round_to_int(radius),x2-x1));
			int h(min(round_to_int(radius),y2-y1));

			if(w<=0 || h<=0)
				continue;

			Surface::alpha_pen surface_pen(dest_surface.get_pen(x1,y1),1.0f);

			dest_surface.fill(color,surface_pen,w,h);
		}
	}
	else
	{
		//radius/=0.01;
		radius*=sqrt(step)*12.0f;
		int x,y;
		float a,b,c,d;
		for(iter=particle_list.rbegin();iter!=particle_list.rend();++iter)
		{
			float radius(radius);
			Color color(iter->color);
			if(size_as_alpha)
			{
				radius*=color.get_a();
				color.set_a(1);
			}

			x=floor_to_int((iter->point[0]-tl[0])/pw-0.5f);
			y=floor_to_int((iter->point[1]-tl[1])/ph-0.5f);

			if(x>=surface->get_w()-1 || y>=surface->get_h()-1 || x<0 || y<0)
			{
				continue;
			}

			a=((iter->point[0]-tl[0])/pw-0.5f-x)*radius;
			b=((iter->point[1]-tl[1])/ph-0.5f-y)*radius;
			c=radius-a;
			d=radius-b;

			Surface::alpha_pen surface_pen(dest_surface.get_pen(x,y),1.0f);

			surface_pen.set_alpha(c*d);
			surface_pen.put_value(color);
			surface_pen.inc_x();
			surface_pen.set_alpha(a*d);
			surface_pen.put_value(color);
			surface_pen.inc_y();
			surface_pen.set_alpha(a*b);
			surface_pen.put_value(color);
			surface_pen.dec_x();
			surface_pen.set_alpha(c*b);
			surface_pen.put_value(color);
		}
	}

	Surface::alpha_pen pen(surface->get_pen(0,0),get_amount(),get_blend_method());
	dest_surface.blit_to(pen);

	return true;
}

Rect
Plant::get_bounding_rect(Context context)const
{
	if(needs_sync_==true)
		sync();

	if(is_disabled())
		return Rect::zero();

	if(Color::is_onto(get_blend_method()))
		return context.get_full_bounding_rect() & bounding_rect;

	//if(get_blend_method()==Color::BLEND_BEHIND)
	//	return context.get_full_bounding_rect() | bounding_rect;
	return bounding_rect;
}
