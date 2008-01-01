/* === S Y N F I G ========================================================= */
/*!	\file timeloop.cpp
**	\brief Implementation of the "Time Loop" layer
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

#include "timeloop.h"
#include <synfig/valuenode.h>
#include <synfig/valuenode_const.h>
#include <synfig/valuenode_subtract.h>
#include <synfig/time.h>
#include <synfig/context.h>
#include <synfig/paramdesc.h>
#include <synfig/renddesc.h>
#include <synfig/value.h>

#endif

using namespace synfig;
using namespace std;
using namespace etl;

/* === M A C R O S ========================================================= */

/* === G L O B A L S ======================================================= */

SYNFIG_LAYER_INIT(Layer_TimeLoop);
SYNFIG_LAYER_SET_NAME(Layer_TimeLoop,"timeloop");
SYNFIG_LAYER_SET_LOCAL_NAME(Layer_TimeLoop,N_("Time Loop"));
SYNFIG_LAYER_SET_CATEGORY(Layer_TimeLoop,N_("Other"));
SYNFIG_LAYER_SET_VERSION(Layer_TimeLoop,"0.2");
SYNFIG_LAYER_SET_CVS_ID(Layer_TimeLoop,"$Id$");

/* === P R O C E D U R E S ================================================= */

/* === M E T H O D S ======================================================= */

Layer_TimeLoop::Layer_TimeLoop()
{
	link_time=0;
	local_time=0;
	duration=1;
}

Layer_TimeLoop::~Layer_TimeLoop()
{
}

bool
Layer_TimeLoop::set_param(const String & param, const ValueBase &value)
{
	if(old_version)
	{
		IMPORT(start_time);
		IMPORT(end_time);
	}
	else
	{
		IMPORT(local_time);
		IMPORT(link_time);
		IMPORT(duration);
	}

	return Layer::set_param(param,value);
}

ValueBase
Layer_TimeLoop::get_param(const String & param)const
{
	EXPORT(link_time);
	EXPORT(local_time);
	EXPORT(duration);
	EXPORT_NAME();
	EXPORT_VERSION();

	return Layer::get_param(param);
}

Layer::Vocab
Layer_TimeLoop::get_param_vocab()const
{
	Layer::Vocab ret(Layer::get_param_vocab());

	ret.push_back(ParamDesc("link_time")
		.set_local_name(_("Link Time"))
	);

	ret.push_back(ParamDesc("local_time")
		.set_local_name(_("Local Time"))
	);

	ret.push_back(ParamDesc("duration")
		.set_local_name(_("Duration"))
	);

	return ret;
}

bool
Layer_TimeLoop::set_version(const String &ver)
{
	if (ver=="0.1")
		old_version = true;

	return true;
}

void
Layer_TimeLoop::reset_version()
{
	// if we're not converting from an old version of the layer, there's nothing to do
	if (!old_version)
		return;

	old_version = false;

	// these are the conversions to go from 0.1 to 0.2:
	//
	//	 local_time = start_time
	//	 duration = end_time - start_time
	//	 if (time < start_time)
	//	   link_time = -duration : if we want to reproduce the old behaviour - do we?
	//	 else
	//		 link_time = 0

	// convert the static parameters
	local_time = start_time;
	duration = end_time - start_time;
	//! \todo layer version 0.1 acted differently before start_time was reached - possibly due to a bug
	link_time = 0;

	// convert the dynamic parameters
	const DynamicParamList &dpl = dynamic_param_list();

	// if neither start_time nor end_time are dynamic, there's nothing more to do
	if (dpl.count("start_time") == 0 && dpl.count("end_time") == 0)
		return;

	etl::rhandle<ValueNode> start_time_value_node, end_time_value_node;
	LinkableValueNode* duration_value_node;

	if (dpl.count("start_time"))
	{
		start_time_value_node = dpl.find("start_time")->second;
		disconnect_dynamic_param("start_time");
	}
	else
		start_time_value_node = ValueNode_Const::create(start_time);

	if (dpl.count("end_time"))
	{
		end_time_value_node = dpl.find("end_time")->second;
		disconnect_dynamic_param("end_time");
	}
	else
		end_time_value_node = ValueNode_Const::create(end_time);

	duration_value_node = ValueNode_Subtract::create(Time(0));
	duration_value_node->set_link("lhs", end_time_value_node);
	duration_value_node->set_link("rhs", start_time_value_node);

	connect_dynamic_param("local_time", start_time_value_node);
	connect_dynamic_param("duration",   duration_value_node);
	connect_dynamic_param("link_time",  ValueNode_Const::create(Time(0)));
}

void
Layer_TimeLoop::set_time(Context context, Time t)const
{
 	if (duration == 0)
		t = link_time;
	else if (duration > 0)
	{
		t -= local_time;
		t -= floor(t / duration) * duration;
		t  = link_time + t;
	}
	else
	{
		t -= local_time;
		t -= floor(t / -duration) * -duration;
		t  = link_time - t;
	}

	context.set_time(t);
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
