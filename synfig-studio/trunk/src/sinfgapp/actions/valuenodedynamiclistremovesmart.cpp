/* === S I N F G =========================================================== */
/*!	\file valuenodedynamiclistremovesmart.cpp
**	\brief Template File
**
**	$Id: valuenodedynamiclistremovesmart.cpp,v 1.1.1.1 2005/01/07 03:34:37 darco Exp $
**
**	\legal
**	Copyright (c) 2002 Robert B. Quattlebaum Jr.
**
**	This software and associated documentation
**	are CONFIDENTIAL and PROPRIETARY property of
**	the above-mentioned copyright holder.
**
**	You may not copy, print, publish, or in any
**	other way distribute this software without
**	a prior written agreement with
**	the copyright holder.
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

#include "valuenodedynamiclistremovesmart.h"
#include "valuenodedynamiclistremove.h"
#include <sinfgapp/canvasinterface.h>

#endif

using namespace std;
using namespace etl;
using namespace sinfg;
using namespace sinfgapp;
using namespace Action;

/* === M A C R O S ========================================================= */

ACTION_INIT(Action::ValueNodeDynamicListRemoveSmart);
ACTION_SET_NAME(Action::ValueNodeDynamicListRemoveSmart,"value_node_dynamic_list_remove_smart");
ACTION_SET_LOCAL_NAME(Action::ValueNodeDynamicListRemoveSmart,"Remove Item (Smart)");
ACTION_SET_TASK(Action::ValueNodeDynamicListRemoveSmart,"remove");
ACTION_SET_CATEGORY(Action::ValueNodeDynamicListRemoveSmart,Action::CATEGORY_VALUEDESC|Action::CATEGORY_VALUENODE);
ACTION_SET_PRIORITY(Action::ValueNodeDynamicListRemoveSmart,-19);
ACTION_SET_VERSION(Action::ValueNodeDynamicListRemoveSmart,"0.0");
ACTION_SET_CVS_ID(Action::ValueNodeDynamicListRemoveSmart,"$Id: valuenodedynamiclistremovesmart.cpp,v 1.1.1.1 2005/01/07 03:34:37 darco Exp $");

/* === G L O B A L S ======================================================= */

/* === P R O C E D U R E S ================================================= */

/* === M E T H O D S ======================================================= */

Action::ValueNodeDynamicListRemoveSmart::ValueNodeDynamicListRemoveSmart()
{
	index=0;
	time=0;
	origin=0.5f;
	set_dirty(true);
}

Action::ParamVocab
Action::ValueNodeDynamicListRemoveSmart::get_param_vocab()
{
	ParamVocab ret(Action::CanvasSpecific::get_param_vocab());
	
	ret.push_back(ParamDesc("value_desc",Param::TYPE_VALUEDESC)
		.set_local_name(_("ValueDesc"))
	);
	ret.push_back(ParamDesc("time",Param::TYPE_TIME)
		.set_local_name(_("Time"))
		.set_optional()
	);
	ret.push_back(ParamDesc("origin",Param::TYPE_REAL)
		.set_local_name(_("Origin"))
		.set_optional()
	);

	return ret;
}

bool
Action::ValueNodeDynamicListRemoveSmart::is_canidate(const ParamList &x)
{
	if(canidate_check(get_param_vocab(),x))
	{
		ValueDesc value_desc(x.find("value_desc")->second.get_value_desc());
		if(!value_desc.parent_is_value_node() || !ValueNode_DynamicList::Handle::cast_dynamic(value_desc.get_parent_value_node()))
			return false;

		return true;
	}
	return false;
}

bool
Action::ValueNodeDynamicListRemoveSmart::set_param(const sinfg::String& name, const Action::Param &param)
{
	if(name=="value_desc" && param.get_type()==Param::TYPE_VALUEDESC)
	{
		ValueDesc value_desc(param.get_value_desc());
		
		if(!value_desc.parent_is_value_node())
			return false;
		
		value_node=ValueNode_DynamicList::Handle::cast_dynamic(value_desc.get_parent_value_node());
		
		if(!value_node)
			return false;

		index=value_desc.get_index();
				
		return true;
	}
	if(name=="time" && param.get_type()==Param::TYPE_TIME)
	{
		time=param.get_time();
		
		return true;
	}
	if(name=="origin" && param.get_type()==Param::TYPE_REAL)
	{
		origin=param.get_real();
		
		return true;
	}

	return Action::CanvasSpecific::set_param(name,param);
}

bool
Action::ValueNodeDynamicListRemoveSmart::is_ready()const
{
	if(!value_node)
		return false;
	return Action::CanvasSpecific::is_ready();
}

void
Action::ValueNodeDynamicListRemoveSmart::prepare()
{	
	clear();
	
	// If we are in animate editing mode
	if(get_edit_mode()&MODE_ANIMATE)
	{
		Action::Handle action(Action::create("activepoint_set_off"));

		if(!action)
			throw Error(_("Unable to find action (bug)"));
		
		action->set_param("canvas",get_canvas());
		action->set_param("canvas_interface",get_canvas_interface());
		action->set_param("time",time);
		action->set_param("origin",origin);
		action->set_param("value_desc",ValueDesc(value_node,index));
		
		if(!action->is_ready())
			throw Error(Error::TYPE_NOTREADY);

		add_action(action);
	}
	else
	{
		Action::Handle action(Action::create("value_node_dynamic_list_remove"));

		if(!action)
			throw Error(_("Unable to find action (bug)"));
		
		action->set_param("canvas",get_canvas());
		action->set_param("canvas_interface",get_canvas_interface());
		action->set_param("time",time);
		action->set_param("origin",origin);
		action->set_param("value_desc",ValueDesc(value_node,index));
		
		if(!action->is_ready())
			throw Error(Error::TYPE_NOTREADY);

		add_action(action);
	}
}
