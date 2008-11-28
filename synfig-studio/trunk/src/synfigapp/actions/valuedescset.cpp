/* === S Y N F I G ========================================================= */
/*!	\file valuedescset.cpp
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

#include "layerparamset.h"
#include "valuenodeconstset.h"
#include "valuedescconnect.h"
#include "waypointsetsmart.h"

#include "valuedescset.h"
#include <synfigapp/canvasinterface.h>
#include <synfig/valuenode_composite.h>
#include <synfig/valuenode_radialcomposite.h>
#include <synfig/valuenode_reference.h>
#include <synfigapp/main.h>

#include <synfigapp/general.h>

#endif

using namespace std;
using namespace etl;
using namespace synfig;
using namespace synfigapp;
using namespace Action;

/* === M A C R O S ========================================================= */

ACTION_INIT_NO_GET_LOCAL_NAME(Action::ValueDescSet);
ACTION_SET_NAME(Action::ValueDescSet,"ValueDescSet");
ACTION_SET_LOCAL_NAME(Action::ValueDescSet,N_("Set ValueDesc"));
ACTION_SET_TASK(Action::ValueDescSet,"set");
ACTION_SET_CATEGORY(Action::ValueDescSet,Action::CATEGORY_VALUEDESC);
ACTION_SET_PRIORITY(Action::ValueDescSet,0);
ACTION_SET_VERSION(Action::ValueDescSet,"0.0");
ACTION_SET_CVS_ID(Action::ValueDescSet,"$Id$");

/* === G L O B A L S ======================================================= */

/* === P R O C E D U R E S ================================================= */

/* === M E T H O D S ======================================================= */

Action::ValueDescSet::ValueDescSet():
	time(0)
{
}

synfig::String
Action::ValueDescSet::get_local_name()const
{
	return strprintf(_("Set %s"),
					 value_desc
					 ? value_desc.get_description().c_str()
					 : _("ValueDesc"));
}

Action::ParamVocab
Action::ValueDescSet::get_param_vocab()
{
	ParamVocab ret(Action::CanvasSpecific::get_param_vocab());

	ret.push_back(ParamDesc("value_desc",Param::TYPE_VALUEDESC)
		.set_local_name(_("ValueDesc"))
	);

	ret.push_back(ParamDesc("new_value",Param::TYPE_VALUE)
		.set_local_name(_("ValueBase"))
	);

	ret.push_back(ParamDesc("time",Param::TYPE_TIME)
		.set_local_name(_("Time"))
		.set_optional()
	);

	return ret;
}

bool
Action::ValueDescSet::is_candidate(const ParamList &x)
{
	return candidate_check(get_param_vocab(),x);
}

bool
Action::ValueDescSet::set_param(const synfig::String& name, const Action::Param &param)
{
	if(name=="value_desc" && param.get_type()==Param::TYPE_VALUEDESC)
	{
		value_desc=param.get_value_desc();

		return true;
	}

	if(name=="new_value" && param.get_type()==Param::TYPE_VALUE)
	{
		value=param.get_value();

		return true;
	}

	if(name=="time" && param.get_type()==Param::TYPE_TIME)
	{
		time=param.get_time();

		return true;
	}

	return Action::CanvasSpecific::set_param(name,param);
}

bool
Action::ValueDescSet::is_ready()const
{
	if(!value_desc || !value.is_valid())
		return false;
	return Action::CanvasSpecific::is_ready();
}

