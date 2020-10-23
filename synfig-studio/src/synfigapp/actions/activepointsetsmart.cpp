/* === S Y N F I G ========================================================= */
/*!	\file activepointsetsmart.cpp
**	\brief Template File
**
**	$Id$
**
**	\legal
**	Copyright (c) 2002-2005 Robert B. Quattlebaum Jr., Adrian Bentley
**  Copyright (c) 2008 Chris Moore
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

#include "activepointsetsmart.h"
#include "valuenodelinkconnect.h"
#include "valuenodereplace.h"

#include "activepointset.h"
#include "activepointadd.h"

#include "valuedescconnect.h"
#include <synfigapp/canvasinterface.h>

#include <synfigapp/localization.h>

#endif

using namespace etl;
using namespace synfig;
using namespace synfigapp;
using namespace Action;

/* === M A C R O S ========================================================= */

ACTION_INIT(Action::ActivepointSetSmart);
ACTION_SET_NAME(Action::ActivepointSetSmart,"ActivepointSetSmart");
ACTION_SET_LOCAL_NAME(Action::ActivepointSetSmart,N_("Set Activepoint (Smart)"));
ACTION_SET_TASK(Action::ActivepointSetSmart,"set");
ACTION_SET_CATEGORY(Action::ActivepointSetSmart,Action::CATEGORY_ACTIVEPOINT);
ACTION_SET_PRIORITY(Action::ActivepointSetSmart,0);
ACTION_SET_VERSION(Action::ActivepointSetSmart,"0.0");

/* === G L O B A L S ======================================================= */

/* === P R O C E D U R E S ================================================= */

/* === M E T H O D S ======================================================= */

Action::ActivepointSetSmart::ActivepointSetSmart():
	index()
{
	activepoint.set_time(Time::begin()-1);
	time_set=false;
	set_dirty(true);
}

Action::ParamVocab
Action::ActivepointSetSmart::get_param_vocab()
{
	ParamVocab ret(Action::CanvasSpecific::get_param_vocab());

	ret.push_back(ParamDesc("value_desc",Param::TYPE_VALUEDESC)
		.set_local_name(_("ValueDesc"))
	);

	ret.push_back(ParamDesc("activepoint",Param::TYPE_ACTIVEPOINT)
		.set_local_name(_("New Activepoint"))
		.set_desc(_("Activepoint to be added"))
		.set_optional()
	);

	ret.push_back(ParamDesc("time",Param::TYPE_TIME)
		.set_local_name(_("Time"))
		.set_desc(_("Time where activepoint is to be added"))
		.set_optional()
	);

	return ret;
}

bool
Action::ActivepointSetSmart::is_candidate(const ParamList &x)
{
	if (!candidate_check(get_param_vocab(),x))
		return false;

	ValueDesc value_desc(x.find("value_desc")->second.get_value_desc());

	return (value_desc.parent_is_value_node() &&
			// We need a dynamic list.
			ValueNode_DynamicList::Handle::cast_dynamic(value_desc.get_parent_value_node()) &&
			// We need either an activepoint or a time.
			(x.count("activepoint") || x.count("time")));
}

bool
Action::ActivepointSetSmart::set_param(const synfig::String& name, const Action::Param &param)
{
	if(name=="value_desc" && param.get_type()==Param::TYPE_VALUEDESC)
	{
		value_desc=param.get_value_desc();

		if(!value_desc.parent_is_value_node())
			return false;

		value_node=ValueNode_DynamicList::Handle::cast_dynamic(value_desc.get_parent_value_node());

		if(!value_node)
			return false;

		index=value_desc.get_index();

		if(time_set)
			calc_activepoint();

		return true;
	}
	if(name=="activepoint" && param.get_type()==Param::TYPE_ACTIVEPOINT && !time_set)
	{
		activepoint=param.get_activepoint();

		return true;
	}
	if(name=="time" && param.get_type()==Param::TYPE_TIME && activepoint.get_time()==Time::begin()-1)
	{
		activepoint.set_time(param.get_time());
		time_set=true;

		if(value_node)
			calc_activepoint();

		return true;
	}

	return Action::CanvasSpecific::set_param(name,param);
}

