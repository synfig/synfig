/*! ========================================================================
** Sinfg
** Image Layer_TimeLoop Layer Implementation
** $Id: timeloop.cpp,v 1.1.1.1 2005/01/04 01:23:10 darco Exp $
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

#include "timeloop.h"
#include <sinfg/time.h>
#include <sinfg/context.h>
#include <sinfg/paramdesc.h>
#include <sinfg/renddesc.h>
#include <sinfg/value.h>

#endif

using namespace sinfg;
using namespace std;
using namespace etl;

/* === M A C R O S ========================================================= */

/* === G L O B A L S ======================================================= */

SINFG_LAYER_INIT(Layer_TimeLoop);
SINFG_LAYER_SET_NAME(Layer_TimeLoop,"timeloop");
SINFG_LAYER_SET_LOCAL_NAME(Layer_TimeLoop,_("TimeLoop"));
SINFG_LAYER_SET_CATEGORY(Layer_TimeLoop,_("Other"));
SINFG_LAYER_SET_VERSION(Layer_TimeLoop,"0.1");
SINFG_LAYER_SET_CVS_ID(Layer_TimeLoop,"$Id: timeloop.cpp,v 1.1.1.1 2005/01/04 01:23:10 darco Exp $");

/* === P R O C E D U R E S ================================================= */

/* === M E T H O D S ======================================================= */

Layer_TimeLoop::Layer_TimeLoop()
{
	start_time=0;
	end_time=1;
}

Layer_TimeLoop::~Layer_TimeLoop()
{
}

bool
Layer_TimeLoop::set_param(const String & param, const ValueBase &value)
{
	IMPORT(start_time);
	IMPORT(end_time);
	return Layer::set_param(param,value);
}

ValueBase
Layer_TimeLoop::get_param(const String & param)const
{
	EXPORT(start_time);
	EXPORT(end_time);
	EXPORT_NAME();
	EXPORT_VERSION();

	return Layer::get_param(param);
}

Layer::Vocab
Layer_TimeLoop::get_param_vocab()const
{
	Layer::Vocab ret(Layer::get_param_vocab());
	
	ret.push_back(ParamDesc("start_time")
		.set_local_name(_("Start Time"))
	);

	ret.push_back(ParamDesc("end_time")
		.set_local_name(_("End Time"))
	);
	
	return ret;
}

void
Layer_TimeLoop::set_time(Context context, Time time)const
{
	Real diff(end_time-start_time);
	if(diff>0)
		time-=int(Real(time-start_time)/diff)*diff+start_time;
	context.set_time(time);
}

Color
Layer_TimeLoop::get_color(Context context, const Point &pos)const
{
	return context.get_color(pos);
}

bool
Layer_TimeLoop::accelerated_render(Context context,Surface *surface,int quality, const RendDesc &renddesc, ProgressCallback *cb)const
{
	return context.accelerated_render(surface,quality,renddesc,cb);
}