void
Action::ValueDescSet::prepare()
{
	clear();

	// If our tangents are merged, and
	// our first tangent is being manipulated,
	// then we also need to adjust the other
	// tangent.
	if(	value_desc.parent_is_value_node() &&
		value_desc.get_parent_value_node()->get_type()==ValueBase::TYPE_BLINEPOINT &&
		(value_desc.get_index()==4 || value_desc.get_index()==5) &&
		(*value_desc.get_parent_value_node())(time).get(BLinePoint()).get_split_tangent_flag()==false)
	{
		{
			ValueNode_Composite::Handle parent_value_node;
			parent_value_node=parent_value_node.cast_dynamic(value_desc.get_parent_value_node());
			assert(parent_value_node);

			Vector t1((*parent_value_node->get_link("t1"))(time));
			Vector t2((*parent_value_node->get_link("t2"))(time));
		}

		if (value_desc.get_index()==4) {
			ValueNode_Composite::Handle parent_value_node;
			parent_value_node=parent_value_node.cast_dynamic(value_desc.get_parent_value_node());

			assert(parent_value_node);

			Action::Handle action(Action::create("ValueDescSet"));

			if(!action)
				throw Error(_("Unable to find action value_desc_set (bug)"));

			action->set_param("canvas",get_canvas());
			action->set_param("canvas_interface",get_canvas_interface());
			action->set_param("time",time);
			action->set_param("new_value",value);
			action->set_param("value_desc",ValueDesc(parent_value_node,5));

			if(!action->is_ready())
				throw Error(Error::TYPE_NOTREADY);

			add_action(action);
		}
	}

	// If we are a reference value node, then
	// we need to distribute the changes to the
	// referenced value node
	if(value_desc.is_value_node() && ValueNode_Reference::Handle::cast_dynamic(value_desc.get_value_node()))
	{
		ValueDesc reference_value_desc(ValueNode_Reference::Handle::cast_dynamic(value_desc.get_value_node()),0);

		Action::Handle action(Action::create("ValueDescSet"));

		if(!action)
			throw Error(_("Unable to find action value_desc_set (bug)"));

		action->set_param("canvas",get_canvas());
		action->set_param("canvas_interface",get_canvas_interface());
		action->set_param("time",time);
		action->set_param("new_value",value);
		action->set_param("value_desc",reference_value_desc);

		if(!action->is_ready())
			throw Error(Error::TYPE_NOTREADY);

		add_action(action);

		return;
	}

	// If we are a composite value node, then
	// we need to distribute the changes to the
	// individual parts
	if(value_desc.is_value_node() && ValueNode_Composite::Handle::cast_dynamic(value_desc.get_value_node()))
	{
		ValueBase components[6];
		int n_components(0);
		switch(value.get_type())
		{
		case ValueBase::TYPE_VECTOR:
			components[0]=value.get(Vector())[0];
			components[1]=value.get(Vector())[1];
			n_components=2;
			break;
		case ValueBase::TYPE_COLOR:
			components[0]=value.get(Color()).get_r();
			components[1]=value.get(Color()).get_g();
			components[2]=value.get(Color()).get_b();
			components[3]=value.get(Color()).get_a();
			n_components=4;
			break;
		case ValueBase::TYPE_SEGMENT:
			components[0]=value.get(Segment()).p1;
			components[1]=value.get(Segment()).t1;
			components[2]=value.get(Segment()).p2;
			components[3]=value.get(Segment()).t2;
			n_components=4;
			break;
		case ValueBase::TYPE_BLINEPOINT:
		{
			BLinePoint bline_point(value);
			components[0]=bline_point.get_vertex();
			components[1]=bline_point.get_width();
			components[2]=bline_point.get_origin();
			components[3]=bline_point.get_split_tangent_flag();
			components[4]=bline_point.get_tangent1();
			components[5]=bline_point.get_tangent2();
			n_components=6;
			break;
		}
		default:
			throw Error(_("Bad type for composite (%s)"),ValueBase::type_local_name(value.get_type()).c_str());
			break;
		}

		for(int i=0;i<n_components;i++)
		{
			ValueDesc component_value_desc(ValueNode_Composite::Handle::cast_dynamic(value_desc.get_value_node()),i);

			Action::Handle action(Action::create("ValueDescSet"));

			if(!action)
				throw Error(_("Unable to find action value_desc_set (bug)"));

			action->set_param("canvas",get_canvas());
			action->set_param("canvas_interface",get_canvas_interface());
			action->set_param("time",time);
			action->set_param("new_value",components[i]);
			action->set_param("value_desc",component_value_desc);

			if(!action->is_ready())
				throw Error(Error::TYPE_NOTREADY);

			add_action(action);
		}

		return;
	}


	// If we are a RADIAL composite value node, then
	// we need to distribute the changes to the
	// individual parts
	if(value_desc.is_value_node() && ValueNode_RadialComposite::Handle::cast_dynamic(value_desc.get_value_node()))
	{
		ValueBase components[6];
		int n_components(0);
		switch(value.get_type())
		{
		case ValueBase::TYPE_VECTOR:
		{
			Angle old_angle = (*(ValueNode_RadialComposite::Handle::cast_dynamic(
									 value_desc.get_value_node())->get_link_vfunc(1)))(time).get(Angle());
			Vector vect(value.get(Vector()));
			components[0]=vect.mag();
			Angle change = Angle(Angle::tan(vect[1],vect[0])) - old_angle;
			while (change < Angle::deg(-180)) change += Angle::deg(360);
			while (change > Angle::deg(180)) change -= Angle::deg(360);
			components[1]=old_angle + change;
			n_components=2;
		}
			break;
		case ValueBase::TYPE_COLOR:
			components[0]=value.get(Color()).get_y();
			components[1]=value.get(Color()).get_s();
			components[2]=value.get(Color()).get_hue();
			components[3]=value.get(Color()).get_a();
			n_components=4;
			break;
		default:
			throw Error(_("Bad type for radial composite (%s)"),ValueBase::type_local_name(value.get_type()).c_str());
			break;
		}
		for(int i=0;i<n_components;i++)
		{
			ValueDesc component_value_desc(ValueNode_RadialComposite::Handle::cast_dynamic(value_desc.get_value_node()),i);

			Action::Handle action(Action::create("ValueDescSet"));

			if(!action)
				throw Error(_("Unable to find action value_desc_set (bug)"));

			action->set_param("canvas",get_canvas());
			action->set_param("canvas_interface",get_canvas_interface());
			action->set_param("time",time);
			action->set_param("new_value",components[i]);
			action->set_param("value_desc",component_value_desc);

			if(!action->is_ready())
				throw Error(Error::TYPE_NOTREADY);

			add_action(action);
		}

		return;
	}

	// If we are merging the tangents of a BLinePoint,
	// we must also set the second tangent for things
	// to interpolate properly
	if (value_desc.parent_is_value_node() &&
	    value_desc.get_parent_value_node()->get_type()==ValueBase::TYPE_BLINEPOINT &&
	    value_desc.get_index()==3)
	{
		ValueNode_Composite::Handle parent_value_node;
		parent_value_node=parent_value_node.cast_dynamic(value_desc.get_parent_value_node());

		assert(parent_value_node);

		// are we splitting or merging the tangents?
	    if (value.get(bool()))
	    {
			// we are splitting tangents

			Action::Handle action(Action::create("ValueDescSet"));

			if(!action)
				throw Error(_("Unable to find action value_desc_set (bug)"));

			action->set_param("canvas",get_canvas());
			action->set_param("canvas_interface",get_canvas_interface());
			action->set_param("time",time);
			action->set_param("new_value",(*parent_value_node->get_link(4))(time));
			action->set_param("value_desc",ValueDesc(parent_value_node,5));

			if(!action->is_ready())
				throw Error(Error::TYPE_NOTREADY);

			add_action(action);
	    }
	    else
	    {
			// we are merging tangents

			// the merged tangent should be the average of the 2 tangents we're merging
			ValueBase average(((Vector)((*parent_value_node->get_link("t1"))(time)) +
							   (Vector)((*parent_value_node->get_link("t2"))(time))) / 2);

			{
				Action::Handle action(Action::create("ValueDescSet"));

				if(!action)
					throw Error(_("Unable to find action value_desc_set (bug)"));

				action->set_param("canvas",get_canvas());
				action->set_param("canvas_interface",get_canvas_interface());
				action->set_param("time",time);
				action->set_param("new_value",average);
				action->set_param("value_desc",ValueDesc(parent_value_node,4));

				if(!action->is_ready())
					throw Error(Error::TYPE_NOTREADY);

				add_action(action);
			}

			{
				Action::Handle action(Action::create("ValueDescSet"));

				if(!action)
					throw Error(_("Unable to find action value_desc_set (bug)"));

				action->set_param("canvas",get_canvas());
				action->set_param("canvas_interface",get_canvas_interface());
				action->set_param("time",time);
				action->set_param("new_value",average);
				action->set_param("value_desc",ValueDesc(parent_value_node,5));

				if(!action->is_ready())
					throw Error(Error::TYPE_NOTREADY);

				add_action(action);
			}
	    }

	}

/*	DEBUGPOINT();
	if(	value_desc.parent_is_value_node())
	{
		DEBUGPOINT();
		if(value_desc.get_parent_value_node()->get_type()==ValueBase::TYPE_BLINEPOINT)
		{
			DEBUGPOINT();
			if(value_desc.get_index()==4)
			{
				DEBUGPOINT();
				if((*value_desc.get_parent_value_node())(time).get(BLinePoint()).get_split_tangent_flag()==false)
				{
					DEBUGPOINT();
				}
			}
		}
	}
*/


	// If we are in animate editing mode
	if(get_edit_mode()&MODE_ANIMATE)
	{

		ValueNode_Animated::Handle& value_node(value_node_animated);

		// If this value isn't a ValueNode_Animated, but
		// it is somewhat constant, then go ahead and convert
		// it to a ValueNode_Animated.
		if(!value_desc.is_value_node() || ValueNode_Const::Handle::cast_dynamic(value_desc.get_value_node()))
		{
			ValueBase value;
			if(value_desc.is_value_node())
				value=ValueNode_Const::Handle::cast_dynamic(value_desc.get_value_node())->get_value();
			else
				value=value_desc.get_value();

			if(!value_node)value_node=ValueNode_Animated::create(value,time);
			//if(!value_node)value_node=ValueNode_Animated::create(value.get_type());

			Action::Handle action;

			if(!value_desc.is_value_node())
			{
				action=(ValueDescConnect::create());
				action->set_param("dest",value_desc);
				action->set_param("src",ValueNode::Handle(value_node));
			}
			else
			{
				action=Action::create("ValueNodeReplace");
				action->set_param("dest",value_desc.get_value_node());
				action->set_param("src",ValueNode::Handle(value_node));
			}

			action->set_param("canvas",get_canvas());
			action->set_param("canvas_interface",get_canvas_interface());

			if(!action->is_ready())
				throw Error(Error::TYPE_NOTREADY);

			add_action_front(action);
		}
		else
		{
			value_node=value_node.cast_dynamic(value_desc.get_value_node());
		}

		if(!value_node)
			throw Error(_("Direct manipulation of this ValueNode type is not yet supported"));

		Action::Handle action(WaypointSetSmart::create());

		//Waypoint waypoint(value,time);

		Waypoint waypoint(value_node->new_waypoint_at_time(time));
		waypoint.set_value(value);

		waypoint.set_before(synfigapp::Main::get_interpolation());
		waypoint.set_after(synfigapp::Main::get_interpolation());

		action->set_param("canvas",get_canvas());
		action->set_param("canvas_interface",get_canvas_interface());
		action->set_param("value_node",ValueNode::Handle(value_node));
		action->set_param("waypoint",waypoint);

		if(!action->is_ready())
			throw Error(Error::TYPE_NOTREADY);

		add_action(action);

		return;
	}
	else
	{
		if(value_desc.is_value_node())
		{
			if(ValueNode_Const::Handle::cast_dynamic(value_desc.get_value_node()))
			{
				Action::Handle action(ValueNodeConstSet::create());

				action->set_param("canvas",get_canvas());
				action->set_param("canvas_interface",get_canvas_interface());
				action->set_param("value_node",value_desc.get_value_node());
				action->set_param("new_value",value);

				if(!action->is_ready())
					throw Error(Error::TYPE_NOTREADY);

				add_action_front(action);
				return;
			}
			else
			if(ValueNode_Animated::Handle::cast_dynamic(value_desc.get_value_node()))
				throw Error(_("You must be in Animate-Editing-Mode to directly manipulate this value"));
			else
				throw Error(_("Direct manipulation of this ValueNode type is not yet supported"));
		}
		else
		if(value_desc.parent_is_layer_param() && !value_desc.is_value_node())
		{
			Action::Handle layer_param_set(LayerParamSet::create());

			layer_param_set->set_param("canvas",get_canvas());
			layer_param_set->set_param("canvas_interface",get_canvas_interface());
			layer_param_set->set_param("layer",value_desc.get_layer());
			layer_param_set->set_param("param",value_desc.get_param_name());
			layer_param_set->set_param("new_value",value);

			if(!layer_param_set->is_ready())
				throw Error(Error::TYPE_NOTREADY);

			add_action_front(layer_param_set);
			return;
		}

		throw Error(_("Unsupported ValueDesc type"));
	}
}
