/* === S Y N F I G ========================================================= */
/*!	\file conicalgradient.cpp
**	\brief Implementation of the "Conical Gradient" layer
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
#include <synfig/angle.h>

#include "conicalgradient.h"

#endif

/* === U S I N G =========================================================== */

using namespace etl;
using namespace std;
using namespace synfig;

/* === G L O B A L S ======================================================= */

SYNFIG_LAYER_INIT(ConicalGradient);
SYNFIG_LAYER_SET_NAME(ConicalGradient,"conical_gradient");
SYNFIG_LAYER_SET_LOCAL_NAME(ConicalGradient,N_("Conical Gradient"));
SYNFIG_LAYER_SET_CATEGORY(ConicalGradient,N_("Gradients"));
SYNFIG_LAYER_SET_VERSION(ConicalGradient,"0.1");

/* === P R O C E D U R E S ================================================= */

/* === M E T H O D S ======================================================= */

/* === E N T R Y P O I N T ================================================= */

ConicalGradient::ConicalGradient():
	Layer_Composite(1.0,Color::BLEND_COMPOSITE),
	param_gradient(ValueBase(Gradient(Color::black(),Color::white()))),
	param_center(ValueBase(Point(0,0))),
	param_angle(ValueBase(Angle::zero())),
	param_symmetric(ValueBase(false))
{
	SET_INTERPOLATION_DEFAULTS();
	SET_STATIC_DEFAULTS();
}

bool
ConicalGradient::set_param(const String & param, const ValueBase &value)
{
	IMPORT_VALUE_PLUS(param_gradient, compile());
	IMPORT_VALUE(param_center);
	IMPORT_VALUE(param_angle);
	IMPORT_VALUE_PLUS(param_symmetric, compile());
	return Layer_Composite::set_param(param,value);
}

ValueBase
ConicalGradient::get_param(const String &param)const
{
	EXPORT_VALUE(param_gradient);
	EXPORT_VALUE(param_center);
	EXPORT_VALUE(param_angle);
	EXPORT_VALUE(param_symmetric);

	EXPORT_NAME();
	EXPORT_VERSION();

	return Layer_Composite::get_param(param);
}

Layer::Vocab
ConicalGradient::get_param_vocab()const
{
	Layer::Vocab ret(Layer_Composite::get_param_vocab());

	ret.push_back(ParamDesc("gradient")
		.set_local_name(_("Gradient"))
		.set_description(_("Gradient to apply"))
	);

	ret.push_back(ParamDesc("center")
		.set_local_name(_("Center"))
		.set_description(_("Center of the cone"))
	);

	ret.push_back(ParamDesc("angle")
		.set_local_name(_("Angle"))
		.set_origin("center")
		.set_description(_("Rotation of the gradient around the center"))
	);

	ret.push_back(ParamDesc("symmetric")
		.set_local_name(_("Symmetric"))
		.set_description(_("When checked, the gradient is looped"))
	);

	return ret;
}

void
ConicalGradient::compile()
{
	compiled_gradient.set(
		param_gradient.get(Gradient()),
		true,
		param_symmetric.get(bool()) );
}

inline Color
ConicalGradient::color_func(const Point &pos, Real supersample)const
{
	Point center = param_center.get(Point());
	Angle angle = param_angle.get(Angle());
	
	const Point centered(pos-center);
	Angle::rot a = Angle::tan(-centered[1],centered[0]).mod();
	a += angle;
	Real dist(a.mod().get());

	supersample *= 0.5;
	return compiled_gradient.average(dist - supersample, dist + supersample);
}

Real
ConicalGradient::calc_supersample(const synfig::Point &x, Real pw, Real ph)const
{
	Point center=param_center.get(Point());

	Point adj(x-center);
	if(abs(adj[0])<abs(pw*0.5) && abs(adj[1])<abs(ph*0.5))
		return 0.5;
	return (pw/Point(x-center).mag())/(PI*2);
}

synfig::Layer::Handle
ConicalGradient::hit_check(synfig::Context context, const synfig::Point &point)const
{
	if(get_blend_method()==Color::BLEND_STRAIGHT && get_amount()>=0.5)
		return const_cast<ConicalGradient*>(this);
	if(get_amount()==0.0)
		return context.hit_check(point);
	if((get_blend_method()==Color::BLEND_STRAIGHT || get_blend_method()==Color::BLEND_COMPOSITE) && color_func(point).get_a()>0.5)
		return const_cast<ConicalGradient*>(this);
	return context.hit_check(point);
}

