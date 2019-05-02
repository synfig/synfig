/* === S Y N F I G ========================================================= */
/*!	\file valuedesclink.cpp
**	\brief Template File
**
**	$Id$
**
**	\legal
**	Copyright (c) 2002-2005 Robert B. Quattlebaum Jr., Adrian Bentley
**	Copyright (c) 2007, 2008 Chris Moore
**	Copyright (c) 2010, 2011 Carlos López
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

#include "valuedesclink.h"

#include <synfigapp/canvasinterface.h>
#include <synfig/valuenodes/valuenode_const.h>
#include <synfig/valuenodes/valuenode_scale.h>
#include <synfig/valuenodes/valuenode_composite.h>
#include <synfig/valuenode_registry.h>

#include <synfig/blinepoint.h>

#include <synfigapp/localization.h>
#include <zconf.h>

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

ACTION_INIT(Action::ValueDescLinkOpposite);
ACTION_SET_NAME(Action::ValueDescLinkOpposite,"ValueDescLinkOpposite");
ACTION_SET_LOCAL_NAME(Action::ValueDescLinkOpposite,N_("Link Opposite"));
ACTION_SET_TASK(Action::ValueDescLinkOpposite,"connect");
ACTION_SET_CATEGORY(Action::ValueDescLinkOpposite,Action::CATEGORY_VALUEDESC);
ACTION_SET_PRIORITY(Action::ValueDescLinkOpposite,0);
ACTION_SET_VERSION(Action::ValueDescLinkOpposite,"0.0");
ACTION_SET_CVS_ID(Action::ValueDescLinkOpposite,"$Id$");

/* === G L O B A L S ======================================================= */

/* === P R O C E D U R E S ================================================= */

/* === M E T H O D S ======================================================= */

Action::ValueDescLink::ValueDescLink():
poison(false), status_level(0), link_scalar(0.0), link_opposite(false)
{
}

