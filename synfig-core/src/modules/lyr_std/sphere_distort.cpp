/* === S Y N F I G ========================================================= */
/*!	\file sphere_distort.cpp
**	\brief Implementation of the "Spherize" layer
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

#include "sphere_distort.h"

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
#include <synfig/transform.h>
#include <synfig/cairo_renddesc.h>

#include <synfig/curve_helper.h>

#endif

/* === U S I N G =========================================================== */

using namespace std;
using namespace etl;
using namespace synfig;
using namespace modules;
using namespace lyr_std;

/* === M A C R O S ========================================================= */

#ifndef PI
const double PI = 3.14159265;
#endif

enum
{
	TYPE_NORMAL = 0,
	TYPE_DISTH = 1, //axe the horizontal axis
	TYPE_DISTV = 2, //axe the vertical axis
	N_TYPES
};

/* === G L O B A L S ======================================================= */

SYNFIG_LAYER_INIT(Layer_SphereDistort);
SYNFIG_LAYER_SET_NAME(Layer_SphereDistort,"spherize");
SYNFIG_LAYER_SET_LOCAL_NAME(Layer_SphereDistort,N_("Spherize"));
SYNFIG_LAYER_SET_CATEGORY(Layer_SphereDistort,N_("Distortions"));
SYNFIG_LAYER_SET_VERSION(Layer_SphereDistort,"0.2");
SYNFIG_LAYER_SET_CVS_ID(Layer_SphereDistort,"$Id$");

/* === P R O C E D U R E S ================================================= */

/* === M E T H O D S ======================================================= */

/* === E N T R Y P O I N T ================================================= */

Layer_SphereDistort::Layer_SphereDistort():
param_center(ValueBase(Vector(0,0))),
param_radius(ValueBase(double(1))),
param_amount(ValueBase(double(1))),
param_type(ValueBase(int(TYPE_NORMAL))),
param_clip(ValueBase(false))
{
	SET_INTERPOLATION_DEFAULTS();
	SET_STATIC_DEFAULTS();
}


bool
Layer_SphereDistort::set_param(const String & param, const ValueBase &value)
{
	IMPORT_VALUE_PLUS(param_center,sync());
	IMPORT_VALUE_PLUS(param_radius,sync());
	IMPORT_VALUE(param_type);
	IMPORT_VALUE(param_amount);
	IMPORT_VALUE(param_clip);

	if(param=="percent" && param_amount.get_type()==value.get_type())
		return set_param("amount", value);

	return Layer::set_param(param,value);
}

ValueBase
Layer_SphereDistort::get_param(const String &param)const
{
	EXPORT_VALUE(param_center);
	EXPORT_VALUE(param_radius);
	EXPORT_VALUE(param_type);
	EXPORT_VALUE(param_amount);
	EXPORT_VALUE(param_clip);
	if(param=="percent")
		return get_param("amount");

	EXPORT_NAME();
	EXPORT_VERSION();

	return Layer::get_param(param);
}

void
Layer_SphereDistort::sync()
{
// Remove me?
}

Layer::Vocab
Layer_SphereDistort::get_param_vocab()const
{
	Layer::Vocab ret;

	ret.push_back(ParamDesc("center")
		.set_local_name(_("Position"))
		.set_description(_("Where the sphere distortion is centered"))
	);

	ret.push_back(ParamDesc("radius")
		.set_local_name(_("Radius"))
		.set_origin("center")
		.set_is_distance()
		.set_description(_("The size of the sphere distortion"))
	);

	ret.push_back(ParamDesc("amount")
		.set_local_name(_("Amount"))
		.set_is_distance(false)
		.set_description(_("The distortion intensity (negative values inverts effect)"))
	);

	ret.push_back(ParamDesc("clip")
		.set_local_name(_("Clip"))
		.set_description(_("When checked, the area outside the Radius are not distorted"))
	);

	ret.push_back(ParamDesc("type")
		.set_local_name(_("Distort Type"))
		.set_description(_("The direction of the distortion"))
		.set_hint("enum")
		.add_enum_value(TYPE_NORMAL,"normal",_("Spherize"))
		.add_enum_value(TYPE_DISTH,"honly",_("Vertical Bar"))
		.add_enum_value(TYPE_DISTV,"vonly",_("Horizontal Bar"))
	);

	return ret;
}

