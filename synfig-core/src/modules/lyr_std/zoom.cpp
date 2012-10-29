/* === S Y N F I G ========================================================= */
/*!	\file zoom.cpp
**	\brief Implementation of the "Zoom" layer, aka "Scale"
**
**	$Id$
**
**	\legal
**	Copyright (c) 2002-2005 Robert B. Quattlebaum Jr., Adrian Bentley
**	Copyright (c) 2012 Diego Barrios Romero
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

#include "zoom.h"
#include <synfig/string.h>
#include <synfig/time.h>
#include <synfig/context.h>
#include <synfig/paramdesc.h>
#include <synfig/renddesc.h>
#include <synfig/surface.h>
#include <synfig/value.h>
#include <synfig/valuenode.h>
#include <synfig/transform.h>
#include <synfig/transformationchain.h>

#endif

/* === M A C R O S ========================================================= */

/* === G L O B A L S ======================================================= */

SYNFIG_LAYER_INIT(Zoom);
SYNFIG_LAYER_SET_NAME(Zoom,"zoom");
SYNFIG_LAYER_SET_LOCAL_NAME(Zoom,N_("Scale"));
SYNFIG_LAYER_SET_CATEGORY(Zoom,N_("Transform"));
SYNFIG_LAYER_SET_VERSION(Zoom,"0.1");
SYNFIG_LAYER_SET_CVS_ID(Zoom,"$Id$");

/* === P R O C E D U R E S ================================================= */

/* === M E T H O D S ======================================================= */

/* === E N T R Y P O I N T ================================================= */

Zoom::Zoom():
	center(0,0),
	amount(0)
{
	Layer::Vocab voc(get_param_vocab());
	Layer::fill_static(voc);
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
		.set_description(_("Amount to scale to"))
	);

	ret.push_back(ParamDesc("center")
		.set_local_name(_("Origin"))
		.set_description(_("Point to scale from"))
	);

	return ret;
}

synfig::Layer::Handle
Zoom::hit_check(synfig::Context context, const synfig::Point &pos)const
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

	synfig::Vector perform(const synfig::Vector& x)const
	{
		return (x-layer->center)*exp(layer->amount)+layer->center;
	}

	synfig::Vector unperform(const synfig::Vector& x)const
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
	RendDesc desc(renddesc);
	desc.clear_flags();

	// In order to perform a scale transformation in 2D with an arbitrary scale
	// center, we first have to translate the object according to the center,
	// then scale it and then translate the object back to its position.
	Matrix m1, m2, m3;
	m1.set_translate(center);
	m2.set_scale(exp(amount), exp(amount));
	m3.set_translate(-center);

	desc.get_transformation_chain().push(m1);
	desc.get_transformation_chain().push(m2);
	desc.get_transformation_chain().push(m3);

	// Render the scene
	return context.accelerated_render(surface,quality,desc,cb);
}

synfig::Rect
Zoom::get_full_bounding_rect(synfig::Context context)const
{
	return (context.get_full_bounding_rect()-center)*exp(amount)+center;
}

