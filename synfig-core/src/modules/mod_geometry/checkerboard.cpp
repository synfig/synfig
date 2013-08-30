/* === S Y N F I G ========================================================= */
/*!	\file checkerboard.cpp
**	\brief Implementation of the "Checkerboard" layer
**
**	$Id$
**
**	\legal
**	Copyright (c) 2002-2005 Robert B. Quattlebaum Jr., Adrian Bentley
**	Copyright (c) 2007, 2008 Chris Moore
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

#include "checkerboard.h"

#include <synfig/string.h>
#include <synfig/time.h>
#include <synfig/context.h>
#include <synfig/paramdesc.h>
#include <synfig/renddesc.h>
#include <synfig/surface.h>
#include <synfig/value.h>
#include <synfig/valuenode.h>
#include <synfig/segment.h>

#endif

using namespace synfig;
using namespace std;
using namespace etl;

/* === M A C R O S ========================================================= */

/* === G L O B A L S ======================================================= */

SYNFIG_LAYER_INIT(CheckerBoard);
SYNFIG_LAYER_SET_NAME(CheckerBoard,"checker_board");
SYNFIG_LAYER_SET_LOCAL_NAME(CheckerBoard,N_("Checkerboard"));
SYNFIG_LAYER_SET_CATEGORY(CheckerBoard,N_("Geometry"));
SYNFIG_LAYER_SET_VERSION(CheckerBoard,"0.1");
SYNFIG_LAYER_SET_CVS_ID(CheckerBoard,"$Id$");

/* === P R O C E D U R E S ================================================= */

/* === M E T H O D S ======================================================= */

CheckerBoard::CheckerBoard():
	Layer_Composite	(1.0,Color::BLEND_COMPOSITE),
	param_color (ValueBase(Color::black())),
	param_origin (ValueBase(Point(0.125,0.125))),
	param_size (ValueBase(Point(0.25,0.25)))
{
	SET_INTERPOLATION_DEFAULTS();
	SET_STATIC_DEFAULTS();
}

inline bool
CheckerBoard::point_test(const synfig::Point& getpos)const
{
	Point origin=param_origin.get(Point());
	Point size=param_size.get(Point());
	
	int val=((int)((getpos[0]-origin[0])/size[0])+(int)((getpos[1]-origin[1])/size[1]));
	if(getpos[0]-origin[0] < 0.0)
		val++;
	if(getpos[1]-origin[1] < 0.0)
		val++;
	return val&1;
}

bool
CheckerBoard::set_param(const String &param, const ValueBase &value)
{
	IMPORT_VALUE_PLUS(param_color,
	  {
		  Color color(param_color.get(Color()));
		  if (color.get_a() == 0)
		  {
			  if(converted_blend_)
			  {
				  set_blend_method(Color::BLEND_ALPHA_OVER);
				  color.set_a(1);
				  param_color.set(color);
			  }
			  else
				  transparent_color_ = true;
		  }
	  }
	  );
	IMPORT_VALUE(param_origin);
	IMPORT_VALUE(param_size);

	if(param=="pos")
		return set_param("origin", value);

	for(int i=0;i<2;i++)
		if(param==strprintf("pos[%d]",i) && value.get_type()==ValueBase::TYPE_REAL)
		{
			Point p=param_origin.get(Point());
			p[i]=value.get(Real());
			param_origin.set(p);
			return true;
		}

	return Layer_Composite::set_param(param,value);
}

ValueBase
CheckerBoard::get_param(const String &param)const
{
	EXPORT_VALUE(param_color);
	EXPORT_VALUE(param_origin);
	EXPORT_VALUE(param_size);
	EXPORT_NAME();
	EXPORT_VERSION();

	return Layer_Composite::get_param(param);
}

Layer::Vocab
CheckerBoard::get_param_vocab()const
{
	Layer::Vocab ret(Layer_Composite::get_param_vocab());

	ret.push_back(ParamDesc("color")
		.set_local_name(_("Color"))
		.set_description(_("Color of checkers"))
	);
	ret.push_back(ParamDesc("origin")
		.set_local_name(_("Origin"))
		.set_description(_("Center of the checkers"))
	);
	ret.push_back(ParamDesc("size")
		.set_local_name(_("Size"))
		.set_description(_("Size of checkers"))
		.set_origin("origin")
	);

	return ret;
}

synfig::Layer::Handle
CheckerBoard::hit_check(synfig::Context context, const synfig::Point &getpos)const
{
	if(get_amount()!=0.0 && point_test(getpos))
	{
		synfig::Layer::Handle tmp;
		if(get_blend_method()==Color::BLEND_BEHIND && (tmp=context.hit_check(getpos)))
			return tmp;
		if(Color::is_onto(get_blend_method()) && !(tmp=context.hit_check(getpos)))
			return 0;
		return const_cast<CheckerBoard*>(this);
	}
	else
		return context.hit_check(getpos);
}

Color
CheckerBoard::get_color(Context context, const Point &getpos)const
{
	Color color=param_color.get(Color());

	if(get_amount()!=0.0 && point_test(getpos))
	{
		if(get_amount()==1.0 && get_blend_method()==Color::BLEND_STRAIGHT)
			return color;
		else
			return Color::blend(color,context.get_color(getpos),get_amount(),get_blend_method());
	}
	else
		return Color::blend(Color::alpha(),context.get_color(getpos),get_amount(),get_blend_method());
}

