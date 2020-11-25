/* === S Y N F I G ========================================================= */
/*!	\file valuedesccreatechildbone.cpp
**	\brief Template File
**
**	$Id$
**
**	\legal
**  Copyright (c) 2013 Ivan Mahonin
**  Copyright (c) 2020 Aditya Abhiram J
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

#include "valuedesccreatechildbone.h"
#include "valuenodestaticlistinsertsmart.h"
#include "valuenodestaticlistinsert.h"
#include <synfigapp/canvasinterface.h>
#include <synfigapp/localization.h>
#include <synfig/valuenodes/valuenode_bone.h>
#include <synfig/valuenodes/valuenode_composite.h>

#endif

using namespace std;
using namespace etl;
using namespace synfig;
using namespace synfigapp;
using namespace Action;

/* === M A C R O S ========================================================= */

ACTION_INIT(Action::ValueDescCreateChildBone);
ACTION_SET_NAME(Action::ValueDescCreateChildBone,"ValueDescCreateChildBone");
ACTION_SET_LOCAL_NAME(Action::ValueDescCreateChildBone,N_("Create Child Bone"));
ACTION_SET_TASK(Action::ValueDescCreateChildBone,"create_child_bone");
ACTION_SET_CATEGORY(Action::ValueDescCreateChildBone,Action::CATEGORY_HIDDEN);
ACTION_SET_PRIORITY(Action::ValueDescCreateChildBone,0);
ACTION_SET_VERSION(Action::ValueDescCreateChildBone,"0.0");

/* === G L O B A L S ======================================================= */

/* === P R O C E D U R E S ================================================= */

/* === M E T H O D S ======================================================= */

Action::ValueDescCreateChildBone::ValueDescCreateChildBone():
	time(0),
	origin(ValueBase(Point(1.1,0))),
	angle(Angle::rad(0)),
	scalelx(ValueBase(1.0)),
	width(0.1),
	tipwidth(0.1),
	c_parent(false),
	c_active_bone(false)
{
}

Action::ParamVocab
Action::ValueDescCreateChildBone::get_param_vocab()
{
	ParamVocab ret(Action::CanvasSpecific::get_param_vocab());

	ret.push_back(ParamDesc("value_desc",Param::TYPE_VALUEDESC)
		.set_local_name(_("ValueDesc on parent Bone"))
	);
	ret.push_back(ParamDesc("time",Param::TYPE_TIME)
		.set_local_name(_("Time"))
		.set_optional()
	);
	ret.push_back(ParamDesc("origin",Param::TYPE_VALUE)
		.set_local_name(_("Origin of the child bone"))
		.set_optional()
	);
	ret.push_back(ParamDesc("scalelx",Param::TYPE_VALUE)
		.set_local_name(_("Scale of the child bone"))
		.set_optional()
	);
	ret.push_back(ParamDesc("angle",Param::TYPE_VALUE)
		.set_local_name(_("Angle of the child bone"))
		.set_optional()
	);
	ret.push_back(ParamDesc("c_parent",Param::TYPE_BOOL)
			                    .set_local_name(_("Change the parent of the child bone?"))
			                    .set_optional()
	);
	ret.push_back(ParamDesc("width",Param::TYPE_VALUE)
			                    .set_local_name(_("Origin Width of the child bone"))
			                    .set_optional()
	);
	ret.push_back(ParamDesc("tipwidth",Param::TYPE_VALUE)
			                    .set_local_name(_("Tip Width of the child bone"))
			                    .set_optional()
	);
	ret.push_back(ParamDesc("c_active_bone",Param::TYPE_BOOL)
			                    .set_local_name(_("Highlight active bone?"))
			                    .set_optional()
	);
	ret.push_back(ParamDesc("prev_active_bone",Param::TYPE_VALUEDESC)
		.set_local_name(_("ValueNode of previous active Bone"))
		.set_optional()
	);
	return ret;
}

bool
Action::ValueDescCreateChildBone::is_candidate(const ParamList &x)
{
	ParamList::const_iterator i;
	
	i = x.find("value_desc");
	if (i == x.end()) return false;

	ValueDesc value_desc(i->second.get_value_desc());
	//ValueDesc value_desc(x.find("value_desc")->second.get_value_desc());

	if (!candidate_check(get_param_vocab(),x))
		return false;

	return value_desc.parent_is_value_node()
		&& ValueNode_Bone::Handle::cast_dynamic(value_desc.get_parent_value_node());
}

bool
Action::ValueDescCreateChildBone::set_param(const synfig::String& name, const Action::Param &param)
{
	if (name == "value_desc" && param.get_type() == Param::TYPE_VALUEDESC
	 && param.get_value_desc().parent_is_value_node()
	 && ValueNode_Bone::Handle::cast_dynamic(param.get_value_desc().get_parent_value_node()) )
	{
		value_desc = param.get_value_desc();
		return true;
	}

	if(name=="time" && param.get_type()==Param::TYPE_TIME)
	{
		time=param.get_time();
		return true;
	}

	if(param.get_type()==Param::TYPE_VALUE)
	{
		if(name=="origin"){
			origin=param.get_value();
			return true;
		}else if(name=="scalelx"){
			scalelx=param.get_value();
			return true;
		}else if(name=="angle"){
			angle=param.get_value();
			return true;
		}else if(name=="width"){
			width = param.get_value();
			return true;
		}else if(name=="tipwidth"){
			tipwidth = param.get_value();
		}
	}
	if(param.get_type()==Param::TYPE_BOOL){
		if(name=="parent"){
			c_parent=param.get_bool();
			return true;
		}else if(name=="highlight"){
			c_active_bone = param.get_bool();
			return true;
		}
	}

	if(name == "prev_active_bone_node" && param.get_type()==Param::TYPE_VALUENODE
	&& (param.get_value_node()==ValueNode::Handle() || ValueNode_Bone::Handle::cast_dynamic(param.get_value_node()))){
		prev_active_bone = param.get_value_node();
		return true;
	}


	return Action::CanvasSpecific::set_param(name,param);
}

