/* === S Y N F I G ========================================================= */
/*!	\file insideout.cpp
**	\brief Implementation of the "Inside Out" layer
**
**	$Id$
**
**	\legal
**	Copyright (c) 2002-2005 Robert B. Quattlebaum Jr., Adrian Bentley
**	Copyright (c) 2007 Chris Moore
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
**
** === N O T E S ===========================================================
**
** ========================================================================= */

/* === H E A D E R S ======================================================= */

#ifdef USING_PCH
#	include "pch.h"
#else
#ifdef HAVE_CONFIG_H
#	include <config.h>
#endif

#include "insideout.h"

#include <synfig/localization.h>
#include <synfig/general.h>

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

using namespace std;
using namespace etl;
using namespace synfig;
using namespace modules;
using namespace lyr_std;

/* === M A C R O S ========================================================= */

/* === G L O B A L S ======================================================= */

SYNFIG_LAYER_INIT(InsideOut);
SYNFIG_LAYER_SET_NAME(InsideOut,"inside_out");
SYNFIG_LAYER_SET_LOCAL_NAME(InsideOut,N_("Inside Out"));
SYNFIG_LAYER_SET_CATEGORY(InsideOut,N_("Distortions"));
SYNFIG_LAYER_SET_VERSION(InsideOut,"0.1");

/* === P R O C E D U R E S ================================================= */

/* === M E T H O D S ======================================================= */

InsideOut::InsideOut():
	param_origin(ValueBase(Point(0,0)))
{
	SET_INTERPOLATION_DEFAULTS();
	SET_STATIC_DEFAULTS();
}

bool
InsideOut::set_param(const String & param, const ValueBase &value)
{
	IMPORT_VALUE(param_origin);
	return false;
}

ValueBase
InsideOut::get_param(const String & param)const
{
	EXPORT_VALUE(param_origin);

	EXPORT_NAME();
	EXPORT_VERSION();

	return ValueBase();
}

Layer::Handle
InsideOut::hit_check(Context context, const Point &p)const
{
	Point origin=param_origin.get(Point());
	Point pos(p-origin);
	Real inv_mag=pos.inv_mag();
	Point invpos(pos*inv_mag*inv_mag);
	return context.hit_check(invpos+origin);
}

RendDesc
InsideOut::get_sub_renddesc_vfunc(const RendDesc &renddesc) const
{
	RendDesc desc(renddesc);
	desc.set_wh(512, 512);
	desc.set_tl(Vector(-5.0, -5.0));
	desc.set_br(Vector( 5.0,  5.0));
	return desc;
}

Color
InsideOut::get_color(Context context, const Point &p)const
{
	Point origin=param_origin.get(Point());
	Point pos(p-origin);
	Real inv_mag=pos.inv_mag();
	Point invpos(pos*inv_mag*inv_mag);
	return context.get_color(invpos+origin);
}

CairoColor
InsideOut::get_cairocolor(Context context, const Point &p)const
{
	Point origin=param_origin.get(Point());
	Point pos(p-origin);
	Real inv_mag=pos.inv_mag();
	Point invpos(pos*inv_mag*inv_mag);
	return context.get_cairocolor(invpos+origin);
}

class lyr_std::InsideOut_Trans : public Transform
{
	etl::handle<const InsideOut> layer;
public:
	InsideOut_Trans(const InsideOut* x):Transform(x->get_guid()),layer(x) { }

	Vector perform(const Vector& x)const
	{
		Point origin=layer->param_origin.get(Point());
		Point pos(x-origin);
		Real inv_mag=pos.inv_mag();
		if(!std::isnan(inv_mag))
			return (pos*(inv_mag*inv_mag)+origin);
		return x;
	}

	Vector unperform(const Vector& x)const
	{
		Point origin=layer->param_origin.get(Point());
		Point pos(x-origin);
		Real inv_mag=pos.inv_mag();
		if(!std::isnan(inv_mag))
			return (pos*(inv_mag*inv_mag)+origin);
		return x;
	}

	String get_string()const
	{
		return "insideout";
	}
};
etl::handle<Transform>
InsideOut::get_transform()const
{
	return new InsideOut_Trans(this);
}

Layer::Vocab
InsideOut::get_param_vocab()const
{
	Layer::Vocab ret;

	ret.push_back(ParamDesc("origin")
		.set_local_name(_("Origin"))
		.set_description(_("Center of the distortion"))
		.set_is_distance()
	);

	return ret;
}
