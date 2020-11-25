/* === S Y N F I G ========================================================= */
/*!	\file plant.cpp
**	\brief Implementation of the "Plant" layer
**
**	$Id$
**
**	\legal
**	Copyright (c) 2002-2005 Robert B. Quattlebaum Jr., Adrian Bentley
**	Copyright (c) 2007, 2008 Chris Moore
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
#include <time.h>

#include <synfig/valuenodes/valuenode_bline.h>

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
SYNFIG_LAYER_SET_LOCAL_NAME(Plant,N_("Plant"));
SYNFIG_LAYER_SET_CATEGORY(Plant,N_("Other"));
SYNFIG_LAYER_SET_VERSION(Plant,"0.2");

/* === P R O C E D U R E S ================================================= */

/* === M E T H O D S ======================================================= */


Plant::Plant():
	param_bline(ValueBase(std::vector<BLinePoint>())),
	param_origin(ValueBase(Vector(0,0))),
	param_gradient(ValueBase(Gradient(Color::black(), Color::white()))),
	param_split_angle(ValueBase(Angle::deg(10))),
	param_gravity(ValueBase(Vector(0,-0.1))),
	param_velocity(ValueBase(Real(0.3))),
	param_perp_velocity(ValueBase(Real(0.0))),
	param_size(ValueBase(Real(0.015))),
	param_size_as_alpha(ValueBase(false)),
	param_reverse(ValueBase(true)),
	param_step(ValueBase(Real(0.01))),
	param_splits(ValueBase(int(5))),
	param_sprouts(ValueBase(int(10))),
	param_random_factor(ValueBase(Real(0.2))),
	param_drag(ValueBase(Real(0.1))),
	param_use_width(ValueBase(true)),
	version(version__)
{
	bounding_rect=Rect::zero();
	Random random;
	random.set_seed(time(NULL));
	param_random.set(random.get_seed());
	
	std::vector<BLinePoint> bline;
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
	param_bline.set_list_of(bline);
	
	bline_loop=true;
	mass=(0.5);
	needs_sync_=true;
	sync();
	
	SET_INTERPOLATION_DEFAULTS();
	SET_STATIC_DEFAULTS();
}

void
Plant::branch(int n,int depth,float t, float stunt_growth, synfig::Point position,synfig::Vector vel)const
{
	int splits=param_splits.get(int());
	Real step=param_step.get(Real());
	Vector gravity=param_gravity.get(Vector());
	Real drag=param_drag.get(Real());
	Gradient gradient=param_gradient.get(Gradient());
	Angle split_angle=param_split_angle.get(Angle());
	Real random_factor=param_random_factor.get(Real());
	Random random;
	random.set_seed(param_random.get(int()));
	
	float next_split((1.0-t)/(splits-depth)+t/*+random_factor*random(40+depth,t*splits,0,0)/splits*/);
	for(;t<next_split;t+=step)
	{
		vel[0]+=gravity[0]*step;
		vel[1]+=gravity[1]*step;
		vel*=(1.0-(drag)*step);
		position[0]+=vel[0]*step;
		position[1]+=vel[1]*step;

		particle_list.push_back(Particle(position, gradient(t)));
		if (particle_list.size() % 1000000 == 0)
			synfig::info("constructed %d million particles...", particle_list.size()/1000000);

		bounding_rect.expand(position);
	}

	if(t>=1.0-stunt_growth)return;

	synfig::Real sin_v=synfig::Angle::cos(split_angle).get();
	synfig::Real cos_v=synfig::Angle::sin(split_angle).get();

	synfig::Vector velocity1(vel[0]*sin_v - vel[1]*cos_v + random_factor*random(Random::SMOOTH_COSINE, 30+n+depth, t*splits, 0.0f, 0.0f),
							 vel[0]*cos_v + vel[1]*sin_v + random_factor*random(Random::SMOOTH_COSINE, 32+n+depth, t*splits, 0.0f, 0.0f));
	synfig::Vector velocity2(vel[0]*sin_v + vel[1]*cos_v + random_factor*random(Random::SMOOTH_COSINE, 31+n+depth, t*splits, 0.0f, 0.0f),
							-vel[0]*cos_v + vel[1]*sin_v + random_factor*random(Random::SMOOTH_COSINE, 33+n+depth, t*splits, 0.0f, 0.0f));

	Plant::branch(n,depth+1,t,stunt_growth,position,velocity1);
	Plant::branch(n,depth+1,t,stunt_growth,position,velocity2);
}

