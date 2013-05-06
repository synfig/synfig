/* === S Y N F I G ========================================================= */
/*!	\file colorcorrect.cpp
**	\brief Implementation of the "Color Correct" layer
**
**	$Id$
**
**	\legal
**	Copyright (c) 2002-2005 Robert B. Quattlebaum Jr., Adrian Bentley
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

#include "colorcorrect.h"
#include <synfig/string.h>
#include <synfig/time.h>
#include <synfig/context.h>
#include <synfig/paramdesc.h>
#include <synfig/renddesc.h>
#include <synfig/surface.h>
#include <synfig/value.h>
#include <synfig/valuenode.h>

#endif

/* === U S I N G =========================================================== */

using namespace etl;
using namespace std;
using namespace synfig;

/* === G L O B A L S ======================================================= */

SYNFIG_LAYER_INIT(Layer_ColorCorrect);
SYNFIG_LAYER_SET_NAME(Layer_ColorCorrect,"colorcorrect");
SYNFIG_LAYER_SET_LOCAL_NAME(Layer_ColorCorrect,N_("Color Correct"));
SYNFIG_LAYER_SET_CATEGORY(Layer_ColorCorrect,N_("Filters"));
SYNFIG_LAYER_SET_VERSION(Layer_ColorCorrect,"0.1");
SYNFIG_LAYER_SET_CVS_ID(Layer_ColorCorrect,"$Id$");

/* === P R O C E D U R E S ================================================= */

/* === M E T H O D S ======================================================= */

/* === E N T R Y P O I N T ================================================= */

Layer_ColorCorrect::Layer_ColorCorrect():
	hue_adjust(Angle::zero()),
	brightness(0),
	contrast(1.0),
	exposure(0.0)
{
	Layer::Vocab voc(get_param_vocab());
	Layer::fill_static(voc);
}

inline Color
Layer_ColorCorrect::correct_color(const Color &in)const
{
	Color ret(in);
	Real brightness((this->brightness-0.5)*this->contrast+0.5);

	if(gamma.get_gamma_r()!=1.0)
	{
		if(ret.get_r() < 0)
		{
			ret.set_r(-gamma.r_F32_to_F32(-ret.get_r()));
		}else
		{
			ret.set_r(gamma.r_F32_to_F32(ret.get_r()));
		}
	}
	if(gamma.get_gamma_g()!=1.0)
	{
		if(ret.get_g() < 0)
		{
			ret.set_g(-gamma.g_F32_to_F32(-ret.get_g()));
		}else
		{
			ret.set_g(gamma.g_F32_to_F32(ret.get_g()));
		}
	}
	if(gamma.get_gamma_b()!=1.0)
	{
		if(ret.get_b() < 0)
		{
			ret.set_b(-gamma.b_F32_to_F32(-ret.get_b()));
		}else
		{
			ret.set_b(gamma.b_F32_to_F32(ret.get_b()));
		}
	}

	assert(!isnan(ret.get_r()));
	assert(!isnan(ret.get_g()));
	assert(!isnan(ret.get_b()));

	if(exposure!=0.0)
	{
		const float factor(exp(exposure));
		ret.set_r(ret.get_r()*factor);
		ret.set_g(ret.get_g()*factor);
		ret.set_b(ret.get_b()*factor);
	}

	// Adjust Contrast
	if(contrast!=1.0)
	{
		ret.set_r(ret.get_r()*contrast);
		ret.set_g(ret.get_g()*contrast);
		ret.set_b(ret.get_b()*contrast);
	}

	if(brightness)
	{
		// Adjust R Channel Brightness
		if(ret.get_r()>-brightness)
			ret.set_r(ret.get_r()+brightness);
		else if(ret.get_r()<brightness)
			ret.set_r(ret.get_r()-brightness);
		else
			ret.set_r(0);

		// Adjust G Channel Brightness
		if(ret.get_g()>-brightness)
			ret.set_g(ret.get_g()+brightness);
		else if(ret.get_g()<brightness)
			ret.set_g(ret.get_g()-brightness);
		else
			ret.set_g(0);

		// Adjust B Channel Brightness
		if(ret.get_b()>-brightness)
			ret.set_b(ret.get_b()+brightness);
		else if(ret.get_b()<brightness)
			ret.set_b(ret.get_b()-brightness);
		else
			ret.set_b(0);
	}

	// Return the color, adjusting the hue if necessary
	if(!!hue_adjust)
		return ret.rotate_uv(hue_adjust);
	else
		return ret;
}

