/* === S Y N F I G ========================================================= */
/*!	\file keyframeset.cpp
**	\brief Template File
**
**	$Id$
**
**	\legal
**	Copyright (c) 2002-2005 Robert B. Quattlebaum Jr., Adrian Bentley
**	Copyright (c) 2012-2013 Konstantin Dmitriev
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

#include "keyframeset.h"
#include <synfigapp/canvasinterface.h>
#include <synfig/valuenodes/valuenode_dynamiclist.h>
#include <synfig/valuenodes/valuenode_animated.h>
#include "activepointsetsmart.h"
#include "waypointsetsmart.h"
#include <synfigapp/main.h>

#include <synfigapp/localization.h>

#endif

using namespace synfig;
using namespace synfigapp;
using namespace Action;

/* === M A C R O S ========================================================= */

ACTION_INIT(Action::KeyframeSet);
ACTION_SET_NAME(Action::KeyframeSet,"KeyframeSet");
ACTION_SET_LOCAL_NAME(Action::KeyframeSet,N_("Set Keyframe"));
ACTION_SET_TASK(Action::KeyframeSet,"set");
ACTION_SET_CATEGORY(Action::KeyframeSet,Action::CATEGORY_KEYFRAME|Action::CATEGORY_HIDDEN);
ACTION_SET_PRIORITY(Action::KeyframeSet,0);
ACTION_SET_VERSION(Action::KeyframeSet,"0.0");

/* === G L O B A L S ======================================================= */

/* === P R O C E D U R E S ================================================= */

/* === M E T H O D S ======================================================= */

Action::KeyframeSet::KeyframeSet()
{
	keyframe.set_time(Time::begin()-1);
	set_dirty(false);
}

Action::ParamVocab
Action::KeyframeSet::get_param_vocab()
{
	ParamVocab ret(Action::CanvasSpecific::get_param_vocab());

	ret.push_back(ParamDesc("keyframe",Param::TYPE_KEYFRAME)
		.set_local_name(_("New Keyframe"))
		.set_desc(_("Keyframe to be added"))
	);

	return ret;
}

bool
Action::KeyframeSet::is_candidate(const ParamList &x)
{
	return candidate_check(get_param_vocab(),x);
}

bool
Action::KeyframeSet::set_param(const synfig::String& name, const Action::Param &param)
{
	if(name=="keyframe" && param.get_type()==Param::TYPE_KEYFRAME)
	{
		keyframe=param.get_keyframe();
		return true;
	}

	return Action::CanvasSpecific::set_param(name,param);
}

bool
Action::KeyframeSet::is_ready()const
{
	if(keyframe.get_time()==(Time::begin()-1))
		return false;
	return Action::CanvasSpecific::is_ready();
}

void
Action::KeyframeSet::prepare()
{
	clear();
	guid_set.clear();

	//synfig::info("new_time: %s",new_time.get_string().c_str());
	//synfig::info("old_time: %s",old_time.get_string().c_str());

	// If the times are different and keyframe is not disabled, then we
	// will need to romp through the valuenodes
	// and add actions to update their values.
	if (new_time != old_time && keyframe.active()) {
		std::vector<synfigapp::ValueDesc> value_desc_list;
		get_canvas_interface()->find_important_value_descs(value_desc_list);
		while (!value_desc_list.empty()) {
			process_value_desc(value_desc_list.back());
			value_desc_list.pop_back();
		}
	}
}

#define old_2_new(x)	(((x)-old_begin)/(old_end-old_begin)*(new_end-new_begin)+new_begin)

