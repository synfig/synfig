/* === S Y N F I G ========================================================= */
/*!	\file valuenodesetactivebone.cpp
**	\brief Template File
**
**	$Id$
**
**	\legal
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

#include "valuenodesetactivebone.h"
#include <synfigapp/canvasinterface.h>
#include <synfigapp/localization.h>
#include <synfig/valuenodes/valuenode_bone.h>

#endif

using namespace etl;
using namespace synfig;
using namespace synfigapp;
using namespace Action;

/* === M A C R O S ========================================================= */

ACTION_INIT(Action::ValueNodeSetActiveBone);
ACTION_SET_NAME(Action::ValueNodeSetActiveBone,"ValueNodeSetActiveBone");
ACTION_SET_LOCAL_NAME(Action::ValueNodeSetActiveBone,N_("Set Active bone"));
ACTION_SET_TASK(Action::ValueNodeSetActiveBone,"set_active_bone");
ACTION_SET_CATEGORY(Action::ValueNodeSetActiveBone,Action::CATEGORY_VALUEDESC|Action::CATEGORY_VALUENODE|Action::CATEGORY_HIDDEN);
ACTION_SET_PRIORITY(Action::ValueNodeSetActiveBone,0);
ACTION_SET_VERSION(Action::ValueNodeSetActiveBone,"0.0");

/* === G L O B A L S ======================================================= */

/* === P R O C E D U R E S ================================================= */

/* === M E T H O D S ======================================================= */


Action::ValueNodeSetActiveBone::ValueNodeSetActiveBone(){}

void
Action::ValueNodeSetActiveBone::undo()
{
	get_canvas_interface()->signal_active_bone_changed()(prev_active_bone);
}


Action::ParamVocab
Action::ValueNodeSetActiveBone::get_param_vocab()
{
	ParamVocab ret(Action::CanvasSpecific::get_param_vocab());

	ret.push_back(ParamDesc("active_bone_desc",Param::TYPE_VALUEDESC)
		.set_local_name(_("ValueDesc on active Bone to be set"))
	);
	ret.push_back(ParamDesc("prev_active_bone",Param::TYPE_VALUEDESC)
		.set_local_name(_("ValueNode of previous active Bone"))
	);
	
	return ret;
}

bool
Action::ValueNodeSetActiveBone::is_candidate(const ParamList &)
{
	return false;
}

bool
Action::ValueNodeSetActiveBone::set_param(const synfig::String& name, const Action::Param &param)
{
	if (name == "active_bone_node" && param.get_type() == Param::TYPE_VALUENODE
	 && ValueNode_Bone::Handle::cast_dynamic(param.get_value_node()) )
	{
		active_bone_node = param.get_value_node();
		return true;
	}

	if(name == "prev_active_bone_node" && param.get_type()==Param::TYPE_VALUENODE
	&& param.get_value_node()){
		if(param.get_value_node()->get_type()==type_bone_object)
			prev_active_bone = param.get_value_node();
		return true;
	}

	return Action::CanvasSpecific::set_param(name,param);
}

bool
Action::ValueNodeSetActiveBone::is_ready()const
{
	if (active_bone_node == prev_active_bone)
		return false;
	return Action::CanvasSpecific::is_ready();
}

void
Action::ValueNodeSetActiveBone::perform()
{
	get_canvas_interface()->signal_active_bone_changed()(active_bone_node);
}
