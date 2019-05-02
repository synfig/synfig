/* === S Y N F I G ========================================================= */
/*!	\file timeloop.cpp
**	\brief Implementation of the "Time Loop" layer
**
**	$Id$
**
**	\legal
**	Copyright (c) 2002-2005 Robert B. Quattlebaum Jr., Adrian Bentley
**	Copyright (c) 2007, 2008 Chris Moore
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

#include <synfig/localization.h>
#include <synfig/general.h>

#include "timeloop.h"
#include <synfig/valuenode.h>
#include <synfig/valuenodes/valuenode_const.h>
#include <synfig/valuenodes/valuenode_subtract.h>
#include <synfig/time.h>
#include <synfig/context.h>
#include <synfig/paramdesc.h>
#include <synfig/renddesc.h>
#include <synfig/value.h>
#include <synfig/canvas.h>

#endif

using namespace std;
using namespace etl;
using namespace synfig;
using namespace modules;
using namespace lyr_std;

/* === M A C R O S ========================================================= */

/* === G L O B A L S ======================================================= */

SYNFIG_LAYER_INIT(Layer_TimeLoop);
SYNFIG_LAYER_SET_NAME(Layer_TimeLoop,"timeloop");
SYNFIG_LAYER_SET_LOCAL_NAME(Layer_TimeLoop,N_("Time Loop"));
SYNFIG_LAYER_SET_CATEGORY(Layer_TimeLoop,N_("Time"));
SYNFIG_LAYER_SET_VERSION(Layer_TimeLoop,"0.2");
SYNFIG_LAYER_SET_CVS_ID(Layer_TimeLoop,"$Id$");

/* === P R O C E D U R E S ================================================= */

/* === M E T H O D S ======================================================= */

Layer_TimeLoop::Layer_TimeLoop()
{
	old_version=false;
	param_only_for_positive_duration=ValueBase(false);
	param_symmetrical=ValueBase(true);
	param_link_time=ValueBase(Time(0));
	param_local_time=ValueBase(Time(0));
	param_duration=ValueBase(Time(1));

	SET_INTERPOLATION_DEFAULTS();
	SET_STATIC_DEFAULTS();
}

Layer_TimeLoop::~Layer_TimeLoop()
{
}

bool
Layer_TimeLoop::set_param(const String & param, const ValueBase &value)
{
	if(old_version)
	{
		if(param=="start_time" && value.same_type_as(start_time))
		{
			value.put(&start_time);
			return true;
		}
		if(param=="end_time" && value.same_type_as(end_time))
		{
			value.put(&end_time);
			return true;
		}
	}
	else
	{
		IMPORT_VALUE(param_local_time);
		IMPORT_VALUE(param_link_time);
		IMPORT_VALUE(param_duration);
		IMPORT_VALUE(param_only_for_positive_duration);
		IMPORT_VALUE(param_symmetrical);
	}

	return Layer::set_param(param,value);
}

ValueBase
Layer_TimeLoop::get_param(const String & param)const
{
	EXPORT_VALUE(param_link_time);
	EXPORT_VALUE(param_local_time);
	EXPORT_VALUE(param_duration);
	EXPORT_VALUE(param_only_for_positive_duration);
	EXPORT_VALUE(param_symmetrical);
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
		.set_description(_("Start time of the loop for the cycled context"))
		.set_static(true)
	);

	ret.push_back(ParamDesc("local_time")
		.set_local_name(_("Local Time"))
		.set_description(_("The time when the resulted loop starts"))
		.set_static(true)
	);

	ret.push_back(ParamDesc("duration")
		.set_local_name(_("Duration"))
		.set_description(_("Length of the loop"))
		.set_static(true)
	);

	ret.push_back(ParamDesc("only_for_positive_duration")
		.set_local_name(_("Only For Positive Duration"))
		.set_description(_("When checked will loop only positive durations"))
		.set_static(true)
	);

	ret.push_back(ParamDesc("symmetrical")
		.set_local_name(_("Symmetrical"))
		.set_description(_("When checked, loops are mirrored centered at Local Time"))
		.set_static(true)
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
	Time link_time, local_time, duration;
	bool only_for_positive_duration, symmetrical;
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
	only_for_positive_duration = true;
	symmetrical = false;

	param_local_time.set(local_time);
	param_duration.set(duration);
	param_only_for_positive_duration.set(only_for_positive_duration);
	param_symmetrical.set(symmetrical);
	
	//! \todo layer version 0.1 acted differently before start_time was reached - possibly due to a bug
	link_time = 0;
	param_link_time.set(link_time);

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
}

void
Layer_TimeLoop::set_time_vfunc(IndependentContext context, Time t)const
{
	Time link_time=param_link_time.get(Time());
	Time local_time=param_local_time.get(Time());
	Time duration=param_duration.get(Time());
	bool only_for_positive_duration=param_only_for_positive_duration.get(bool());
	bool symmetrical=param_symmetrical.get(bool());
	
	Time time = t;
	float document_fps=get_canvas()->rend_desc().get_frame_rate();
	if (!only_for_positive_duration || duration > 0)
	{
		if (duration == 0)
			t = link_time;
		else {
			float t_frames = round(t*document_fps);
			float duration_frames = round(duration*document_fps);
			if (duration > 0)
			{
				t -= local_time;
				// Simple formula looks like this:
				// t -= floor(t / duration) * duration;
				// but we should make all calculations in frames to avoid round errors
				t_frames -= floor(t_frames / duration_frames) * duration_frames;
				// converting back to seconds:
				t = t_frames / document_fps;
				t = link_time + t;
			}
			else
			{
				t -= local_time;
				// Simple formula looks like this:
				// t -= floor(t / -duration) * -duration;
				// but we should make all calculations in frames to avoid round errors
				t_frames -= floor(t_frames / -duration_frames) * -duration_frames;
				// converting back to seconds:
				t = t_frames / document_fps;
				t = link_time - t;
			}
		}
		// for compatibility with v0.1 layers; before local_time is reached, take a step back
		if (!symmetrical && time < local_time)
			t -= duration;
	}
	context.set_time(t);
}
