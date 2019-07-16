/* === S Y N F I G ========================================================= */
/*!	\file halftone3.cpp
**	\brief Implementation of the "Halftone 3" layer
**
**	$Id$
**
**	\legal
**	Copyright (c) 2002-2005 Robert B. Quattlebaum Jr., Adrian Bentley
**	Copyright (c) 2007-2008 Chris Moore
**	Copyright (c) 2012-2013 Carlos López
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

#include "halftone3.h"
#include "halftone.h"

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

#endif

/* === M A C R O S ========================================================= */

using namespace synfig;
using namespace std;
using namespace etl;

/* === G L O B A L S ======================================================= */

SYNFIG_LAYER_INIT(Halftone3);
SYNFIG_LAYER_SET_NAME(Halftone3,"halftone3");
SYNFIG_LAYER_SET_LOCAL_NAME(Halftone3,N_("Halftone 3"));
SYNFIG_LAYER_SET_CATEGORY(Halftone3,N_("Filters"));
SYNFIG_LAYER_SET_VERSION(Halftone3,"0.0");
SYNFIG_LAYER_SET_CVS_ID(Halftone3,"$Id$");

/* === P R O C E D U R E S ================================================= */

#define HALFSQRT2	(0.7)
#define SQRT2	(1.414213562f)

/* === M E T H O D S ======================================================= */

Halftone3::Halftone3():
Layer_CompositeFork(1.0,Color::BLEND_STRAIGHT)
{
	param_size=ValueBase(synfig::Vector(0.25,0.25));
	param_type=ValueBase(int(TYPE_SYMMETRIC));

	for(int i=0;i<3;i++)
	{
		tone[i].param_size=param_size;
		tone[i].param_type=param_type;
		tone[i].param_origin=ValueBase(synfig::Point(0,0));
		tone[i].param_angle=ValueBase(Angle::deg(30.0)*(float)i);
	}

	param_subtractive=ValueBase(true);

	if(param_subtractive.get(bool()))
	{
		param_color[0].set(Color::cyan());
		param_color[1].set(Color::magenta());
		param_color[2].set(Color::yellow());
	}
	else
	{
		param_color[0].set(Color::red());
		param_color[1].set(Color::green());
		param_color[2].set(Color::blue());
	}

	set_blend_method(Color::BLEND_STRAIGHT);

	for(int i=0;i<3;i++)
		for(int j=0;j<3;j++)
			inverse_matrix[i][j]=(j==i)?1.0f:0.0f;

	sync();
	
	SET_INTERPOLATION_DEFAULTS();
	SET_STATIC_DEFAULTS();
}

void
Halftone3::sync()
{
	bool subtractive=param_subtractive.get(bool());
	Color color[3];
	for(int i=0;i<3;i++)
		color[i]=param_color[i].get(Color());

	// Is this needed? set_param does the same!
	for(int i=0;i<3;i++)
	{
		tone[i].param_size=param_size;
		tone[i].param_type=param_type;
	}

#define matrix inverse_matrix
	//float matrix[3][3];
	if(subtractive)
	{
		for(int i=0;i<3;i++)
		{
			matrix[i][0]=1.0f-(color[i].get_r());
			matrix[i][1]=1.0f-(color[i].get_g());
			matrix[i][2]=1.0f-(color[i].get_b());
			float mult=sqrt(matrix[i][0]*matrix[i][0]+matrix[i][1]*matrix[i][1]+matrix[i][2]*matrix[i][2]);
			if(mult)
			{
				matrix[i][0]/=mult;
				matrix[i][1]/=mult;
				matrix[i][2]/=mult;
				matrix[i][0]/=mult;
				matrix[i][1]/=mult;
				matrix[i][2]/=mult;
			}
		}
	}
	else
	{
		for(int i=0;i<3;i++)
		{
			matrix[i][0]=color[i].get_r();
			matrix[i][1]=color[i].get_g();
			matrix[i][2]=color[i].get_b();
			float mult=sqrt(matrix[i][0]*matrix[i][0]+matrix[i][1]*matrix[i][1]+matrix[i][2]*matrix[i][2]);
			if(mult)
			{
				matrix[i][0]/=mult;
				matrix[i][1]/=mult;
				matrix[i][2]/=mult;
				matrix[i][0]/=mult;
				matrix[i][1]/=mult;
				matrix[i][2]/=mult;
			}
		}
	}
#undef matrix



#if 0
	// Insert guass-jordan elimination code here
	int k=0,i=0,j=0,z_size=3;
#define A inverse_matrix

	for (k=0;k<z_size;k++)
  // the pivot element
    { A[k][k]= -1/A[k][k];

  //the pivot column
     for (i=0;i<z_size;i++)
         if (i!=k) A[i][k]*=A[k][k];

 //elements not in a pivot row or column
     for (i=0;i<z_size;i++)
        if (i!=k)
            for (j=0;j<z_size;j++)
                      if (j!=k)
                          A[i][j]+=A[i][k]*A[k][j];

 //elements in a pivot row
    for (i=0;i<z_size;i++)
       if (i!=k)
            A[k][i]*=A[k][k];
   }

 //change sign
   for (i=0;i<z_size;i++)        /*reverse sign*/
     for (j=0;j<z_size;j++)
        A[i][j]=-A[i][j];
#undef A
#endif
}

