/* === S Y N F I G ========================================================= */
/*!	\file valuedesclink.cpp
**	\brief Template File
**
**	$Id$
**
**	\legal
**	Copyright (c) 2002-2005 Robert B. Quattlebaum Jr., Adrian Bentley
**	Copyright (c) 2007, 2008 Chris Moore
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

#include "valuedesclink.h"

#include <synfigapp/canvasinterface.h>
#include <synfig/valuenode_const.h>

#include <synfigapp/general.h>

#endif

using namespace std;
using namespace etl;
using namespace synfig;
using namespace synfigapp;
using namespace Action;

/* === M A C R O S ========================================================= */

ACTION_INIT(Action::ValueDescLink);
ACTION_SET_NAME(Action::ValueDescLink,"ValueDescLink");
ACTION_SET_LOCAL_NAME(Action::ValueDescLink,N_("Link"));
ACTION_SET_TASK(Action::ValueDescLink,"connect");
ACTION_SET_CATEGORY(Action::ValueDescLink,Action::CATEGORY_VALUEDESC);
ACTION_SET_PRIORITY(Action::ValueDescLink,0);
ACTION_SET_VERSION(Action::ValueDescLink,"0.0");
ACTION_SET_CVS_ID(Action::ValueDescLink,"$Id$");

/* === G L O B A L S ======================================================= */

/* === P R O C E D U R E S ================================================= */

/* === M E T H O D S ======================================================= */

Action::ValueDescLink::ValueDescLink(): poison(false), status_level(0)
{
}

Action::ParamVocab
Action::ValueDescLink::get_param_vocab()
{
	ParamVocab ret(Action::CanvasSpecific::get_param_vocab());

	ret.push_back(ParamDesc("value_desc",Param::TYPE_VALUEDESC)
		.set_local_name(_("ValueDesc to link"))
		.set_requires_multiple()
	);

	return ret;
}

bool
Action::ValueDescLink::is_candidate(const ParamList &x)
{
	return candidate_check(get_param_vocab(),x);
}

bool
Action::ValueDescLink::set_param(const synfig::String& name, const Action::Param &param)
{
	if(name=="time" && param.get_type()==Param::TYPE_TIME)
	{
		time=param.get_time();
		return true;
	}

	// don't bother looking for the best value to use if there's already been an error
	if (poison==true) return false;

	if(name=="value_desc" && param.get_type()==Param::TYPE_VALUEDESC)
	{
		ValueDesc value_desc(param.get_value_desc());

		if(value_desc.is_value_node() && value_desc.get_value_node()->is_exported())
		{
			if(link_value_node==value_desc.get_value_node())
				return true;

			if(link_value_node && link_value_node->is_exported())
			{
				poison=true;
				status_message = (_("Cannot link two different exported values ('") +
								  value_desc.get_value_node()->get_id() + _("' and '") +
								  link_value_node->get_id()) + _("')");
				return false;
			}

			link_value_node=value_desc.get_value_node();
			status_message = _("Used exported ValueNode ('") + link_value_node->get_id() + _("').");
		}
		else if(value_desc.is_value_node())
		{
			if(!link_value_node)
			{
				status_level = 1;
				status_message = _("Using the only available ValueNode.");
				link_value_node=value_desc.get_value_node();
			}
			else if(link_value_node->is_exported())
			{
				// we've already seen an exported value, so use that rather than the current value
			}
			// Use the one that is referenced more
			else if(link_value_node->rcount()!=value_desc.get_value_node()->rcount())
			{
				if(link_value_node->rcount()<value_desc.get_value_node()->rcount())
				{
					status_level = 2;
					status_message = _("Using the most referenced ValueNode.");
					link_value_node=value_desc.get_value_node();
				}
				else if (status_level <= 2)
				{
					status_level = 2;
					status_message = _("Using the most referenced ValueNode.");
				}
			}
			// If the current link value node is a constant and
			// this one isn't, then give preference to the exotic
			else if(ValueNode_Const::Handle::cast_dynamic(link_value_node) && !ValueNode_Const::Handle::cast_dynamic(value_desc.get_value_node()))
			{
				status_level = 3;
				status_message = _("There's a tie for most referenced; using the animated ValueNode.");
				link_value_node=value_desc.get_value_node();
			}
			else if(ValueNode_Const::Handle::cast_dynamic(value_desc.get_value_node()) && !ValueNode_Const::Handle::cast_dynamic(link_value_node))
			{
				if (status_level <= 3)
				{
					status_level = 3;
					status_message = _("There's a tie for most referenced; using the animated ValueNode.");
				}
			}
			// If both are animated, and this one has more waypoints, then use the one with more waypoints
			else if(ValueNode_Animated::Handle::cast_dynamic(link_value_node) &&
					ValueNode_Animated::Handle::cast_dynamic(value_desc.get_value_node()) &&
					ValueNode_Animated::Handle::cast_dynamic(link_value_node)->waypoint_list().size() !=
					ValueNode_Animated::Handle::cast_dynamic(value_desc.get_value_node())->waypoint_list().size())
			{
				if (ValueNode_Animated::Handle::cast_dynamic(link_value_node)->waypoint_list().size() <
					ValueNode_Animated::Handle::cast_dynamic(value_desc.get_value_node())->waypoint_list().size())
				{
					status_level = 4;
					status_message = _("There's a tie for most referenced, and both are animated; using the one with the most waypoints.");
					link_value_node=value_desc.get_value_node();
				}
				else if (status_level <= 4)
				{
					status_level = 4;
					status_message = _("There's a tie for most referenced, and both are animated; using the one with the most waypoints.");
				}
			}
			// If both are Linkable Value Nodes and has waypoint in its children, use the one with more waypoints
			else if(LinkableValueNode::Handle::cast_dynamic(link_value_node) &&
					LinkableValueNode::Handle::cast_dynamic(value_desc.get_value_node()) &&
					LinkableValueNode::Handle::cast_dynamic(link_value_node)->get_times().size() !=
					LinkableValueNode::Handle::cast_dynamic(value_desc.get_value_node())->get_times().size())
			{
				if(LinkableValueNode::Handle::cast_dynamic(link_value_node)->get_times().size() <
				LinkableValueNode::Handle::cast_dynamic(value_desc.get_value_node())->get_times().size())
				{
					status_level = 4;
					status_message = _("There's a tie for most referenced, and both are linkable value node animated; using the one with the most waypoints.");
					link_value_node=value_desc.get_value_node();
				}
				else if (status_level <= 4)
				{
					status_level = 4;
					status_message = _("There's a tie for most referenced, and both are linkable value node animated; using the one with the most waypoints.");
				}
			}
			// Use the one that was least recently changed
			else if(link_value_node->get_time_last_changed()!=value_desc.get_value_node()->get_time_last_changed())
			{
				if(link_value_node->get_time_last_changed()>value_desc.get_value_node()->get_time_last_changed())
				{
					status_level = 5;
					status_message = _("Everything is tied; using the least recently modified value.");
					link_value_node=value_desc.get_value_node();
				}
				else if (status_level <= 5)
				{
					status_level = 5;
					status_message = _("Everything is tied; using the least recently modified value.");
				}
			}
			else
			{
				status_level = 6;
				status_message = _("Absolutely everything is tied.");
			}
		}

		if(value_desc_list.size() && value_desc.get_value_type()!=value_desc_list.front().get_value_type())
		{
			// Everything must be of the same type
			poison=true;
			status_message = (strprintf(_("Cannot link two values of different types ('%s' and '%s')"),
										ValueBase::type_local_name(value_desc.get_value_type()).c_str(),
										ValueBase::type_local_name(value_desc_list.front().get_value_type()).c_str()));
			return false;
		}

		value_desc_list.push_back(value_desc);

		return true;
	}

	return Action::CanvasSpecific::set_param(name,param);
}