bool
Action::ValueDescCreateChildBone::is_ready()const
{
	if (!value_desc)
		return false;
	return Action::CanvasSpecific::is_ready();
}

void
Action::ValueDescCreateChildBone::prepare()
{
	clear();

	if (!value_desc.parent_is_value_node()
	 || !value_desc.is_parent_desc_declared()
	 || !value_desc.get_parent_desc().is_value_node() )
			throw Error(Error::TYPE_NOTREADY);

	Action::Handle action;

	action = ValueNodeStaticListInsert::create();
	action->set_param("canvas", get_canvas());
	action->set_param("canvas_interface", get_canvas_interface());
	action->set_param("time", time);
	
	const ValueDesc &parent_desc = value_desc.get_parent_desc();
	if (parent_desc.get_parent_desc().get_value_type() == type_list)
	{
		ValueNode_StaticList::Handle value_node=ValueNode_StaticList::Handle::cast_dynamic(parent_desc.get_parent_value_node());
		if(!value_node){
			cout<<"Error"<<endl;
			throw Error(Error::TYPE_NOTREADY);

		}

		int index=parent_desc.get_index();

		Bone bone = Bone();
		if(c_parent){
			bone.set_parent(ValueNode_Bone_Root::create(Bone()));
		}else{
			bone.set_parent(ValueNode_Bone::Handle::cast_dynamic(ValueNode::Handle(value_node->list[index])).get());
		}
		bone.set_origin(origin.get(Point()));
		bone.set_scalelx(scalelx.get(Real()));
		bone.set_width(width.get(Real()));
		bone.set_tipwidth(tipwidth.get(Real()));
		bone.set_angle(angle.get(Angle()));

		ValueNode_Bone::Handle bone_node = ValueNode_Bone::create(bone,get_canvas());
		action->set_param("item",ValueNode::Handle::cast_dynamic(bone_node));

		action->set_param("value_desc",ValueDesc(value_node,index));

		if(c_active_bone){
			Action::Handle setActiveBone(Action::Handle(Action::create("ValueNodeSetActiveBone")));
			setActiveBone->set_param("canvas",get_canvas());
			setActiveBone->set_param("canvas_interface",get_canvas_interface());

			setActiveBone->set_param("active_bone_node",ValueNode::Handle::cast_dynamic(bone_node));
			setActiveBone->set_param("prev_active_bone_node",prev_active_bone);

			if (!setActiveBone->is_ready())
				throw Error(Error::TYPE_NOTREADY);
			add_action_front(setActiveBone);

		}
	} else {
		ValueNode_StaticList::Handle value_node=ValueNode_StaticList::Handle::cast_dynamic(parent_desc.get_parent_desc().get_parent_value_node());
		if(!value_node){
			cout<<"Error"<<endl;
			throw Error(Error::TYPE_NOTREADY);
		}
		int index=parent_desc.get_parent_desc().get_index();
		action->set_param("value_desc",ValueDesc(value_node,index));

		ValueNode_Composite::Handle bone_pair = ValueNode_Composite::Handle::cast_dynamic(value_node->create_list_entry(index));
		ValueNode_Bone::Handle bone = ValueNode_Bone::Handle::cast_dynamic(bone_pair->get_link("second"));

		bone->set_link("origin",ValueNode_Const::create(origin.get(Point())));
		bone->set_link("scalelx",ValueNode_Const::create(scalelx.get(Real())));
		bone->set_link("angle",ValueNode_Const::create(angle.get(Angle())));
		bone->set_link("width",ValueNode_Const::create(width.get(Real())));
		bone->set_link("tipwidth",ValueNode_Const::create(tipwidth.get(Real())));
		if(c_parent){
			bone->set_link("parent",ValueNode_Bone::create(bone->get_root_bone()));
		}

		bone = ValueNode_Bone::Handle::cast_dynamic(bone_pair->get_link("first"));

		bone->set_link("origin",ValueNode_Const::create(origin.get(Point())));
		bone->set_link("scalelx",ValueNode_Const::create(scalelx.get(Real())));
		bone->set_link("angle",ValueNode_Const::create(angle.get(Angle())));
		bone->set_link("width",ValueNode_Const::create(width.get(Real())));
		bone->set_link("tipwidth",ValueNode_Const::create(tipwidth.get(Real())));

		if(c_parent){
			bone->set_link("parent",ValueNode_Bone::create(bone->get_root_bone(),get_canvas()));
		}

		action->set_param("item",ValueNode::Handle::cast_dynamic(bone_pair));
		if(c_active_bone){
			Action::Handle setActiveBone(Action::Handle(Action::create("ValueNodeSetActiveBone")));
			setActiveBone->set_param("canvas",get_canvas());
			setActiveBone->set_param("canvas_interface",get_canvas_interface());

			setActiveBone->set_param("active_bone_node",ValueNode::Handle::cast_dynamic(bone));
			setActiveBone->set_param("prev_active_bone_node",prev_active_bone);

			if (!setActiveBone->is_ready())
				throw Error(Error::TYPE_NOTREADY);
			add_action_front(setActiveBone);

		}

	}
		
	
	if (!action->is_ready())
		throw Error(Error::TYPE_NOTREADY);
	add_action_front(action);
}