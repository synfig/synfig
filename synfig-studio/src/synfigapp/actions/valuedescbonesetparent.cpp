/* === S Y N F I G ========================================================= */
/*!	\file ValueDescBoneSetParent.cpp
**	\brief Template File
**
**	$Id$
**
**	\legal
**	......... ... 2020 Ivan Mahonin
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

#include "valuedescbonesetparent.h"
#include "valuenodestaticlistinsertsmart.h"
#include <synfigapp/canvasinterface.h>
#include <synfigapp/localization.h>
#include <synfig/valuenodes/valuenode_bone.h>

#endif

using namespace std;
using namespace etl;
using namespace synfig;
using namespace synfigapp;
using namespace Action;

/* === M A C R O S ========================================================= */

ACTION_INIT(Action::ValueDescBoneSetParent);
ACTION_SET_NAME(Action::ValueDescBoneSetParent,"ValueDescBoneSetParent");
ACTION_SET_LOCAL_NAME(Action::ValueDescBoneSetParent,N_("Make Parent To Active Bone"));
ACTION_SET_TASK(Action::ValueDescBoneSetParent,"make_parent_to_active");
ACTION_SET_CATEGORY(Action::ValueDescBoneSetParent,Action::CATEGORY_VALUEDESC);
ACTION_SET_PRIORITY(Action::ValueDescBoneSetParent,0);
ACTION_SET_VERSION(Action::ValueDescBoneSetParent,"0.0");
ACTION_SET_CVS_ID(Action::ValueDescBoneSetParent,"$Id$");

/* === G L O B A L S ======================================================= */

/* === P R O C E D U R E S ================================================= */

/* === M E T H O D S ======================================================= */

Action::ValueDescBoneSetParent::ValueDescBoneSetParent():
	time(0)
{
}

Action::ParamVocab
Action::ValueDescBoneSetParent::get_param_vocab()
{
	ParamVocab ret(Action::CanvasSpecific::get_param_vocab());

	ret.push_back(ParamDesc("value_desc",Param::TYPE_VALUEDESC)
		.set_local_name(_("ValueDesc on parent Bone"))
	);
	ret.push_back(ParamDesc("time",Param::TYPE_TIME)
		.set_local_name(_("Time"))
		.set_optional()
	);
	ret.push_back(ParamDesc("child",Param::TYPE_VALUENODE)
						  .set_local_name(_("ValueNode of Active Bone"))
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
	//ValueDesc value_desc(x.find("value_desc")->second.get_value_desc());
	i=x.find("child");
	if(i==x.end()) return false;

	ValueNode::Handle child(i->second.get_value_node());
	if (!candidate_check(get_param_vocab(),x))
		return false;

	return value_desc.parent_is_value_node()
		&& ValueNode_Bone::Handle::cast_dynamic(value_desc.get_parent_value_node())
		&& child
		&& ValueNode_Bone::Handle::cast_dynamic(child);
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

	if(name=="child" && param.get_type()==Param::TYPE_VALUENODE){
		child = param.get_value_node();
		prev_parent = ValueNode_Bone::Handle::cast_dynamic(child)->get_link("parent");
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
Action::ValueDescBoneSetParent::is_ready()const
{
	if (!value_desc)
		return false;
	return Action::CanvasSpecific::is_ready();
}

void
Action::ValueDescBoneSetParent::perform()
{
	if (!value_desc.parent_is_value_node()
	 || !value_desc.is_parent_desc_declared()
	 || !value_desc.get_parent_desc().is_value_node()
	 || !child)
			throw Error(Error::TYPE_NOTREADY);

	ValueNode_Bone::Handle child_bone;
	if((child_bone = ValueNode_Bone::Handle::cast_dynamic(child))){
		if(ValueNode_Bone::Handle::cast_dynamic(value_desc.get_parent_value_node())){
			ValueDesc new_parent_bone_desc = value_desc.get_parent_desc();
			Matrix new_parent_matrix = new_parent_bone_desc.get_value(time).get(Bone()).get_animated_matrix();
			Angle new_parent_angle = Angle::rad(atan2(new_parent_matrix.axis(0)[1],new_parent_matrix.axis(0)[0]));
			new_parent_matrix = new_parent_matrix.get_inverted();

			ValueNode_Bone::Handle old_parent_bone = ValueNode_Const::Handle::cast_dynamic(prev_parent)->get_value().get(ValueNode_Bone::Handle());
			Matrix old_parent_matrix = old_parent_bone->operator()(time).get(Bone()).get_animated_matrix();
			Angle old_parent_angle = Angle::rad(atan2(old_parent_matrix.axis(0)[1],old_parent_matrix.axis(0)[0]));
			Real old_parent_scale = old_parent_bone->get_link("scalelx")->operator()(time).get(Real());

			Point origin = child_bone->get_link("origin")->operator()(time).get(Point());
			Angle angle = child_bone->get_link("angle")->operator()(time).get(Angle());

			angle+=old_parent_angle-new_parent_angle;
			origin[0] *= old_parent_scale;
			origin = old_parent_matrix.get_transformed(origin);
			origin = new_parent_matrix.get_transformed(origin);
			origin[0]/=new_parent_bone_desc.get_value(time).get(Bone()).get_scalelx();
			if(child_bone->set_link("parent",ValueNode_Const::create(ValueNode_Bone::Handle::cast_dynamic(new_parent_bone_desc.get_value_node())))){
				child_bone->set_link("origin",ValueNode_Const::create(origin));
				child_bone->set_link("angle",ValueNode_Const::create(angle));
			}

		}
	}else{
		get_canvas_interface()->get_ui_interface()->error("Please set an active bone");
	}

}

void
Action::ValueDescBoneSetParent::undo() {
	if(prev_parent){
		ValueNode_Bone::Handle child_bone = ValueNode_Bone::Handle::cast_dynamic(child);
		ValueDesc new_parent_bone_desc = value_desc.get_parent_desc();
		Matrix new_parent_matrix = new_parent_bone_desc.get_value(time).get(Bone()).get_animated_matrix();
		Angle new_parent_angle = Angle::rad(atan2(new_parent_matrix.axis(0)[1],new_parent_matrix.axis(0)[0]));


		ValueNode_Bone::Handle old_parent_bone = ValueNode_Const::Handle::cast_dynamic(prev_parent)->get_value().get(ValueNode_Bone::Handle());
		Matrix old_parent_matrix = old_parent_bone->operator()(time).get(Bone()).get_animated_matrix();
		Angle old_parent_angle = Angle::rad(atan2(old_parent_matrix.axis(0)[1],old_parent_matrix.axis(0)[0]));
		Real old_parent_scale = old_parent_bone->get_link("scalelx")->operator()(time).get(Real());
		old_parent_matrix = old_parent_matrix.get_inverted();

		Point origin = child_bone->get_link("origin")->operator()(time).get(Point());
		Angle angle = child_bone->get_link("angle")->operator()(time).get(Angle());


		angle+=new_parent_angle-old_parent_angle;
		origin[0] *= new_parent_bone_desc.get_value(time).get(Bone()).get_scalelx();
		origin = new_parent_matrix.get_transformed(origin);
		origin = old_parent_matrix.get_transformed(origin);
		origin[0]/=old_parent_scale;
		if(child_bone->set_link("parent",ValueNode_Const::create(old_parent_bone))){
			child_bone->set_link("origin",ValueNode_Const::create(origin));
			child_bone->set_link("angle",ValueNode_Const::create(angle));
		}
	}else{
		get_canvas_interface()->get_ui_interface()->error("Could'nt find parent to active bone");
	}
}