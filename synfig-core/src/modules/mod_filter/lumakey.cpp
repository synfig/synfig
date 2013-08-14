/* === S Y N F I G ========================================================= */
/*!	\file lumakey.cpp
**	\brief Implementation of the "Luma Key" layer
**
**	$Id$
**
**	\legal
**	Copyright (c) 2002-2005 Robert B. Quattlebaum Jr., Adrian Bentley
**	Copyright (c) 2007 Chris Moore
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

#include "lumakey.h"

#include <synfig/string.h>
#include <synfig/time.h>
#include <synfig/context.h>
#include <synfig/paramdesc.h>
#include <synfig/renddesc.h>
#include <synfig/surface.h>
#include <synfig/value.h>
#include <synfig/valuenode.h>
#include <synfig/segment.h>
#include <synfig/cairo_renddesc.h>

#endif

using namespace synfig;
using namespace std;
using namespace etl;

/* === M A C R O S ========================================================= */

/* === G L O B A L S ======================================================= */

SYNFIG_LAYER_INIT(LumaKey);
SYNFIG_LAYER_SET_NAME(LumaKey,"lumakey");
SYNFIG_LAYER_SET_LOCAL_NAME(LumaKey,N_("Luma Key"));
SYNFIG_LAYER_SET_CATEGORY(LumaKey,N_("Filters"));
SYNFIG_LAYER_SET_VERSION(LumaKey,"0.1");
SYNFIG_LAYER_SET_CVS_ID(LumaKey,"$Id$");

/* === P R O C E D U R E S ================================================= */

/* === M E T H O D S ======================================================= */

LumaKey::LumaKey():
	Layer_Composite	(1.0,Color::BLEND_STRAIGHT)
{
	set_blend_method(Color::BLEND_STRAIGHT);

}


bool
LumaKey::set_param(const String &param, const ValueBase &value)
{
	return Layer_Composite::set_param(param,value);
}

ValueBase
LumaKey::get_param(const String &param)const
{
	EXPORT_NAME();
	EXPORT_VERSION();

	return Layer_Composite::get_param(param);
}

Layer::Vocab
LumaKey::get_param_vocab()const
{
	Layer::Vocab ret(Layer_Composite::get_param_vocab());

/*	ret.push_back(ParamDesc("color")
		.set_local_name(_("Color"))
		.set_description(_("Color of checkers"))
	);
	ret.push_back(ParamDesc("pos")
		.set_local_name(_("Offset"))
	);
	ret.push_back(ParamDesc("size")
		.set_local_name(_("Size"))
		.set_description(_("Size of checkers"))
		.set_origin("pos")
	);
*/
	return ret;
}

synfig::Layer::Handle
LumaKey::hit_check(synfig::Context context, const synfig::Point &getpos)const
{
	return context.hit_check(getpos);
}

Color
LumaKey::get_color(Context context, const Point &getpos)const
{
	const Color color(context.get_color(getpos));

	if(get_amount()==0.0)
		return color;

	Color ret(color);
	ret.set_a(ret.get_y()*ret.get_a());
	ret.set_y(1);

	if(get_amount()==1.0 && get_blend_method()==Color::BLEND_STRAIGHT)
		return ret;

	return Color::blend(ret,color,get_amount(),get_blend_method());
}

bool
LumaKey::accelerated_render(Context context,Surface *surface,int quality, const RendDesc &renddesc, ProgressCallback *cb)const
{
	SuperCallback supercb(cb,0,9500,10000);

	if(!context.accelerated_render(surface,quality,renddesc,&supercb))
		return false;

	int x,y;

	Surface::pen pen(surface->begin());

	for(y=0;y<renddesc.get_h();y++,pen.inc_y(),pen.dec_x(x))
		for(x=0;x<renddesc.get_w();x++,pen.inc_x())
		{
			Color tmp(pen.get_value());
			tmp.set_a(tmp.get_y()*tmp.get_a());
			tmp.set_y(1);
			pen.put_value(tmp);
		}

	// Mark our progress as finished
	if(cb && !cb->amount_complete(10000,10000))
		return false;

	return true;
}

////
bool
LumaKey::accelerated_cairorender(Context context, cairo_t *cr, int quality, const RendDesc &renddesc_, ProgressCallback *cb)const
{
	RendDesc	renddesc(renddesc_);
	
	// Untransform the render desc
	if(!cairo_renddesc_untransform(cr, renddesc))
		return false;
	
	const Real pw(renddesc.get_pw()),ph(renddesc.get_ph());
	const Point tl(renddesc.get_tl());
	const int w(renddesc.get_w());
	const int h(renddesc.get_h());
	
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

	int x,y;
	
	CairoSurface cairosurface(surface);
	if(!cairosurface.map_cairo_image())
	{
		synfig::info("map cairo image failed");
		return false;
	}
	CairoSurface::pen pen(cairosurface.begin());
	
	for(y=0;y<h;y++,pen.inc_y(),pen.dec_x(x))
		for(x=0;x<w;x++,pen.inc_x())
		{
			Color tmp(Color(pen.get_value().demult_alpha()));
			tmp.set_a(tmp.get_y()*tmp.get_a());
			tmp.set_y(1);
			pen.put_value(CairoColor(tmp.clamped()).premult_alpha());
		}
	
	cairosurface.unmap_cairo_image();
	// paint surface on cr
	cairo_save(cr);
	cairo_translate(cr, tl[0], tl[1]);
	cairo_scale(cr, pw, ph);
	cairo_set_source_surface(cr, surface, 0, 0);
	cairo_set_operator(cr, CAIRO_OPERATOR_SOURCE);
	cairo_paint(cr);
	cairo_restore(cr);
	// Mark our progress as finished
	if(cb && !cb->amount_complete(10000,10000))
		return false;
	
	return true;
}

////


Rect
LumaKey::get_bounding_rect(Context context)const
{
	if(is_disabled())
		return Rect::zero();

	return context.get_full_bounding_rect();
}
