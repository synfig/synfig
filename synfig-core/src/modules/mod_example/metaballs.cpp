/* === S Y N F I G ========================================================= */
/*!	\file metaballs.cpp
**	\brief Implementation of the "Metaballs" layer
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

#include <synfig/localization.h>

#include <synfig/string.h>
#include <synfig/context.h>
#include <synfig/paramdesc.h>
#include <synfig/renddesc.h>
#include <synfig/surface.h>
#include <synfig/value.h>

#include "metaballs.h"

#endif

/* === U S I N G =========================================================== */

using namespace etl;
using namespace std;
using namespace synfig;

/* === G L O B A L S ======================================================= */

SYNFIG_LAYER_INIT(Metaballs);
SYNFIG_LAYER_SET_NAME(Metaballs,"metaballs");
SYNFIG_LAYER_SET_LOCAL_NAME(Metaballs,N_("Metaballs"));
SYNFIG_LAYER_SET_CATEGORY(Metaballs,N_("Example"));
SYNFIG_LAYER_SET_VERSION(Metaballs,"0.1");

/* === P R O C E D U R E S ================================================= */

/* === M E T H O D S ======================================================= */

/* === E N T R Y P O I N T ================================================= */

Metaballs::Metaballs():
	Layer_Composite(1.0,Color::BLEND_COMPOSITE),
	param_gradient(ValueBase(Gradient(Color::black(), Color::white()))),
	param_centers(ValueBase(std::vector<synfig::Point>())),
	param_radii(ValueBase(std::vector<synfig::Real>())),
	param_weights(ValueBase(std::vector<synfig::Real>())),
	param_threshold(ValueBase(Real(0))),
	param_threshold2(ValueBase(Real(1))),
	param_positive(ValueBase(false))
{
	std::vector<synfig::Point> centers;
	std::vector<synfig::Real> radii;
	std::vector<synfig::Real> weights;
	centers.push_back(Point( 0, -1.5));	radii.push_back(2.5);	weights.push_back(1);
	centers.push_back(Point(-2,  1));	radii.push_back(2.5);	weights.push_back(1);
	centers.push_back(Point( 2,  1));	radii.push_back(2.5);	weights.push_back(1);
	param_centers.set_list_of(centers);
	param_radii.set_list_of(radii);
	param_weights.set_list_of(weights);
	
	SET_INTERPOLATION_DEFAULTS();
	SET_STATIC_DEFAULTS();
}

bool
Metaballs::set_param(const String & param, const ValueBase &value)
{
	IMPORT_VALUE(param_centers);
	IMPORT_VALUE(param_radii);
	IMPORT_VALUE(param_weights);
	IMPORT_VALUE(param_gradient);
	IMPORT_VALUE(param_threshold);
	IMPORT_VALUE(param_threshold2);
	IMPORT_VALUE(param_positive);

	return Layer_Composite::set_param(param,value);
}

ValueBase
Metaballs::get_param(const String &param)const
{
	EXPORT_VALUE(param_gradient);
	EXPORT_VALUE(param_radii);
	EXPORT_VALUE(param_weights);
	EXPORT_VALUE(param_centers);
	EXPORT_VALUE(param_threshold);
	EXPORT_VALUE(param_threshold2);
	EXPORT_VALUE(param_positive);

	EXPORT_NAME();
	EXPORT_VERSION();

	return Layer_Composite::get_param(param);
}

Layer::Vocab
Metaballs::get_param_vocab()const
{
	Layer::Vocab ret(Layer_Composite::get_param_vocab());

	ret.push_back(ParamDesc("gradient")
		.set_local_name(_("Gradient"))
	);

	ret.push_back(ParamDesc("centers")
		.set_local_name(_("Balls"))
	);

	ret.push_back(ParamDesc("radii")
		.set_local_name(_("Radii"))
	);

	ret.push_back(ParamDesc("weights")
		.set_local_name(_("Weights"))
	);

	ret.push_back(ParamDesc("threshold")
		.set_local_name(_("Gradient Left"))
	);

	ret.push_back(ParamDesc("threshold2")
		.set_local_name(_("Gradient Right"))
	);

	ret.push_back(ParamDesc("positive")
		.set_local_name(_("Positive Only"))
	);

	return ret;
}