bool
Action::ActivepointSetSmart::is_ready()const
{
	if(!value_node)
		synfig::error("Missing value_node");

	if(activepoint.get_time()==(Time::begin()-1))
		synfig::error("Missing activepoint");

	if(!value_node || activepoint.get_time()==(Time::begin()-1))
		return false;
	return Action::CanvasSpecific::is_ready();
}

// This function is called if a time is specified, but not
// a activepoint. In this case, we need to calculate the value
// of the activepoint
void
Action::ActivepointSetSmart::calc_activepoint()
{
/*
	const Time time(activepoint.get_time());
	activepoint.set_state(value_node->list[index].status_at_time(time));
	activepoint.set_priority(0);
*/

	activepoint=value_node->list[index].new_activepoint_at_time(activepoint.get_time());

	// In this case, nothing is really changing, so there will be
	// no need to redraw the window
	set_dirty(false);
}

void
Action::ActivepointSetSmart::enclose_activepoint(const synfig::Activepoint& activepoint)
{
	times.insert(activepoint.get_time());

	if (get_edit_mode()&MODE_ANIMATE_PAST) {
		// Try to find prev keyframe
		// Keyframe keyframe(*get_canvas()->keyframe_list().find_prev(activepoint.get_time()));
		KeyframeList::iterator iter;
		if (get_canvas()->keyframe_list().find_prev(activepoint.get_time(), iter)) {
			Keyframe keyframe(*iter);

			if (!times.count(keyframe.get_time())) {
				times.insert(keyframe.get_time());

			try { value_node->list[index].find(keyframe.get_time()); }
			catch (const synfig::Exception::NotFound&)
			{
				Action::Handle action(ActivepointAdd::create());

				action->set_param("canvas",get_canvas());
				action->set_param("canvas_interface",get_canvas_interface());
				action->set_param("value_desc",value_desc);

				if(!value_node->list[index].timing_info.empty())
				{
					action->set_param("time",keyframe.get_time());
				}
				else
				{
					synfig::Activepoint tmp;

					tmp.set_state(true);
					tmp.set_time(keyframe.get_time());
					action->set_param("activepoint",tmp);
				}

				assert(action->is_ready());
				if(!action->is_ready())
					throw Error(Error::TYPE_NOTREADY);

				add_action_front(action);
			}


			}

		}
	}
	/*try
	{
		// Try to find prev keyframe
		Keyframe keyframe(*get_canvas()->keyframe_list().find_prev(activepoint.get_time()));

		if(times.count(keyframe.get_time()))
			throw int();
		else
			times.insert(keyframe.get_time());

		try { value_node->list[index].find(keyframe.get_time()); }
		catch(synfig::Exception::NotFound)
		{
			Action::Handle action(ActivepointAdd::create());

			action->set_param("canvas",get_canvas());
			action->set_param("canvas_interface",get_canvas_interface());
			action->set_param("value_desc",value_desc);

			if(!value_node->list[index].timing_info.empty())
			{
				action->set_param("time",keyframe.get_time());
			}
			else
			{
				synfig::Activepoint tmp;

				tmp.set_state(true);
				tmp.set_time(keyframe.get_time());
				action->set_param("activepoint",tmp);
			}

			assert(action->is_ready());
			if(!action->is_ready())
				throw Error(Error::TYPE_NOTREADY);

			add_action_front(action);
		}
	}
	catch(int) { }
	catch(synfig::Exception::NotFound) { }*/

	if(get_edit_mode() & MODE_ANIMATE_FUTURE)
	{
		// Try to find next keyframe
		KeyframeList::iterator iter;
		if (get_canvas()->keyframe_list().find_next(activepoint.get_time(), iter)) {
			Keyframe keyframe(*iter);

			if (!times.count(keyframe.get_time())) {
				times.insert(keyframe.get_time());

				try { value_node->list[index].find(keyframe.get_time()); }
				catch (const synfig::Exception::NotFound&)
				{
					Action::Handle action(ActivepointAdd::create());

					action->set_param("canvas",get_canvas());
					action->set_param("canvas_interface",get_canvas_interface());
					action->set_param("value_desc",value_desc);

					if(!value_node->list[index].timing_info.empty())
					{
						action->set_param("time",keyframe.get_time());
					}
					else
					{
						synfig::Activepoint tmp;

						tmp.set_state(true);
						tmp.set_time(keyframe.get_time());
						action->set_param("activepoint",tmp);
					}

					assert(action->is_ready());
					if(!action->is_ready())
						throw Error(Error::TYPE_NOTREADY);

					add_action_front(action);
				}

			}
		}
	}
	/*try
	{
		// Try to find next keyframe
		Keyframe keyframe(*get_canvas()->keyframe_list().find_next(activepoint.get_time()));

		if(times.count(keyframe.get_time()))
			throw int();
		else
			times.insert(keyframe.get_time());

		try { value_node->list[index].find(keyframe.get_time()); }
		catch(synfig::Exception::NotFound)
		{
			Action::Handle action(ActivepointAdd::create());

			action->set_param("canvas",get_canvas());
			action->set_param("canvas_interface",get_canvas_interface());
			action->set_param("value_desc",value_desc);

			if(!value_node->list[index].timing_info.empty())
			{
				action->set_param("time",keyframe.get_time());
			}
			else
			{
				synfig::Activepoint tmp;

				tmp.set_state(true);
				tmp.set_time(keyframe.get_time());
				action->set_param("activepoint",tmp);
			}

			assert(action->is_ready());
			if(!action->is_ready())
				throw Error(Error::TYPE_NOTREADY);

			add_action_front(action);
		}
	}
	catch(int) { }
	catch(synfig::Exception::NotFound) { }*/
}

