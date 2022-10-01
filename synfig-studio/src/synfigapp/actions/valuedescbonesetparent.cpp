/* === S Y N F I G ========================================================= */
/*!	\file synfigapp/actions/valuedescbonesetparent.cpp
**	\brief Template File
**
**	\legal
**  Copyright (c) 2020 Aditya Abhiram J
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

#include "valuedescbonesetparent.h"

#include <synfig/layers/layer_skeletondeformation.h>
#include <synfig/pair.h>
#include <synfig/valuenodes/valuenode_bone.h>
#include <synfig/valuenodes/valuenode_composite.h>
#include <synfigapp/canvasinterface.h>
#include <synfigapp/localization.h>

#endif

using namespace synfig;
using namespace synfigapp;
using namespace Action;

/* === M A C R O S ========================================================= */

ACTION_INIT(Action::ValueDescBoneSetParent);
ACTION_SET_NAME(Action::ValueDescBoneSetParent,"ValueDescBoneSetParent");
ACTION_SET_LOCAL_NAME(Action::ValueDescBoneSetParent,N_("Set as Parent to Active Bone"));
ACTION_SET_TASK(Action::ValueDescBoneSetParent,"make_parent_to_active");
ACTION_SET_CATEGORY(Action::ValueDescBoneSetParent,Action::CATEGORY_VALUEDESC);
ACTION_SET_PRIORITY(Action::ValueDescBoneSetParent,0);
ACTION_SET_VERSION(Action::ValueDescBoneSetParent,"0.0");

/* === G L O B A L S ======================================================= */

static const types_namespace::TypePair<Bone,Bone>& type_bone_pair = types_namespace::TypePair<Bone,Bone>::instance;

/* === P R O C E D U R E S ================================================= */

static ValueNode_Composite::Handle
get_bone_pair_composite(const ValueDesc& value_desc)
{
	ValueNode_Composite::Handle deformation_bone_pair_composite;

	// Skeleton Deformation layer > Bone pair list > Bone pair item > Bone > bone parameter
	const ValueDesc grand_parent = value_desc.get_parent_desc().get_parent_desc();
	if (grand_parent.get_parent_desc().parent_is_layer()) {
		if (Layer_SkeletonDeformation::Handle::cast_dynamic(grand_parent.get_parent_desc().get_layer())) {
			if (grand_parent.parent_is_value_node() && grand_parent.get_parent_value_node()->get_type() == type_list) {
				if (value_desc.get_parent_desc().parent_is_value_node()
					&& value_desc.get_parent_desc().get_parent_value_node()->get_type() == type_bone_pair)
				{
					deformation_bone_pair_composite = ValueNode_Composite::Handle::cast_dynamic(grand_parent.get_value_node());
				}
			}
		}
	}
	return deformation_bone_pair_composite;
}

static bool
get_sibbling_bone_pair(ValueNode_Bone::Handle child, Time time, std::pair<ValueNode_Bone::Handle, ValueNode_Bone::Handle>& sibbing_bone_pair) {
	sibbing_bone_pair = {nullptr, nullptr};

	if (!child)
		return false;

	bool found = false;

	// Skeleton Deformation layer > Bone pair list > Bone pair item > Bone
	child->foreach_parent([child, time, &sibbing_bone_pair, &found](const Node* parent_node) -> bool {
		ValueNode::ConstHandle parent_value_node(ValueNode::ConstHandle::cast_dynamic(parent_node));
		if (!parent_value_node)
			return false;
		if (parent_value_node->get_type() != type_bone_pair)
			return false;

		const auto pair = (*parent_value_node)(time).get(std::pair<Bone, Bone>());
		const auto& rest_bone = ValueNode_Bone::find(pair.first.get_name(), parent_value_node->get_root_canvas());
		const auto& pose_bone = ValueNode_Bone::find(pair.second.get_name(), parent_value_node->get_root_canvas());
		if (child == rest_bone || child == pose_bone) {
			sibbing_bone_pair.first = rest_bone;
			sibbing_bone_pair.second = pose_bone;
			found = true;
			return true;
		}
		return false;
	});

	return found;
}
/* === M E T H O D S ======================================================= */

Action::ValueDescBoneSetParent::ValueDescBoneSetParent()
{
}

Action::ParamVocab
Action::ValueDescBoneSetParent::get_param_vocab()
{
	ParamVocab ret(Action::CanvasSpecific::get_param_vocab());

	ret.push_back(ParamDesc("value_desc",Param::TYPE_VALUEDESC)
		.set_local_name(_("ValueDesc of new parent Bone"))
	);
	ret.push_back(ParamDesc("time",Param::TYPE_TIME)
		.set_local_name(_("Time"))
		.set_optional()
	);
	ret.push_back(ParamDesc("active_bone",Param::TYPE_VALUENODE)
		.set_local_name(_("ValueNode of Bone to be reparented"))
	);

	return ret;
}

