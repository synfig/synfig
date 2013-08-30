/* === S Y N F I G ========================================================= */
/*!	\file lineargradient.cpp
**	\brief Implementation of the "Linear Gradient" layer
**
**	$Id$
**
**	\legal
**	Copyright (c) 2002-2005 Robert B. Quattlebaum Jr., Adrian Bentley
**	Copyright (c) 2011-2013 Carlos López
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

#include "lineargradient.h"

#include <synfig/string.h>
#include <synfig/time.h>
#include <synfig/context.h>
#include <synfig/paramdesc.h>
#include <synfig/renddesc.h>
#include <synfig/surface.h>
#include <synfig/value.h>
#include <synfig/valuenode.h>

#endif

/* === M A C R O S ========================================================= */

/* === G L O B A L S ======================================================= */

SYNFIG_LAYER_INIT(LinearGradient);
SYNFIG_LAYER_SET_NAME(LinearGradient,"linear_gradient");
SYNFIG_LAYER_SET_LOCAL_NAME(LinearGradient,N_("Linear Gradient"));
SYNFIG_LAYER_SET_CATEGORY(LinearGradient,N_("Gradients"));
SYNFIG_LAYER_SET_VERSION(LinearGradient,"0.0");
SYNFIG_LAYER_SET_CVS_ID(LinearGradient,"$Id$");

/* === P R O C E D U R E S ================================================= */

/* === M E T H O D S ======================================================= */

inline void
LinearGradient::sync()
{
	Point p1=param_p1.get(Point());
	Point p2=param_p2.get(Point());
	
	diff=(p2-p1);
	const Real mag(diff.inv_mag());
	diff*=mag*mag;
}


LinearGradient::LinearGradient():
	Layer_Composite(1.0,Color::BLEND_COMPOSITE),
	param_p1(ValueBase(Point(1,1))),
	param_p2(ValueBase(Point(-1,-1))),
	param_gradient(ValueBase(Gradient(Color::black(), Color::white()))),
	param_loop(ValueBase(false)),
	param_zigzag(ValueBase(false))
{
	sync();
	SET_INTERPOLATION_DEFAULTS();
	SET_STATIC_DEFAULTS();
}

inline Color
LinearGradient::color_func(const Point &point, float supersample)const
{
	Point p1=param_p1.get(Point());
	Gradient gradient=param_gradient.get(Gradient());
	bool loop=param_loop.get(bool());
	bool zigzag=param_zigzag.get(bool());
	
	Real dist(point*diff-p1*diff);

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
			float  left(supersample*0.5-(dist-1.0));
			float right(supersample*0.5+(dist-1.0));
			Color pool(gradient(1.0-(left*0.5),left).premult_alpha()*left/supersample);
			if (zigzag) pool+=gradient(1.0-right*0.5,right).premult_alpha()*right/supersample;
			else		pool+=gradient(right*0.5,right).premult_alpha()*right/supersample;
			return pool.demult_alpha();
		}
		if(dist-supersample*0.5<0.0)
		{
			float  left(supersample*0.5-dist);
			float right(supersample*0.5+dist);
			Color pool(gradient(right*0.5,right).premult_alpha()*right/supersample);
			if (zigzag) pool+=gradient(left*0.5,left).premult_alpha()*left/supersample;
			else		pool+=gradient(1.0-left*0.5,left).premult_alpha()*left/supersample;
			return pool.demult_alpha();
		}
	}
	return gradient(dist,supersample);
}

float
LinearGradient::calc_supersample(const synfig::Point &/*x*/, float pw,float /*ph*/)const
{
	Point p1=param_p1.get(Point());
	Point p2=param_p2.get(Point());

	return pw/(p2-p1).mag();
}

synfig::Layer::Handle
LinearGradient::hit_check(synfig::Context context, const synfig::Point &point)const
{
	if(get_blend_method()==Color::BLEND_STRAIGHT && get_amount()>=0.5)
		return const_cast<LinearGradient*>(this);
	if(get_amount()==0.0)
		return context.hit_check(point);
	if((get_blend_method()==Color::BLEND_STRAIGHT || get_blend_method()==Color::BLEND_COMPOSITE) && color_func(point).get_a()>0.5)
		return const_cast<LinearGradient*>(this);
	return context.hit_check(point);
}

bool
LinearGradient::set_param(const String & param, const ValueBase &value)
{
	IMPORT_VALUE_PLUS(param_p1,sync());
	IMPORT_VALUE_PLUS(param_p2,sync());
	IMPORT_VALUE(param_gradient);
	IMPORT_VALUE(param_loop);
	IMPORT_VALUE(param_zigzag);
	return Layer_Composite::set_param(param,value);
}

ValueBase
LinearGradient::get_param(const String & param)const
{
	EXPORT_VALUE(param_p1);
	EXPORT_VALUE(param_p2);
	EXPORT_VALUE(param_gradient);
	EXPORT_VALUE(param_loop);
	EXPORT_VALUE(param_zigzag);

	EXPORT_NAME();
	EXPORT_VERSION();

	return Layer_Composite::get_param(param);
}

