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
	 && ValueNode_Bone::Handle::cast_dynamic(param.get_value_desc().get_parent_value_node())
	 && param.get_value_desc().get_parent_desc().parent_is_value_node()
	 && ValueNode_StaticList::Handle::cast_dynamic(param.get_value_desc().get_parent_desc().get_parent_value_node()) )
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

	// get bone
	ValueNode_Bone::Handle bone_value_node;
	if (value_desc.parent_is_value_node())
		bone_value_node = ValueNode_Bone::Handle::cast_dynamic(value_desc.get_parent_value_node());

	if (!bone_value_node)
		throw Error(Error::TYPE_NOTREADY);

	// get static list of bones (skeleton layer)
	const ValueDesc &parent = value_desc.get_parent_desc();
	ValueNode_StaticList::Handle bone_list_value_node;
	if (parent.parent_is_value_node())
		bone_list_value_node = ValueNode_StaticList::Handle::cast_dynamic(parent.get_parent_value_node());

	if (!bone_list_value_node)
		throw Error(Error::TYPE_NOTREADY);

	// bones list
	typedef std::vector<ValueNode_Bone::Handle> List;
	const ValueBase::List &value_bone_list = (*bone_list_value_node)(time).get_list();
	List list;
	list.reserve(value_bone_list.size());
	for(ValueBase::List::const_iterator i = value_bone_list.begin(); i != value_bone_list.end(); ++i)
		list.push_back(i->get(ValueNode_Bone::Handle()));
	
	// process all selected ducks
	List current_list;
	current_list.reserve(list.size());
	for(std::list<ValueDesc>::iterator iter = value_desc_list.begin(); iter != value_desc_list.end(); ++iter)
	{
		ValueDesc& value_desc(*iter);

		// check type
		if (!ValueNode_BoneLink::check_type(value_desc.get_value_type()))
			continue;
		// don't link bones to bones
		if (value_desc.parent_is_value_node() && ValueNode_Bone::Handle::cast_dynamic(value_desc.get_parent_value_node()))
			continue;
		
		// List of bones influencing current item
		for(List::iterator i = list.begin(); i != list.end(); ++i)
			if ((*i)->have_influence_on(value_desc.get_value(time)))
				current_list.push_back(*i);

		if (current_list.empty()) continue;

		ValueNode::Handle node;

		if (current_list.size() > 1)
		{
			// make average node
			Type &type(value_desc.get_value_type());
			ValueNode_Average::Handle average_node = new ValueNode_Average(type, get_canvas());

			// get type of weighted value
			types_namespace::TypeWeightedValueBase *wt = ValueAverage::get_weighted_type_for(type);
			assert(wt != NULL);

			// add each bone from influence_list to Average convert
			for(List::iterator i = current_list.begin() + 1; i != current_list.end(); ++i)
			{
				// make bone link
				ValueNode_BoneLink::Handle bone_link_node
					= ValueNode_BoneLink::create(value_desc.get_value_type(), get_canvas());

				bone_link_node->set_link("bone", ValueNode_Const::create(ValueBase(*i), get_canvas()));
				bone_link_node->set_link("base_value",
					ValueNode_Composite::create(
						ValueTransformation::back_transform(
							bone_link_node->get_bone_transformation(time),
							value_desc.get_value(time) )));

				// make weighted value
				ValueNode_Composite::Handle weighted_node
					= ValueNode_Composite(wt->create_weighted_value(1, value_desc.get_value(time)));

				weighted_node->set_link("value", bone_link_node);

				// add
				average_node->add(ValueNode::Handle(weighted_node));
			}

			node = average_node;
		}
		else
		{
			// make bone link
			ValueNode_BoneLink::Handle bone_link_node
				= ValueNode_BoneLink::create(value_desc.get_value_type(), get_canvas());

			bone_link_node->set_link("bone", ValueNode_Const::create(ValueBase(current_list.front()), get_canvas()));
			bone_link_node->set_link("base_value",
				ValueNode_Composite::create(
					ValueTransformation::back_transform(
						bone_link_node->get_bone_transformation(time),
						value_desc.get_value(time) )));

			node = bone_link_node;
		}

		if (!node) continue;

		// enqueue suitable action to assign node
		if (value_desc.parent_is_canvas())
		{
			Action::Handle action = ValueNodeReplace::create();
			action->set_param("canvas", get_canvas());
			action->set_param("canvas_interface", get_canvas_interface());
			action->set_param("src", ValueNode::Handle(node));
			action->set_param("dest", value_desc.get_value_node());

			assert(action->is_ready());
			if (!action->is_ready()) throw Error(Error::TYPE_NOTREADY);
			add_action_front(action);
		}
		else
		if (value_desc.parent_is_layer_param())
		{
			Action::Handle action = LayerParamConnect::create();
			action->set_param("layer", value_desc.get_layer());
			action->set_param("param", value_desc.get_param_name());
			action->set_param("canvas", get_canvas());
			action->set_param("canvas_interface", get_canvas_interface());
			action->set_param("value_node", ValueNode::Handle(node));

			assert(action->is_ready());
			if (!action->is_ready()) throw Error(Error::TYPE_NOTREADY);
			add_action_front(action);
		}
		else
		if (value_desc.parent_is_value_node())
		{
			Action::Handle action = ValueNodeLinkConnect::create();
			action->set_param("canvas", get_canvas());
			action->set_param("canvas_interface", get_canvas_interface());
			action->set_param("parent_value_node", value_desc.get_parent_value_node());
			action->set_param("index", value_desc.get_index());
			action->set_param("value_node", ValueNode::Handle(node));

			assert(action->is_ready());
			if (!action->is_ready()) throw Error(Error::TYPE_NOTREADY);
			add_action_front(action);
		}
	}
}
