/* === S Y N F I G ========================================================= */
/*!	\file valuenodestaticlistinsert.cpp
**	\brief Template File
**
**	$Id$
**
**	\legal
**	Copyright (c) 2002-2005 Robert B. Quattlebaum Jr., Adrian Bentley
**  Copyright (c) 2008 Chris Moore
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

#include "valuenodestaticlistinsert.h"
#include <synfigapp/canvasinterface.h>

#include <synfigapp/localization.h>

#endif

using namespace synfig;
using namespace synfigapp;
using namespace Action;

/* === M A C R O S ========================================================= */

ACTION_INIT(Action::ValueNodeStaticListInsert);
ACTION_SET_NAME(Action::ValueNodeStaticListInsert,"ValueNodeStaticListInsert");
ACTION_SET_LOCAL_NAME(Action::ValueNodeStaticListInsert,N_("Insert Item"));
ACTION_SET_TASK(Action::ValueNodeStaticListInsert,"insert");
ACTION_SET_CATEGORY(Action::ValueNodeStaticListInsert,Action::CATEGORY_VALUEDESC|Action::CATEGORY_VALUENODE|Action::CATEGORY_HIDDEN);
ACTION_SET_PRIORITY(Action::ValueNodeStaticListInsert,-20);
ACTION_SET_VERSION(Action::ValueNodeStaticListInsert,"0.0");

/* === G L O B A L S ======================================================= */

/* === P R O C E D U R E S ================================================= */

/* === M E T H O D S ======================================================= */

Action::ValueNodeStaticListInsert::ValueNodeStaticListInsert()
{
	index=0;
	time=0;
	origin=0.5f;
	set_dirty(true);
}

Action::ParamVocab
Action::ValueNodeStaticListInsert::get_param_vocab()
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
Action::ValueNodeStaticListInsert::is_candidate(const ParamList &x)
{
	if (!candidate_check(get_param_vocab(),x))
		return false;

	ValueDesc value_desc(x.find("value_desc")->second.get_value_desc());

	return (value_desc.parent_is_value_node() &&
			// We need a static list.
			ValueNode_StaticList::Handle::cast_dynamic(value_desc.get_parent_value_node()));
}

// called from two places - both set 'canvas' and 'canvasinterface', and then:
//
//		ValueNodeStaticListRotateOrder::prepare() sets "value_desc", "item"
//			action->set_param("item",child);									<- sets item=list_entry=child
//			action->set_param("value_desc",ValueDesc(value_node,0));			<- sets value_node=value_node, index=0, list_entry=item
//
//		ValueNodeStaticListInsertSmart::prepare() sets "time", "origin", "value_desc"
//			action->set_param("time",time);
//			action->set_param("origin",origin);
//			action->set_param("value_desc",ValueDesc(value_node,index));		<- sets value_node=value_node, index=index, list_entry=create_list_entry(...)
//
// all the perform() does is 	value_node->add(list_entry,index);
//
bool
Action::ValueNodeStaticListInsert::set_param(const synfig::String& name, const Action::Param &param)
{
	if(name=="value_desc" && param.get_type()==Param::TYPE_VALUEDESC)
	{
		ValueDesc value_desc(param.get_value_desc());

		if(!value_desc.parent_is_value_node())
			return false;

		value_node=ValueNode_StaticList::Handle::cast_dynamic(value_desc.get_parent_value_node());

		if(!value_node)
			return false;

		index=value_desc.get_index();

		if(item)
			list_entry=item;
		else
			list_entry=value_node->create_list_entry(index,time,origin);

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
			list_entry=item;

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
Action::ValueNodeStaticListInsert::is_ready()const
{
	if(!value_node)
		return false;
	return Action::CanvasSpecific::is_ready();
}

void
Action::ValueNodeStaticListInsert::perform()
{
	if(index>value_node->link_count())
		index=value_node->link_count();

	value_node->add(list_entry,index);

	// Signal that a layer has been inserted
	value_node->changed();
/*_if(get_canvas_interface())
	{
		get_canvas_interface()->signal_value_node_changed()(value_node);
	}
	else synfig::warning("CanvasInterface not set on action");*/
}

void
Action::ValueNodeStaticListInsert::undo()
{
	value_node->erase(*(value_node->list.begin()+index));

	// Signal that a layer has been inserted
	value_node->changed();
/*_if(get_canvas_interface())
	{
		get_canvas_interface()->signal_value_node_changed()(value_node);
	}
	else synfig::warning("CanvasInterface not set on action");*/
}
