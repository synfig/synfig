/* === S Y N F I G ========================================================= */
/*!	\file valuedescresetpose.cpp
**	\brief Template File
**
**	\legal
**	......... ... 2014 Ivan Mahonin
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

#include <synfig/general.h>

#include "valuedescresetpose.h"
#include "valuedescset.h"
#include <synfigapp/canvasinterface.h>
#include <synfigapp/localization.h>
#include <synfig/valuenodes/valuenode_bone.h>
#include <synfig/valuenodes/valuenode_composite.h>

#endif

using namespace synfig;
using namespace synfigapp;
using namespace Action;

/* === M A C R O S ========================================================= */

ACTION_INIT(Action::ValueDescResetPose);
ACTION_SET_NAME(Action::ValueDescResetPose,"ValueDescResetPose");
ACTION_SET_LOCAL_NAME(Action::ValueDescResetPose,N_("Reset Pose"));
ACTION_SET_TASK(Action::ValueDescResetPose,"reset_pose");
ACTION_SET_CATEGORY(Action::ValueDescResetPose,Action::CATEGORY_VALUEDESC);
ACTION_SET_PRIORITY(Action::ValueDescResetPose,0);
ACTION_SET_VERSION(Action::ValueDescResetPose,"0.0");

/* === G L O B A L S ======================================================= */

/* === P R O C E D U R E S ================================================= */

/* === M E T H O D S ======================================================= */

Action::ValueDescResetPose::ValueDescResetPose():
	time(0)
{
}

Action::ParamVocab
Action::ValueDescResetPose::get_param_vocab()
{
	ParamVocab ret(Action::CanvasSpecific::get_param_vocab());

	ret.push_back(ParamDesc("value_desc",Param::TYPE_VALUEDESC)
		.set_local_name(_("ValueDesc on parent Bone"))
	);
	ret.push_back(ParamDesc("time",Param::TYPE_TIME)
		.set_local_name(_("Time"))
		.set_optional()
	);

	return ret;
}

bool
Action::ValueDescResetPose::is_candidate(const ParamList &x)
{
	if (!candidate_check(get_param_vocab(),x))
		return false;

	ValueDesc value_desc(x.find("value_desc")->second.get_value_desc());
	return value_desc.parent_is_value_node()
		&& ValueNode_Bone::Handle::cast_dynamic(value_desc.get_parent_value_node())
		&& value_desc.get_parent_desc().parent_is_value_node()
		&& ValueNode_Composite::Handle::cast_dynamic(value_desc.get_parent_desc().get_parent_value_node());
}

bool
Action::ValueDescResetPose::set_param(const synfig::String& name, const Action::Param &param)
{
	if (name == "value_desc" && param.get_type() == Param::TYPE_VALUEDESC
	 && param.get_value_desc().parent_is_value_node()
	 && ValueNode_Bone::Handle::cast_dynamic(param.get_value_desc().get_parent_value_node())
	 && param.get_value_desc().get_parent_desc().parent_is_value_node()
	 && ValueNode_Composite::Handle::cast_dynamic(param.get_value_desc().get_parent_desc().get_parent_value_node()) )
	{
		value_desc = param.get_value_desc();
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
Action::ValueDescResetPose::is_ready()const
{
	if (!value_desc)
		return false;
	return Action::CanvasSpecific::is_ready();
}

void
Action::ValueDescResetPose::prepare()
{
	clear();

	if (!value_desc)
		throw Error(Error::TYPE_NOTREADY);

	ValueNode_Composite::Handle composite_node =
		ValueNode_Composite::Handle::cast_dynamic(
			value_desc.get_parent_desc().get_parent_value_node() );

	ValueNode_Bone::Handle first_bone_node =
		ValueNode_Bone::Handle::cast_dynamic(
			composite_node->get_link("first") );

	if (!first_bone_node)
		throw Error(Error::TYPE_NOTREADY);

	Action::Handle action = ValueDescSet::create();
	action->set_param("canvas", get_canvas());
	action->set_param("canvas_interface", get_canvas_interface());
	action->set_param("value_desc", ValueDesc(composite_node, composite_node->get_link_index_from_name("second")));
	action->set_param("new_value", (*first_bone_node)(time));
	action->set_param("time", time);

	if (!action->is_ready())
		throw Error(Error::TYPE_NOTREADY);
	add_action_front(action);
}
