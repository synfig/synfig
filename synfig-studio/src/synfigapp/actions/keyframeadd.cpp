/* === S Y N F I G ========================================================= */
/*!	\file keyframeadd.cpp
**	\brief Template File
**
**	$Id$
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

#include <synfig/general.h>

#include "keyframeadd.h"
#include <synfigapp/canvasinterface.h>

#include <synfigapp/localization.h>

#endif

using namespace std;
using namespace etl;
using namespace synfig;
using namespace synfigapp;
using namespace Action;

/* === M A C R O S ========================================================= */

ACTION_INIT(Action::KeyframeAdd);
ACTION_SET_NAME(Action::KeyframeAdd,"KeyframeAdd");
ACTION_SET_LOCAL_NAME(Action::KeyframeAdd,N_("Add Keyframe"));
ACTION_SET_TASK(Action::KeyframeAdd,"add");
ACTION_SET_CATEGORY(Action::KeyframeAdd,Action::CATEGORY_KEYFRAME);
ACTION_SET_PRIORITY(Action::KeyframeAdd,0);
ACTION_SET_VERSION(Action::KeyframeAdd,"0.0");

/* === G L O B A L S ======================================================= */

/* === P R O C E D U R E S ================================================= */

/* === M E T H O D S ======================================================= */

Action::KeyframeAdd::KeyframeAdd()
{
	keyframe.set_time(Time::begin()-1);
	set_dirty(false);
}

Action::ParamVocab
Action::KeyframeAdd::get_param_vocab()
{
	ParamVocab ret(Action::CanvasSpecific::get_param_vocab());

	ret.push_back(ParamDesc("keyframe",Param::TYPE_KEYFRAME)
		.set_local_name(_("New Keyframe"))
		.set_desc(_("Keyframe to be added"))
	);

	return ret;
}

bool
Action::KeyframeAdd::is_candidate(const ParamList &x)
{
	if(!candidate_check(get_param_vocab(),x))
		return false;

	return true;
}

bool
Action::KeyframeAdd::set_param(const synfig::String& name, const Action::Param &param)
{
	if(name=="keyframe" && param.get_type()==Param::TYPE_KEYFRAME)
	{
		keyframe=param.get_keyframe();

		return true;
	}

	return Action::CanvasSpecific::set_param(name,param);
}

bool
Action::KeyframeAdd::is_ready()const
{
	if(keyframe.get_time()==(Time::begin()-1))
		return false;
	return Action::CanvasSpecific::is_ready();
}

void
Action::KeyframeAdd::perform()
{
	KeyframeList::iterator iter;
	if (get_canvas()->keyframe_list().find(keyframe.get_time(), iter))
		throw Error(_("A Keyframe already exists at this point in time"));
	if (get_canvas()->keyframe_list().find(keyframe, iter))
		throw Error(_("This keyframe is already in the ValueNode"));

	get_canvas()->keyframe_list().add(keyframe);
	if(get_canvas_interface())
		get_canvas_interface()->signal_keyframe_added()(keyframe);
	else
		synfig::warning("CanvasInterface not set on action");
}

void
Action::KeyframeAdd::undo()
{
	
	get_canvas()->keyframe_list().erase(keyframe);

	if(get_canvas_interface())
	{
		get_canvas_interface()->signal_keyframe_removed()(keyframe);
	}
	else synfig::warning("CanvasInterface not set on action");
}
