/* === S Y N F I G ========================================================= */
/*!	\file waypointsetsmart.cpp
**	\brief Template File
**
**	$Id$
**
**	\legal
**	Copyright (c) 2002-2005 Robert B. Quattlebaum Jr., Adrian Bentley
**	Copyright (c) 2007 Chris Moore
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

#include "waypointsetsmart.h"
#include "valuenodelinkconnect.h"
#include "valuenodereplace.h"

#include "waypointset.h"
#include "waypointadd.h"

#include "valuedescconnect.h"
#include <synfigapp/canvasinterface.h>
#include <synfig/exception.h>
#include <synfigapp/main.h>

#include <synfigapp/localization.h>

#endif

using namespace etl;
using namespace synfig;
using namespace synfigapp;
using namespace Action;

/* === M A C R O S ========================================================= */

ACTION_INIT(Action::WaypointSetSmart);
ACTION_SET_NAME(Action::WaypointSetSmart,"WaypointSetSmart");
ACTION_SET_LOCAL_NAME(Action::WaypointSetSmart,N_("Add Waypoint"));
ACTION_SET_TASK(Action::WaypointSetSmart,"set");
ACTION_SET_CATEGORY(Action::WaypointSetSmart,Action::CATEGORY_WAYPOINT|Action::CATEGORY_VALUEDESC|Action::CATEGORY_VALUENODE);
ACTION_SET_PRIORITY(Action::WaypointSetSmart,0);
ACTION_SET_VERSION(Action::WaypointSetSmart,"0.0");

/* === G L O B A L S ======================================================= */

/* === P R O C E D U R E S ================================================= */

/* === M E T H O D S ======================================================= */

Action::WaypointSetSmart::WaypointSetSmart()
{
	waypoint.set_time(Time::begin()-1);
	time_set=false;
	set_dirty(true);
}

Action::ParamVocab
Action::WaypointSetSmart::get_param_vocab()
{
	ParamVocab ret(Action::CanvasSpecific::get_param_vocab());

	ret.push_back(ParamDesc("value_node",Param::TYPE_VALUENODE)
		.set_local_name(_("Destination ValueNode (Animated)"))
	);

	ret.push_back(ParamDesc("waypoint",Param::TYPE_WAYPOINT)
		.set_local_name(_("New Waypoint"))
		.set_desc(_("Waypoint to be added"))
		.set_optional()
	);

	ret.push_back(ParamDesc("waypoint_model",Param::TYPE_WAYPOINTMODEL)
		.set_local_name(_("Waypoint Model"))
		.set_optional()
	);

	ret.push_back(ParamDesc("time",Param::TYPE_TIME)
		.set_local_name(_("Time"))
		.set_desc(_("Time where waypoint is to be added"))
		.set_optional()
	);

	return ret;
}

bool
Action::WaypointSetSmart::is_candidate(const ParamList &x)
{
	return (candidate_check(get_param_vocab(),x) &&
			// We need an animated valuenode.
			ValueNode_Animated::Handle::cast_dynamic(x.find("value_node")->second.get_value_node()) &&
			// We need either a waypoint or a time.
			(x.count("waypoint") || x.count("time")));
}

bool
Action::WaypointSetSmart::set_param(const synfig::String& name, const Action::Param &param)
{
	if(name=="value_node" && param.get_type()==Param::TYPE_VALUENODE)
	{
		value_node=ValueNode_Animated::Handle::cast_dynamic(param.get_value_node());
		if(time_set)
			calc_waypoint();

		return static_cast<bool>(value_node);
	}
	if(name=="waypoint" && param.get_type()==Param::TYPE_WAYPOINT && !time_set)
	{
		waypoint=param.get_waypoint();

		return true;
	}

	if(name=="time" && param.get_type()==Param::TYPE_TIME && waypoint.get_time()==(Time::begin()-1))
	{
		waypoint.set_time(param.get_time());
		time_set=true;

		if(value_node)
			calc_waypoint();

		return true;
	}

	if(name=="model" && param.get_type()==Param::TYPE_WAYPOINTMODEL)
	{
		if(value_node)
			calc_waypoint();

		waypoint.apply_model(param.get_waypoint_model());

		return true;
	}

	return Action::CanvasSpecific::set_param(name,param);
}

bool
Action::WaypointSetSmart::is_ready()const
{
	if(!value_node)
		synfig::error("Missing value_node");

	if(waypoint.get_time()==(Time::begin()-1))
		synfig::error("Missing waypoint");

	if(!value_node || waypoint.get_time()==(Time::begin()-1))
		return false;
	return Action::CanvasSpecific::is_ready();
}