bool
Action::ValueDescLink::is_ready()const
{
	if(poison)
		return true;
	if(value_desc_list.size()<=1)
		return false;
	return Action::CanvasSpecific::is_ready();
}

void
Action::ValueDescLink::prepare()
{
	if(poison)
		throw Error(status_message.c_str());

	if(value_desc_list.empty())
		throw Error(Error::TYPE_NOTREADY);

	clear();

	if(!link_value_node)
	{
		status_message = _("No ValueNodes were available, so one was created.");
		ValueDesc& value_desc(value_desc_list.back());

		link_value_node=ValueNode_Const::create(value_desc.get_value(time));

		Action::Handle action(Action::create("ValueDescConnect"));

		action->set_param("canvas",get_canvas());
		action->set_param("canvas_interface",get_canvas_interface());
		action->set_param("src",link_value_node);
		action->set_param("dest",value_desc);

		assert(action->is_ready());
		if(!action->is_ready())
			throw Error(Error::TYPE_NOTREADY);

		add_action_front(action);
	}

	/*
	if(!link_value_node->is_exported())
	{
		Action::Handle action(Action::create("ValueNodeAdd"));

		action->set_param("canvas",get_canvas());
		action->set_param("canvas_interface",get_canvas_interface());
		action->set_param("new",link_value_node);
		action->set_param("name",strprintf(_("Unnamed%08d"),synfig::UniqueID().get_uid()));

		assert(action->is_ready());
		if(!action->is_ready())
			throw Error(Error::TYPE_NOTREADY);

		add_action_front(action);
	}
	*/

	std::list<ValueDesc>::iterator iter;
	for(iter=value_desc_list.begin();iter!=value_desc_list.end();++iter)
	{
		ValueDesc& value_desc(*iter);

		// only one of the selected items can be exported - that's the one we're linking to
		// don't link it to itself
		if (value_desc.is_exported())
			continue;

		Action::Handle action(Action::create("ValueDescConnect"));

		action->set_param("canvas",get_canvas());
		action->set_param("canvas_interface",get_canvas_interface());
		action->set_param("src",link_value_node);
		action->set_param("dest",value_desc);

		assert(action->is_ready());
		if(!action->is_ready())
			throw Error(Error::TYPE_NOTREADY);

		add_action_front(action);
	}

	synfig::info("http://synfig.org/Linking#Tier_%d : %s", status_level, status_message.c_str());
}
