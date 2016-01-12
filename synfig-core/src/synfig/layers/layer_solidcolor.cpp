/* === S Y N F I G ========================================================= */
/*!	\file layer_solidcolor.cpp
**	\brief Implementation of the "Solid Color" layer
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

#include "layer_solidcolor.h"

#include <synfig/general.h>
#include <synfig/localization.h>

#include <synfig/context.h>
#include <synfig/paramdesc.h>
#include <synfig/renddesc.h>
#include <synfig/time.h>
#include <synfig/string.h>
#include <synfig/surface.h>
#include <synfig/value.h>
#include <synfig/valuenode.h>

#endif

/* === U S I N G =========================================================== */

using namespace etl;
using namespace std;
using namespace synfig;

/* === G L O B A L S ======================================================= */

SYNFIG_LAYER_INIT(Layer_SolidColor);
SYNFIG_LAYER_SET_NAME(Layer_SolidColor,"SolidColor"); // todo: use solid_color
SYNFIG_LAYER_SET_LOCAL_NAME(Layer_SolidColor,N_("Solid Color"));
SYNFIG_LAYER_SET_CATEGORY(Layer_SolidColor,N_("Geometry"));
SYNFIG_LAYER_SET_VERSION(Layer_SolidColor,"0.1");
SYNFIG_LAYER_SET_CVS_ID(Layer_SolidColor,"$Id$");

/* === P R O C E D U R E S ================================================= */

/* === M E T H O D S ======================================================= */

/* === E N T R Y P O I N T ================================================= */

Layer_SolidColor::Layer_SolidColor():
	Layer_Composite(1.0,Color::BLEND_COMPOSITE),
	param_color(ValueBase(Color::black()))
{
	SET_INTERPOLATION_DEFAULTS();
	SET_STATIC_DEFAULTS();
}

bool
Layer_SolidColor::set_param(const String & param, const ValueBase &value)
{
	IMPORT_VALUE_PLUS(param_color,
		{
			Color color=param_color.get(Color());
			if (color.get_a() == 0)
			{
				if (converted_blend_)
				{
					set_blend_method(Color::BLEND_ALPHA_OVER);
					color.set_a(1);
					param_color.set(color);
				}
				else transparent_color_ = true;
			}
		}
		);

	return Layer_Composite::set_param(param,value);
}

ValueBase
Layer_SolidColor::get_param(const String &param)const
{
	EXPORT_VALUE(param_color);

	EXPORT_NAME();
	EXPORT_VERSION();

	return Layer_Composite::get_param(param);
}

Layer::Vocab
Layer_SolidColor::get_param_vocab()const
{
	Layer::Vocab ret(Layer_Composite::get_param_vocab());

	ret.push_back(ParamDesc("color")
		.set_local_name(_("Color"))
		.set_description(_("Fill color of the layer"))
	);

	return ret;
}

synfig::Layer::Handle
Layer_SolidColor::hit_check(synfig::Context context, const synfig::Point &point)const
{
	Color color=param_color.get(Color());
	if(get_blend_method()==Color::BLEND_STRAIGHT && get_amount()>=0.5)
		return const_cast<Layer_SolidColor*>(this);
	else
	if(get_blend_method()==Color::BLEND_COMPOSITE && get_amount()*color.get_a()>=0.5)
		return const_cast<Layer_SolidColor*>(this);

	Layer::Handle layer(context.hit_check(point));

	return layer?layer:const_cast<Layer_SolidColor*>(this);
}

Color
Layer_SolidColor::get_color(Context context, const Point &pos)const
{
	Color color=param_color.get(Color());
	if(get_amount()==1.0 && get_blend_method()==Color::BLEND_STRAIGHT)
		return color;
	else
		return Color::blend(color,context.get_color(pos),get_amount(),get_blend_method());
}

bool
Layer_SolidColor::accelerated_render(Context context,Surface *surface,int quality, const RendDesc &renddesc, ProgressCallback *cb)const
{
	Color color=param_color.get(Color());
	if(get_amount()==1.0 && get_blend_method()==Color::BLEND_STRAIGHT)
	{
		// Mark our progress as starting
		if(cb && !cb->amount_complete(0,1000))
			return false;

		surface->set_wh(renddesc.get_w(),renddesc.get_h());
		surface->fill(color);

		// Mark our progress as finished
		if(cb && !cb->amount_complete(1000,1000))
			return false;

		return true;
	}

	SuperCallback supercb(cb,0,9500,10000);

	if(!context.accelerated_render(surface,quality,renddesc,&supercb))
		return false;

	int x,y;

	Surface::alpha_pen apen(surface->begin());

	apen.set_value(color);
	apen.set_alpha(get_amount());
	apen.set_blend_method(get_blend_method());

	for(y=0;y<renddesc.get_h();y++,apen.inc_y(),apen.dec_x(x))
		for(x=0;x<renddesc.get_w();x++,apen.inc_x())
			apen.put_value();

	// Mark our progress as finished
	if(cb && !cb->amount_complete(10000,10000))
		return false;

	return true;
}

//////

bool
Layer_SolidColor::accelerated_cairorender(Context context, cairo_t *cr, int quality, const RendDesc &renddesc, ProgressCallback *cb)const
{
	Color color=param_color.get(Color());
	float r(color.get_r()),
	      g(color.get_g()),
		  b(color.get_b()),
		  a(color.get_a());
	
	if((get_amount()==1.f && get_blend_method()==Color::BLEND_STRAIGHT)
	   ||
	   (get_amount()==1.f && color.get_a()==1.f && get_blend_method()==Color::BLEND_COMPOSITE)
	   )
	{
		// Mark our progress as starting
		if(cb && !cb->amount_complete(0,1000))
			return false;
		cairo_save(cr);
		cairo_set_source_rgba(cr, r, g, b, a);
		cairo_set_operator(cr, CAIRO_OPERATOR_SOURCE);
		cairo_paint(cr);
		cairo_restore(cr);
		// Mark our progress as finished
		if(cb && !cb->amount_complete(1000,1000))
			return false;
		return true;
	}
	
	SuperCallback supercb(cb,0,9500,10000);
	
	if(!context.accelerated_cairorender(cr,quality,renddesc,&supercb))
		return false;
	
	cairo_save(cr);
	cairo_reset_clip(cr);
	cairo_set_source_rgba(cr, r, g, b, a);
	cairo_paint_with_alpha_operator(cr, get_amount(), get_blend_method());
	cairo_restore(cr);

	// Mark our progress as finished
	if(cb && !cb->amount_complete(10000,10000))
		return false;
	
	return true;
}

//////
