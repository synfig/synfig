/* === S Y N F I G ========================================================= */
/*!	\file ValueDescMakeParentToActive.cpp
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

#include "valuedescmakeparenttoactive.h"
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

ACTION_INIT(Action::ValueDescMakeParentToActive);
ACTION_SET_NAME(Action::ValueDescMakeParentToActive,"ValueDescMakeParentToActive");
ACTION_SET_LOCAL_NAME(Action::ValueDescMakeParentToActive,N_("Make Parent To Active Bone"));
ACTION_SET_TASK(Action::ValueDescMakeParentToActive,"make_parent_to_active");
ACTION_SET_CATEGORY(Action::ValueDescMakeParentToActive,Action::CATEGORY_VALUEDESC);
ACTION_SET_PRIORITY(Action::ValueDescMakeParentToActive,0);
ACTION_SET_VERSION(Action::ValueDescMakeParentToActive,"0.0");
ACTION_SET_CVS_ID(Action::ValueDescMakeParentToActive,"$Id$");

/* === G L O B A L S ======================================================= */

/* === P R O C E D U R E S ================================================= */

/* === M E T H O D S ======================================================= */

Action::ValueDescMakeParentToActive::ValueDescMakeParentToActive():
	time(0)
{
}

Action::ParamVocab
Action::ValueDescMakeParentToActive::get_param_vocab()
{
	ParamVocab ret(Action::CanvasSpecific::get_param_vocab());

	ret.push_back(ParamDesc("value_desc",Param::TYPE_VALUEDESC)
		.set_local_name(_("ValueDesc on parent Bone"))
	);
	ret.push_back(ParamDesc("time",Param::TYPE_TIME)
		.set_local_name(_("Time"))
		.set_optional()
	);
	ret.push_back(ParamDesc("active_bone",Param::TYPE_VALUENODE)
						  .set_local_name(_("ValueNode of Active Bone"))
	);

	return ret;
}

bool
Action::ValueDescMakeParentToActive::is_candidate(const ParamList &x)
{
	ParamList::const_iterator i;
	
	i = x.find("value_desc");
	if (i == x.end()) return false;

	ValueDesc value_desc(i->second.get_value_desc());
	//ValueDesc value_desc(x.find("value_desc")->second.get_value_desc());
	i=x.find("active_bone");
	if(i==x.end()) return false;

	ValueNode::Handle active_bone(i->second.get_value_node());
	if (!candidate_check(get_param_vocab(),x))
		return false;

	return value_desc.parent_is_value_node()
		&& ValueNode_Bone::Handle::cast_dynamic(value_desc.get_parent_value_node())
		&& active_bone
		&& ValueNode_Bone::Handle::cast_dynamic(active_bone);
}

bool
Action::ValueDescMakeParentToActive::set_param(const synfig::String& name, const Action::Param &param)
{
	if (name == "value_desc" && param.get_type() == Param::TYPE_VALUEDESC
	 && param.get_value_desc().parent_is_value_node()
	 && ValueNode_Bone::Handle::cast_dynamic(param.get_value_desc().get_parent_value_node()) )
	{
		value_desc = param.get_value_desc();
		return true;
	}

	if(name=="active_bone" && param.get_type()==Param::TYPE_VALUENODE){
		active_bone = param.get_value_node();
		prev_parent = ValueNode_Bone::Handle::cast_dynamic(active_bone)->get_link("parent");
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
Action::ValueDescMakeParentToActive::is_ready()const
{
	if (!value_desc)
		return false;
	return Action::CanvasSpecific::is_ready();
}

void
Action::ValueDescMakeParentToActive::perform()
{
	if (!value_desc.parent_is_value_node()
	 || !value_desc.is_parent_desc_declared()
	 || !value_desc.get_parent_desc().is_value_node()
	 || !active_bone)
			throw Error(Error::TYPE_NOTREADY);

	info("Received : "+ active_bone->get_description());
	ValueNode_Bone::Handle bone;
	if((bone = ValueNode_Bone::Handle::cast_dynamic(active_bone))){
		if(ValueNode_Bone::Handle::cast_dynamic(value_desc.get_parent_value_node())){
			ValueDesc bone_desc = value_desc.get_parent_desc();
			Matrix T = bone_desc.get_value(time).get(Bone()).get_animated_matrix();
			Angle T_angle = Angle::rad(atan2(T.axis(0)[1],T.axis(0)[0]));
			T = T.get_inverted();

			ValueNode_Bone::Handle parent_bone = ValueNode_Const::Handle::cast_dynamic(prev_parent)->get_value().get(ValueNode_Bone::Handle());
			info(parent_bone->operator()(time).get_type().description.name);
			Matrix p_M = parent_bone->operator()(time).get(Bone()).get_animated_matrix();
			Angle p_angle = Angle::rad(atan2(p_M.axis(0)[1],p_M.axis(0)[0]));

			Point origin = bone->get_link("origin")->operator()(time).get(Point());
			Angle angle = bone->get_link("angle")->operator()(time).get(Angle());
			Real sx = bone->get_link("scalelx")->operator()(time).get(Real());

			angle+=p_angle-T_angle;
			origin = p_M.get_transformed(origin);
			origin[0] *= sx;
			origin = T.get_transformed(origin);
			origin[0]/=bone_desc.get_value(time).get(Bone()).get_scalelx();
			bone->set_link("origin",ValueNode_Const::create(origin));
			bone->set_link("angle",ValueNode_Const::create(angle));
			bone->set_link("parent",ValueNode_Const::create(ValueNode_Bone::Handle::cast_dynamic(bone_desc.get_value_node())));

		}
	}else{
		get_canvas_interface()->get_ui_interface()->error("Please set an active bone");
	}

}

void
Action::ValueDescMakeParentToActive::undo() {
	if(prev_parent){
		ValueNode_Bone::Handle bone = ValueNode_Bone::Handle::cast_dynamic(active_bone);
		ValueDesc bone_desc = value_desc.get_parent_desc();
		Matrix T = bone_desc.get_value(time).get(Bone()).get_animated_matrix();
		Angle T_angle = Angle::rad(atan2(T.axis(0)[1],T.axis(0)[0]));


		ValueNode_Bone::Handle parent_bone = ValueNode_Const::Handle::cast_dynamic(prev_parent)->get_value().get(ValueNode_Bone::Handle());
		info(parent_bone->operator()(time).get_type().description.name);
		Matrix p_M = parent_bone->operator()(time).get(Bone()).get_animated_matrix();
		Angle p_angle = Angle::rad(atan2(p_M.axis(0)[1],p_M.axis(0)[0]));
		p_M = p_M.get_inverted();

		Point origin = bone->get_link("origin")->operator()(time).get(Point());
		Angle angle = bone->get_link("angle")->operator()(time).get(Angle());
		Real sx = bone->get_link("scalelx")->operator()(time).get(Real());

		angle+=T_angle-p_angle;
		origin = p_M.get_transformed(T.get_transformed(origin));
		origin[0]/=sx;
		origin[0]*=bone_desc.get_value(time).get(Bone()).get_scalelx();

		bone->set_link("origin",ValueNode_Const::create(origin));
		bone->set_link("angle",ValueNode_Const::create(angle));
		bone->set_link("parent",prev_parent);
	}else{
		get_canvas_interface()->get_ui_interface()->error("Could'nt find parent to active bone");
	}
}