// This function is called if a time is specified, but not
// a waypoint. In this case, we need to calculate the value
// of the waypoint
void
Action::WaypointSetSmart::calc_waypoint()
{
	Time time=waypoint.get_time();
	try
	{
		// Trivial case, we are sitting on a waypoint
		waypoint=*value_node->find(waypoint.get_time());
	}
	catch(...)
	{
		waypoint=value_node->new_waypoint_at_time(time);
		Interpolation interp=value_node->get_interpolation();
		waypoint.set_before(interp==INTERPOLATION_UNDEFINED?synfigapp::Main::get_interpolation():interp);
		waypoint.set_after(interp==INTERPOLATION_UNDEFINED?synfigapp::Main::get_interpolation():interp);
	}
/*
	Time time=waypoint.get_time();
	ValueNode_Animated::WaypointList &waypoint_list(value_node->waypoint_list());
	ValueNode_Animated::WaypointList::iterator iter;

	if(waypoint_list.empty())
	{
		waypoint.set_value((*value_node)(time));
		return;
	}

	ValueNode_Animated::WaypointList::iterator closest=waypoint_list.begin();

	for(iter=waypoint_list.begin();iter!=waypoint_list.end();++iter)
	{
		const Real dist(abs(iter->get_time()-time));
		if(dist<abs(closest->get_time()-time))
			closest=iter;
	}
	if(!closest->is_static())
		waypoint.set_value_node(closest->get_value_node());
	else
		waypoint.set_value((*value_node)(time));
*/
}

void
Action::WaypointSetSmart::enclose_waypoint(const synfig::Waypoint& waypoint)
{
	times.insert(waypoint.get_time());

	try {
		times.insert(value_node->find(waypoint)->get_time());
//		synfig::info(__FILE__":%d: value_node->find(waypoint)->get_time()=%s",__LINE__,value_node->find(waypoint)->get_time().get_string().c_str());
	}catch (...) { }

	// First we need to add any waypoints necessary to
	// maintain the integrity of the keyframes.
	if(get_edit_mode()&MODE_ANIMATE_PAST) try
	{
		Time curr_time(waypoint.get_time());

		//while(value_node->waypoint_list().front().get_time()<=curr_time)
		{
			// Try to find prev keyframe
			KeyframeList::iterator iter;
			if (get_canvas()->keyframe_list().find_prev(curr_time, iter)) {
				//Keyframe keyframe(*get_canvas()->keyframe_list().find_prev(curr_time));
				Keyframe keyframe(*iter);
				curr_time=keyframe.get_time();

	//			synfig::info(__FILE__":%d: prev_keyframe->time=%s",__LINE__,keyframe.get_time().get_string().c_str());
	//			synfig::info(__FILE__":%d: waypoint->time=%s",__LINE__,waypoint.get_time().get_string().c_str());

				if(times.count(keyframe.get_time()))
				{
					throw int();
				}
				if(waypoint.get_time().is_equal(keyframe.get_time()))
				{
					throw int();
				}

				times.insert(keyframe.get_time());
				try
				{
					value_node->find(keyframe.get_time());
	//				synfig::info(__FILE__":%d: waypointtime=%s",__LINE__,value_node->find(keyframe.get_time())->get_time().get_string().c_str());
				}
				catch (const synfig::Exception::NotFound&)
				{
					Action::Handle action(WaypointAdd::create());

					action->set_param("canvas",get_canvas());
					action->set_param("canvas_interface",get_canvas_interface());
					action->set_param("value_node",ValueNode::Handle(value_node));

					if(!value_node->waypoint_list().empty())
					{
						action->set_param("time",keyframe.get_time());
					}
					else
					{
						synfig::Waypoint tmp;

						tmp.set_value(waypoint.get_value());
						tmp.set_time(keyframe.get_time());
						action->set_param("waypoint",tmp);
					}

					assert(action->is_ready());
					if(!action->is_ready())
						throw Error(Error::TYPE_NOTREADY);

					add_action(action);
				}
			}
		}
	}
	catch(Error &x) { throw x; }
	catch(synfig::Exception::NotFound&) { }
	catch(int&) { }
	catch(...) { }

	if(get_edit_mode()&MODE_ANIMATE_FUTURE)try
	{
		Time curr_time(waypoint.get_time());

		//while(value_node->waypoint_list().back().get_time()>=curr_time)
		{

			// Try to find next keyframe
			//synfig::info("FUTURE waypoint.get_time()=%s",waypoint.get_time().get_string().c_str());
			KeyframeList::iterator iter;
			if (get_canvas()->keyframe_list().find_next(curr_time, iter)) {
				//Keyframe keyframe(*get_canvas()->keyframe_list().find_next(curr_time));
				Keyframe keyframe(*iter);
				//synfig::info("FUTURE keyframe.get_time()=%s",keyframe.get_time().get_string().c_str());
				curr_time=keyframe.get_time();

				if(times.count(keyframe.get_time())|| waypoint.get_time().is_equal(keyframe.get_time()))
					throw int();
				else
					times.insert(keyframe.get_time());

				try
				{
					value_node->find(keyframe.get_time());
					//synfig::info(__FILE__":%d: time=%s",__LINE__,keyframe.get_time().get_string().c_str());
					//synfig::info(__FILE__":%d: waypointtime=%s",__LINE__,value_node->find(keyframe.get_time())->get_time().get_string().c_str());

				}
				catch (const synfig::Exception::NotFound&)
				{
					Action::Handle action(WaypointAdd::create());

					action->set_param("canvas",get_canvas());
					action->set_param("canvas_interface",get_canvas_interface());
					action->set_param("value_node",ValueNode::Handle(value_node));

					if(!value_node->waypoint_list().empty())
					{
						action->set_param("time",keyframe.get_time());
					}
					else
					{
						synfig::Waypoint tmp;

						tmp.set_value(waypoint.get_value());
						tmp.set_time(keyframe.get_time());
						action->set_param("waypoint",tmp);
					}

					assert(action->is_ready());
					if(!action->is_ready())
						throw Error(Error::TYPE_NOTREADY);

					add_action(action);
				}
			}
		}
	}
	catch(Error& x) { throw x; }
	catch(synfig::Exception::NotFound&) { }
	catch(int&) { }
	catch(...) { }
}

