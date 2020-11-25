/* === S Y N F I G ========================================================= */
/*!	\file valuenodereplace.cpp
**	\brief Template File
**
**	$Id$
**
**	\legal
**	Copyright (c) 2002-2005 Robert B. Quattlebaum Jr., Adrian Bentley
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

#include "valuenodereplace.h"
#include <synfigapp/canvasinterface.h>

#include <synfigapp/localization.h>

#endif

using namespace std;
using namespace etl;
using namespace synfig;
using namespace synfigapp;
using namespace Action;

/* === M A C R O S ========================================================= */

ACTION_INIT(Action::ValueNodeReplace);
ACTION_SET_NAME(Action::ValueNodeReplace,"ValueNodeReplace");
ACTION_SET_LOCAL_NAME(Action::ValueNodeReplace,N_("Replace ValueNode"));
ACTION_SET_TASK(Action::ValueNodeReplace,"replace");
ACTION_SET_CATEGORY(Action::ValueNodeReplace,Action::CATEGORY_VALUENODE|Action::CATEGORY_DRAG);
ACTION_SET_PRIORITY(Action::ValueNodeReplace,0);
ACTION_SET_VERSION(Action::ValueNodeReplace,"0.0");

/* === G L O B A L S ======================================================= */

/* === P R O C E D U R E S ================================================= */

void swap_guid(const ValueNode::Handle& a,const ValueNode::Handle& b)
{
	synfig::GUID old_a(a->get_guid());
	a->set_guid(synfig::GUID());

	synfig::GUID old_b(b->get_guid());
	b->set_guid(synfig::GUID());

	a->set_guid(old_b);
	b->set_guid(old_a);
}

/* === M E T H O D S ======================================================= */

Action::ValueNodeReplace::ValueNodeReplace():
	is_undoable(true)
{
}

Action::ParamVocab
Action::ValueNodeReplace::get_param_vocab()
{
	ParamVocab ret(Action::CanvasSpecific::get_param_vocab());

	ret.push_back(ParamDesc("dest",Param::TYPE_VALUENODE)
		.set_local_name(_("Destination ValueNode"))
		.set_desc(_("ValueNode to replaced"))
	);

	ret.push_back(ParamDesc("src",Param::TYPE_VALUENODE)
		.set_local_name(_("Source ValueNode"))
		.set_desc(_("ValueNode that will replace the destination"))
	);

	return ret;
}

bool
Action::ValueNodeReplace::is_candidate(const ParamList &x)
{
	return candidate_check(get_param_vocab(),x);
}

bool
Action::ValueNodeReplace::set_param(const synfig::String& name, const Action::Param &param)
{
	if(name=="dest" && param.get_type()==Param::TYPE_VALUENODE)
	{
		dest_value_node=param.get_value_node();

		return true;
	}

	if(name=="src" && param.get_type()==Param::TYPE_VALUENODE)
	{
		src_value_node=param.get_value_node();

		return true;
	}

	return Action::CanvasSpecific::set_param(name,param);
}

bool
Action::ValueNodeReplace::is_ready()const
{
	if(!dest_value_node || !src_value_node)
		return false;
	return Action::CanvasSpecific::is_ready();
}

void
Action::ValueNodeReplace::perform()
{
	set_dirty(true);

	if(dest_value_node == src_value_node)
		throw Error(_("Attempted to replace valuenode with itself"));

	if(dest_value_node->get_type() != src_value_node->get_type())
		throw Error(_("You cannot replace ValueNodes with different types!"));

	is_undoable=true;

	if(!src_value_node->is_exported())
	{
		src_value_node->set_id(dest_value_node->get_id());
		src_value_node->set_parent_canvas(dest_value_node->get_parent_canvas());

		ValueNode::RHandle value_node(src_value_node);

		if(!value_node.runique() && value_node.rcount()>1)
			is_undoable=false;	// !!!
	}
	else
		is_undoable=false;	// !!!

	if(!is_undoable)
		synfig::warning("ValueNodeReplace: Circumstances make undoing this action impossible at the current time. :(");

	ValueNode::RHandle value_node(dest_value_node);

	if(value_node.runique() || value_node.rcount()<=1)
		throw Error(_("Nothing to replace."));

	int replacements;

	replacements=value_node->replace(src_value_node);
	assert(replacements);
	if(!replacements)
		throw Error(_("Action Failure. This is a bug. Please report it."));
	swap_guid(dest_value_node,src_value_node);

	//src_value_node->parent_set.swap(dest_value_node->parent_set);

	// synfig::info(get_local_name()+_(": ")+strprintf(_("Replaced %d ValueNode instances"),replacements));

	// Signal that a layer has been inserted
	if(get_canvas_interface())
	{
		get_canvas_interface()->signal_value_node_replaced()(dest_value_node,src_value_node);
	}
	else synfig::warning("CanvasInterface not set on action");

}

void
Action::ValueNodeReplace::undo()
{
	if(!is_undoable)
		throw Error(_("This action cannot be undone under these circumstances."));

	set_dirty(true);

	if(dest_value_node == src_value_node)
		throw Error(_("Attempted to replace valuenode with itself"));

	if(dest_value_node->get_type() != src_value_node->get_type())
		throw Error(_("You cannot replace ValueNodes with different types!"));

	ValueNode::RHandle value_node(src_value_node);

	if(value_node.runique() || value_node.rcount()<=1)
		throw Error(_("Nothing to replace."));

	int replacements;

	replacements=value_node->replace(dest_value_node);
	assert(replacements);
	if(!replacements)
		throw Error(_("Action Failure. This is a bug. Please report it."));
	swap_guid(dest_value_node,src_value_node);

	//src_value_node->parent_set.swap(dest_value_node->parent_set);

	// synfig::info(get_local_name()+_(": (Undo) ")+strprintf(_("Replaced %d ValueNode instances"),replacements));

	src_value_node->set_id(String());
	src_value_node->set_parent_canvas(0);

	// Signal that a layer has been inserted
	if(get_canvas_interface())
	{
		get_canvas_interface()->signal_value_node_replaced()(src_value_node,dest_value_node);
	}
	else synfig::warning("CanvasInterface not set on action");

}
