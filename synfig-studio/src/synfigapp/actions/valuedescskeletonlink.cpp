/* === S Y N F I G ========================================================= */
/*!	\file valuedescskeletonlink.cpp
**	\brief Template File
**
**	$Id$
**
**	\legal
**	......... ... 2013 Ivan Mahonin
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

#include "layerparamconnect.h"
#include "valuenodelinkconnect.h"
#include "valuenodereplace.h"
#include "valuedescskeletonlink.h"
#include "valuenodestaticlistinsert.h"

#include <synfigapp/canvasinterface.h>
#include <synfig/boneweightpair.h>
#include <synfig/valuenode_const.h>
#include <synfig/valuenode_composite.h>
#include <synfig/valuenode_bone.h>
#include <synfig/valuenode_bonelink.h>
#include <synfig/valuenode_boneweightpair.h>
#include <synfig/valuenode_staticlist.h>
#include <synfig/valueoperations.h>

#include <synfigapp/general.h>

#endif

using namespace std;
using namespace etl;
using namespace synfig;
using namespace synfigapp;
using namespace Action;

/* === M A C R O S ========================================================= */

ACTION_INIT(Action::ValueDescSkeletonLink);
ACTION_SET_NAME(Action::ValueDescSkeletonLink,"ValueDescSkeletonLink");
ACTION_SET_LOCAL_NAME(Action::ValueDescSkeletonLink,N_("Link to Skeleton"));
ACTION_SET_TASK(Action::ValueDescSkeletonLink,"bone_link");
ACTION_SET_CATEGORY(Action::ValueDescSkeletonLink,Action::CATEGORY_VALUEDESC);
ACTION_SET_PRIORITY(Action::ValueDescSkeletonLink,0);
ACTION_SET_VERSION(Action::ValueDescSkeletonLink,"0.0");
ACTION_SET_CVS_ID(Action::ValueDescSkeletonLink,"$Id$");

/* === G L O B A L S ======================================================= */

/* === P R O C E D U R E S ================================================= */

/* === M E T H O D S ======================================================= */

Action::ValueDescSkeletonLink::ValueDescSkeletonLink():
	time(0)
{
}

Action::ParamVocab
Action::ValueDescSkeletonLink::get_param_vocab()
{
	ParamVocab ret(Action::CanvasSpecific::get_param_vocab());

	ret.push_back(ParamDesc("selected_value_desc",Param::TYPE_VALUEDESC)
		.set_local_name(_("ValueDesc to link"))
		.set_supports_multiple()
	);
	ret.push_back(ParamDesc("value_desc",Param::TYPE_VALUEDESC)
		.set_local_name(_("ValueDesc of Skeleton's Bone"))
	);
	ret.push_back(ParamDesc("time",Param::TYPE_TIME)
		.set_local_name(_("Time"))
		.set_optional()
	);

	return ret;
}

bool
Action::ValueDescSkeletonLink::is_candidate(const ParamList &x)
{
	ParamList::const_iterator i;

	ValueDesc value_desc(x.find("value_desc")->second.get_value_desc());

	if (!candidate_check(get_param_vocab(),x))
		return false;

	return value_desc.parent_is_value_node()
		&& ValueNode_Bone::Handle::cast_dynamic(value_desc.get_parent_value_node());
}

