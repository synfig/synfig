/* === S I N F G =========================================================== */
/*!	\file layer_solidcolor.cpp
**	\brief Template Header
**
**	$Id: layer_solidcolor.cpp,v 1.1.1.1 2005/01/04 01:23:14 darco Exp $
**
**	\legal
**	Copyright (c) 2002 Robert B. Quattlebaum Jr.
**
**	This software and associated documentation
**	are CONFIDENTIAL and PROPRIETARY property of
**	the above-mentioned copyright holder.
**
**	You may not copy, print, publish, or in any
**	other way distribute this software without
**	a prior written agreement with
**	the copyright holder.
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
#include "string.h"
#include "time.h"
#include "context.h"
#include "paramdesc.h"
#include "renddesc.h"
#include "surface.h"
#include "value.h"
#include "valuenode.h"

#endif

/* === U S I N G =========================================================== */

using namespace etl;
using namespace std;
using namespace sinfg;

/* === G L O B A L S ======================================================= */

SINFG_LAYER_INIT(Layer_SolidColor);
SINFG_LAYER_SET_NAME(Layer_SolidColor,"SolidColor");
SINFG_LAYER_SET_LOCAL_NAME(Layer_SolidColor,_("Solid Color"));
SINFG_LAYER_SET_CATEGORY(Layer_SolidColor,_("Geometry"));
SINFG_LAYER_SET_VERSION(Layer_SolidColor,"0.1");
SINFG_LAYER_SET_CVS_ID(Layer_SolidColor,"$Id: layer_solidcolor.cpp,v 1.1.1.1 2005/01/04 01:23:14 darco Exp $");

/* === P R O C E D U R E S ================================================= */

/* === M E T H O D S ======================================================= */

/* === E N T R Y P O I N T ================================================= */

Layer_SolidColor::Layer_SolidColor():
	Layer_Composite(1.0,Color::BLEND_STRAIGHT),
	color(Color::black())
{
}
	
bool
Layer_SolidColor::set_param(const String & param, const ValueBase &value)
{
	IMPORT(color);
	
	return Layer_Composite::set_param(param,value);
}

ValueBase
Layer_SolidColor::get_param(const String &param)const
{
	EXPORT(color);

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
	);
	
	return ret;
}

sinfg::Layer::Handle
Layer_SolidColor::hit_check(sinfg::Context context, const sinfg::Point &point)const
{
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
	if(get_amount()==1.0 && get_blend_method()==Color::BLEND_STRAIGHT)
		return color;
	else
		return Color::blend(color,context.get_color(pos),get_amount(),get_blend_method());
}
	
bool
Layer_SolidColor::accelerated_render(Context context,Surface *surface,int quality, const RendDesc &renddesc, ProgressCallback *cb)const
{
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