/*
	Spherical Distortion: maps an image onto a ellipsoid of some sort

	so the image coordinate (i.e. distance away from the center)
	will determine how things get mapped

	so with the radius and position the mapping would go as follows

	r = (pos - center) / radius	clamped to [-1,1]

	if it's outside of that range then it's not distorted
	but if it's inside of that range then it goes as follows

	angle = r * pi/2 (-pi/2,pi/2)

	newr = cos(angle)*radius

	the inverse of this is (which is actually what we'd be transforming it from


*/

inline float spherify(float f)
{
	if(f > -1 && f < 1 && f!=0)
		return sinf(f*(PI/2));
	else return f;
}

inline float unspherify(float f)
{
	if(f > -1 && f < 1 && f!=0)
		return asin(f)/(PI/2);
	else return f;
}

Point sphtrans(const Point &p, const Point &center, const float &radius,
											const Real &percent, int type, bool& clipped)
{
	const Vector v = (p - center) / radius;

	Point newp = p;
	const float t = percent;

	clipped=false;

	if(type == TYPE_NORMAL)
	{
		const float m = v.mag();
		float lerp(0);

		if(m <= -1 || m >= 1)
		{
			clipped=true;
			return newp;
		}else
		if(m==0)
			return newp;
		else
		if(t > 0)
		{
			lerp = (t*unspherify(m) + (1-t)*m);
		}else if(t < 0)
		{
			lerp = ((1+t)*m - t*spherify(m));
		}else lerp = m;

		const float d = lerp*radius;
		newp = center + v*(d/m);
	}

	else if(type == TYPE_DISTH)
	{
		float lerp(0);
		if(v[0] <= -1 || v[0] >= 1)
		{
			clipped=true;
			return newp;
		}else
		if(v[0]==0)
			return newp;
		else
		if(t > 0)
		{
			lerp = (t*unspherify(v[0]) + (1-t)*v[0]);
		}else if(t < 0)
		{
			lerp = ((1+t)*v[0] - t*spherify(v[0]));
		}else lerp = v[0];

		newp[0] = center[0] + lerp*radius;
	}

	else if(type == TYPE_DISTV)
	{
		float lerp(0);
		if(v[1] <= -1 || v[1] >= 1)
		{
			clipped=true;
			return newp;
		}
		else
		if(v[1]==0)
			return newp;
		else
		if(t > 0)
		{
			lerp = (t*unspherify(v[1]) + (1-t)*v[1]);
		}else if(t < 0)
		{
			lerp = ((1+t)*v[1] - t*spherify(v[1]));
		}else lerp = v[1];

		newp[1] = center[1] + lerp*radius;
	}

	return newp;
}

inline Point sphtrans(const Point &p, const Point &center, const Real &radius,
											const Real &percent, int type)
{
	bool tmp;
	return sphtrans(p, center, radius, percent, type, tmp);
}

Layer::Handle
Layer_SphereDistort::hit_check(Context context, const Point &pos)const
{
	Vector center=param_center.get(Vector());
	double radius=param_radius.get(double());
	double percent=param_amount.get(double());
	int type=param_type.get(int());
	bool clip=param_clip.get(bool());
	
	bool clipped;
	Point point(sphtrans(pos,center,radius,percent,type,clipped));
	if(clip && clipped)
		return 0;
	return context.hit_check(point);
}

Color
Layer_SphereDistort::get_color(Context context, const Point &pos)const
{
	Vector center=param_center.get(Vector());
	double radius=param_radius.get(double());
	double percent=param_amount.get(double());
	int type=param_type.get(int());
	bool clip=param_clip.get(bool());

	bool clipped;
	Point point(sphtrans(pos,center,radius,percent,type,clipped));
	if(clip && clipped)
		return Color::alpha();
	return context.get_color(point);
}

RendDesc
Layer_SphereDistort::get_sub_renddesc_vfunc(const RendDesc &renddesc) const
{
	RendDesc desc(renddesc);
	Real pw = desc.get_pw();
	Real ph = desc.get_ph();
	desc.set_tl(Vector(-10.0, -10.0));
	desc.set_br(Vector( 10.0,  10.0));
	desc.set_wh(
		(int)approximate_ceil(fabs((desc.get_br()[0] - desc.get_tl()[0])/pw)),
		(int)approximate_ceil(fabs((desc.get_br()[1] - desc.get_tl()[1])/ph)) );
	return desc;
}

