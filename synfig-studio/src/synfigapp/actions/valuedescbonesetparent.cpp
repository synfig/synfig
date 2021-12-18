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

#include <synfig/valuenodes/valuenode_bone.h>
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

/* === P R O C E D U R E S ================================================= */

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

	ValueNode_Bone::Handle child_bone = active_bone_;

	if(child_bone) {
		if(auto new_parent_bone_valuenode = ValueNode_Bone::Handle::cast_dynamic(value_desc.get_parent_value_node())){
			ValueNode_Bone::BoneSet possible_parents = child_bone->get_possible_parent_bones(child_bone);
			if (possible_parents.count(new_parent_bone_valuenode) <= 0) {
				get_canvas_interface()->get_ui_interface()->error(_("This bone can't be parent of the active one"));
				return;
			}

			ValueDesc new_parent_bone_desc = value_desc.get_parent_desc();
			Bone new_parent_bone = new_parent_bone_desc.get_value(time).get(Bone());

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
	}else{
		get_canvas_interface()->get_ui_interface()->error(_("Please set an active bone"));
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