void
Plant::calc_bounding_rect()const
{
	std::vector<BLinePoint> bline(param_bline.get_list_of(BLinePoint()));
	Real velocity=param_velocity.get(Real());
	Vector gravity=param_gravity.get(Vector());
	Real size=param_size.get(Real());

	std::vector<synfig::BLinePoint>::const_iterator iter,next;

	bounding_rect=Rect::zero();

	// Bline must have at least 2 points in it
	if(bline.size()<2)
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
	std::vector<BLinePoint> bline(param_bline.get_list_of(BLinePoint()));
	Real step_=param_step.get(Real());
	Gradient gradient=param_gradient.get(Gradient());
	Real random_factor=param_random_factor.get(Real());
	Random random;
	random.set_seed(param_random.get(int()));
	int sprouts=param_sprouts.get(int());
	Real velocity=param_velocity.get(Real());
	Real perp_velocity=param_perp_velocity.get(Real());
	int splits=param_splits.get(int());
	bool use_width=param_use_width.get(bool());
	
	std::lock_guard<std::mutex> lock(mutex);
	if (!needs_sync_) return;
	time_t start_time; time(&start_time);
	particle_list.clear();

	bounding_rect=Rect::zero();

	// Bline must have at least 2 points in it
	if(bline.size()<2)
	{
		needs_sync_=false;
		return;
	}

	std::vector<synfig::BLinePoint>::const_iterator iter,next;

	etl::hermite<Vector> curve;

	Real step(abs(step_));

	int seg(0);

	next=bline.begin();

	if(bline_loop)	iter=--bline.end(); // iter is the last  bline in the list; next is the first  bline in the list
	else			iter=next++;		// iter is the first bline in the list; next is the second bline in the list

	// loop through the bline; seg counts the blines as we do so; stop before iter is the last bline in the list
	for(;next!=bline.end();iter=next++,seg++)
	{
		float iterw=iter->get_width();	// the width value of the iter vertex
		float nextw=next->get_width();	// the width value of the next vertex
		float width;					// the width at an intermediate position
		curve.p1()=iter->get_vertex();
		curve.t1()=iter->get_tangent2();
		curve.p2()=next->get_vertex();
		curve.t2()=next->get_tangent1();
		curve.sync();
		etl::derivative<etl::hermite<Vector> > deriv(curve);

		Real f;

		int i=0, branch_count = 0, steps = round_to_int(1.0/step);
		if (steps < 1) steps = 1;
		for(f=0.0;f<1.0;f+=step,i++)
		{
			Point point(curve(f));

			particle_list.push_back(Particle(point, gradient(0)));
			if (particle_list.size() % 1000000 == 0)
				synfig::info("constructed %d million particles...", particle_list.size()/1000000);

			bounding_rect.expand(point);

			Real stunt_growth(random_factor * (random(Random::SMOOTH_COSINE,i,f+seg,0.0f,0.0f)/2.0+0.5));
			stunt_growth*=stunt_growth;

			if((((i+1)*sprouts + steps/2) / steps) > branch_count) {
				Vector branch_velocity(deriv(f).norm()*velocity + deriv(f).perp().norm()*perp_velocity);

				if (std::isnan(branch_velocity[0]) || std::isnan(branch_velocity[1]))
					continue;

				branch_velocity[0] += random_factor * random(Random::SMOOTH_COSINE, 1, f*splits, 0.0f, 0.0f);
				branch_velocity[1] += random_factor * random(Random::SMOOTH_COSINE, 2, f*splits, 0.0f, 0.0f);

				if (use_width)
				{
					width = iterw+(nextw-iterw)*f; // calculate the width based on the current position

					branch_velocity[0] *= width; // scale the velocity accordingly to the current width
					branch_velocity[1] *= width;
				}

				branch_count++;
				branch(i, 0, 0,		 // time
					   stunt_growth, // stunt growth
					   point, branch_velocity);
			}
		}
	}

	time_t end_time; time(&end_time);
	if (end_time-start_time > 4)
		synfig::info("Plant::sync() constructed %d particles in %d seconds\n",
					 particle_list.size(), int(end_time-start_time));
	needs_sync_=false;
}