Action::ParamVocab
Action::ValueDescLink::get_param_vocab()
{
	ParamVocab ret(Action::CanvasSpecific::get_param_vocab());

	ret.push_back(ParamDesc("value_desc",Param::TYPE_VALUEDESC)
		.set_local_name(_("ValueDesc to smart link"))
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
		// If we are handling a Composite WidthPoint then use its position as param
		if(value_desc.is_value_node() && value_desc.parent_is_linkable_value_node())
		{
			synfig::ValueNode_Composite::Handle compo(synfig::ValueNode_Composite::Handle::cast_dynamic(value_desc.get_value_node()));
			if(compo)
			{
				String param_name;
				if (value_desc.parent_is_value_desc() && !value_desc.get_sub_name().empty())
				{
					LinkableValueNode::Vocab vocab = compo->get_children_vocab();
					for(LinkableValueNode::Vocab::const_iterator i = vocab.begin(); i != vocab.end(); ++i)
						if (i->get_name() == value_desc.get_sub_name())
							param_name = value_desc.get_sub_name();
				}
				
				//! Test used for with point handle
				if (param_name.empty() && compo->get_type() == type_width_point)
					param_name = "position";

				if (param_name.empty() && compo->get_type() == type_bline_point)
					param_name = "point";
				
				if ( compo->get_type() == type_bline_point
				  && param_name == "t2"
				  && (*compo)(time).get(BLinePoint()).get_merge_tangent_both())
				{
					param_name = "t1";
				}
				
				if (!param_name.empty())
				{
					synfigapp::Action::Param param(synfigapp::ValueDesc(compo, compo->get_link_index_from_name(param_name)));
					return set_param("value_desc", param);
				}
			}
		}

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
			link_scalar=value_desc.parent_is_linkable_value_node()?value_desc.get_scalar():1.0;
			status_message = _("Used exported ValueNode ('") + link_value_node->get_id() + _("').");
		}
		else if(value_desc.is_value_node())
		{
			if(!link_value_node)
			{
				status_level = 1;
				status_message = _("Using the only available ValueNode.");
				link_value_node=value_desc.get_value_node();
				link_scalar=value_desc.parent_is_linkable_value_node()?value_desc.get_scalar():1.0;
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
					link_scalar=value_desc.parent_is_linkable_value_node()?value_desc.get_scalar():1.0;
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
				link_scalar=value_desc.parent_is_linkable_value_node()?value_desc.get_scalar():1.0;
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
					link_scalar=value_desc.parent_is_linkable_value_node()?value_desc.get_scalar():1.0;
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
					link_scalar=value_desc.parent_is_linkable_value_node()?value_desc.get_scalar():1.0;
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
					link_scalar=value_desc.parent_is_linkable_value_node()?value_desc.get_scalar():1.0;
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
										value_desc.get_value_type().description.local_name.c_str(),
										value_desc_list.front().get_value_type().description.local_name.c_str()));
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

	// Check if the selected  link value node is already a scale -1.0 Linkable Value Node
	bool link_is_scaled(false);
	if(synfig::ValueNode_Scale::Handle::cast_dynamic(link_value_node))
		{
			synfig::ValueNode_Const::Handle scale_vn(
					synfig::ValueNode_Const::Handle::cast_dynamic(
							synfig::ValueNode_Scale::Handle::cast_dynamic(link_value_node)->get_link(1)
							)
						);
			if(scale_vn)
				if((*scale_vn)(synfig::Time(0))==synfig::ValueBase(Real(-1.0)))
					link_is_scaled=true;
		}

	std::list<ValueDesc>::iterator iter;
	// Gets the scalar value of the current value node
	Real current_scalar(0);
	if(value_desc_list.begin()->parent_is_linkable_value_node())
		current_scalar=value_desc_list.begin()->get_scalar();
	bool found_inverse(false);
	// Check if we are dealing the case of linking different types of tangents
	for(iter=value_desc_list.begin();iter!=value_desc_list.end();++iter)
	{
		ValueDesc& v_desc(*iter);
		// If parent is linkable value node
		if(v_desc.parent_is_linkable_value_node())
		{
			// if the link describe to any tangent (index 4 or 5), continue
			if(v_desc.get_index() == 4 || v_desc.get_index() == 5)
			{
				synfig::Real iter_scalar=v_desc.get_scalar();
				// Let's compare the sign  of scalar of the value node with the current one
				// and remember if a change of sign is seen.
				if(iter_scalar*current_scalar < 0) // if they have different signs
				{
					found_inverse=true;
					current_scalar=iter_scalar;
				}
			}
			else // link doesn't describe a tangent
			{
				found_inverse=false;
				break;
			}
		}
		else // parent is not a linkable value node
		{
			found_inverse=false;
			break;
		}
	}
	// found_inverse =  true only if all they are tangents and some are inversed tangents
	//Now let's loop through all the value desc
	for(iter=value_desc_list.begin();iter!=value_desc_list.end();++iter)
	{
		ValueDesc& value_desc(*iter);

		// only one of the selected items can be exported - that's the one we're linking to
		// don't link it to itself
		if (value_desc.is_exported())
			continue;
		// Don't link the selected to itself (maybe it is redundant with the previous check)
		if(value_desc.get_value_node() == link_value_node)
			continue;
		// found_inverse xor link_opposite
		// If     found_inverse and not link_opposite then scale by -1 first (smart link)
		// If     found inverse and     link_opposite then do a direct link instead
		// If not found_inverse and not link_opposite then do a direct link instead
		// If not found_inverse and     link_opposite then scale by -1 first (smart link)
		if((found_inverse && !link_opposite) || (!found_inverse && link_opposite))
		{
			//Check if the current value node has opposite scalar than the link
			// value node to convert to scale -1.0 before connect.
			// Check also if the link value node is NOT also a scale -1
			// And check also if we are linking opposite
			if( (value_desc.get_scalar()*link_scalar<0 || link_opposite) && (link_is_scaled==false))
			{
				//Let's create a Scale Value Node
				synfig::ValueNode::Handle scale_value_node=synfig::ValueNodeRegistry::create("scale",iter->get_value(time));
				if(!scale_value_node)
					throw Error(Error::TYPE_BUG);
				scale_value_node->set_parent_canvas(get_canvas());
				//Let's connect the new Scale Value Node to the value node
				Action::Handle action1(Action::create("ValueDescConnect"));
				if(!action1)
					throw Error(Error::TYPE_CRITICAL);
				action1->set_param("canvas",get_canvas());
				action1->set_param("canvas_interface",get_canvas_interface());
				action1->set_param("dest",value_desc);
				action1->set_param("src",scale_value_node);
				assert(action1->is_ready());
				if(!action1->is_ready())
					throw Error(Error::TYPE_NOTREADY);
				add_action_front(action1);

				//Let's Connect the link value node to the scale value node link subparam
				Action::Handle action2(Action::create("ValueNodeLinkConnect"));
				if(!action2)
					throw Error(Error::TYPE_CRITICAL);

				action2->set_param("canvas",get_canvas());
				action2->set_param("canvas_interface",get_canvas_interface());
				action2->set_param("parent_value_node",scale_value_node);
				action2->set_param("index",0);
				action2->set_param("value_node",link_value_node);
				assert(action2->is_ready());
				if(!action2->is_ready())
					throw Error(Error::TYPE_NOTREADY);
				add_action_front(action2);

				//Let's Set the scale to -1
				Action::Handle action3(Action::create("ValueNodeConstSet"));
				if(!action3)
					throw Error(Error::TYPE_CRITICAL);

				action3->set_param("canvas",get_canvas());
				action3->set_param("canvas_interface",get_canvas_interface());
				action3->set_param("value_node",synfig::LinkableValueNode::Handle::cast_dynamic(scale_value_node)->get_link(1));
				action3->set_param("new_value",synfig::ValueBase(Real(-1.0)));
				assert(action3->is_ready());
				if(!action3->is_ready())
					throw Error(Error::TYPE_NOTREADY);
				add_action_front(action3);
			}
			else if((iter->get_scalar()*link_scalar<0 || link_opposite) && (link_is_scaled==true))
			{
				//Let's connect the link value node -> link to the value node
				// There is not needed conversion to scale of the value node
				// because the link value node is already a scale -1
				Action::Handle action4(Action::create("ValueDescConnect"));
				if(!action4)
					throw Error(Error::TYPE_CRITICAL);
				action4->set_param("canvas",get_canvas());
				action4->set_param("canvas_interface",get_canvas_interface());
				action4->set_param("dest",value_desc);
				action4->set_param("src",synfig::ValueNode_Scale::Handle::cast_dynamic(link_value_node)->get_link(0));
				assert(action4->is_ready());
				if(!action4->is_ready())
					throw Error(Error::TYPE_NOTREADY);
				add_action_front(action4);
			}
			else
			{
				//Let's connect the link value node to the value node
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
		} // found inverse
		else // Not found inverse so do a regular link
		{
			//Let's connect the link value node to the value node
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
	}

	synfig::info("http://synfig.org/Linking#Tier_%d : %s", status_level, status_message.c_str());
}


Action::ValueDescLinkOpposite::ValueDescLinkOpposite()
{
}

Action::ParamVocab
Action::ValueDescLinkOpposite::get_param_vocab()
{
	return Action::ValueDescLink::get_param_vocab();
}

bool
Action::ValueDescLinkOpposite::is_candidate(const ParamList &x)
{
	// If action parameters are not Value Desc
	if(!candidate_check(get_param_vocab(),x))
		return false;

	int total_tangents=0;
	ParamList::const_iterator iter;
	//Search thru all the Param and pick up the value descriptions
	for(iter=x.begin(); iter!=x.end(); iter++)
	{
		if(iter->first == "value_desc")
		{
			ValueDesc v_desc(iter->second.get_value_desc());
			// if the value description parent is linkable value node, continue
			if(!v_desc.parent_is_linkable_value_node())
				return false;
			// if the link describe to any tangent (index 4 or 5), continue
			if(v_desc.get_index() != 4 && v_desc.get_index() != 5)
				return false;
			total_tangents++;
		}
	}
	// If we found two tangents then continue
	if(total_tangents!=2)
		return false;
	// We have reached exactly two tangents
	return true;
}

bool
Action::ValueDescLinkOpposite::set_param(const synfig::String& name, const Action::Param &param)
{
	return Action::ValueDescLink::set_param(name,param);
}

bool
Action::ValueDescLinkOpposite::is_ready()const
{
	return Action::ValueDescLink::is_ready();
}

void
Action::ValueDescLinkOpposite::prepare()
{
	// prepare to do a opposite link and reuse the code from ValueDescLink
	link_opposite=true;
	ValueDescLink::prepare();
}
