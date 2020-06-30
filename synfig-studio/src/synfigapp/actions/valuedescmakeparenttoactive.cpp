/* === S Y N F I G ========================================================= */
/*!	\file ValueDescMakeParentToActive.cpp
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

	if(name=="active_bone" && param.get_type()==Param::TYPE_VALUENODE
	&& ValueNode_Bone::Handle::cast_dynamic(param.get_value_node())){
		active_bone = param.get_value_node();
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
Action::ValueDescMakeParentToActive::prepare()
{
	clear();

	if (!value_desc.parent_is_value_node()
	 || !value_desc.is_parent_desc_declared()
	 || !value_desc.get_parent_desc().is_value_node()
	 || !active_bone
	 || !ValueNode_Bone::Handle::cast_dynamic(active_bone))
			throw Error(Error::TYPE_NOTREADY);



}