bool
Plant::set_param(const String & param, const ValueBase &value)
{
	IMPORT_VALUE_PLUS(param_bline,
	{
		bline_loop=value.get_loop();
		needs_sync_=true;
		return true;
	});
	IMPORT_VALUE_PLUS(param_random, needs_sync_=true);
	IMPORT_VALUE(param_origin);
	IMPORT_VALUE_PLUS(param_split_angle,needs_sync_=true);
	IMPORT_VALUE_PLUS(param_gravity,needs_sync_=true);
	IMPORT_VALUE_PLUS(param_gradient,needs_sync_=true);
	IMPORT_VALUE_PLUS(param_velocity,needs_sync_=true);
	IMPORT_VALUE_PLUS(param_perp_velocity,needs_sync_=true);
	IMPORT_VALUE_PLUS(param_step,{
			needs_sync_ = true;
			Real step=param_step.get(Real());
			if (step <= 0)
				step=0.01; // user is probably clueless - give a good default
			else if (step < 0.00001)
				step=0.00001; // 100K should be enough for anyone
			else if (step > 1)
				step=1;
			param_step.set(step);
		});
	IMPORT_VALUE_PLUS(param_splits,{
			needs_sync_=true;
			int splits=param_splits.get(int());
			if (splits < 1)
				splits = 1;
			param_splits.set(splits);
		});
	IMPORT_VALUE_PLUS(param_sprouts,needs_sync_=true);
	IMPORT_VALUE_PLUS(param_random_factor,needs_sync_=true);
	IMPORT_VALUE_PLUS(param_drag,needs_sync_=true);
	IMPORT_VALUE(param_size);
	IMPORT_VALUE(param_size_as_alpha);
	IMPORT_VALUE(param_reverse);
	IMPORT_VALUE(param_use_width);

	if(param=="offset")
		return set_param("origin", value);
	if(param=="seed")
		return set_param("random", value);

	return Layer_Composite::set_param(param,value);
}

ValueBase
Plant::get_param(const String& param)const
{
	if(param=="seed")
		return get_param("random");

	EXPORT_VALUE(param_bline);
	EXPORT_VALUE(param_origin);
	EXPORT_VALUE(param_split_angle);
	EXPORT_VALUE(param_gravity);
	EXPORT_VALUE(param_velocity);
	EXPORT_VALUE(param_perp_velocity);
	EXPORT_VALUE(param_step);
	EXPORT_VALUE(param_gradient);
	EXPORT_VALUE(param_splits);
	EXPORT_VALUE(param_sprouts);
	EXPORT_VALUE(param_random_factor);
	EXPORT_VALUE(param_drag);
	EXPORT_VALUE(param_size);
	EXPORT_VALUE(param_size_as_alpha);
	EXPORT_VALUE(param_reverse);
	EXPORT_VALUE(param_use_width);
	EXPORT_VALUE(param_random);

	EXPORT_NAME();

	if(param=="Version" || param=="version" || param=="version__")
		return version;

	return Layer_Composite::get_param(param);
}