bool
Action::ValueDescBoneSetParent::is_candidate(const ParamList &x)
{
	ParamList::const_iterator i;
	
	i = x.find("value_desc");
	if (i == x.end()) return false;

	ValueDesc value_desc(i->second.get_value_desc());
	i=x.find("active_bone");
	if(i==x.end()) return false;

	ValueNode::Handle child(i->second.get_value_node());
	if (!candidate_check(get_param_vocab(),x))
		return false;

	if (value_desc.parent_is_value_node() && child) {
		if (auto valuenode_bone = ValueNode_Bone::Handle::cast_dynamic(value_desc.get_parent_value_node())) {
			if (auto child_bone = ValueNode_Bone::Handle::cast_dynamic(child)) {
				i = x.find("time");
				Time time = i == x.end() ? Time(0) : i->second.get_time();
				if ((*child_bone->get_link("parent"))(time).get(ValueNode_Bone::Handle()) == valuenode_bone)
					return false;
				ValueNode_Bone::BoneSet possible_parents = ValueNode_Bone::get_possible_parent_bones(child_bone);
				return possible_parents.count(valuenode_bone) > 0;
			}
		}
	}
	return false;
}

bool
Action::ValueDescBoneSetParent::set_param(const synfig::String& name, const Action::Param &param)
{
	if (name == "value_desc" && param.get_type() == Param::TYPE_VALUEDESC
	 && param.get_value_desc().parent_is_value_node()
	 && ValueNode_Bone::Handle::cast_dynamic(param.get_value_desc().get_parent_value_node()) )
	{
		value_desc = param.get_value_desc();
		return true;
	}

	if(name=="active_bone" && param.get_type()==Param::TYPE_VALUENODE){
		if (ValueNode_Bone::Handle bone_valuenode = ValueNode_Bone::Handle::cast_dynamic(param.get_value_node())) {
			active_bone_ = bone_valuenode;
			return true;
		}
	}

	if(name=="time" && param.get_type()==Param::TYPE_TIME)
	{
		time=param.get_time();
		return true;
	}

	return Action::CanvasSpecific::set_param(name,param);
}

bool
Action::ValueDescBoneSetParent::is_ready()const
{
	if (!value_desc
	 || !value_desc.parent_is_value_node()
	 || !value_desc.is_parent_desc_declared()
	 || !value_desc.get_parent_desc().is_value_node()
	 || !active_bone_)
		return false;
	return Action::CanvasSpecific::is_ready();
}