Color
ConicalGradient::get_color(Context context, const Point &pos)const
{
	const Color color(color_func(pos));

	if(get_amount()==1.0 && get_blend_method()==Color::BLEND_STRAIGHT)
		return color;
	else
		return Color::blend(color,context.get_color(pos),get_amount(),get_blend_method());
}

bool
ConicalGradient::accelerated_render(Context context,Surface *surface,int quality, const RendDesc &renddesc, ProgressCallback *cb)const
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
		if(quality<9)
		{
			for(y=0,pos[1]=tl[1];y<h;y++,pen.inc_y(),pen.dec_x(x),pos[1]+=ph)
				for(x=0,pos[0]=tl[0];x<w;x++,pen.inc_x(),pos[0]+=pw)
					pen.put_value(color_func(pos,calc_supersample(pos,pw,ph)));
		}
		else
		{
			for(y=0,pos[1]=tl[1];y<h;y++,pen.inc_y(),pen.dec_x(x),pos[1]+=ph)
				for(x=0,pos[0]=tl[0];x<w;x++,pen.inc_x(),pos[0]+=pw)
					pen.put_value(color_func(pos,0));
		}
	}
	else
	{
		if(quality<9)
		{
			for(y=0,pos[1]=tl[1];y<h;y++,pen.inc_y(),pen.dec_x(x),pos[1]+=ph)
				for(x=0,pos[0]=tl[0];x<w;x++,pen.inc_x(),pos[0]+=pw)
					pen.put_value(Color::blend(color_func(pos,calc_supersample(pos,pw,ph)),pen.get_value(),get_amount(),get_blend_method()));
		}
		else
		{
			for(y=0,pos[1]=tl[1];y<h;y++,pen.inc_y(),pen.dec_x(x),pos[1]+=ph)
				for(x=0,pos[0]=tl[0];x<w;x++,pen.inc_x(),pos[0]+=pw)
					pen.put_value(Color::blend(color_func(pos,0),pen.get_value(),get_amount(),get_blend_method()));
		}
	}

	// Mark our progress as finished
	if(cb && !cb->amount_complete(10000,10000))
		return false;

	return true;
}

/////////
bool
ConicalGradient::accelerated_cairorender(Context context,cairo_t *cr,int quality, const RendDesc &renddesc, ProgressCallback *cb)const
{
	Gradient gradient=param_gradient.get(Gradient());
	Point center=param_center.get(Point());

	cairo_save(cr);
	const Point	tl(renddesc.get_tl());
	const Point br(renddesc.get_br());
	const Point tr(Point(tl[1], br[0]));
	const Point bl(Point(tl[0], br[1]));
		
	cairo_pattern_t* pattern=cairo_pattern_create_mesh();
	// Calculate the outer radius of the mesh pattern. It has to
	// cover the whole render desc
	Real c1=(tl-center).mag_squared();
	Real c2=(br-center).mag_squared();
	Real c3=(bl-center).mag_squared();
	Real c4=(tr-center).mag_squared();
	Real radius(max(max(max(c1,c2),c3),c4));
	radius=sqrt(radius)*1.20;

	bool cpoints_all_opaque=compile_mesh(pattern, gradient, radius);
	if(quality>8) cairo_set_antialias(cr, CAIRO_ANTIALIAS_NONE);
	else if(quality>=4) cairo_set_antialias(cr, CAIRO_ANTIALIAS_GOOD);
	else cairo_set_antialias(cr, CAIRO_ANTIALIAS_BEST);
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
	cairo_translate(cr, center[0], center[1]);
	cairo_set_source(cr, pattern);
	cairo_paint_with_alpha_operator(cr, get_amount(), get_blend_method());
	
	cairo_pattern_destroy(pattern); // Not needed more
	cairo_restore(cr);
	return true;
	
}
////////

