/* === S Y N F I G ========================================================= */
/*!	\file valuedesccreatechildbone.cpp
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

#include <synfig/general.h>

#include "valuedesccreatechildbone.h"
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

ACTION_INIT(Action::ValueDescCreateChildBone);
ACTION_SET_NAME(Action::ValueDescCreateChildBone,"ValueDescCreateChildBone");
ACTION_SET_LOCAL_NAME(Action::ValueDescCreateChildBone,N_("Create Child Bone"));
ACTION_SET_TASK(Action::ValueDescCreateChildBone,"create_child_bone");
ACTION_SET_CATEGORY(Action::ValueDescCreateChildBone,Action::CATEGORY_VALUEDESC);
ACTION_SET_PRIORITY(Action::ValueDescCreateChildBone,0);
ACTION_SET_VERSION(Action::ValueDescCreateChildBone,"0.0");
ACTION_SET_CVS_ID(Action::ValueDescCreateChildBone,"$Id$");

/* === G L O B A L S ======================================================= */

/* === P R O C E D U R E S ================================================= */

/* === M E T H O D S ======================================================= */

Action::ValueDescCreateChildBone::ValueDescCreateChildBone():
	time(0)
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

	return ret;
}

bool
Action::ValueDescCreateChildBone::is_candidate(const ParamList &x)
{
	ParamList::const_iterator i;

	ValueDesc value_desc(x.find("value_desc")->second.get_value_desc());

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

	Action::Handle action = ValueNodeStaticListInsertSmart::create();
	action->set_param("canvas", get_canvas());
	action->set_param("canvas_interface", get_canvas_interface());
	action->set_param("time", time);
	
	const ValueDesc &parent_desc = value_desc.get_parent_desc();
	if (parent_desc.get_parent_desc().get_value_type() == type_list)
	{
		// Adding bone to Skeleton layer
		action->set_param("value_desc", parent_desc);
	} else {
		// Adding bone to Skeleton Deform layer
		action->set_param("value_desc", parent_desc.get_parent_desc());
	}
		

	if (!action->is_ready())
		throw Error(Error::TYPE_NOTREADY);
	add_action_front(action);
}
