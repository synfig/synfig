/* === S Y N F I G ========================================================= */
/*!	\file stretch.cpp
**	\brief Implementation of the "Stretch" layer
**
**	$Id$
**
**	\legal
**	Copyright (c) 2002-2005 Robert B. Quattlebaum Jr., Adrian Bentley
**	Copyright (c) 2007 Chris Moore
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

#include "stretch.h"
#include <synfig/string.h>
#include <synfig/time.h>
#include <synfig/context.h>
#include <synfig/paramdesc.h>
#include <synfig/renddesc.h>
#include <synfig/surface.h>
#include <synfig/value.h>
#include <synfig/valuenode.h>
#include <synfig/transform.h>

#endif

/* === U S I N G =========================================================== */

using namespace etl;
using namespace std;
using namespace synfig;

/* === G L O B A L S ======================================================= */

SYNFIG_LAYER_INIT(Layer_Stretch);
SYNFIG_LAYER_SET_NAME(Layer_Stretch,"stretch");
SYNFIG_LAYER_SET_LOCAL_NAME(Layer_Stretch,N_("Stretch"));
SYNFIG_LAYER_SET_CATEGORY(Layer_Stretch,N_("Distortions"));
SYNFIG_LAYER_SET_VERSION(Layer_Stretch,"0.1");
SYNFIG_LAYER_SET_CVS_ID(Layer_Stretch,"$Id$");

/* === P R O C E D U R E S ================================================= */

/* === M E T H O D S ======================================================= */

/* === E N T R Y P O I N T ================================================= */

Layer_Stretch::Layer_Stretch():
	amount(1,1),
	center(0,0)
{
	Layer::Vocab voc(get_param_vocab());
	Layer::fill_static(voc);
}


bool
Layer_Stretch::set_param(const String & param, const ValueBase &value)
{
	IMPORT(amount);
	IMPORT(center);

	return false;
}

ValueBase
Layer_Stretch::get_param(const String &param)const
{
	EXPORT(amount);
	EXPORT(center);

	EXPORT_NAME();
	EXPORT_VERSION();

	return ValueBase();
}

Layer::Vocab
Layer_Stretch::get_param_vocab()const
{
	Layer::Vocab ret;

	ret.push_back(ParamDesc("amount")
		.set_local_name(_("Amount"))
		.set_origin("center")
		.set_description(_("Size of the stretch relative to its Center"))
	);

	ret.push_back(ParamDesc("center")
		.set_local_name(_("Center"))
		.set_description(_("Where the stretch distortion is centered"))
	);

	return ret;
}

synfig::Layer::Handle
Layer_Stretch::hit_check(synfig::Context context, const synfig::Point &pos)const
{
	Point npos(pos);
	npos[0]=(npos[0]-center[0])/amount[0]+center[0];
	npos[1]=(npos[1]-center[1])/amount[1]+center[1];
	return context.hit_check(npos);
}

Color
Layer_Stretch::get_color(Context context, const Point &pos)const
{
	Point npos(pos);
	npos[0]=(npos[0]-center[0])/amount[0]+center[0];
	npos[1]=(npos[1]-center[1])/amount[1]+center[1];
	return context.get_color(npos);
}

class Stretch_Trans : public Transform
{
	etl::handle<const Layer_Stretch> layer;
public:
	Stretch_Trans(const Layer_Stretch* x):Transform(x->get_guid()),layer(x) { }

	synfig::Vector perform(const synfig::Vector& x)const
	{
		return Vector((x[0]-layer->center[0])*layer->amount[0]+layer->center[0],
					  (x[1]-layer->center[1])*layer->amount[1]+layer->center[1]);
	}

	synfig::Vector unperform(const synfig::Vector& x)const
	{
		return Vector((x[0]-layer->center[0])/layer->amount[0]+layer->center[0],
					  (x[1]-layer->center[1])/layer->amount[1]+layer->center[1]);
	}

	synfig::String get_string()const
	{
		return "stretch";
	}
};
etl::handle<Transform>
Layer_Stretch::get_transform()const
{
	return new Stretch_Trans(this);
}

bool
Layer_Stretch::accelerated_render(Context context,Surface *surface,int quality, const RendDesc &renddesc, ProgressCallback *cb)const
{
	if (amount[0] == 0 || amount[1] == 0)
	{
		surface->set_wh(renddesc.get_w(), renddesc.get_h());
		surface->clear();
		return true;
	}

	RendDesc desc(renddesc);
	desc.clear_flags();
    // Adjust the top_left and bottom_right points
	// for our zoom amount
	Point npos;
	npos[0]=(desc.get_tl()[0]-center[0])/amount[0]+center[0];
	npos[1]=(desc.get_tl()[1]-center[1])/amount[1]+center[1];
	desc.set_tl(npos);
	npos[0]=(desc.get_br()[0]-center[0])/amount[0]+center[0];
	npos[1]=(desc.get_br()[1]-center[1])/amount[1]+center[1];
	desc.set_br(npos);

	// Render the scene
	return context.accelerated_render(surface,quality,desc,cb);
}

