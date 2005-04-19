/*! ========================================================================
** Synfig
** Template File
** $Id: lineargradient.cpp,v 1.1.1.1 2005/01/04 01:23:10 darco Exp $
**
** Copyright (c) 2002 Robert B. Quattlebaum Jr.
**
** This software and associated documentation
** are CONFIDENTIAL and PROPRIETARY property of
** the above-mentioned copyright holder.
**
** You may not copy, print, publish, or in any
** other way distribute this software without
** a prior written agreement with
** the copyright holder.
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
SYNFIG_LAYER_SET_LOCAL_NAME(LinearGradient,_("Linear Gradient"));
SYNFIG_LAYER_SET_CATEGORY(LinearGradient,_("Gradients"));
SYNFIG_LAYER_SET_VERSION(LinearGradient,"0.0");
SYNFIG_LAYER_SET_CVS_ID(LinearGradient,"$Id: lineargradient.cpp,v 1.1.1.1 2005/01/04 01:23:10 darco Exp $");

/* === P R O C E D U R E S ================================================= */

/* === M E T H O D S ======================================================= */

inline void
LinearGradient::sync()
{
	diff=(p2-p1);
	const Real mag(diff.inv_mag());
	diff*=mag*mag;
}


LinearGradient::LinearGradient():
	p1(1,1),
	p2(-1,-1),
	gradient(Color::black(), Color::white()),
	loop(false),
	zigzag(false)
{
	sync();
}

inline Color
LinearGradient::color_func(const Point &point, float supersample)const
{
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
LinearGradient::calc_supersample(const synfig::Point &x, float pw,float ph)const
{
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
	
	
	IMPORT(gradient);
	IMPORT(loop);
	IMPORT(zigzag);
	return Layer_Composite::set_param(param,value);	
}

ValueBase
LinearGradient::get_param(const String & param)const
{
	EXPORT(p1);
	EXPORT(p2);
	EXPORT(gradient);
	EXPORT(loop);
	EXPORT(zigzag);
	
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
	);
	ret.push_back(ParamDesc("p2")
		.set_local_name(_("Point 2"))
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
