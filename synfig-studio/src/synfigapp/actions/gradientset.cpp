/* === S Y N F I G ========================================================= */
/*!	\file gradientset.cpp
**	\brief Template File
**
**	\legal
**	Copyright (c) 2002-2005 Robert B. Quattlebaum Jr., Adrian Bentley
**
**	This file is part of Synfig.
**
**	Synfig is free software: you can redistribute it and/or modify
**	it under the terms of the GNU General Public License as published by
**	the Free Software Foundation, either version 2 of the License, or
**	(at your option) any later version.
**
**	Synfig is distributed in the hope that it will be useful,
**	but WITHOUT ANY WARRANTY; without even the implied warranty of
**	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
**	GNU General Public License for more details.
**
**	You should have received a copy of the GNU General Public License
**	along with Synfig.  If not, see <https://www.gnu.org/licenses/>.
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

#include <synfig/general.h>

#include "layerparamset.h"
#include "valuenodeconstset.h"
#include "valuedescconnect.h"
#include "waypointsetsmart.h"

#include "gradientset.h"
#include <synfigapp/canvasinterface.h>
#include <synfigapp/main.h>

#include <synfigapp/localization.h>

#endif

using namespace etl;
using namespace synfig;
using namespace synfigapp;
using namespace Action;

/* === M A C R O S ========================================================= */

ACTION_INIT(Action::GradientSet);
ACTION_SET_NAME(Action::GradientSet,"GradientSet");
ACTION_SET_LOCAL_NAME(Action::GradientSet,N_("Apply Default Gradient"));
ACTION_SET_TASK(Action::GradientSet,"set");
ACTION_SET_CATEGORY(Action::GradientSet,Action::CATEGORY_VALUEDESC);
ACTION_SET_PRIORITY(Action::GradientSet,0);
ACTION_SET_VERSION(Action::GradientSet,"0.0");

/* === G L O B A L S ======================================================= */

/* === P R O C E D U R E S ================================================= */

/* === M E T H O D S ======================================================= */

Action::GradientSet::GradientSet():
	time(0)
{
}

Action::ParamVocab
Action::GradientSet::get_param_vocab()
{
	ParamVocab ret(Action::CanvasSpecific::get_param_vocab());

	ret.push_back(ParamDesc("value_desc",Param::TYPE_VALUEDESC)
		.set_local_name(_("ValueDesc"))
	);

	ret.push_back(ParamDesc("time",Param::TYPE_TIME)
		.set_local_name(_("Time"))
		.set_optional()
	);

	return ret;
}

bool
Action::GradientSet::is_candidate(const ParamList &x)
{
	if(!candidate_check(get_param_vocab(),x))
		return false;
	return x.find("value_desc")->second.get_value_desc().get_value_type()==type_gradient;
}

bool
Action::GradientSet::set_param(const synfig::String& name, const Action::Param &param)
{
	if(name=="value_desc" && param.get_type()==Param::TYPE_VALUEDESC)
	{
		// Grab the value_desc
		value_desc=param.get_value_desc();

		// Grab the current gradient
		gradient=synfigapp::Main::get_gradient();

		return value_desc.get_value_type()==type_gradient;
	}

	if(name=="time" && param.get_type()==Param::TYPE_TIME)
	{
		time=param.get_time();

		return true;
	}

	return Action::CanvasSpecific::set_param(name,param);
}

bool
Action::GradientSet::is_ready()const
{
	if(!value_desc || value_desc.get_value_type()!=type_gradient)
		return false;
	return Action::CanvasSpecific::is_ready();
}

void
Action::GradientSet::prepare()
{
	clear();

	Action::Handle action;
	action=Action::create("ValueDescSet");

	action->set_param("canvas",get_canvas());
	action->set_param("canvas_interface",get_canvas_interface());
	action->set_param("value_desc",value_desc);
	action->set_param("new_value",ValueBase(gradient));
	action->set_param("time",time);

	if(!action->is_ready())
		throw Error(Error::TYPE_NOTREADY);

	add_action_front(action);
}