inline Color
Halftone3::color_func(const Point &point, float supersample,const Color& in_color)const
{
	bool subtractive=param_subtractive.get(bool());
	Color color[3];
	for(int i=0;i<3;i++)
		color[i]=param_color[i].get(Color());

	Color halfcolor;

	float chan[3];


	if(subtractive)
	{
		chan[0]=inverse_matrix[0][0]*(1.0f-in_color.get_r())+inverse_matrix[0][1]*(1.0f-in_color.get_g())+inverse_matrix[0][2]*(1.0f-in_color.get_b());
		chan[1]=inverse_matrix[1][0]*(1.0f-in_color.get_r())+inverse_matrix[1][1]*(1.0f-in_color.get_g())+inverse_matrix[1][2]*(1.0f-in_color.get_b());
		chan[2]=inverse_matrix[2][0]*(1.0f-in_color.get_r())+inverse_matrix[2][1]*(1.0f-in_color.get_g())+inverse_matrix[2][2]*(1.0f-in_color.get_b());

		halfcolor=Color::white();
		halfcolor-=(~color[0])*tone[0](point,chan[0],supersample);
		halfcolor-=(~color[1])*tone[1](point,chan[1],supersample);
		halfcolor-=(~color[2])*tone[2](point,chan[2],supersample);

		halfcolor.set_a(in_color.get_a());
	}
	else
	{
		chan[0]=inverse_matrix[0][0]*in_color.get_r()+inverse_matrix[0][1]*in_color.get_g()+inverse_matrix[0][2]*in_color.get_b();
		chan[1]=inverse_matrix[1][0]*in_color.get_r()+inverse_matrix[1][1]*in_color.get_g()+inverse_matrix[1][2]*in_color.get_b();
		chan[2]=inverse_matrix[2][0]*in_color.get_r()+inverse_matrix[2][1]*in_color.get_g()+inverse_matrix[2][2]*in_color.get_b();

		halfcolor=Color::black();
		halfcolor+=color[0]*tone[0](point,chan[0],supersample);
		halfcolor+=color[1]*tone[1](point,chan[1],supersample);
		halfcolor+=color[2]*tone[2](point,chan[2],supersample);

		halfcolor.set_a(in_color.get_a());
	}

	return halfcolor;
}

inline float
Halftone3::calc_supersample(const synfig::Point &/*x*/, float pw,float /*ph*/)const
{
	return abs(pw/(tone[0].param_size.get(Vector())).mag());
}

synfig::Layer::Handle
Halftone3::hit_check(synfig::Context /*context*/, const synfig::Point &/*point*/)const
{
	return const_cast<Halftone3*>(this);
}

