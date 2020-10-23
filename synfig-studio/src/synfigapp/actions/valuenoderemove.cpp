/* === S Y N F I G ========================================================= */
/*!	\file valuenoderemove.cpp
**	\brief Template File
**
**	$Id$
**
**	\legal
**	Copyright (c) 2002-2005 Robert B. Quattlebaum Jr., Adrian Bentley
**	Copyright (c) 2008 Chris Moore
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

#include "valuenoderemove.h"
#include <synfigapp/canvasinterface.h>

#include <synfigapp/localization.h>

#endif

using namespace etl;
using namespace synfig;
using namespace synfigapp;
using namespace Action;

/* === M A C R O S ========================================================= */

ACTION_INIT_NO_GET_LOCAL_NAME(Action::ValueNodeRemove);
ACTION_SET_NAME(Action::ValueNodeRemove,"ValueNodeRemove");
ACTION_SET_LOCAL_NAME(Action::ValueNodeRemove,N_("Unexport"));
ACTION_SET_TASK(Action::ValueNodeRemove,"unexport");
ACTION_SET_CATEGORY(Action::ValueNodeRemove,Action::CATEGORY_VALUENODE);
ACTION_SET_PRIORITY(Action::ValueNodeRemove,0);
ACTION_SET_VERSION(Action::ValueNodeRemove,"0.0");

/* === G L O B A L S ======================================================= */

/* === P R O C E D U R E S ================================================= */

/* === M E T H O D S ======================================================= */

Action::ValueNodeRemove::ValueNodeRemove()
{
}

synfig::String
Action::ValueNodeRemove::get_local_name()const
{
	// TRANSLATORS: This is used in the 'history' dialog when a ValueNode is unexported.
	return strprintf(_("Unexport '%s'"), old_name.c_str());
}

Action::ParamVocab
Action::ValueNodeRemove::get_param_vocab()
{
	ParamVocab ret(Action::CanvasSpecific::get_param_vocab());

	ret.push_back(ParamDesc("value_node",Param::TYPE_VALUENODE)
		.set_local_name(_("ValueNode"))
	);

	return ret;
}

bool
Action::ValueNodeRemove::is_candidate(const ParamList &x)
{
	if(candidate_check(get_param_vocab(),x))
	{
		ValueNode::Handle value_node=x.find("value_node")->second.get_value_node();
		if(!value_node->is_exported())
			return false;
//		if(value_node->rcount()!=1)
//			return false;
		return true;
	}
	return false;
}

bool
Action::ValueNodeRemove::set_param(const synfig::String& name, const Action::Param &param)
{
	if(name=="value_node" && param.get_type()==Param::TYPE_VALUENODE)
	{
		value_node=param.get_value_node();

		if(value_node && !value_node->is_exported())
		{
			synfig::error("Action::ValueNodeRemove::set_param(): ValueBase node not exported!");
			value_node=0;
		}

		return (bool)value_node;
	}

	return Action::CanvasSpecific::set_param(name,param);
}

bool
Action::ValueNodeRemove::is_ready()const
{
	if(!value_node)
		synfig::error("Action::ValueNodeRemove::is_ready(): ValueNode not set!");

	if(!value_node)
		return false;
	return Action::CanvasSpecific::is_ready();
}

void
Action::ValueNodeRemove::perform()
{
//	if(value_node->rcount()!=1)
//		throw Error(_("ValueNode is still being used by something"));

	old_name=value_node->get_id();
	parent_canvas=value_node->get_parent_canvas();
	parent_canvas->remove_value_node(value_node, false);

	if(get_canvas_interface())
	{
		get_canvas_interface()->signal_value_node_deleted()(value_node);
	}

	//throw Error(_("Not yet implemented"));
/*
	assert(value_node->is_exported());

	if(get_canvas()->value_node_list().count(new_name))
		throw Error(_("A ValueNode with this ID already exists in this canvas"));

	old_name=value_node->get_id();

	value_node->set_id(new_name);

	if(get_canvas_interface())
	{
		get_canvas_interface()->signal_value_node_changed()(value_node);
	}
*/
}

void
Action::ValueNodeRemove::undo()
{
	parent_canvas->add_value_node(value_node,old_name);
	if(get_canvas_interface())
	{
		get_canvas_interface()->signal_value_node_added()(value_node);
	}

	//throw Error(_("Not yet implemented"));
/*
	assert(value_node->is_exported());

	if(get_canvas()->value_node_list().count(old_name))
		throw Error(_("A ValueNode with the old ID already exists in this canvas (BUG)"));

	value_node->set_id(old_name);

	if(get_canvas_interface())
	{
		get_canvas_interface()->signal_value_node_changed()(value_node);
	}
*/
}
