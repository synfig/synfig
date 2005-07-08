/* === S Y N F I G ========================================================= */
/*!	\file valuenodedynamiclistinsert.cpp
**	\brief Template File
**
**	$Id: valuenodedynamiclistinsert.cpp,v 1.1.1.1 2005/01/07 03:34:37 darco Exp $
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

#include "valuenodedynamiclistinsert.h"
#include <synfigapp/canvasinterface.h>

#endif

using namespace std;
using namespace etl;
using namespace synfig;
using namespace synfigapp;
using namespace Action;

/* === M A C R O S ========================================================= */

ACTION_INIT(Action::ValueNodeDynamicListInsert);
ACTION_SET_NAME(Action::ValueNodeDynamicListInsert,"value_node_dynamic_list_insert");
ACTION_SET_LOCAL_NAME(Action::ValueNodeDynamicListInsert,"Insert Item");
ACTION_SET_TASK(Action::ValueNodeDynamicListInsert,"insert");
ACTION_SET_CATEGORY(Action::ValueNodeDynamicListInsert,Action::CATEGORY_VALUEDESC|Action::CATEGORY_VALUENODE|Action::CATEGORY_HIDDEN);
ACTION_SET_PRIORITY(Action::ValueNodeDynamicListInsert,-20);
ACTION_SET_VERSION(Action::ValueNodeDynamicListInsert,"0.0");
ACTION_SET_CVS_ID(Action::ValueNodeDynamicListInsert,"$Id: valuenodedynamiclistinsert.cpp,v 1.1.1.1 2005/01/07 03:34:37 darco Exp $");

/* === G L O B A L S ======================================================= */

/* === P R O C E D U R E S ================================================= */

/* === M E T H O D S ======================================================= */

Action::ValueNodeDynamicListInsert::ValueNodeDynamicListInsert()
{
	index=0;
	time=0;
	origin=0.5f;
	set_dirty(true);
}

Action::ParamVocab
Action::ValueNodeDynamicListInsert::get_param_vocab()
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
	ret.push_back(ParamDesc("item",Param::TYPE_VALUENODE)
		.set_local_name(_("ValueNode to insert"))
		.set_optional()
	);

	return ret;
}

bool
Action::ValueNodeDynamicListInsert::is_canidate(const ParamList &x)
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
Action::ValueNodeDynamicListInsert::set_param(const synfig::String& name, const Action::Param &param)
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

		value_node_bline=ValueNode_BLine::Handle::cast_dynamic(value_desc.get_parent_value_node());

		list_entry=value_node->create_list_entry(index,time,origin);
		if(item)
			list_entry.value_node=item;
		
		assert(list_entry.value_node.rcount()==1);
				
		return true;
	}
	if(name=="time" && param.get_type()==Param::TYPE_TIME)
	{
		time=param.get_time();
		
		return true;
	}
	if(name=="item" && param.get_type()==Param::TYPE_VALUENODE)
	{
		item=param.get_value_node();
		if(item)
			list_entry.value_node=item;
		
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
Action::ValueNodeDynamicListInsert::is_ready()const
{
	if(!value_node)
		return false;
	return Action::CanvasSpecific::is_ready();
}

void
Action::ValueNodeDynamicListInsert::perform()
{	
	if(index>value_node->link_count())
		index=value_node->link_count();
	
	value_node->add(list_entry,index);
	assert(list_entry.value_node.rcount()>=2);
	
	// Signal that a layer has been inserted
	value_node->changed();
/*_if(get_canvas_interface())
	{
		get_canvas_interface()->signal_value_node_changed()(value_node);
	}
	else synfig::warning("CanvasInterface not set on action");*/
}

void
Action::ValueNodeDynamicListInsert::undo()
{
	assert(list_entry.value_node.rcount()>=2);
	value_node->erase((value_node->list.begin()+index)->value_node);
	assert(list_entry.value_node.rcount()>=1);
	
	// Signal that a layer has been inserted
	value_node->changed();
/*_if(get_canvas_interface())
	{
		get_canvas_interface()->signal_value_node_changed()(value_node);
	}
	else synfig::warning("CanvasInterface not set on action");*/
}