bool
ConicalGradient::compile_mesh(cairo_pattern_t* pattern, Gradient mygradient, Real radius)const
{
	Angle angle=param_angle.get(Angle());
	bool symmetric=param_symmetric.get(bool());

	bool cpoints_all_opaque=true;
	float a1, r1, g1, b1, a2, r2, g2, b2;
	Gradient::CPoint cp;
	Gradient::const_iterator iter, iter2;
	mygradient.sort();
	// Handle symmetric conical gradients
	if(symmetric)
	{
		Gradient sgradient;
		for(iter=mygradient.begin();iter!=mygradient.end(); iter++)
		{
			cp=*iter;
			cp.pos=cp.pos/2;
			sgradient.push_back(cp);
		}
		for(iter=mygradient.begin();iter!=mygradient.end(); iter++)
		{
			cp=*iter;
			cp.pos=1.0-cp.pos/2;
			sgradient.push_back(cp);
		}
		mygradient=sgradient;
		mygradient.sort();
	}
	// Complete the gradient to be sure that always there is a color
	// stop at start and end of gradient.
	cp=*mygradient.begin();
	if(cp.pos!=0.0)
	{
		mygradient.push_back(GradientCPoint(0.0, cp.color));
		mygradient.sort();
	}
	cp=*(--mygradient.end());
	if(cp.pos!=1.0)
	{
		mygradient.push_back(GradientCPoint(1.0, cp.color));
		mygradient.sort();
	}
	mygradient.sort();
	
	// Add as many color stops as needed to be sure
	// that there is not a space >0.4 between color stops
	bool long_segment;
	do
	{
		long_segment=false;
		Gradient cgradient=mygradient;
		for(iter=cgradient.begin();iter!=cgradient.end(); iter++)
		{
			iter2=iter+1;
			if(iter2==cgradient.end()) break;
			Real pos1(iter->pos);
			Real pos2(iter2->pos);
			if(fabs(pos2-pos1)>=0.4)
			{
				long_segment=true;
				Real pos((pos1+pos2)/2.0);
				mygradient.push_back(GradientCPoint(pos, cgradient(pos)));
			}
		}
		mygradient.sort();
	} while (long_segment);
	
	mygradient.sort();
	//// Debug
	if(0)
	{
		int i = 0;
		for (Gradient::const_iterator iter = mygradient.begin(); iter != mygradient.end(); iter++)
			printf("%3d : %.3f %s\n", i++, (*iter).pos, (*iter).color.get_string().c_str());
	}
	////
	// Now insert the mesh patches
	Color c1, c2;
	Angle beta, beta1, beta2;
	Real t;
	Real v1x,v1y,t1x,t1y;
	Real v2x,v2y,t2x,t2y;
	for(iter=mygradient.begin();iter!=mygradient.end(); iter++)
	{
		iter2=iter+1;
		if(iter2==mygradient.end()) break;
		c1=iter->color;
		c2=iter2->color;
		if(iter->pos == iter2->pos) continue;
		beta1=(Angle::deg(-360.0*(iter->pos)))+angle;
		beta2=(Angle::deg(-360.0*(iter2->pos)))+angle;
		beta=beta2-beta1;
		t=(4 * ( (mygradient.size() == 3)
				? 1
				:((2 * Angle::cos((beta)/2).get() - Angle::cos(beta).get() - 1) / Angle::sin(beta).get())
				));
		v1x=(radius*Angle::cos(beta1).get());
		v1y=(radius*Angle::sin(beta1).get());
		v2x=(radius*Angle::cos(beta2).get());
		v2y=(radius*Angle::sin(beta2).get());
		t1x=(-radius*t*Angle::sin(beta1).get());
		t1y=(+radius*t*Angle::cos(beta1).get());
		t2x=(+radius*t*Angle::sin(beta2).get());
		t2y=(-radius*t*Angle::cos(beta2).get());
		a1=iter->color.get_a();
		r1=iter->color.get_r();
		g1=iter->color.get_g();
		b1=iter->color.get_b();
		a2=iter2->color.get_a();
		r2=iter2->color.get_r();
		g2=iter2->color.get_g();
		b2=iter2->color.get_b();
		// Do the patch!
		cairo_mesh_pattern_begin_patch(pattern);
		cairo_mesh_pattern_move_to(pattern, 0.0, 0.0);
		cairo_mesh_pattern_line_to(pattern, v1x, v1y);
		cairo_mesh_pattern_curve_to(pattern, v1x+t1x/3, v1y+t1y/3, v2x+t2x/3, v2y+t2y/3, v2x, v2y);
		cairo_mesh_pattern_line_to(pattern, 0.0, 0.0);
		cairo_mesh_pattern_line_to(pattern, 0.0, 0.0);
		cairo_mesh_pattern_set_corner_color_rgba(pattern, 0, r1, g1, b1, a1);
		cairo_mesh_pattern_set_corner_color_rgba(pattern, 1, r1, g1, b1, a1);
		cairo_mesh_pattern_set_corner_color_rgba(pattern, 2, r2, g2, b2, a2);
		cairo_mesh_pattern_set_corner_color_rgba(pattern, 3, r2, g2, b2, a2);
		cairo_mesh_pattern_end_patch(pattern);
		
		if(a1!=1.0 && a2!=0.0) cpoints_all_opaque=false;
	}
	return cpoints_all_opaque;
}