//////
bool
Layer_Stretch::accelerated_cairorender(Context context,cairo_surface_t *surface,int quality, const RendDesc &renddesc, ProgressCallback *cb)const
{
	if (amount[0] == 0 || amount[1] == 0)
	{
		cairo_t* cr=cairo_create(surface);
		cairo_set_operator(cr, CAIRO_OPERATOR_CLEAR);
		cairo_fill(cr);
		cairo_destroy(cr);
		return true;
	}

	const Point	otl(renddesc.get_tl());
	const Point obr(renddesc.get_br());
	const int	w(renddesc.get_w());
	const int	h(renddesc.get_h());
	
	// Width and Height of a pixel
	const Real pw = (obr[0] - otl[0]) / w;
	const Real ph = (obr[1] - otl[1]) / h;
	
	// These are the scale values
	const double sx(1/pw);
	const double sy(1/ph);
	
	RendDesc desc(renddesc);
	
	desc.clear_flags();
    // Adjust the top_left and bottom_right points
	// for our zoom amount
	Point tl_new, br_new, tl, br;
	tl_new[0]=(desc.get_tl()[0]-center[0])/amount[0]+center[0];
	tl_new[1]=(desc.get_tl()[1]-center[1])/amount[1]+center[1];
	br_new[0]=(desc.get_br()[0]-center[0])/amount[0]+center[0];
	br_new[1]=(desc.get_br()[1]-center[1])/amount[1]+center[1];

	// This is a woraround to make the rotate layer rotate properly
	// First rorder the tl and br to make it normal
	tl[0]=min(tl_new[0],br_new[0]);
	tl[1]=max(tl_new[1],br_new[1]);
	br[0]=max(tl_new[0],br_new[0]);
	br[1]=min(tl_new[1],br_new[1]);

	//Second render the secene
	desc.set_tl_br(tl, br);
	// Render the scene
	cairo_surface_t* source=cairo_surface_create_similar(surface, CAIRO_CONTENT_COLOR_ALPHA, desc.get_w(), desc.get_h());
	
	if(!context.accelerated_cairorender(source,quality,desc,cb))
		return false;
		
	cairo_t* cr=cairo_create(surface);
	cairo_set_operator(cr, CAIRO_OPERATOR_CLEAR);
	cairo_fill(cr);
	
	cairo_translate(cr, (obr[0]-otl[0])*sx/2.0, (obr[1]-otl[1])*sy/2.0);
	if(amount[0] != 1.0 || amount[1] != 1.0)
		cairo_scale(cr, amount[0]/fabs(amount[0]), amount[1]/fabs(amount[1]));
	cairo_translate(cr, (otl[0]-obr[0])*sx/2.0, (otl[1]-obr[1])*sy/2.0);

	cairo_set_source_surface(cr, source, 0, 0);
	cairo_set_operator(cr, CAIRO_OPERATOR_SOURCE);
	cairo_paint(cr);
	cairo_destroy(cr);
	cairo_surface_destroy(source);
	/// End of workaround
	return true;
}

/////


bool
Layer_Stretch::accelerated_cairorender(Context context, cairo_t *cr, int quality, const RendDesc &renddesc, ProgressCallback *cb)const
{
	if (amount[0] == 0 || amount[1] == 0)
	{
		cairo_set_operator(cr, CAIRO_OPERATOR_CLEAR);
		cairo_fill(cr);
		return true;
	}
	const double stx(center[0]);
	const double sty(center[1]);
		
	cairo_translate(cr, stx, sty);
	cairo_scale(cr, amount[0], amount[1]);
	cairo_translate(cr, -stx, -sty);

	if(!context.accelerated_cairorender(cr,quality,renddesc,cb))
		return false;
	
	return true;
}


Rect
Layer_Stretch::get_full_bounding_rect(Context context)const
{
	Rect rect(context.get_full_bounding_rect());
	Point min(rect.get_min()), max(rect.get_max());

	return Rect(Point((min[0]-center[0])*amount[0]+center[0],
					  (min[1]-center[1])*amount[1]+center[1]),
				Point((max[0]-center[0])*amount[0]+center[0],
					  (max[1]-center[1])*amount[1]+center[1]));
}