bool
CheckerBoard::accelerated_render(Context context,Surface *surface,int quality, const RendDesc &renddesc, ProgressCallback *cb)const
{
	Color color=param_color.get(Color());

	SuperCallback supercb(cb,0,9500,10000);

	if(!context.accelerated_render(surface,quality,renddesc,&supercb))
		return false;
	if(get_amount()==0)
		return true;

	int x,y;

	const Point tl(renddesc.get_tl());
	Point pos;
	const int w(surface->get_w());
	const int h(surface->get_h());
	const Real pw(renddesc.get_pw()),ph(renddesc.get_ph());

	Surface::alpha_pen apen(surface->begin());

	apen.set_alpha(get_amount());
	apen.set_blend_method(get_blend_method());
	apen.set_value(color);

	for(y=0,pos[1]=tl[1];y<h;y++,apen.inc_y(),apen.dec_x(x),pos[1]+=ph)
		for(x=0,pos[0]=tl[0];x<w;x++,apen.inc_x(),pos[0]+=pw)
			if(point_test(pos))
				apen.put_value();

	// Mark our progress as finished
	if(cb && !cb->amount_complete(10000,10000))
		return false;

	return true;
}

//////////
bool
CheckerBoard::accelerated_cairorender(Context context, cairo_t *cr, int quality, const RendDesc &renddesc, ProgressCallback *cb)const
{
	Color color=param_color.get(Color());
	Point origin=param_origin.get(Point());
	Point size=param_size.get(Point());

	SuperCallback supercb(cb,0,9500,10000);
	
	if(!is_solid_color())
		if(!context.accelerated_cairorender(cr,quality,renddesc,&supercb))
			return false;
	
	if(get_amount()==0)
		return true;
	
	const float r(color.get_r());
	const float g(color.get_g());
	const float b(color.get_b());
	const float a(color.get_a());
	
	const Point	tl(renddesc.get_tl());
	const Point br(renddesc.get_br());
	
	const int	w(renddesc.get_w());
	const int	h(renddesc.get_h());
	
	// Width and Height of a pixel
	const Real pw = (br[0] - tl[0]) / w;
	const Real ph = (br[1] - tl[1]) / h;
	
	// These are translation and scale values
	const double sx(1/pw);
	const double sy(1/ph);
	
	Point newsize(size);
	// Normalize the size
	if(newsize[0] <0.0) newsize[0]=-newsize[0];
	if(newsize[1] <0.0) newsize[1]=-newsize[1];
	// Calculate one average size that fits in a number integer of pixels
	newsize[0]=((int)(2.0*newsize[0]*sx))/(2.0*sx);
	newsize[1]=((int)(2.0*newsize[1]*sy))/(2.0*sy);
	
	if(!context.accelerated_cairorender(cr,quality,renddesc,cb))
	{
		if(cb)cb->error(strprintf(__FILE__"%d: Accelerated Cairo Renderer Failure",__LINE__));
		return false;
	}
	// Now let's render the minimum checkerboard in other surface
	// Initially I'll fill it completely with the alpha color
	// Create a similar image with the same dimensions than the minimum checkerboard
	RendDesc desc(renddesc);
	// this will modify the w and h values in pixels.
	desc.set_flags(RendDesc::PX_ASPECT|RendDesc::IM_SPAN);
	desc.set_tl_br(Point(-newsize[0], +newsize[1]), Point(+newsize[0], -newsize[1]));
	cairo_surface_t* subimage=cairo_surface_create_similar(cairo_get_target(cr), CAIRO_CONTENT_COLOR_ALPHA, desc.get_w(), desc.get_h());
	
	cairo_t* subcr=cairo_create(subimage);
	cairo_save(subcr);
	cairo_set_source_rgba(subcr, r, g, b, a);
	cairo_rectangle(subcr, 0, 0, desc.get_w()/2, desc.get_h()/2);
	cairo_clip(subcr);
	cairo_paint(subcr);
	cairo_restore(subcr);
	cairo_save(subcr);
	cairo_set_source_rgba(subcr, r, g, b, a);
	cairo_rectangle(subcr, desc.get_w()/2, desc.get_h()/2, desc.get_w()/2, desc.get_h()/2);
	cairo_clip(subcr);
	cairo_paint(subcr);
	cairo_restore(subcr);

	cairo_save(cr);
	cairo_translate(cr, origin[0], origin[1]);
	cairo_scale(cr, 1/sx, 1/sy);
	cairo_pattern_t* pattern=cairo_pattern_create_for_surface(subimage);

	cairo_pattern_set_extend (pattern, CAIRO_EXTEND_REPEAT);
	cairo_set_source(cr, pattern);
	if(is_solid_color())
	{
		cairo_set_operator(cr, CAIRO_OPERATOR_SOURCE);
		cairo_paint_with_alpha(cr, get_amount());
	}
	else
	{
		cairo_paint_with_alpha_operator(cr, get_amount(), get_blend_method());
	}
	cairo_restore(cr);

	cairo_pattern_destroy(pattern);
	
	// Mark our progress as finished
	if(cb && !cb->amount_complete(10000,10000))
		return false;
	
	return true;
}

//////////