bool
Halftone3::set_param(const String & param, const ValueBase &value)
{
	IMPORT_VALUE_PLUS(param_size,
		{
			for(int i=0;i<3;i++)
				tone[i].param_size=param_size;

		});
	IMPORT_VALUE_PLUS(param_type,
		{
			for(int i=0;i<3;i++)
				tone[i].param_type=param_type;
		});

	IMPORT_VALUE_PLUS(param_color[0],sync());
	IMPORT_VALUE_PLUS(param_color[1],sync());
	IMPORT_VALUE_PLUS(param_color[2],sync());

	IMPORT_VALUE_PLUS(param_subtractive,sync());
	for(int i=0;i<3;i++)
		if (param==strprintf("tone[%d].angle",i)
			&& tone[i].param_angle.get_type()==value.get_type())
		{
			tone[i].param_angle=value;
			return true;
		}
	for(int i=0;i<3;i++)
		if ((param==strprintf("tone[%d].origin",i) || param==strprintf("tone[%d].offset",i))
			&& tone[i].param_origin.get_type()==value.get_type())
		{
			tone[i].param_origin=value;
			return true;
		}

	return Layer_Composite::set_param(param,value);
}

ValueBase
Halftone3::get_param(const String & param)const
{
	EXPORT_VALUE(param_size);
	EXPORT_VALUE(param_type);

	EXPORT_VALUE(param_color[0]);
	EXPORT_VALUE(param_color[1]);
	EXPORT_VALUE(param_color[2]);

	EXPORT_VALUE(param_subtractive);
	for(int i=0;i<3;i++)
		if (param==strprintf("tone[%d].angle",i))
			return tone[i].param_angle;
	for(int i=0;i<3;i++)
		if (param==strprintf("tone[%d].origin",i))
			return tone[i].param_origin;

	EXPORT_NAME();
	EXPORT_VERSION();

	return Layer_Composite::get_param(param);
}

Layer::Vocab
Halftone3::get_param_vocab()const
{
	Layer::Vocab ret(Layer_Composite::get_param_vocab());

	ret.push_back(ParamDesc("size")
		.set_local_name(_("Mask Size"))
	);
	ret.push_back(ParamDesc("type")
		.set_local_name(_(" Type"))
		.set_hint("enum")
		.add_enum_value(TYPE_SYMMETRIC,"symmetric",_("Symmetric"))
		.add_enum_value(TYPE_LIGHTONDARK,"lightondark",_("Light On Dark"))
		//.add_enum_value(TYPE_DARKONLIGHT,"darkonlight",_("Dark on Light"))
		.add_enum_value(TYPE_DIAMOND,"diamond",_("Diamond"))
		.add_enum_value(TYPE_STRIPE,"stripe",_("Stripe"))
	);
	ret.push_back(ParamDesc("subtractive")
		.set_local_name(_("Subtractive Flag"))
	);

	for(int i=0;i<3;i++)
	{
		String chan_name(strprintf("Chan%d",i));

		ret.push_back(ParamDesc(strprintf("color[%d]",i))
			.set_local_name(chan_name+_(" Color"))
		);

		ret.push_back(ParamDesc(strprintf("tone[%d].origin",i))
			.set_local_name(chan_name+_(" Mask Origin"))
			.set_is_distance()
		);
		ret.push_back(ParamDesc(strprintf("tone[%d].angle",i))
			.set_local_name(chan_name+_(" Mask Angle"))
			.set_origin(strprintf("tone[%d].origin",i))
		);
	}

	return ret;
}

Color
Halftone3::get_color(Context context, const Point &point)const
{
	const Color undercolor(context.get_color(point));
	const Color color(color_func(point,0,undercolor));

	if(get_amount()==1.0 && get_blend_method()==Color::BLEND_STRAIGHT)
		return color;
	else
		return Color::blend(color,undercolor,get_amount(),get_blend_method());
}

