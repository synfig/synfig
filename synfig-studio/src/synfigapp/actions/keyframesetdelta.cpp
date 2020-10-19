/* === S Y N F I G ========================================================= */
/*!	\file keyframesetdelta.cpp
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

#include "keyframesetdelta.h"
#include "keyframeset.h"

#include <synfigapp/localization.h>

#endif

using namespace std;
using namespace etl;
using namespace synfig;
using namespace synfigapp;
using namespace Action;

/* === M A C R O S ========================================================= */

ACTION_INIT(Action::KeyframeSetDelta);
ACTION_SET_NAME(Action::KeyframeSetDelta,"KeyframeSetDelta");
ACTION_SET_LOCAL_NAME(Action::KeyframeSetDelta,N_("Set Keyframe Delta"));
ACTION_SET_TASK(Action::KeyframeSetDelta,"set");
ACTION_SET_CATEGORY(Action::KeyframeSetDelta,Action::CATEGORY_KEYFRAME|Action::CATEGORY_HIDDEN);
ACTION_SET_PRIORITY(Action::KeyframeSetDelta,0);
ACTION_SET_VERSION(Action::KeyframeSetDelta,"0.0");

/* === G L O B A L S ======================================================= */

/* === P R O C E D U R E S ================================================= */

/* === M E T H O D S ======================================================= */

Action::KeyframeSetDelta::KeyframeSetDelta():
	delta(0)
{
	keyframe.set_time(Time::end());
	set_dirty(false);
}

Action::ParamVocab
Action::KeyframeSetDelta::get_param_vocab()
{
	ParamVocab ret(Action::CanvasSpecific::get_param_vocab());

	ret.push_back(ParamDesc("keyframe",Param::TYPE_KEYFRAME)
		.set_local_name(_("Keyframe"))
	);
	ret.push_back(ParamDesc("delta",Param::TYPE_KEYFRAME)
		.set_local_name(_("Delta"))
	);

	return ret;
}

bool
Action::KeyframeSetDelta::is_candidate(const ParamList &x)
{
	return candidate_check(get_param_vocab(),x);
}

bool
Action::KeyframeSetDelta::set_param(const synfig::String& name, const Action::Param &param)
{
	if(name=="keyframe" && param.get_type()==Param::TYPE_KEYFRAME)
	{
		keyframe=param.get_keyframe();
		return true;
	}
	if(name=="delta" && param.get_type()==Param::TYPE_TIME)
	{
		delta=param.get_time();
		return true;
	}

	return Action::CanvasSpecific::set_param(name,param);
}

bool
Action::KeyframeSetDelta::is_ready()const
{
	if(keyframe.get_time()==Time::end())
		return false;
	return Action::CanvasSpecific::is_ready();
}

void
Action::KeyframeSetDelta::prepare()
{
	KeyframeList &list = get_canvas()->keyframe_list();
	KeyframeList::iterator next;
	if (!list.find(keyframe, next)) throw Error(_("Unable to find the given keyframe")); // ???
	++next;
	if (next != list.end() && fabs(delta) > 0.00000001)
	{
		for(KeyframeList::iterator i = next; i != list.end(); ++i) {
			Keyframe keyframe(*i);
			keyframe.set_time( keyframe.get_time() + delta );

			Action::Handle action(KeyframeSet::create());
			action->set_param("canvas",get_canvas());
			action->set_param("canvas_interface",get_canvas_interface());
			action->set_param("keyframe", keyframe);
			assert(action->is_ready());
			if(!action->is_ready())
				throw Error(Error::TYPE_NOTREADY);

			if (delta > 0) add_action_front(action); else add_action(action);
		}
	}
}
