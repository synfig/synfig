/* === S Y N F I G ========================================================= */
/*!	\file colorset.cpp
**	\brief Template File
**
**	$Id: colorset.cpp,v 1.1.1.1 2005/01/07 03:34:37 darco Exp $
**
**	\legal
**	Copyright (c) 2002-2005 Robert B. Quattlebaum Jr., Adrian Bentley
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

#include "layerparamset.h"
#include "valuenodeconstset.h"
#include "valuedescconnect.h"
#include "waypointsetsmart.h"

#include "colorset.h"
#include <synfigapp/canvasinterface.h>
#include <synfigapp/main.h>

#endif

using namespace std;
using namespace etl;
using namespace synfig;
using namespace synfigapp;
using namespace Action;

/* === M A C R O S ========================================================= */

ACTION_INIT(Action::ColorSet);
ACTION_SET_NAME(Action::ColorSet,"color_set");
ACTION_SET_LOCAL_NAME(Action::ColorSet,"Apply Default Color");
ACTION_SET_TASK(Action::ColorSet,"set");
ACTION_SET_CATEGORY(Action::ColorSet,Action::CATEGORY_VALUEDESC);
ACTION_SET_PRIORITY(Action::ColorSet,0);
ACTION_SET_VERSION(Action::ColorSet,"0.0");
ACTION_SET_CVS_ID(Action::ColorSet,"$Id: colorset.cpp,v 1.1.1.1 2005/01/07 03:34:37 darco Exp $");

/* === G L O B A L S ======================================================= */

/* === P R O C E D U R E S ================================================= */

/* === M E T H O D S ======================================================= */

Action::ColorSet::ColorSet():
	time(0)
{
}

Action::ParamVocab
Action::ColorSet::get_param_vocab()
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
Action::ColorSet::is_candidate(const ParamList &x)
{
	if(!candidate_check(get_param_vocab(),x))
		return false;
	return x.find("value_desc")->second.get_value_desc().get_value_type()==ValueBase::TYPE_COLOR;
}

bool
Action::ColorSet::set_param(const synfig::String& name, const Action::Param &param)
{
	if(name=="value_desc" && param.get_type()==Param::TYPE_VALUEDESC)
	{
		// Grab the value_desc
		value_desc=param.get_value_desc();

		// Grab the current color
		color=synfigapp::Main::get_foreground_color();

		return value_desc.get_value_type()==ValueBase::TYPE_COLOR;
	}

	if(name=="time" && param.get_type()==Param::TYPE_TIME)
	{
		time=param.get_time();

		return true;
	}

	return Action::CanvasSpecific::set_param(name,param);
}

bool
Action::ColorSet::is_ready()const
{
	if(!value_desc || value_desc.get_value_type()!=ValueBase::TYPE_COLOR)
		return false;
	return Action::CanvasSpecific::is_ready();
}

void
Action::ColorSet::prepare()
{
	clear();

	Action::Handle action;
	action=Action::create("value_desc_set");

	action->set_param("canvas",get_canvas());
	action->set_param("canvas_interface",get_canvas_interface());
	action->set_param("value_desc",value_desc);
	action->set_param("new_value",ValueBase(color));
	action->set_param("time",time);

	if(!action->is_ready())
		throw Error(Error::TYPE_NOTREADY);

	add_action_front(action);
}
