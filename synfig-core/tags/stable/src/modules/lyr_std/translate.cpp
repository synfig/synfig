/*! ========================================================================
** Sinfg
** Template File
** $Id: translate.cpp,v 1.2 2005/01/24 03:08:17 darco Exp $
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

#include "translate.h"
#include <sinfg/string.h>
#include <sinfg/time.h>
#include <sinfg/context.h>
#include <sinfg/paramdesc.h>
#include <sinfg/renddesc.h>
#include <sinfg/surface.h>
#include <sinfg/value.h>
#include <sinfg/valuenode.h>
#include <sinfg/canvas.h>
#include <sinfg/transform.h>

#endif

/* === M A C R O S ========================================================= */

/* === G L O B A L S ======================================================= */

SINFG_LAYER_INIT(Translate);
SINFG_LAYER_SET_NAME(Translate,"translate");
SINFG_LAYER_SET_LOCAL_NAME(Translate,_("Translate"));
SINFG_LAYER_SET_CATEGORY(Translate,_("Transform"));
SINFG_LAYER_SET_VERSION(Translate,"0.1");
SINFG_LAYER_SET_CVS_ID(Translate,"$Id: translate.cpp,v 1.2 2005/01/24 03:08:17 darco Exp $");

/* === P R O C E D U R E S ================================================= */

/* === M E T H O D S ======================================================= */

/* === E N T R Y P O I N T ================================================= */

Translate::Translate():origin(0,0)
{
}

Translate::~Translate()
{
}
	
bool
Translate::set_param(const String & param, const ValueBase &value)
{
	IMPORT(origin);
	
	return false;
}

ValueBase
Translate::get_param(const String& param)const
{
	EXPORT(origin);
	EXPORT_NAME();
	EXPORT_VERSION();
		
	return ValueBase();	
}

Layer::Vocab
Translate::get_param_vocab()const
{
	Layer::Vocab ret;
	
	ret.push_back(ParamDesc("origin")
		.set_local_name(_("Origin"))
		.set_description(_("Point where you want the origin to be"))
	);
	
	return ret;
}

sinfg::Layer::Handle
Translate::hit_check(sinfg::Context context, const sinfg::Point &pos)const
{
	return context.hit_check(pos-origin);
}

Color
Translate::get_color(Context context, const Point &pos)const
{
	return context.get_color(pos-origin);
}

class Translate_Trans : public Transform
{
	etl::handle<const Translate> layer;
public:
	Translate_Trans(const Translate* x):Transform(x->get_guid()),layer(x) { }
	
	sinfg::Vector perform(const sinfg::Vector& x)const
	{
		return x+layer->origin;
	}
	
	sinfg::Vector unperform(const sinfg::Vector& x)const
	{
		return x-layer->origin;
	}
};
etl::handle<Transform>
Translate::get_transform()const
{
	return new Translate_Trans(this);
}

bool
Translate::accelerated_render(Context context,Surface *surface,int quality, const RendDesc &renddesc, ProgressCallback *cb)const
{
	RendDesc desc(renddesc);

	desc.clear_flags();
	desc.set_tl(desc.get_tl()-origin);
	desc.set_br(desc.get_br()-origin);

	// Render the scene
	if(!context.accelerated_render(surface,quality,desc,cb))
	{
		if(cb)cb->error(strprintf(__FILE__"%d: Accelerated Renderer Failure",__LINE__));
		return false;
	}

	return true;
}

Rect
Translate::get_full_bounding_rect(Context context)const
{
	return context.get_full_bounding_rect() + origin;
}