#if 1
bool
Layer_SphereDistort::accelerated_render(Context context,Surface *surface,int quality, const RendDesc &renddesc, ProgressCallback *cb)const
{
	RENDER_TRANSFORMED_IF_NEED(__FILE__, __LINE__)

	/*	Things to consider:
		1) Block expansion for distortion (ouch... quality level??)
		2) Bounding box clipping
		3) Super sampling for better visual quality (based on the quality level?)
		4) Interpolation type for sampling (based on quality level?)

		//things to defer until after
		super sampling, non-linear interpolation
	*/

	//bounding box reject
	Vector center=param_center.get(Vector());
	double radius=param_radius.get(double());
	double percent=param_amount.get(double());
	int type=param_type.get(int());
	bool clip=param_clip.get(bool());
	{
		Rect	sphr;

		sphr.set_point(center[0]-radius,center[1]-radius);
		sphr.expand(center[0]+radius,center[1]+radius);

		//get the bounding box of the transform
		Rect	windr;

		//and the bounding box of the rendering
		windr.set_point(renddesc.get_tl()[0],renddesc.get_tl()[1]);
		windr.expand(renddesc.get_br()[0],renddesc.get_br()[1]);

		//test bounding boxes for collision
		if( (type == TYPE_NORMAL && !intersect(sphr,windr)) ||
			(type == TYPE_DISTH && (sphr.minx >= windr.maxx || windr.minx >= sphr.maxx)) ||
			(type == TYPE_DISTV && (sphr.miny >= windr.maxy || windr.miny >= sphr.maxy)) )
		{
			//warning("Spherize: Bounding box reject");
			if (clip)
			{
				surface->set_wh(renddesc.get_w(), renddesc.get_h());
				surface->clear();
				return true;
			}
			else
				return context.accelerated_render(surface,quality,renddesc,cb);
		}

		//warning("Spherize: Bounding box accept");
	}

	//Ok, so we overlap some... now expand the window for rendering
	RendDesc r = renddesc;
	Surface background;
	Real pw = renddesc.get_pw(),ph = renddesc.get_ph();

	int nl=0,nt=0,nr=0,nb=0, nw=0,nh=0;
	Point tl = renddesc.get_tl(), br = renddesc.get_br();

	{
		//must enlarge window by pixel coordinates so go!

		//need to figure out closest and farthest point and distort THOSE

		Point origin[4] = {tl,tl,br,br};
		Vector v[4] = {Vector(0,br[1]-tl[1]),
					   Vector(br[0]-tl[0],0),
					   Vector(0,tl[1]-br[1]),
					   Vector(tl[0]-br[0],0)};

		Point close(0,0);
		Real t = 0;
		Rect	expandr(tl,br);

		//expandr.set_point(tl[0],tl[1]);
		//expandr.expand(br[0],br[1]);

		//warning("Spherize: Loop through lines and stuff");
		for(int i=0; i<4; ++i)
		{
			//warning("Spherize: 	%d", i);
			Vector p_o = center-origin[i];

			//project onto left line
			t = (p_o*v[i])/v[i].mag_squared();

			//clamp
			if (t < 0) t = 0;
			if (t > 1) t = 1;

			close = origin[i] + v[i]*t;

			//now get transforms and expand the rectangle to accommodate
			Point p = sphtrans(close,center,radius,percent,type);
			expandr.expand(p[0],p[1]);
			p = sphtrans(origin[i],center,radius,percent,type);
			expandr.expand(p[0],p[1]);
			p = sphtrans(origin[i]+v[i],center,radius,percent,type);
			expandr.expand(p[0],p[1]);
		}

		/*warning("Spherize: Bounding box (%f,%f)-(%f,%f)",
							expandr.minx,expandr.miny,expandr.maxx,expandr.maxy);*/

		//now that we have the bounding rectangle of ALL the pixels (should be...)
		//order it so that it's in the same orientation as the tl,br pair

		//warning("Spherize: Organize like tl,br");
		Point ntl(0,0),nbr(0,0);

		//sort x
		if(tl[0] < br[0])
		{
			ntl[0] = expandr.minx;
			nbr[0] = expandr.maxx;
		}
		else
		{
			ntl[0] = expandr.maxx;
			nbr[0] = expandr.minx;
		}

		//sort y
		if(tl[1] < br[1])
		{
			ntl[1] = expandr.miny;
			nbr[1] = expandr.maxy;
		}
		else
		{
			ntl[1] = expandr.maxy;
			nbr[1] = expandr.miny;
		}

		//now expand the window as needed
		Vector temp = ntl-tl;

		//pixel offset
		nl = (int)(temp[0]/pw)-1;
		nt = (int)(temp[1]/ph)-1;

		temp = nbr - br;
		nr = (int)(temp[0]/pw)+1;
		nb = (int)(temp[1]/ph)+1;

		nw = renddesc.get_w() + nr - nl;
		nh = renddesc.get_h() + nb - nt;

		//warning("Spherize: Setting subwindow (%d,%d) (%d,%d) (%d,%d)",nl,nt,nr,nb,nw,nh);
		r.set_subwindow(nl,nt,nw,nh);

		/*r = renddesc;
		nw = r.get_w(), nh = r.get_h();
		nl = 0, nt = 0;*/
	}

	//warning("Spherize: render background");
	if(!context.accelerated_render(&background,quality,r,cb))
	{
		warning("SphereDistort: Layer below failed");
		return false;
	}

	//now distort and check to make sure we aren't overshooting our bounds here
	int w = renddesc.get_w(), h = renddesc.get_h();
	surface->set_wh(w,h);

	Point sample = tl, sub = tl, trans(0,0);
	float xs = 0,ys = 0;
	int y=0,x=0;
	Real invpw = 1/pw, invph = 1/ph;
	Surface::pen	p = surface->begin();

	Point rtl = r.get_tl();

	//warning("Spherize: About to transform");

	for(y = 0; y < h; ++y, sample[1] += ph, p.inc_y())
	{
		sub = sample;
		for(x = 0; x < w; ++x, sub[0] += pw, p.inc_x())
		{
			bool clipped;
			trans=sphtrans(sub,center,radius,percent,type,clipped);
			if(clip && clipped)
			{
				p.put_value(Color::alpha());
				continue;
			}

			xs = (trans[0]-rtl[0])*invpw;
			ys = (trans[1]-rtl[1])*invph;

			if(!(xs >= 0 && xs < nw && ys >= 0 && ys < nh))
			{
				//warning("Spherize: we failed to account for %f,%f",xs,ys);
				p.put_value(context.get_color(trans));//Color::alpha());
				continue;
			}

			//sample at that pixel location based on the quality
			if(quality <= 4)	// cubic
				p.put_value(background.cubic_sample(xs,ys));
			else if(quality <= 5) // cosine
				p.put_value(background.cosine_sample(xs,ys));
			else if(quality <= 6) // linear
				p.put_value(background.linear_sample(xs,ys));
			else				// nearest
				p.put_value(background[round_to_int(ys)][round_to_int(xs)]);
		}
		p.dec_x(w);
	}

	return true;
}

