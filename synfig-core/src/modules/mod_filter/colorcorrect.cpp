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

#include <synfig/localization.h>
#include <synfig/general.h>

#include "colorcorrect.h"
#include <synfig/string.h>
#include <synfig/time.h>
#include <synfig/context.h>
#include <synfig/paramdesc.h>
#include <synfig/renddesc.h>
#include <synfig/surface.h>
#include <synfig/value.h>
#include <synfig/valuenode.h>
#include <synfig/cairo_renddesc.h>

#include <synfig/rendering/common/task/taskpixelprocessor.h>

#endif

/* === U S I N G =========================================================== */

using namespace etl;
using namespace std;
using namespace synfig;
using namespace modules;
using namespace mod_filter;

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
	param_hue_adjust(ValueBase(Angle::zero())),
	param_brightness(ValueBase(Real(0))),
	param_contrast(ValueBase(Real(1.0))),
	param_exposure(ValueBase(Real(0.0))),
	param_gamma(ValueBase(Real(1.0)))
{
	SET_INTERPOLATION_DEFAULTS();
	SET_STATIC_DEFAULTS();
}

inline Color
Layer_ColorCorrect::correct_color(const Color &in)const
{
	Angle hue_adjust=param_hue_adjust.get(Angle());
	Real _brightness=param_brightness.get(Real());
	Real contrast=param_contrast.get(Real());
	Real exposure=param_exposure.get(Real());
	
	Color ret(in);
	Real brightness((_brightness-0.5)*contrast+0.5);

	if(gamma.get_gamma_r()!=1.0) {
		if(ret.get_r() < 0)
			ret.set_r(-gamma.r_F32_to_F32(-ret.get_r()));
		else
			ret.set_r(gamma.r_F32_to_F32(ret.get_r()));
	}
	if(gamma.get_gamma_g()!=1.0) {
		if(ret.get_g() < 0)
			ret.set_g(-gamma.g_F32_to_F32(-ret.get_g()));
		else
			ret.set_g(gamma.g_F32_to_F32(ret.get_g()));
	}
	if(gamma.get_gamma_b()!=1.0) {
		if(ret.get_b() < 0)
			ret.set_b(-gamma.b_F32_to_F32(-ret.get_b()));
		else
			ret.set_b(gamma.b_F32_to_F32(ret.get_b()));
	}
	if(gamma.get_gamma_a()!=1.0) {
		if(ret.get_a() < 0)
			ret.set_a(-gamma.a_F32_to_F32(-ret.get_a()));
		else
			ret.set_a(gamma.a_F32_to_F32(ret.get_a()));
	}

	assert(!std::isnan(ret.get_r()));
	assert(!std::isnan(ret.get_g()));
	assert(!std::isnan(ret.get_b()));
	assert(!std::isnan(ret.get_a()));

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
	IMPORT_VALUE(param_hue_adjust);
	IMPORT_VALUE(param_brightness);
	IMPORT_VALUE(param_contrast);
	IMPORT_VALUE(param_exposure);

	IMPORT_VALUE_PLUS(param_gamma,
		{
			gamma.set_gamma(1.0/param_gamma.get(Real()));
			return true;
		});
	return false;
}

ValueBase
Layer_ColorCorrect::get_param(const String &param)const
{
	EXPORT_VALUE(param_hue_adjust);
	EXPORT_VALUE(param_brightness);
	EXPORT_VALUE(param_contrast);
	EXPORT_VALUE(param_exposure);

	if(param=="gamma")
	{
		ValueBase ret=param_gamma;
		ret.set(1.0/gamma.get_gamma());
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
	RENDER_TRANSFORMED_IF_NEED(__FILE__, __LINE__)

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
Layer_ColorCorrect::accelerated_cairorender(Context context, cairo_t *cr, int quality, const RendDesc &renddesc_, ProgressCallback *cb)const
{
	RendDesc	renddesc(renddesc_);
	
	// Untransform the render desc
	if(!cairo_renddesc_untransform(cr, renddesc))
		return false;
	
	const Real pw(renddesc.get_pw()),ph(renddesc.get_ph());
	const Point tl(renddesc.get_tl());
	const int w(renddesc.get_w());
	const int h(renddesc.get_h());

	int x,y;

	SuperCallback supercb(cb,0,9500,10000);

	cairo_surface_t *surface;
	
	surface=cairo_surface_create_similar(cairo_get_target(cr), CAIRO_CONTENT_COLOR_ALPHA, w, h);
	cairo_t* subcr=cairo_create(surface);
	cairo_scale(subcr, 1/pw, 1/ph);
	cairo_translate(subcr, -tl[0], -tl[1]);
	
	if(!context.accelerated_cairorender(subcr,quality,renddesc,&supercb))
		return false;

	cairo_destroy(subcr);
	
	CairoSurface csurface(surface);
	csurface.map_cairo_image();
	
	CairoSurface::pen pen(csurface.begin());
	
	for(y=0;y<renddesc.get_h();y++,pen.inc_y(),pen.dec_x(x))
		for(x=0;x<renddesc.get_w();x++,pen.inc_x())
			pen.put_value(CairoColor(correct_color(Color(pen.get_value().demult_alpha())).clamped()).premult_alpha());
	
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

Rect
Layer_ColorCorrect::get_full_bounding_rect(Context context)const
{
	return context.get_full_bounding_rect();
}

rendering::Task::Handle
Layer_ColorCorrect::build_rendering_task_vfunc(Context context)const
{
	rendering::Task::Handle task = context.build_rendering_task();

	ColorReal gamma = param_gamma.get(Real());
	if (!approximate_equal_lp(gamma, ColorReal(1.0)))
	{
		rendering::TaskPixelGamma::Handle task_gamma(new rendering::TaskPixelGamma());
		task_gamma->gamma_r = gamma;
		task_gamma->gamma_g = gamma;
		task_gamma->gamma_b = gamma;
		task_gamma->gamma_a = gamma;
		task_gamma->sub_task() = task;
		task = task_gamma;
	}

	ColorMatrix matrix;
	matrix *= ColorMatrix().set_hue( param_hue_adjust.get(Angle()) );
	matrix *= ColorMatrix().set_exposure( param_exposure.get(Real()) );
	matrix *= ColorMatrix().set_brightness( param_brightness.get(Real()) );
	matrix *= ColorMatrix().set_contrast( param_contrast.get(Real()) );

	if (!matrix.is_copy())
	{
		rendering::TaskPixelColorMatrix::Handle task_matrix(new rendering::TaskPixelColorMatrix());
		task_matrix->matrix = matrix;
		task_matrix->sub_task() = task;
		task = task_matrix;
	}

	return task;
}
