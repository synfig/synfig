/* === S I N F G =========================================================== */
/*!	\file zoom.cpp
**	\brief writeme
**
**	$Id: zoom.cpp,v 1.2 2005/01/24 03:08:17 darco Exp $
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

#include "zoom.h"
#include <sinfg/string.h>
#include <sinfg/time.h>
#include <sinfg/context.h>
#include <sinfg/paramdesc.h>
#include <sinfg/renddesc.h>
#include <sinfg/surface.h>
#include <sinfg/value.h>
#include <sinfg/valuenode.h>
#include <sinfg/transform.h>

#endif

/* === M A C R O S ========================================================= */

/* === G L O B A L S ======================================================= */

SINFG_LAYER_INIT(Zoom);
SINFG_LAYER_SET_NAME(Zoom,"zoom");
SINFG_LAYER_SET_LOCAL_NAME(Zoom,_("Zoom"));
SINFG_LAYER_SET_CATEGORY(Zoom,_("Transform"));
SINFG_LAYER_SET_VERSION(Zoom,"0.1");
SINFG_LAYER_SET_CVS_ID(Zoom,"$Id: zoom.cpp,v 1.2 2005/01/24 03:08:17 darco Exp $");

/* === P R O C E D U R E S ================================================= */

/* === M E T H O D S ======================================================= */

/* === E N T R Y P O I N T ================================================= */

Zoom::Zoom():
	center(0,0),
	amount(0)
{
}
	
bool
Zoom::set_param(const String & param, const ValueBase &value)
{

	IMPORT(center);
	IMPORT(amount);
	
	return false;
}

ValueBase
Zoom::get_param(const String &param)const
{
	EXPORT(center);
	EXPORT(amount);

	EXPORT_NAME();
	EXPORT_VERSION();
		
	return ValueBase();	
}

Layer::Vocab
Zoom::get_param_vocab()const
{
	Layer::Vocab ret;
	
	ret.push_back(ParamDesc("amount")
		.set_local_name(_("Amount"))
		.set_description(_("Amount to zoom in"))
	);

	ret.push_back(ParamDesc("center")
		.set_local_name(_("Center"))
		.set_description(_("Point to zoom in to"))
	);
	
	return ret;
}

sinfg::Layer::Handle
Zoom::hit_check(sinfg::Context context, const sinfg::Point &pos)const
{
	return context.hit_check((pos-center)/exp(amount)+center);
}

Color
Zoom::get_color(Context context, const Point &pos)const
{
	return context.get_color((pos-center)/exp(amount)+center);
}

class Zoom_Trans : public Transform
{
	etl::handle<const Zoom> layer;
public:
	Zoom_Trans(const Zoom* x):Transform(x->get_guid()),layer(x) { }
	
	sinfg::Vector perform(const sinfg::Vector& x)const
	{
		return (x-layer->center)*exp(layer->amount)+layer->center;
	}
	
	sinfg::Vector unperform(const sinfg::Vector& x)const
	{
		return (x-layer->center)/exp(layer->amount)+layer->center;
	}
};
etl::handle<Transform>
Zoom::get_transform()const
{
	return new Zoom_Trans(this);
}

bool
Zoom::accelerated_render(Context context,Surface *surface,int quality, const RendDesc &renddesc, ProgressCallback *cb)const
{
	Vector::value_type zoomfactor=1.0/exp(amount);
	RendDesc desc(renddesc);
	desc.clear_flags();

    // Adjust the top_left and bottom_right points
	// for our zoom amount
	desc.set_tl((desc.get_tl()-center)*zoomfactor+center);
	desc.set_br((desc.get_br()-center)*zoomfactor+center);

	// Render the scene
	return context.accelerated_render(surface,quality,desc,cb);
}

sinfg::Rect
Zoom::get_full_bounding_rect(sinfg::Context context)const
{
	return context.get_full_bounding_rect();
}

