/*! ========================================================================
** Sinfg
** Template File
** $Id: insideout.cpp,v 1.1.1.1 2005/01/04 01:23:10 darco Exp $
**
** Copyright (c) 2002 Robert B. Quattlebaum Jr.
**
** This software and associated documentation
** are CONFIDENTIAL and PROPRIETARY property of
** the above-mentioned copyright holder.
**
** You may not copy, print, publish, or in any
** other way distribute this software without
** a prior written agreement with
** the copyright holder.
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

SINFG_LAYER_INIT(InsideOut);
SINFG_LAYER_SET_NAME(InsideOut,"inside_out");
SINFG_LAYER_SET_LOCAL_NAME(InsideOut,_("InsideOut"));
SINFG_LAYER_SET_CATEGORY(InsideOut,_("Distortions"));
SINFG_LAYER_SET_VERSION(InsideOut,"0.1");
SINFG_LAYER_SET_CVS_ID(InsideOut,"$Id: insideout.cpp,v 1.1.1.1 2005/01/04 01:23:10 darco Exp $");

/* === P R O C E D U R E S ================================================= */

/* === M E T H O D S ======================================================= */

InsideOut::InsideOut():
	origin(0,0)
{
}
	
bool
InsideOut::set_param(const String & param, const ValueBase &value)
{
	IMPORT(origin);
	return false;
}

ValueBase
InsideOut::get_param(const String & param)const
{
	EXPORT(origin);
	
	EXPORT_NAME();
	EXPORT_VERSION();
		
	return ValueBase();	
}

sinfg::Layer::Handle
InsideOut::hit_check(sinfg::Context context, const sinfg::Point &p)const
{
	Point pos(p-origin);
	Real inv_mag=pos.inv_mag();
	Point invpos(pos*inv_mag*inv_mag);
	return context.hit_check(invpos+origin);
}

Color
InsideOut::get_color(Context context, const Point &p)const
{
	Point pos(p-origin);
	Real inv_mag=pos.inv_mag();
	Point invpos(pos*inv_mag*inv_mag);
	return context.get_color(invpos+origin);
}

class InsideOut_Trans : public Transform
{
	etl::handle<const InsideOut> layer;
public:
	InsideOut_Trans(const InsideOut* x):layer(x) { }
	
	sinfg::Vector perform(const sinfg::Vector& x)const
	{
		Point pos(x-layer->origin);
		Real inv_mag=pos.inv_mag();
		if(!isnan(inv_mag))
			return (pos*(inv_mag*inv_mag)+layer->origin);
		return x;
	}
	
	sinfg::Vector unperform(const sinfg::Vector& x)const
	{
		Point pos(x-layer->origin);
		Real inv_mag=pos.inv_mag();
		if(!isnan(inv_mag))
			return (pos*(inv_mag*inv_mag)+layer->origin);
		return x;
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
		.set_description(_("Defines the where the center will be"))
	);
	
	return ret;
}