void
Action::ValueDescBoneSetParent::prepare()
{
	if (!first_time())
		return;

	if (!active_bone_) {
		get_canvas_interface()->get_ui_interface()->error(_("Please set an active bone"));
		return;
	}

	// stores pairs of (bone, new parent bone)
	std::vector<std::pair<ValueNode_Bone::Handle,ValueNode_Bone::Handle>> new_parenting_info;

	// checks if user mixed skeleton bone and skeleton deformation bone and prevents it
	const auto skeleton_deformation_new_parent_bone_pair = get_bone_pair_composite(value_desc);
	const bool parent_has_sibbling_bone = skeleton_deformation_new_parent_bone_pair;

	std::pair<ValueNode_Bone::Handle,ValueNode_Bone::Handle> child_bone_pair;
	const bool child_has_sibbling_bone = get_sibbling_bone_pair(active_bone_, time, child_bone_pair);

	if (parent_has_sibbling_bone != child_has_sibbling_bone) {
		get_canvas_interface()->get_ui_interface()->error(_("Don't mix Skeleton Deformation bones with regular Skeleton bones!"));
		return;
	}

	if (!parent_has_sibbling_bone) {
		// regular Skeleton
		if(auto new_parent_bone_valuenode = ValueNode_Bone::Handle::cast_dynamic(value_desc.get_parent_value_node())) {
			new_parenting_info.push_back({active_bone_, new_parent_bone_valuenode});
		}
	} else {
		// Skeleton Deformation
		ValueNode::LooseHandle parent_first_vn = skeleton_deformation_new_parent_bone_pair->get_link("first");
		if (!parent_first_vn) {
			get_canvas_interface()->get_ui_interface()->error(_("Internal error: parent for rest bone is missing"));
			return;
		}
		auto parent_rest_bone = ValueNode_Bone::find((*parent_first_vn)(time).get(Bone()).get_name(), parent_first_vn->get_root_canvas());
		ValueNode::LooseHandle parent_second_vn = skeleton_deformation_new_parent_bone_pair->get_link("second");
		if (!parent_second_vn) {
			get_canvas_interface()->get_ui_interface()->error(_("Internal error: parent for pose bone is missing"));
			return;
		}
		auto parent_pose_bone = ValueNode_Bone::find((*parent_second_vn)(time).get(Bone()).get_name(), parent_second_vn->get_root_canvas());

		new_parenting_info.push_back({child_bone_pair.first, parent_rest_bone});
		new_parenting_info.push_back({child_bone_pair.second, parent_pose_bone});
	}

	// Check parenting issues
	for (const auto& item : new_parenting_info) {
		ValueNode_Bone::BoneSet possible_parents = ValueNode_Bone::get_possible_parent_bones(item.first);
		if (possible_parents.count(item.second) <= 0) {
			std::string error_msg = synfig::strprintf(_("The bone \"%s\" can't be parent of the active one (\"%s\")"),
												   item.second->get_name().c_str(),
												   item.first->get_name().c_str());
			get_canvas_interface()->get_ui_interface()->error(error_msg);
			return;
		}
	}

	// Reparent without changing 'final' current bone setup
	for (const auto& item : new_parenting_info) {
		ValueNode_Bone::Handle child_bone = item.first;
		ValueNode_Bone::Handle new_parent_bone_valuenode = item.second;

		Bone new_parent_bone = (*new_parent_bone_valuenode)(time).get(Bone());

		ValueNode_Bone::Handle old_parent_bone = (*active_bone_->get_link("parent"))(time).get(ValueNode_Bone::Handle());

		add_action_set_valuedesc(child_bone, "parent", ValueBase(new_parent_bone_valuenode));

		Matrix new_parent_matrix = new_parent_bone.get_animated_matrix();
		Angle new_parent_angle = Angle::rad(std::atan2(new_parent_matrix.axis(0)[1],new_parent_matrix.axis(0)[0]));
		Real new_parent_scale = new_parent_bone.get_scalelx();
		new_parent_matrix = new_parent_matrix.get_inverted();

		Matrix old_parent_matrix = old_parent_bone->operator()(time).get(Bone()).get_animated_matrix();
		Angle old_parent_angle = Angle::rad(std::atan2(old_parent_matrix.axis(0)[1],old_parent_matrix.axis(0)[0]));
		Real old_parent_scale = old_parent_bone->is_root()? 1. : old_parent_bone->get_link("scalelx")->operator()(time).get(Real());

		Point origin = child_bone->get_link("origin")->operator()(time).get(Point());
		Angle angle = child_bone->get_link("angle")->operator()(time).get(Angle());

		angle += old_parent_angle;
		origin[0] *= old_parent_scale;
		origin = old_parent_matrix.get_transformed(origin);
		origin = new_parent_matrix.get_transformed(origin);
		origin[0] /= new_parent_scale;
		angle -= new_parent_angle;

		Interpolation interpolation = active_bone_->get_link("origin")->get_interpolation();
		add_action_set_interpolation(child_bone, "origin", Interpolation::INTERPOLATION_CONSTANT);
		add_action_set_valuedesc(child_bone, "origin", ValueBase(origin));
		add_action_set_interpolation(child_bone, "origin", interpolation);

		interpolation = active_bone_->get_link("angle")->get_interpolation();
		add_action_set_interpolation(child_bone, "angle", Interpolation::INTERPOLATION_CONSTANT);
		add_action_set_valuedesc(child_bone, "angle", ValueBase(angle));
		add_action_set_interpolation(child_bone, "angle", interpolation);
	}
}

void
ValueDescBoneSetParent::add_action_set_interpolation(ValueNode_Bone::Handle bone_valuenode, const char *link_name, Interpolation interpolation)
{
	Action::Handle action = Action::create("ValueDescSetInterpolation");
	if(!action)
		throw Error(Error::TYPE_BUG, _("Unable to find action ValueDescSetInterpolation (bug)"));
	action->set_param("canvas",get_canvas());
	action->set_param("canvas_interface",get_canvas_interface());
	action->set_param("new_value",interpolation);
	action->set_param("value_desc",ValueDesc(bone_valuenode,bone_valuenode->get_link_index_from_name(link_name)));
	if(!action->is_ready())
		throw Error(Error::TYPE_NOTREADY);
	add_action(action);
}

void
ValueDescBoneSetParent::add_action_set_valuedesc(ValueNode_Bone::Handle bone_valuenode, const char *link_name, const ValueBase& value)
{
	Action::Handle action = Action::create("ValueDescSet");
	if(!action)
		throw Error(_("Unable to find action ValueDescSet (bug)"));
	action->set_param("canvas",get_canvas());
	action->set_param("canvas_interface",get_canvas_interface());
	action->set_param("time",time);
	action->set_param("new_value",value);
	action->set_param("value_desc",ValueDesc(bone_valuenode,bone_valuenode->get_link_index_from_name(link_name)));
	if(!action->is_ready())
		throw Error(Error::TYPE_NOTREADY);
	add_action(action);
}