bool
Layer_ColorCorrect::set_param(const String & param, const ValueBase &value)
{
	IMPORT(hue_adjust);
	IMPORT(brightness);
	IMPORT(contrast);
	IMPORT(exposure);

	if(param=="gamma" && value.get_type()==ValueBase::TYPE_REAL)
	{
		gamma.set_gamma(1.0/value.get(Real()));
		set_param_static(param, value.get_static());
		return true;
	}
	return false;
}

ValueBase
Layer_ColorCorrect::get_param(const String &param)const
{
	EXPORT(hue_adjust);
	EXPORT(brightness);
	EXPORT(contrast);
	EXPORT(exposure);

	if(param=="gamma")
	{
		ValueBase ret(1.0/gamma.get_gamma());
		ret.set_static(get_param_static(param));
		return ret;
	}

	EXPORT_NAME();
	EXPORT_VERSION();

	return ValueBase();
}

Layer::Vocab
Layer_ColorCorrect::get_param_vocab()const
{
	Layer::Vocab ret;

	ret.push_back(ParamDesc("hue_adjust")
		.set_local_name(_("Hue Adjust"))
	);

	ret.push_back(ParamDesc("brightness")
		.set_local_name(_("Brightness"))
	);

	ret.push_back(ParamDesc("contrast")
		.set_local_name(_("Contrast"))
	);

	ret.push_back(ParamDesc("exposure")
		.set_local_name(_("Exposure Adjust"))
	);

	ret.push_back(ParamDesc("gamma")
		.set_local_name(_("Gamma Adjustment"))
	);

	return ret;
}

Color
Layer_ColorCorrect::get_color(Context context, const Point &pos)const
{
	return correct_color(context.get_color(pos));
}

bool
Layer_ColorCorrect::accelerated_render(Context context,Surface *surface,int quality, const RendDesc &renddesc, ProgressCallback *cb)const
{
	SuperCallback supercb(cb,0,9500,10000);

	if(!context.accelerated_render(surface,quality,renddesc,&supercb))
		return false;

	int x,y;

	Surface::pen pen(surface->begin());

	for(y=0;y<renddesc.get_h();y++,pen.inc_y(),pen.dec_x(x))
		for(x=0;x<renddesc.get_w();x++,pen.inc_x())
			pen.put_value(correct_color(pen.get_value()));

	// Mark our progress as finished
	if(cb && !cb->amount_complete(10000,10000))
		return false;

	return true;
}

bool
Layer_ColorCorrect::accelerated_cairorender(Context context,cairo_surface_t *surface,int quality, const RendDesc &renddesc, ProgressCallback *cb)const
{
	SuperCallback supercb(cb,0,9500,10000);
	
	if(!context.accelerated_cairorender(surface,quality,renddesc,&supercb))
		return false;
	
	int x,y;
	
	CairoSurface csurface(surface);
	csurface.map_cairo_image();
	
	CairoSurface::pen pen(csurface.begin());
	
	for(y=0;y<renddesc.get_h();y++,pen.inc_y(),pen.dec_x(x))
		for(x=0;x<renddesc.get_w();x++,pen.inc_x())
			pen.put_value(CairoColor(correct_color(Color(pen.get_value().demult_alpha())).clamped()).premult_alpha());
	
	csurface.unmap_cairo_image();
	// Mark our progress as finished
	if(cb && !cb->amount_complete(10000,10000))
		return false;
	
	return true;
}


Rect
Layer_ColorCorrect::get_full_bounding_rect(Context context)const
{
	return context.get_full_bounding_rect();
}