////////
bool
Layer_SphereDistort::accelerated_cairorender(Context context, cairo_t *cr, int quality, const RendDesc &renddesc_, ProgressCallback *cb)const
{
	/*	Things to consider:
	 1) Block expansion for distortion (ouch... quality level??)
	 2) Bounding box clipping
	 3) Super sampling for better visual quality (based on the quality level?)
	 4) Interpolation type for sampling (based on quality level?)
	 
	 //things to defer until after
	 super sampling, non-linear interpolation
	 */
	
	Vector center=param_center.get(Vector());
	double radius=param_radius.get(double());
	double percent=param_amount.get(double());
	int type=param_type.get(int());
	bool clip=param_clip.get(bool());

	RendDesc	renddesc(renddesc_);
	
	// Untransform the render desc
	if(!cairo_renddesc_untransform(cr, renddesc))
		return false;
	
	//bounding box reject
	{
		Rect	sphr;
		
		sphr.set_point(center[0]-radius,center[1]-radius);
		sphr.expand(center[0]+radius,center[1]+radius);
		
		//get the bounding box of the transform
		Rect	windr;
		
		//and the bounding box of the rendering
		windr.set_point(renddesc.get_tl()[0],renddesc.get_tl()[1]);
		windr.expand(renddesc.get_br()[0],renddesc.get_br()[1]);
		
		//test bounding boxes for collision
		if( (type == TYPE_NORMAL && !intersect(sphr,windr)) ||
		   (type == TYPE_DISTH && (sphr.minx >= windr.maxx || windr.minx >= sphr.maxx)) ||
		   (type == TYPE_DISTV && (sphr.miny >= windr.maxy || windr.miny >= sphr.maxy)) )
		{
			if (clip)
			{
				cairo_set_operator(cr, CAIRO_OPERATOR_CLEAR);
				cairo_paint(cr);
				return true;
			}
			else
				return context.accelerated_cairorender(cr,quality,renddesc,cb);
		}
	}
	
	//Ok, so we overlap some... now expand the window for rendering
	RendDesc r = renddesc;
	cairo_surface_t* background, *result;
	int nl=0,nt=0,nr=0,nb=0, nw=0,nh=0;
	
	// grab the current renddesc interesting values
	const Real pw = renddesc.get_pw(),ph = renddesc.get_ph();
	const Point tl = renddesc.get_tl(), br = renddesc.get_br();
	const int w = renddesc.get_w(), h = renddesc.get_h();
	
	{
		//must enlarge window by pixel coordinates so go!
		
		//need to figure out closest and farthest point and distort THOSE
		
		Point origin[4] = {tl,tl,br,br};
		Vector v[4] = {Vector(0,br[1]-tl[1]),
			Vector(br[0]-tl[0],0),
			Vector(0,tl[1]-br[1]),
			Vector(tl[0]-br[0],0)};
		
		Point close(0,0);
		Real t = 0;
		Rect	expandr(tl,br);
		
		//expandr.set_point(tl[0],tl[1]);
		//expandr.expand(br[0],br[1]);
		
		for(int i=0; i<4; ++i)
		{
			Vector p_o = center-origin[i];
			
			//project onto left line
			t = (p_o*v[i])/v[i].mag_squared();
			
			//clamp
			if (t < 0) t = 0;
			if (t > 1) t = 1;
			
			close = origin[i] + v[i]*t;
			
			//now get transforms and expand the rectangle to accommodate
			Point p = sphtrans(close,center,radius,percent,type);
			expandr.expand(p[0],p[1]);
			p = sphtrans(origin[i],center,radius,percent,type);
			expandr.expand(p[0],p[1]);
			p = sphtrans(origin[i]+v[i],center,radius,percent,type);
			expandr.expand(p[0],p[1]);
		}
		
		//now that we have the bounding rectangle of ALL the pixels (should be...)
		//order it so that it's in the same orientation as the tl,br pair
		
		Point ntl(0,0),nbr(0,0);
		
		//sort x
		if(tl[0] < br[0])
		{
			ntl[0] = expandr.minx;
			nbr[0] = expandr.maxx;
		}
		else
		{
			ntl[0] = expandr.maxx;
			nbr[0] = expandr.minx;
		}
		
		//sort y
		if(tl[1] < br[1])
		{
			ntl[1] = expandr.miny;
			nbr[1] = expandr.maxy;
		}
		else
		{
			ntl[1] = expandr.maxy;
			nbr[1] = expandr.miny;
		}
		
		//now expand the window as needed
		Vector temp = ntl-tl;
		
		//pixel offset
		nl = (int)(temp[0]/pw)-1;
		nt = (int)(temp[1]/ph)-1;
		
		temp = nbr - br;
		nr = (int)(temp[0]/pw)+1;
		nb = (int)(temp[1]/ph)+1;
		
		nw = renddesc.get_w() + nr - nl;
		nh = renddesc.get_h() + nb - nt;
		
		r.set_subwindow(nl,nt,nw,nh);
	}
	// New values for expanded
	const double wpw =r.get_pw();
	const double wph =r.get_ph();
	const double wtlx=r.get_tl()[0];
	const double wtly=r.get_tl()[1];

	// now we know the sice of the needed background, create the cairo surface
	background=cairo_surface_create_similar(cairo_get_target(cr), CAIRO_CONTENT_COLOR_ALPHA, nw, nh);
	result=cairo_surface_create_similar(cairo_get_target(cr), CAIRO_CONTENT_COLOR_ALPHA, w, h);
	// render the background
	cairo_t* subcr=cairo_create(background);
	cairo_scale(subcr, 1/wpw, 1/wph);
	cairo_translate(subcr, -wtlx, -wtly);
	if(!context.accelerated_cairorender(subcr,quality,r,cb))
	{
		warning("Cairo SphereDistort: Layer below failed");
		return false;
	}
	cairo_destroy(subcr);
	//now distort and check to make sure we aren't overshooting our bounds here
	
	Point sample = tl, sub = tl, trans(0,0);
	float xs = 0,ys = 0;
	int y=0,x=0;
	Real invpw = 1/pw, invph = 1/ph;
	Point rtl = r.get_tl();
	
	CairoSurface cresult(result);
	if(!cresult.map_cairo_image())
	{
		warning("Sphere Distort: map cairo surface failed");
		return false;
	}
	CairoSurface cbackground(background);
	if(!cbackground.map_cairo_image())
	{
		warning("Sphere Distort: map cairo surface failed");
		return false;
	}
	
	for(y = 0; y < h; ++y, sample[1] += ph)
	{
		sub = sample;
		for(x = 0; x < w; ++x, sub[0] += pw)
		{
			bool clipped;
			trans=sphtrans(sub,center,radius,percent,type,clipped);
			if(clip && clipped)
			{
				cresult[y][x]=CairoColor::alpha();
				continue;
			}
			
			xs = (trans[0]-rtl[0])*invpw;
			ys = (trans[1]-rtl[1])*invph;
			
			if(!(xs >= 0 && xs < nw && ys >= 0 && ys < nh))
			{
				cresult[y][x]=context.get_cairocolor(trans).premult_alpha();
				continue;
			}
			
			//sample at that pixel location based on the quality
			if(quality <= 4)	// cubic
				cresult[y][x]=cbackground.cubic_sample_cooked(xs, ys).premult_alpha();
			else if(quality <= 5) // cosine
				cresult[y][x]=cbackground.cosine_sample_cooked(xs, ys).premult_alpha();
			else if(quality <= 6) // linear
				cresult[y][x]=cbackground.linear_sample_cooked(xs, ys).premult_alpha();
			else				// nearest
				cresult[y][x]=cbackground[round_to_int(ys)][round_to_int(xs)].premult_alpha();
		}
	}
	cresult.unmap_cairo_image();
	cbackground.unmap_cairo_image();
	
	cairo_surface_destroy(background);
	
	cairo_save(cr);
	
	cairo_translate(cr, tl[0], tl[1]);
	cairo_scale(cr, pw, ph);
	cairo_set_source_surface(cr, result, 0, 0);
	cairo_set_operator(cr, CAIRO_OPERATOR_SOURCE);
	cairo_paint(cr);
	
	cairo_restore(cr);
	cairo_surface_destroy(result);
	
	return true;
}