bool
Action::ValueDescSkeletonLink::set_param(const synfig::String& name, const Action::Param &param)
{
	if (name == "value_desc" && param.get_type() == Param::TYPE_VALUEDESC
	 && param.get_value_desc().parent_is_value_node()
	 && ValueNode_Bone::Handle::cast_dynamic(param.get_value_desc().get_parent_value_node()) )
	{
		value_desc = param.get_value_desc();
		return true;
	}

	if (name == "selected_value_desc" && param.get_type() == Param::TYPE_VALUEDESC)
	{
		value_desc_list.push_back(param.get_value_desc());
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
Action::ValueDescSkeletonLink::is_ready()const
{
	if (value_desc_list.empty())
		return false;
	if (!value_desc)
		return false;
	return Action::CanvasSpecific::is_ready();
}

void
Action::ValueDescSkeletonLink::prepare()
{
	if (value_desc_list.empty())
		throw Error(Error::TYPE_NOTREADY);

	clear();
	ValueNode_Bone::Handle bone_value_node;
	if (value_desc.parent_is_value_node())
		bone_value_node = ValueNode_Bone::Handle::cast_dynamic(value_desc.get_parent_value_node());

	if (!bone_value_node)
		throw Error(Error::TYPE_NOTREADY);

	/*
	 * 
	 * FURTHER TEXT IS COMMENTED, BECAUSE ITS MOSTLY A PSEUDOCODE
	 * 
	
	// Take bone_value_node and get its parent Skeleton layer:
	skeleton_layer = ...
	// Get list of all bones in this skeleton layer
	bones_list = ...
	

	for (std::list<ValueDesc>::iterator iter = value_desc_list.begin(); iter != value_desc_list.end(); ++iter)
	{
		ValueDesc& value_desc(*iter);

		if (!ValueNode_BoneLink::check_type(value_desc.get_value_type()))
			continue;
		if (value_desc.parent_is_value_node() && bone_value_node == value_desc.get_parent_value_node())
			continue;
		
		// List of bones influencing current item
		std::list<ValueNode_Bone::Handle> influence_list;
		for each bone in bones_list
		{
			if (bone->have_influence_on(value_desc.get_value(time)))
				influence_list.push(bone);
		}
		
		if (influence_list.size() == 1)
		{
			// create new BoneLink
			ValueNode_BoneLink::Handle link_node = ValueNode_BoneLink::create(value_desc.get_value_type(), get_canvas());

			ValueNode_StaticList::Handle list_node = ValueNode_StaticList::Handle::cast_dynamic(link_node->get_link("bone_weight_list"));
			if (!list_node) continue;

			// BoneWeightPair is deprecated. Placed here only for compatibility
			ValueNode_BoneWeightPair::Handle bone_weight_pair_node =
				ValueNode_BoneWeightPair::create(
					BoneWeightPair((*bone_value_node)(time).get(Bone()), 1), get_canvas() );
			bone_weight_pair_node->set_link("bone", ValueNode_Const::create(ValueBase(bone_value_node), get_canvas()));
			list_node->add(bone_weight_pair_node);
			list_node->changed();

			link_node->set_link("base_value",
				ValueNode_Composite::create(
					ValueTransformation::back_transform(
						link_node->get_bone_transformation(time),
						value_desc.get_value(time) )));
		}
		else if ( influence_list.size() > 1 )
		{
			ValueNode_Average::Handle link_node = ValueNode_Average::create(...);
			// add each bone from influence_list ot Average convert
			for (std::list<ValueNode_Bone::Handle>::iterator boneiter = influence_list.begin(); boneiter != influence_list.end(); ++boneiter)
			{
				ValueNode_BoneLink::Handle bone_link_node = ValueNode_BoneLink::create(value_desc.get_value_type(), get_canvas());
				//...
			}

		}
		

		// exported ValueNode
		if (value_desc.parent_is_canvas())
		{
			Action::Handle action = ValueNodeReplace::create();
			action->set_param("canvas", get_canvas());
			action->set_param("canvas_interface", get_canvas_interface());
			action->set_param("src", ValueNode::Handle(link_node));
			action->set_param("dest", value_desc.get_value_node());

			assert(action->is_ready());
			if (!action->is_ready()) throw Error(Error::TYPE_NOTREADY);
			add_action_front(action);
		}
		else if (value_desc.parent_is_layer_param())
		{
			Action::Handle action = LayerParamConnect::create();
			action->set_param("layer", value_desc.get_layer());
			action->set_param("param", value_desc.get_param_name());
			action->set_param("canvas", get_canvas());
			action->set_param("canvas_interface", get_canvas_interface());
			action->set_param("value_node", ValueNode::Handle(link_node));

			assert(action->is_ready());
			if (!action->is_ready()) throw Error(Error::TYPE_NOTREADY);
			add_action_front(action);
		}
		else if (value_desc.parent_is_value_node())
		{
			Action::Handle action = ValueNodeLinkConnect::create();
			action->set_param("canvas", get_canvas());
			action->set_param("canvas_interface", get_canvas_interface());
			action->set_param("parent_value_node", value_desc.get_parent_value_node());
			action->set_param("index", value_desc.get_index());
			action->set_param("value_node", ValueNode::Handle(link_node));

			assert(action->is_ready());
			if (!action->is_ready()) throw Error(Error::TYPE_NOTREADY);
			add_action_front(action);
		}
	}
	 */
}