int
Action::KeyframeSet::scale_activepoints(const synfigapp::ValueDesc& value_desc,const synfig::Time& old_begin,const synfig::Time& old_end,const synfig::Time& new_begin,const synfig::Time& new_end)
{
	ValueNode_DynamicList::Handle value_node(ValueNode_DynamicList::Handle::cast_static(value_desc.get_parent_value_node()));
	ValueNode_DynamicList::ListEntry& list_entry(value_node->list[value_desc.get_index()]);

	std::vector<Activepoint*> selected;
	std::vector<Activepoint*>::iterator iter;

	if(list_entry.find(old_begin,old_end,selected))
	{
		// check to make sure this operation is OK
		for(iter=selected.begin();iter!=selected.end();++iter)
		{
			try
			{
				Time new_time(old_2_new((*iter)->get_time()));
				if(new_time>=old_begin && new_time<old_end)
					continue;
				list_entry.find(new_time);
				// If we found a activepoint already at that time, then
				// we need to abort
				//throw Exception::BadTime(_("Activepoint Conflict"));
			}
			catch(Exception::NotFound&) { }
		}

		int ret(0);
		while(!selected.empty())
		{
			if(selected.back()->get_time()!=old_2_new(selected.back()->get_time()))
			{
				Action::Handle action(Action::create("ActivepointSet"));

				action->set_param("canvas",get_canvas());
				action->set_param("canvas_interface",get_canvas_interface());
				action->set_param("value_desc",value_desc);

				Activepoint activepoint(*selected.back());
				activepoint.set_time(old_2_new(selected.back()->get_time()));

				action->set_param("activepoint",activepoint);

				assert(action->is_ready());
				if(!action->is_ready())
					throw Error(Error::TYPE_NOTREADY);

				add_action_front(action);

				ret++;
			}
			selected.pop_back();
		}
		return ret;
	}
	return 0;
}

int
Action::KeyframeSet::scale_waypoints(const synfigapp::ValueDesc& value_desc,const synfig::Time& old_begin,const synfig::Time& old_end,const synfig::Time& new_begin,const synfig::Time& new_end)
{
	ValueNode_Animated::Handle value_node(ValueNode_Animated::Handle::cast_static(value_desc.get_value_node()));

	std::vector<Waypoint*> selected;
	std::vector<Waypoint*>::iterator iter;

	if(value_node->find(old_begin,old_end,selected))
	{
		// check to make sure this operation is OK
		for(iter=selected.begin();iter!=selected.end();++iter)
		{
			try
			{
				Time new_time(old_2_new((*iter)->get_time()));
				if(new_time>=old_begin && new_time<old_end)
					continue;
				value_node->find(new_time);
				// If we found a waypoint point already at that time, then
				// we need to abort
				//synfig::info(_("old_begin: %s, old_end: %s"),old_begin.get_string().c_str(),old_end.get_string().c_str());
				//synfig::info(_("new_begin: %s, new_end: %s"),new_begin.get_string().c_str(),new_end.get_string().c_str());
				//throw Exception::BadTime(strprintf(_("Waypoint Conflict, old: %s, new: %s"),(*iter)->get_time().get_string().c_str(),new_time.get_string().c_str()));
			}
			catch(Exception::NotFound&) { }
		}

		int ret(0);
		while(!selected.empty())
		{
			if(selected.back()->get_time()!=old_2_new(selected.back()->get_time()))
			{
				Action::Handle action(Action::create("WaypointSet"));

				action->set_param("canvas",get_canvas());
				action->set_param("canvas_interface",get_canvas_interface());
				action->set_param("value_node",ValueNode::Handle::cast_static(value_node));

				Waypoint waypoint(*selected.back());
				waypoint.set_time(old_2_new(selected.back()->get_time()));

				action->set_param("waypoint",waypoint);

				assert(action->is_ready());
				if(!action->is_ready())
					throw Error(Error::TYPE_NOTREADY);

				add_action_front(action);

				ret++;
			}
			selected.pop_back();
		}
		return ret;
	}
	return 0;
}