#endif

class lyr_std::Spherize_Trans : public Transform
{
	etl::handle<const Layer_SphereDistort> layer;
public:
	Spherize_Trans(const Layer_SphereDistort* x):Transform(x->get_guid()),layer(x) { }

	Vector perform(const Vector& x)const
	{
		return sphtrans(x,layer->param_center.get(Vector()),layer->param_radius.get(double()),-layer->param_amount.get(double()),layer->param_type.get(int()));
	}

	Vector unperform(const Vector& x)const
	{
		return sphtrans(x,layer->param_center.get(Vector()),layer->param_radius.get(double()),-layer->param_amount.get(double()),layer->param_type.get(int()));
	}

	String get_string()const
	{
		return "spheredistort";
	}
};

etl::handle<Transform>
Layer_SphereDistort::get_transform()const
{
	return new Spherize_Trans(this);
}

Rect
Layer_SphereDistort::get_bounding_rect()const
{
	Vector center=param_center.get(Vector());
	double radius=param_radius.get(double());
	int type=param_type.get(int());
	bool clip=param_clip.get(bool());

	Rect bounds(Rect::full_plane());

	if (clip)
		return bounds;

	switch(type)
	{
		case TYPE_NORMAL:
			bounds=Rect(center[0]+radius, center[1]+radius,
						center[0]-radius, center[1]-radius);
			break;
		case TYPE_DISTH:
			bounds = Rect::vertical_strip(center[0]-radius, center[0]+radius);
			break;
		case TYPE_DISTV:
			bounds = Rect::horizontal_strip(center[1]-radius, center[1]+radius);
			break;
		default:
			break;
	}

	return bounds;
}