void
Action::ActivepointSetSmart::prepare()
{
	clear();
	times.clear();

	// First, we need to add any activepoints necessary to
	// maintain the integrity of the keyframes.
	enclose_activepoint(activepoint);

	try
	{
		if(value_node->list[index].find(activepoint)==value_node->list[index].timing_info.end())
			throw int();

		// Then, let's try to replace the old activepoint, if it exists
		enclose_activepoint(*value_node->list[index].find(activepoint));

		Action::Handle action(ActivepointSet::create());

		action->set_param("canvas",get_canvas());
		action->set_param("canvas_interface",get_canvas_interface());
		action->set_param("value_desc",value_desc);
		action->set_param("activepoint",activepoint);

		assert(action->is_ready());
		if(!action->is_ready())
			throw Error(Error::TYPE_NOTREADY);

		add_action_front(action);

		return;
	}
	catch(int){}
	catch(Exception::NotFound&){}

	try
	{
		// Check to see if a activepoint exists at this point in time
		activepoint.mimic(*value_node->list[index].find(activepoint.get_time()));

		enclose_activepoint(*value_node->list[index].find(activepoint.get_time()));

		Action::Handle action(ActivepointSet::create());

		action->set_param("canvas",get_canvas());
		action->set_param("canvas_interface",get_canvas_interface());
		action->set_param("value_desc",value_desc);
		action->set_param("activepoint",activepoint);

		assert(action->is_ready());
		if(!action->is_ready())
			throw Error(Error::TYPE_NOTREADY);

		add_action_front(action);

		return;
	}
	catch(int){}
	catch(Exception::NotFound&){}

	try
	{
		// At this point we know that the old activepoint doesn't exist,
		// so we need to create it.
		Action::Handle action(ActivepointAdd::create());

		action->set_param("canvas",get_canvas());
		action->set_param("canvas_interface",get_canvas_interface());
		action->set_param("value_desc",value_desc);
		action->set_param("activepoint",activepoint);

		assert(action->is_ready());
		if(!action->is_ready())
			throw Error(Error::TYPE_NOTREADY);

		add_action_front(action);

		return;
	}
	catch(int){}
	catch(Exception::NotFound&){}

	throw Error(_("Unable to determine how to proceed. This is a bug."));
}
