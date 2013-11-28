/* === S Y N F I G ========================================================= */
/*!	\file valuedescbonelink.cpp
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
#include "valuedescbonelink.h"

#include <synfigapp/canvasinterface.h>
#include <synfig/valuenode_const.h>
#include <synfig/valuenode_composite.h>
#include <synfig/valuenode_bone.h>
#include <synfig/valuenode_bonelink.h>

#include <synfigapp/general.h>

#endif

using namespace std;
using namespace etl;
using namespace synfig;
using namespace synfigapp;
using namespace Action;

/* === M A C R O S ========================================================= */

ACTION_INIT(Action::ValueDescBoneLink);
ACTION_SET_NAME(Action::ValueDescBoneLink,"ValueDescBoneLink");
ACTION_SET_LOCAL_NAME(Action::ValueDescBoneLink,N_("Link to Bone"));
ACTION_SET_TASK(Action::ValueDescBoneLink,"connect");
ACTION_SET_CATEGORY(Action::ValueDescBoneLink,Action::CATEGORY_VALUEDESC);
ACTION_SET_PRIORITY(Action::ValueDescBoneLink,0);
ACTION_SET_VERSION(Action::ValueDescBoneLink,"0.0");
ACTION_SET_CVS_ID(Action::ValueDescBoneLink,"$Id$");

/* === G L O B A L S ======================================================= */

/* === P R O C E D U R E S ================================================= */

/* === M E T H O D S ======================================================= */

Action::ValueDescBoneLink::ValueDescBoneLink()
{
}

Action::ParamVocab
Action::ValueDescBoneLink::get_param_vocab()
{
	ParamVocab ret(Action::CanvasSpecific::get_param_vocab());

	ret.push_back(ParamDesc("selected_value_desc",Param::TYPE_VALUEDESC)
		.set_local_name(_("ValueDesc to link"))
		.set_supports_multiple()
	);
	ret.push_back(ParamDesc("value_desc",Param::TYPE_VALUEDESC)
		.set_local_name(_("ValueDesc on Bone to link to"))
	);

	return ret;
}

bool
Action::ValueDescBoneLink::is_candidate(const ParamList &x)
{
	ParamList::const_iterator i;

	ValueDesc value_desc(x.find("value_desc")->second.get_value_desc());

	if (!candidate_check(get_param_vocab(),x))
		return false;

	return value_desc.parent_is_value_node()
		&& ValueNode_Bone::Handle::cast_dynamic(value_desc.get_parent_value_node());
}

bool
Action::ValueDescBoneLink::set_param(const synfig::String& name, const Action::Param &param)
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

	return Action::CanvasSpecific::set_param(name,param);
}

bool
Action::ValueDescBoneLink::is_ready()const
{
	if (value_desc_list.empty())
		return false;
	if (!value_desc)
		return false;
	return Action::CanvasSpecific::is_ready();
}

void
Action::ValueDescBoneLink::prepare()
{
	if (value_desc_list.empty())
		throw Error(Error::TYPE_NOTREADY);

	clear();
	ValueNode_Bone::Handle bone_value_node;
	if (value_desc.parent_is_value_node())
		bone_value_node = ValueNode_Bone::Handle::cast_dynamic(value_desc.get_parent_value_node());

	for (std::list<ValueDesc>::iterator iter = value_desc_list.begin(); iter != value_desc_list.end(); ++iter)
	{
		ValueDesc& value_desc(*iter);

		if (value_desc.get_value_type() != ValueBase::TYPE_TRANSFORMATION)
			continue;

		// exported ValueNode
		if (value_desc.parent_is_canvas())
		{
			ValueNode_BoneLink::Handle bone_link_node = ValueNode_BoneLink::create(ValueBase::TYPE_TRANSFORMATION);
			bone_link_node->set_link("bone", ValueNode_Const::create(ValueBase(bone_value_node)));

			Action::Handle action = ValueNodeReplace::create();
			action->set_param("canvas", get_canvas());
			action->set_param("canvas_interface", get_canvas_interface());
			action->set_param("src", ValueNode::Handle(bone_link_node));
			action->set_param("dest", value_desc.get_value_node());

			assert(action->is_ready());
			if (!action->is_ready()) throw Error(Error::TYPE_NOTREADY);
			add_action_front(action);
		}
		else if (value_desc.parent_is_layer_param())
		{
			ValueNode_BoneLink::Handle bone_link_node = ValueNode_BoneLink::create(ValueBase::TYPE_TRANSFORMATION);
			bone_link_node->set_link("bone", ValueNode_Const::create(ValueBase(bone_value_node)));

			Action::Handle action = LayerParamConnect::create();
			action->set_param("layer", value_desc.get_layer());
			action->set_param("param", value_desc.get_param_name());
			action->set_param("canvas", get_canvas());
			action->set_param("canvas_interface", get_canvas_interface());
			action->set_param("value_node", ValueNode::Handle(bone_link_node));

			assert(action->is_ready());
			if (!action->is_ready()) throw Error(Error::TYPE_NOTREADY);
			add_action_front(action);
		}
	}
}