Layer::Vocab
Plant::get_param_vocab()const
{
	Layer::Vocab ret(Layer_Composite::get_param_vocab());

	ret.push_back(ParamDesc("bline")
		.set_local_name(_("Vertices"))
		.set_description(_("A list of spline points"))
		.set_origin("origin")
		.set_hint("width")
	);

	ret.push_back(ParamDesc("origin")
		.set_local_name(_("Origin"))
		.set_description(_("Offset for the Vertices List"))
	);

	ret.push_back(ParamDesc("gradient")
		.set_local_name(_("Gradient"))
		.set_description(_("Gradient to be used for coloring the plant"))
	);

	ret.push_back(ParamDesc("split_angle")
		.set_local_name(_("Split Angle"))
		.set_description(_("Angle by which each split deviates from its parent"))
	);

	ret.push_back(ParamDesc("gravity")
		.set_local_name(_("Gravity"))
		.set_description(_("Direction in which the shoots tend to face"))
		.set_is_distance()
	);

	ret.push_back(ParamDesc("velocity")
		.set_local_name(_("Tangential Velocity"))
		.set_description(_("Amount to which shoots tend to grow along the tangent to the spline"))
	);

	ret.push_back(ParamDesc("perp_velocity")
		.set_local_name(_("Perpendicular Velocity"))
		.set_description(_("Amount to which shoots tend to grow perpendicular to the tangent to the spline"))
	);

	ret.push_back(ParamDesc("size")
		.set_local_name(_("Stem Size"))
		.set_description(_("Size of the stem"))
		.set_is_distance()
	);

	ret.push_back(ParamDesc("size_as_alpha")
		.set_local_name(_("Size As Alpha"))
		.set_description(_("If enabled, the alpha channel from the gradient is multiplied by the stem size, and an alpha of 1.0 is used when rendering"))
	);

	ret.push_back(ParamDesc("reverse")
		.set_local_name(_("Reverse"))
		.set_description(_("If enabled, render the plant in the opposite direction"))
	);

	ret.push_back(ParamDesc("step")
		.set_local_name(_("Step"))
		.set_description(_("Measure of the distance between points when rendering"))
	);

	ret.push_back(ParamDesc("seed")
		.set_local_name(_("Seed"))
		.set_description(_("Used to seed the pseudo-random number generator"))
	);

	ret.push_back(ParamDesc("splits")
		.set_local_name(_("Splits"))
		.set_description(_("Maximum number of times that each sprout can sprout recursively"))
	);

	ret.push_back(ParamDesc("sprouts")
		.set_local_name(_("Sprouts"))
		.set_description(_("Number of places that growth occurs on each spline section"))
	);

	ret.push_back(ParamDesc("random_factor")
		.set_local_name(_("Random Factor"))
		.set_description(_("Used to scale down all random effects.  Set to zero to disable randomness"))
	);

	ret.push_back(ParamDesc("drag")
		.set_local_name(_("Drag"))
		.set_description(_("Drag slows the growth"))
	);

	ret.push_back(ParamDesc("use_width")
		.set_local_name(_("Use Width"))
		.set_description(_("Scale the velocity by the spline's width"))
	);

	return ret;
}

bool
Plant::set_version(const String &ver)
{
	version = ver;

	if (version == "0.1")
		param_use_width.set(false);

	return true;
}

bool
Plant::accelerated_render(Context context,Surface *surface,int quality, const RendDesc &renddesc, ProgressCallback *cb)const
{
	RENDER_TRANSFORMED_IF_NEED(__FILE__, __LINE__)

	bool ret(context.accelerated_render(surface,quality,renddesc,cb));
	if(is_disabled() || !ret)
		return ret;

	if(needs_sync_==true)
		sync();

	Surface dest_surface;
	dest_surface.set_wh(surface->get_w(),surface->get_h());
	dest_surface.clear();

	// Here is where drawing occurs 
	draw_particles(&dest_surface, renddesc);

	Surface::alpha_pen pen(surface->get_pen(0,0),get_amount(),get_blend_method());
	dest_surface.blit_to(pen);

	return true;
}


///
bool
Plant::accelerated_cairorender(Context context, cairo_t *cr, int quality, const RendDesc &renddesc, ProgressCallback *cb)const
{
	
	bool ret(context.accelerated_cairorender(cr,quality,renddesc,cb));
	if(is_disabled() || !ret)
		return ret;

	if(needs_sync_==true)
		sync();
	
	cairo_save(cr);
	cairo_push_group(cr);
	// Here is where drawing occurs
	draw_particles(cr);
	cairo_pop_group_to_source(cr);
	// blend the painted particles on the cr
	cairo_paint_with_alpha_operator(cr, get_amount(), get_blend_method());
	cairo_restore(cr);
	
	return true;
}