void
Action::KeyframeSet::process_value_desc(const synfigapp::ValueDesc& value_desc)
{
	if(value_desc.is_value_node())
	{
		ValueNode::Handle value_node(value_desc.get_value_node());

		//if(guid_set.count(value_node->get_guid()))
		//	return;
		//guid_set.insert(value_node->get_guid());

		// If we are a dynamic list, then we need to update the ActivePoints
		if(ValueNode_DynamicList::Handle::cast_dynamic(value_node))
		{
			ValueNode_DynamicList::Handle value_node_dynamic(ValueNode_DynamicList::Handle::cast_dynamic(value_node));
			int i;
			for(i=0;i<value_node_dynamic->link_count();i++)
			{
				synfigapp::ValueDesc value_desc(value_node_dynamic,i);
				if(new_time>keyframe_prev && new_time<keyframe_next)
				{
					// In this circumstance, we need to adjust any
					// activepoints between the previous and next
					// keyframes
					scale_activepoints(value_desc,keyframe_prev,old_time,keyframe_prev,new_time);
					scale_activepoints(value_desc,old_time,keyframe_next,new_time,keyframe_next);
				}
				//else
				{
					Action::Handle action(ActivepointSetSmart::create());

					action->set_param("canvas",get_canvas());
					action->set_param("canvas_interface",get_canvas_interface());
					action->set_param("value_desc",value_desc);

					Activepoint activepoint;
					try
					{
						activepoint=*value_node_dynamic->list[i].find(old_time);
						activepoint.set_time(new_time);
					}
					catch(...)
					{
						activepoint.set_time(new_time);
						activepoint.set_state(value_node_dynamic->list[i].status_at_time(old_time));
						activepoint.set_priority(0);
					}
					action->set_param("activepoint",activepoint);

					assert(action->is_ready());
					if(!action->is_ready())
						throw Error(Error::TYPE_NOTREADY);

					add_action_front(action);
				}
			}
		}
		else if(ValueNode_Animated::Handle::cast_dynamic(value_node))
		{
			if(new_time>keyframe_prev && new_time<keyframe_next)
			{
					// In this circumstance, we need to adjust any
					// waypoints between the previous and next
					// keyframes
					scale_waypoints(value_desc,keyframe_prev,old_time,keyframe_prev,new_time);
					scale_waypoints(value_desc,old_time,keyframe_next,new_time,keyframe_next);
			}
			//else
			{
				ValueNode_Animated::Handle value_node_animated(ValueNode_Animated::Handle::cast_dynamic(value_node));

				Action::Handle action(WaypointSetSmart::create());

				action->set_param("canvas",get_canvas());
				action->set_param("canvas_interface",get_canvas_interface());
				action->set_param("value_node",ValueNode::Handle(value_node_animated));

				Waypoint waypoint;
				try
				{
					waypoint=*value_node_animated->find(old_time);
					waypoint.set_time(new_time);
				}
				catch(...)
				{
					waypoint.set_time(new_time);
					waypoint.set_value((*value_node_animated)(old_time));
					waypoint.set_before(synfigapp::Main::get_interpolation());
					waypoint.set_after(synfigapp::Main::get_interpolation());
				}
				action->set_param("waypoint",waypoint);

				assert(action->is_ready());
				if(!action->is_ready())
					throw Error(Error::TYPE_NOTREADY);

				add_action_front(action);
			}
		}
	}
}


void
Action::KeyframeSet::perform()
{
	KeyframeList::iterator iter;
	if (!get_canvas()->keyframe_list().find(keyframe, iter))
		throw Error(_("Unable to find the given keyframe"));
	old_time = iter->get_time();
	new_time=keyframe.get_time();

	// Check for collisions
	KeyframeList::iterator i;
	if (old_time != new_time && get_canvas()->keyframe_list().find(new_time, i))
		throw Error(_("Cannot change keyframe time because another keyframe already exists with that time."));
	keyframe_prev = get_canvas()->keyframe_list().find_prev(old_time, i)
	              ? i->get_time() : Time::begin();
	keyframe_next = get_canvas()->keyframe_list().find_next(old_time, i)
	              ? i->get_time() : Time::end();
	get_canvas()->keyframe_list().find_prev_next(old_time, keyframe_prev, keyframe_next);

	old_keyframe = *iter;
	*iter = keyframe;
	get_canvas()->keyframe_list().sync();

	try {
		Action::Super::perform();
	} catch(...) {
		if (get_canvas()->keyframe_list().find(old_keyframe, iter)) {
			*iter = old_keyframe;
			get_canvas()->keyframe_list().sync();
		}
		throw;
	}

	// Signal that a keyframe was changed
	if (get_canvas_interface())
		get_canvas_interface()->signal_keyframe_changed()(keyframe);
	else
		synfig::warning("CanvasInterface not set on action");
}

void
Action::KeyframeSet::undo()
{
	Action::Super::undo();

	KeyframeList::iterator iter;
	//*get_canvas()->keyframe_list().find(old_keyframe)=old_keyframe;
	if (get_canvas()->keyframe_list().find(old_keyframe, iter)) {
		*iter = old_keyframe;

		get_canvas()->keyframe_list().sync();

		// Signal that a layer has been inserted
		if(get_canvas_interface())
		{
			get_canvas_interface()->signal_keyframe_changed()(keyframe);
		}
		else synfig::warning("CanvasInterface not set on action");
	}
}
