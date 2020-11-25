/* === S Y N F I G ========================================================= */
/*!	\file valuenodestaticlistremove.cpp
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

#include "valuenodestaticlistremove.h"
#include <synfigapp/canvasinterface.h>

#include <synfigapp/localization.h>

#endif

using namespace std;
using namespace etl;
using namespace synfig;
using namespace synfigapp;
using namespace Action;

/* === M A C R O S ========================================================= */

ACTION_INIT(Action::ValueNodeStaticListRemove);
ACTION_SET_NAME(Action::ValueNodeStaticListRemove,"ValueNodeStaticListRemove");
ACTION_SET_LOCAL_NAME(Action::ValueNodeStaticListRemove,N_("Remove Item"));
ACTION_SET_TASK(Action::ValueNodeStaticListRemove,"remove");
ACTION_SET_CATEGORY(Action::ValueNodeStaticListRemove,Action::CATEGORY_VALUEDESC|Action::CATEGORY_VALUENODE|Action::CATEGORY_HIDDEN);
ACTION_SET_PRIORITY(Action::ValueNodeStaticListRemove,-19);
ACTION_SET_VERSION(Action::ValueNodeStaticListRemove,"0.0");

/* === G L O B A L S ======================================================= */

/* === P R O C E D U R E S ================================================= */

/* === M E T H O D S ======================================================= */

Action::ValueNodeStaticListRemove::ValueNodeStaticListRemove()
{
	index=0;
	set_dirty(true);
}

Action::ParamVocab
Action::ValueNodeStaticListRemove::get_param_vocab()
{
	ParamVocab ret(Action::CanvasSpecific::get_param_vocab());

	ret.push_back(ParamDesc("value_desc",Param::TYPE_VALUEDESC)
		.set_local_name(_("ValueDesc"))
	);

	return ret;
}

bool
Action::ValueNodeStaticListRemove::is_candidate(const ParamList &x)
{
	if (!candidate_check(get_param_vocab(),x))
		return false;

	ValueDesc value_desc(x.find("value_desc")->second.get_value_desc());

	return (value_desc.parent_is_value_node() &&
			// We need a static list.
			ValueNode_StaticList::Handle::cast_dynamic(value_desc.get_parent_value_node()));
}

bool
Action::ValueNodeStaticListRemove::set_param(const synfig::String& name, const Action::Param &param)
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

		return true;
	}

	return Action::CanvasSpecific::set_param(name,param);
}

bool
Action::ValueNodeStaticListRemove::is_ready()const
{
	if(!value_node)
		return false;
	return Action::CanvasSpecific::is_ready();
}

void
Action::ValueNodeStaticListRemove::perform()
{
	if(index>=value_node->link_count())
		index=value_node->link_count()-1;

	list_entry=value_node->list[index];
	value_node->erase(*(value_node->list.begin()+index));

	// Signal that a layer has been inserted
	value_node->changed();
/*_if(get_canvas_interface())
	{
		get_canvas_interface()->signal_value_node_changed()(value_node);
	}
	else synfig::warning("CanvasInterface not set on action");*/
}

void
Action::ValueNodeStaticListRemove::undo()
{
	value_node->add(list_entry,index);

	// Signal that a layer has been inserted
	value_node->changed();
/*_if(get_canvas_interface())
	{
		get_canvas_interface()->signal_value_node_changed()(value_node);
	}
	else synfig::warning("CanvasInterface not set on action");*/
}
