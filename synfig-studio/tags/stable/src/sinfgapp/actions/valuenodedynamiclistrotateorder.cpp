/* === S I N F G =========================================================== */
/*!	\file valuenodedynamiclistinsert.cpp
**	\brief Template File
**
**	$Id: valuenodedynamiclistrotateorder.cpp,v 1.1.1.1 2005/01/07 03:34:37 darco Exp $
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

#include "valuenodedynamiclistrotateorder.h"
#include <sinfgapp/canvasinterface.h>

#endif

using namespace std;
using namespace etl;
using namespace sinfg;
using namespace sinfgapp;
using namespace Action;

/* === M A C R O S ========================================================= */

ACTION_INIT(Action::ValueNodeDynamicListRotateOrder);
ACTION_SET_NAME(Action::ValueNodeDynamicListRotateOrder,"value_node_dynamic_list_rotate_order");
ACTION_SET_LOCAL_NAME(Action::ValueNodeDynamicListRotateOrder,"Rotate Order");
ACTION_SET_TASK(Action::ValueNodeDynamicListRotateOrder,"rotate");
ACTION_SET_CATEGORY(Action::ValueNodeDynamicListRotateOrder,Action::CATEGORY_VALUEDESC|Action::CATEGORY_VALUENODE);
ACTION_SET_PRIORITY(Action::ValueNodeDynamicListRotateOrder,0);
ACTION_SET_VERSION(Action::ValueNodeDynamicListRotateOrder,"0.0");
ACTION_SET_CVS_ID(Action::ValueNodeDynamicListRotateOrder,"$Id: valuenodedynamiclistrotateorder.cpp,v 1.1.1.1 2005/01/07 03:34:37 darco Exp $");

/* === G L O B A L S ======================================================= */

/* === P R O C E D U R E S ================================================= */

/* === M E T H O D S ======================================================= */

Action::ValueNodeDynamicListRotateOrder::ValueNodeDynamicListRotateOrder()
{
	index=0;
	set_dirty(true);
}

Action::ParamVocab
Action::ValueNodeDynamicListRotateOrder::get_param_vocab()
{
	ParamVocab ret(Action::CanvasSpecific::get_param_vocab());
	
	ret.push_back(ParamDesc("value_desc",Param::TYPE_VALUEDESC)
		.set_local_name(_("ValueDesc"))
	);

	return ret;
}

bool
Action::ValueNodeDynamicListRotateOrder::is_canidate(const ParamList &x)
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
Action::ValueNodeDynamicListRotateOrder::set_param(const sinfg::String& name, const Action::Param &param)
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

	return Action::CanvasSpecific::set_param(name,param);
}

bool
Action::ValueNodeDynamicListRotateOrder::is_ready()const
{
	if(!value_node)
		return false;
	return Action::CanvasSpecific::is_ready();
}

void
Action::ValueNodeDynamicListRotateOrder::prepare()
{
	clear();
	
	for(int i(0);i<(value_node->link_count()-index)%value_node->link_count();++i)
	{
		ValueDesc value_desc(value_node,value_node->link_count()-1-i);
		ValueNode::Handle child(value_desc.get_value_node());


		Action::Handle action(Action::create("value_node_dynamic_list_remove"));
		
		action->set_param("canvas",get_canvas());
		action->set_param("canvas_interface",get_canvas_interface());
		action->set_param("value_desc",ValueDesc(value_node,value_node->link_count()-1));
		
		assert(action->is_ready());
		if(!action->is_ready())
			throw Error(Error::TYPE_NOTREADY);
	
		add_action(action);
		
		
		action=Action::create("value_node_dynamic_list_insert");
		
		action->set_param("canvas",get_canvas());
		action->set_param("canvas_interface",get_canvas_interface());
		action->set_param("value_desc",ValueDesc(value_node,0));
		action->set_param("item",child);
	
		assert(action->is_ready());
		if(!action->is_ready())
			throw Error(Error::TYPE_NOTREADY);
	
		add_action(action);

		
		
	
	}
}