void
Plant::draw_particles(Surface *dest_surface, const RendDesc &renddesc)const
{
	Point origin=param_origin.get(Vector());
	Real size=param_size.get(Real());
	bool reverse=param_reverse.get(bool());
	bool size_as_alpha=param_size_as_alpha.get(bool());
	
	const Point	tl(renddesc.get_tl()-origin);
	const Point br(renddesc.get_br()-origin);
	
	const int	w(renddesc.get_w());
	const int	h(renddesc.get_h());
	
	const int	surface_width(dest_surface->get_w());
	const int	surface_height(dest_surface->get_h());
	
	// Width and Height of a pixel
	const Real pw = (br[0] - tl[0]) / w;
	const Real ph = (br[1] - tl[1]) / h;
	
	if (std::isinf(pw) || std::isinf(ph))
		return;
	
	if (particle_list.begin() != particle_list.end())
	{
		std::vector<Particle>::iterator iter;
		Particle *particle;
		
		float radius(size*sqrt(1.0f/(abs(pw)*abs(ph))));
		
		int x1,y1,x2,y2;
		
		if (reverse)	iter = particle_list.end();
		else			iter = particle_list.begin();
		
		while (true)
		{
			if (reverse)	particle = &(*(iter-1));
			else			particle = &(*iter);
			
			float scaled_radius(radius);
			Color color(particle->color);
			if(size_as_alpha)
			{
				scaled_radius*=color.get_a();
				color.set_a(1);
			}
			
			// previously, radius was multiplied by sqrt(step)*12 only if
			// the radius came out at less than 1 (pixel):
			//   if (radius<=1.0f) radius*=sqrt(step)*12.0f;
			// seems a little arbitrary - does it help?
			
			// calculate the box that this particle will be drawn as
			float x1f=(particle->point[0]-tl[0])/pw-(scaled_radius*0.5);
			float x2f=(particle->point[0]-tl[0])/pw+(scaled_radius*0.5);
			float y1f=(particle->point[1]-tl[1])/ph-(scaled_radius*0.5);
			float y2f=(particle->point[1]-tl[1])/ph+(scaled_radius*0.5);
			x1=ceil_to_int(x1f);
			x2=ceil_to_int(x2f)-1;
			y1=ceil_to_int(y1f);
			y2=ceil_to_int(y2f)-1;
			
			// if the box isn't entirely off the canvas, draw it
			if(x1<=surface_width && y1<=surface_height && x2>=0 && y2>=0)
			{
				float x1e=x1-x1f, x2e=x2f-x2, y1e=y1-y1f, y2e=y2f-y2;
				// printf("x1e %.4f x2e %.4f y1e %.4f y2e %.4f\n", x1e, x2e, y1e, y2e);
				
				// adjust the box so it's entirely on the canvas
				if(x1<=0) { x1=0; x1e=0; }
				if(y1<=0) { y1=0; y1e=0; }
				if(x2>=surface_width)  { x2=surface_width;  x2e=0; }
				if(y2>=surface_height) { y2=surface_height; y2e=0; }
				
				int w(x2-x1), h(y2-y1);
				
				Surface::alpha_pen surface_pen(dest_surface->get_pen(x1,y1),1.0f);
				if(w>0 && h>0)
					dest_surface->fill(color,surface_pen,w,h);
				
				/* the rectangle doesn't cross any vertical pixel boundaries so we don't
				 * need to draw any top or bottom edges
				 */
				if(x2<x1)
				{
					// case 1 - a single pixel
					if(y2<y1)
					{
						surface_pen.move_to(x2,y2);
						surface_pen.set_alpha((x2f-x1f)*(y2f-y1f));
						surface_pen.put_value(color);
					}
					// case 2 - a single vertical column of pixels
					else
					{
						surface_pen.move_to(x2,y1-1);
						if (y1e!=0)	// maybe draw top pixel
						{
							surface_pen.set_alpha(y1e*(x2f-x1f));
							surface_pen.put_value(color);
						}
						surface_pen.inc_y();
						surface_pen.set_alpha(x2f-x1f);
						for(int i=y1; i<y2; i++) // maybe draw pixels between
						{
							surface_pen.put_value(color);
							surface_pen.inc_y();
						}
						if (y2e!=0)	// maybe draw bottom pixel
						{
							surface_pen.set_alpha(y2e*(x2f-x1f));
							surface_pen.put_value(color);
						}
					}
				}
				else
				{
					// case 3 - a single horizontal row of pixels
					if(y2<y1)
					{
						surface_pen.move_to(x1-1,y2);
						if (x1e!=0)	// maybe draw left pixel
						{
							surface_pen.set_alpha(x1e*(y2f-y1f));
							surface_pen.put_value(color);
						}
						surface_pen.inc_x();
						surface_pen.set_alpha(y2f-y1f);
						for(int i=x1; i<x2; i++) // maybe draw pixels between
						{
							surface_pen.put_value(color);
							surface_pen.inc_x();
						}
						if (x2e!=0)	// maybe draw right pixel
						{
							surface_pen.set_alpha(x2e*(y2f-y1f));
							surface_pen.put_value(color);
						}
					}
					// case 4 - a proper block of pixels
					else
					{
						if (x1e!=0)	// maybe draw left edge
						{
							surface_pen.move_to(x1-1,y1-1);
							if (y1e!=0)	// maybe draw top left pixel
							{
								surface_pen.set_alpha(x1e*y1e);
								surface_pen.put_value(color);
							}
							surface_pen.inc_y();
							surface_pen.set_alpha(x1e);
							for(int i=y1; i<y2; i++) // maybe draw pixels along the left edge
							{
								surface_pen.put_value(color);
								surface_pen.inc_y();
							}
							if (y2e!=0)	// maybe draw bottom left pixel
							{
								surface_pen.set_alpha(x1e*y2e);
								surface_pen.put_value(color);
							}
							surface_pen.inc_x();
						}
						else
							surface_pen.move_to(x1,y2);
						
						if (y2e!=0)	// maybe draw bottom edge
						{
							surface_pen.set_alpha(y2e);
							for(int i=x1; i<x2; i++) // maybe draw pixels along the bottom edge
							{
								surface_pen.put_value(color);
								surface_pen.inc_x();
							}
							if (x2e!=0)	// maybe draw bottom right pixel
							{
								surface_pen.set_alpha(x2e*y2e);
								surface_pen.put_value(color);
							}
							surface_pen.dec_y();
						}
						else
							surface_pen.move_to(x2,y2-1);
						
						if (x2e!=0)	// maybe draw right edge
						{
							surface_pen.set_alpha(x2e);
							for(int i=y1; i<y2; i++) // maybe draw pixels along the right edge
							{
								surface_pen.put_value(color);
								surface_pen.dec_y();
							}
							if (y1e!=0)	// maybe draw top right pixel
							{
								surface_pen.set_alpha(x2e*y1e);
								surface_pen.put_value(color);
							}
							surface_pen.dec_x();
						}
						else
							surface_pen.move_to(x2-1,y1-1);
						
						if (y1e!=0)	// maybe draw top edge
						{
							surface_pen.set_alpha(y1e);
							for(int i=x1; i<x2; i++) // maybe draw pixels along the top edge
							{
								surface_pen.put_value(color);
								surface_pen.dec_x();
							}
						}
					}
				}
			}
			
			if (reverse)
			{
				if (--iter == particle_list.begin())
					break;
			}
			else
			{
				if (++iter == particle_list.end())
					break;
			}
		}
	}
}