bool
Halftone3::accelerated_render(Context context,Surface *surface,int quality, const RendDesc &renddesc, ProgressCallback *cb)const
{
	RENDER_TRANSFORMED_IF_NEED(__FILE__, __LINE__)

	SuperCallback supercb(cb,0,9500,10000);

	if(!context.accelerated_render(surface,quality,renddesc,&supercb))
		return false;
	if(get_amount()==0)
		return true;

	const Real pw(renddesc.get_pw()),ph(renddesc.get_ph());
	const Point tl(renddesc.get_tl());
	const int w(surface->get_w());
	const int h(surface->get_h());
	const float supersample_size(abs(pw/(tone[0].param_size.get(Vector())).mag()));

	Surface::pen pen(surface->begin());
	Point pos;
	int x,y;

	if(is_solid_color())
	{
		for(y=0,pos[1]=tl[1];y<h;y++,pen.inc_y(),pen.dec_x(x),pos[1]+=ph)
			for(x=0,pos[0]=tl[0];x<w;x++,pen.inc_x(),pos[0]+=pw)
				pen.put_value(
					color_func(
						pos,
						supersample_size,
						pen.get_value()
					)
				);
	}
	else
	{
		for(y=0,pos[1]=tl[1];y<h;y++,pen.inc_y(),pen.dec_x(x),pos[1]+=ph)
			for(x=0,pos[0]=tl[0];x<w;x++,pen.inc_x(),pos[0]+=pw)
				pen.put_value(
					Color::blend(
				 		color_func(
							pos,
							supersample_size,
							pen.get_value()
						),
						pen.get_value(),
						get_amount(),
						get_blend_method()
					)
				);
	}

	// Mark our progress as finished
	if(cb && !cb->amount_complete(10000,10000))
		return false;

	return true;
}

////
////
bool
Halftone3::accelerated_cairorender(Context context, cairo_t *cr, int quality, const RendDesc &renddesc_, ProgressCallback *cb)const
{
	RendDesc	renddesc(renddesc_);
	
	// Untransform the render desc
	if(!cairo_renddesc_untransform(cr, renddesc))
		return false;
	
	const Real pw(renddesc.get_pw()),ph(renddesc.get_ph());
	const Point tl(renddesc.get_tl());
	const int w(renddesc.get_w());
	const int h(renddesc.get_h());
	const float supersample_size(abs(pw/(tone[0].param_size.get(Vector())).mag()));
	
	SuperCallback supercb(cb,0,9500,10000);
	
	if(get_amount()==0)
		return true;
	
	cairo_surface_t *surface;
	
	surface=cairo_surface_create_similar(cairo_get_target(cr), CAIRO_CONTENT_COLOR_ALPHA, w, h);
	cairo_t* subcr=cairo_create(surface);
	cairo_scale(subcr, 1/pw, 1/ph);
	cairo_translate(subcr, -tl[0], -tl[1]);
	if(!context.accelerated_cairorender(subcr,quality,renddesc,&supercb))
	{
		if(cb)cb->error(strprintf(__FILE__"%d: Accelerated Cairo Renderer Failure",__LINE__));
		return false;
	}
	cairo_destroy(subcr);
	
	CairoSurface csurface(surface);
	
	if(!csurface.map_cairo_image())
		return false;
	
	CairoSurface::pen pen(csurface.begin());
	
	Point pos;
	int x,y;
	
	if(is_solid_color())
	{
		for(y=0,pos[1]=tl[1];y<h;y++,pen.inc_y(),pen.dec_x(x),pos[1]+=ph)
			for(x=0,pos[0]=tl[0];x<w;x++,pen.inc_x(),pos[0]+=pw)
				pen.put_value(
							  CairoColor(color_func(
													pos,
													supersample_size,
													Color(pen.get_value().demult_alpha())
													)
										 ).premult_alpha()
							  );
	}
	else
	{
		for(y=0,pos[1]=tl[1];y<h;y++,pen.inc_y(),pen.dec_x(x),pos[1]+=ph)
			for(x=0,pos[0]=tl[0];x<w;x++,pen.inc_x(),pos[0]+=pw)
			{
				Color val=Color(pen.get_value().demult_alpha());
				pen.put_value(
							  CairoColor(Color::blend(
													  color_func(
																 pos,
																 supersample_size,
																 val
																 ),
													  val,
													  get_amount(),
													  get_blend_method()
													  ).clamped()
										 ).premult_alpha()
							  );
			}
		
	}
	
	csurface.unmap_cairo_image();
	// paint surface on cr
	cairo_save(cr);
	cairo_translate(cr, tl[0], tl[1]);
	cairo_scale(cr, pw, ph);
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

rendering::Task::Handle
Halftone3::build_rendering_task_vfunc(Context context) const
	{ return Layer::build_rendering_task_vfunc(context); }

////