void
Action::WaypointSetSmart::prepare()
{
	clear();
	times.clear();

	// First we need to add any waypoints necessary to
	// maintain the integrity of the keyframes.
	enclose_waypoint(waypoint);

	try
	{
		//synfig::info("WaypointSetSmart: Move/Update?");
		// Let's try to replace the old waypoint, if it exists
		WaypointList::iterator iter(value_node->find(waypoint));

		if(iter == value_node->waypoint_list().end())
			throw int();

		enclose_waypoint(*iter);

		Action::Handle action(WaypointSet::create());

		action->set_param("canvas",get_canvas());
		action->set_param("canvas_interface",get_canvas_interface());
		action->set_param("value_node",ValueNode::Handle(value_node));
		action->set_param("waypoint",waypoint);

		assert(action->is_ready());
		if(!action->is_ready())
			throw Error(Error::TYPE_NOTREADY);

		add_action(action);

		return;
	}
	//TODO(ice0): fix that
	catch(const synfig::Exception::NotFound&){ }
	catch(int) { }

	try
	{
		//synfig::info("WaypointSetSmart: Replace?");
		// Check to see if a waypoint exists at this point in time
		WaypointList::iterator iter=value_node->find(waypoint.get_time());

		waypoint.mimic(*iter);

		enclose_waypoint(*iter);

		Action::Handle action(WaypointSet::create());

		action->set_param("canvas",get_canvas());
		action->set_param("canvas_interface",get_canvas_interface());
		action->set_param("value_node",ValueNode::Handle(value_node));
		action->set_param("waypoint",waypoint);

		assert(action->is_ready());
		if(!action->is_ready())
			throw Error(Error::TYPE_NOTREADY);

		add_action(action);

		return;
	}
	catch(const synfig::Exception::NotFound&){ } catch(int){ }

	try
	{
		//synfig::info("WaypointSetSmart: Add?");
		// At this point we know that the old waypoint doesn't exist,
		// so we need to create it.
		Action::Handle action(WaypointAdd::create());

		action->set_param("canvas",get_canvas());
		action->set_param("canvas_interface",get_canvas_interface());
		action->set_param("value_node",ValueNode::Handle(value_node));
		action->set_param("waypoint",waypoint);

		assert(action->is_ready());
		if(!action->is_ready())
			throw Error(Error::TYPE_NOTREADY);

		add_action(action);

		return;
	}
	catch (const synfig::Exception::NotFound&){ }
	catch(int){ }

	throw Error(_("Unable to determine how to proceed. This is a bug."));
}