///
void
Plant::draw_particles(cairo_t *cr)const
{
	Point origin=param_origin.get(Vector());
	Real size=param_size.get(Real());
	bool reverse=param_reverse.get(bool());
	bool size_as_alpha=param_size_as_alpha.get(bool());

	if (particle_list.begin() != particle_list.end())
	{
		std::vector<Particle>::iterator iter;
		Particle *particle;
		
		float radius(size);
		
		if (reverse)	iter = particle_list.end();
		else			iter = particle_list.begin();
		
		while (true)
		{
			if (reverse)	particle = &(*(iter-1));
			else			particle = &(*iter);
			
			float scaled_radius(radius);
			Color color(particle->color);
			if(size_as_alpha)
			{
				scaled_radius*=color.get_a();
				color.set_a(1);
			}
			
			// calculate the box that this particle will be drawn as
			const float x1f=particle->point[0]-scaled_radius*0.5;
			const float x2f=particle->point[0]+scaled_radius*0.5;
			const float y1f=particle->point[1]-scaled_radius*0.5;
			const float y2f=particle->point[1]+scaled_radius*0.5;
			const double width (x2f-x1f);
			const double height(y2f-y1f);
			
			// grab the color components
			const float r=color.clamped().get_r();
			const float g=color.clamped().get_g();
			const float b=color.clamped().get_b();
			const float a=color.clamped().get_a();
			
			cairo_save(cr);
			
			cairo_set_source_rgb(cr, r, g, b);
			cairo_translate(cr, origin[0], origin[1]);
			cairo_rectangle(cr, x1f, y1f, width, height);
			cairo_clip(cr);
			cairo_set_operator(cr, CAIRO_OPERATOR_SOURCE);
			cairo_paint_with_alpha(cr, a);
			
			cairo_restore(cr);
			
			if (reverse)
			{
				if (--iter == particle_list.begin())
					break;
			}
			else
			{
				if (++iter == particle_list.end())
					break;
			}
		}
	}
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