synfig::Layer::Handle
Metaballs::hit_check(synfig::Context context, const synfig::Point &point)const
{
	Real density(totaldensity(point));

	if (density <= 0 || density > 1 || get_amount() == 0)
		return context.hit_check(point);

	synfig::Layer::Handle tmp;

	if (get_blend_method()==Color::BLEND_BEHIND && (tmp=context.hit_check(point)))
		return tmp;

	if (Color::is_onto(get_blend_method()) && !(context.hit_check(point)))
		return 0;

	return const_cast<Metaballs*>(this);
}

Real
Metaballs::densityfunc(const synfig::Point &p, const synfig::Point &c, Real R)const
{
	bool positive=param_positive.get(bool());
	
	const Real dx = p[0] - c[0];
	const Real dy = p[1] - c[1];

	const Real n = (1 - (dx*dx + dy*dy)/(R*R));
	if (positive && n < 0) return 0;
	return (n*n*n);

	/*
	f(d) = (1 - d^2)^3
	f'(d) = -6d * (1 - d^2)^2

	could use this too...
	f(d) = (1 - d^2)^2
	f'(d) = -6d * (1 - d^2)
	*/
}

Real
Metaballs::totaldensity(const Point &pos)const
{
	std::vector<synfig::Point> centers(param_centers.get_list_of(synfig::Point()));
	std::vector<synfig::Real> radii(param_radii.get_list_of(synfig::Real()));
	std::vector<synfig::Real> weights(param_weights.get_list_of(synfig::Real()));
	synfig::Real threshold=param_threshold.get(Real());
	synfig::Real threshold2=param_threshold2.get(Real());

	Real density = 0;

	//sum up weighted functions
	for(unsigned int i=0;i<centers.size();i++)
		density += weights[i] * densityfunc(pos,centers[i], radii[i]);

	return (density - threshold) / (threshold2 - threshold);
}

Color
Metaballs::get_color(Context context, const Point &pos)const
{
	Gradient gradient=param_gradient.get(Gradient());
	if(get_amount()==1.0 && get_blend_method()==Color::BLEND_STRAIGHT)
		return gradient(totaldensity(pos));
	else
		return Color::blend(gradient(totaldensity(pos)),context.get_color(pos),get_amount(),get_blend_method());
}

CairoColor
Metaballs::get_cairocolor(Context context, const Point &pos)const
{
	Gradient gradient=param_gradient.get(Gradient());
	if(get_amount()==1.0 && get_blend_method()==Color::BLEND_STRAIGHT)
		return CairoColor(gradient(totaldensity(pos)));
	else
		return CairoColor::blend(CairoColor(gradient(totaldensity(pos))),context.get_cairocolor(pos),get_amount(),get_blend_method());
}


bool
Metaballs::accelerated_render(Context context,Surface *surface,int quality, const RendDesc &renddesc, ProgressCallback *cb)const
{
	RENDER_TRANSFORMED_IF_NEED(__FILE__, __LINE__)

	Gradient gradient=param_gradient.get(Gradient());
	
	// Width and Height of a pixel
	const Point /*br(renddesc.get_br()),*/ tl(renddesc.get_tl());
	const int 	 w(renddesc.get_w()), 	h(renddesc.get_h());
	const Real	pw(renddesc.get_pw()), ph(renddesc.get_ph());

	SuperCallback supercb(cb,0,9000,10000);

	Point pos(tl[0],tl[1]);

	if(!context.accelerated_render(surface,quality,renddesc,&supercb))
	{
		if(cb)cb->error(strprintf(__FILE__"%d: Accelerated Renderer Failure",__LINE__));
		return false;
	}

	for(int y = 0; y < h; y++, pos[1] += ph)
	{
		pos[0] = tl[0];
		for(int x = 0; x < w; x++, pos[0] += pw)
			(*surface)[y][x] = Color::blend(gradient(totaldensity(pos)),(*surface)[y][x],get_amount(),get_blend_method());
	}

	// Mark our progress as finished
	if(cb && !cb->amount_complete(10000,10000))
		return false;

	return true;
}