Layer::Vocab
LinearGradient::get_param_vocab()const
{
	Layer::Vocab ret(Layer_Composite::get_param_vocab());

	ret.push_back(ParamDesc("p1")
		.set_local_name(_("Point 1"))
		.set_connect("p2")
		.set_description(_("Start point of the gradient"))
	);
	ret.push_back(ParamDesc("p2")
		.set_local_name(_("Point 2"))
		.set_description(_("End point of the gradient"))
	);
	ret.push_back(ParamDesc("gradient")
		.set_local_name(_("Gradient"))
		.set_description(_("Gradient to apply"))
	);
	ret.push_back(ParamDesc("loop")
		.set_local_name(_("Loop"))
		.set_description(_("When checked the gradient is looped"))
	);
	ret.push_back(ParamDesc("zigzag")
		.set_local_name(_("ZigZag"))
		.set_description(_("When checked the gradient is summetrical at the center"))
	);

	return ret;
}

Color
LinearGradient::get_color(Context context, const Point &point)const
{
	const Color color(color_func(point));

	if(get_amount()==1.0 && get_blend_method()==Color::BLEND_STRAIGHT)
		return color;
	else
		return Color::blend(color,context.get_color(point),get_amount(),get_blend_method());
}

bool
LinearGradient::accelerated_render(Context context,Surface *surface,int quality, const RendDesc &renddesc, ProgressCallback *cb)const
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
				pen.put_value(color_func(pos,calc_supersample(pos,pw,ph)));
	}
	else
	{
		for(y=0,pos[1]=tl[1];y<h;y++,pen.inc_y(),pen.dec_x(x),pos[1]+=ph)
			for(x=0,pos[0]=tl[0];x<w;x++,pen.inc_x(),pos[0]+=pw)
				pen.put_value(Color::blend(color_func(pos,calc_supersample(pos,pw,ph)),pen.get_value(),get_amount(),get_blend_method()));
	}

	// Mark our progress as finished
	if(cb && !cb->amount_complete(10000,10000))
		return false;

	return true;
}


bool
LinearGradient::accelerated_cairorender(Context context, cairo_t *cr, int quality, const RendDesc &renddesc, ProgressCallback *cb)const
{
	bool loop=param_loop.get(bool());
	Point p1=param_p1.get(Point());
	Point p2=param_p2.get(Point());
	Gradient gradient=param_gradient.get(Gradient());

	cairo_save(cr);
	cairo_pattern_t* pattern=cairo_pattern_create_linear(p1[0], p1[1], p2[0], p2[1]);
	bool cpoints_all_opaque=compile_gradient(pattern, gradient);
	if(loop)
		cairo_pattern_set_extend(pattern, CAIRO_EXTEND_REPEAT);
	if(quality>8) cairo_pattern_set_filter(pattern, CAIRO_FILTER_FAST);
	else if(quality>=4) cairo_pattern_set_filter(pattern, CAIRO_FILTER_GOOD);
	else cairo_pattern_set_filter(pattern, CAIRO_FILTER_BEST);
	if(
	   !
	   (is_solid_color() ||
		(cpoints_all_opaque && get_blend_method()==Color::BLEND_COMPOSITE && get_amount()==1.f))
	   )
	{
		// Initially render what's behind us
		if(!context.accelerated_cairorender(cr,quality,renddesc,cb))
		{
			if(cb)cb->error(strprintf(__FILE__"%d: Accelerated Cairo Renderer Failure",__LINE__));
			return false;
		}
	}
	cairo_set_source(cr, pattern);
	cairo_paint_with_alpha_operator(cr, get_amount(), get_blend_method());
	
	cairo_pattern_destroy(pattern); // Not needed more
	cairo_restore(cr);
	return true;
}



bool
LinearGradient::compile_gradient(cairo_pattern_t* pattern, Gradient mygradient)const
{
	bool loop=param_loop.get(bool());
	bool zigzag=param_zigzag.get(bool());

	bool cpoints_all_opaque=true;
	float a,r,g,b;
	Gradient::CPoint cp;
	Gradient::const_iterator iter;
	mygradient.sort();
	if(zigzag)
	{
		Gradient zgradient;
		for(iter=mygradient.begin();iter!=mygradient.end(); iter++)
		{
			cp=*iter;
			cp.pos=cp.pos/2;
			zgradient.push_back(cp);
		}
		for(iter=mygradient.begin();iter!=mygradient.end(); iter++)
		{
			cp=*iter;
			cp.pos=1.0-cp.pos/2;
			zgradient.push_back(cp);
		}
		mygradient=zgradient;
	}
	mygradient.sort();
	if(loop)
	{
		cp=*mygradient.begin();
		a=cp.color.get_a();
		r=cp.color.get_r();
		g=cp.color.get_g();
		b=cp.color.get_b();
		cairo_pattern_add_color_stop_rgba(pattern, 0.0, r, g, b, a);
	}
	for(iter=mygradient.begin();iter!=mygradient.end(); iter++)
	{
		cp=*iter;
		a=cp.color.get_a();
		r=cp.color.get_r();
		g=cp.color.get_g();
		b=cp.color.get_b();
		cairo_pattern_add_color_stop_rgba(pattern, cp.pos, r, g, b, a);
		if(a!=1.0) cpoints_all_opaque=false;
	}
	if(loop)
	{
		cp=*(--mygradient.end());
		a=cp.color.get_a();
		r=cp.color.get_r();
		g=cp.color.get_g();
		b=cp.color.get_b();
		cairo_pattern_add_color_stop_rgba(pattern, 1.0, r, g, b, a);
	}
	return cpoints_all_opaque;
}
