/* === S I N F G =========================================================== */
/*!	\file valuenodereplace.cpp
**	\brief Template File
**
**	$Id: valuenodereplace.cpp,v 1.1.1.1 2005/01/07 03:34:37 darco Exp $
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

#include "valuenodereplace.h"
#include <sinfgapp/canvasinterface.h>

#endif

using namespace std;
using namespace etl;
using namespace sinfg;
using namespace sinfgapp;
using namespace Action;

/* === M A C R O S ========================================================= */

ACTION_INIT(Action::ValueNodeReplace);
ACTION_SET_NAME(Action::ValueNodeReplace,"value_node_replace");
ACTION_SET_LOCAL_NAME(Action::ValueNodeReplace,"Replace ValueNode");
ACTION_SET_TASK(Action::ValueNodeReplace,"replace");
ACTION_SET_CATEGORY(Action::ValueNodeReplace,Action::CATEGORY_VALUENODE|Action::CATEGORY_DRAG);
ACTION_SET_PRIORITY(Action::ValueNodeReplace,0);
ACTION_SET_VERSION(Action::ValueNodeReplace,"0.0");
ACTION_SET_CVS_ID(Action::ValueNodeReplace,"$Id: valuenodereplace.cpp,v 1.1.1.1 2005/01/07 03:34:37 darco Exp $");

/* === G L O B A L S ======================================================= */

/* === P R O C E D U R E S ================================================= */

void swap_guid(const ValueNode::Handle& a,const ValueNode::Handle& b)
{
	GUID old_a(a->get_guid());
	a->set_guid(GUID());

	GUID old_b(b->get_guid());
	b->set_guid(GUID());
	
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
Action::ValueNodeReplace::is_canidate(const ParamList &x)
{
	return canidate_check(get_param_vocab(),x);
}

bool
Action::ValueNodeReplace::set_param(const sinfg::String& name, const Action::Param &param)
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
		sinfg::warning("ValueNodeReplace: Circumstances make undoing this action impossible at the current time. :(");
	
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
	
	// Signal that a layer has been inserted
	if(get_canvas_interface())
	{
		get_canvas_interface()->signal_value_node_replaced()(dest_value_node,src_value_node);
	}
	else sinfg::warning("CanvasInterface not set on action");
	
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
	
	sinfg::info(get_name()+_(": (Undo) ")+strprintf("Replaced %d ValueNode instances",replacements));

	src_value_node->set_id(String());
	src_value_node->set_parent_canvas(0);
	
	// Signal that a layer has been inserted
	if(get_canvas_interface())
	{
		get_canvas_interface()->signal_value_node_replaced()(src_value_node,dest_value_node);
	}
	else sinfg::warning("CanvasInterface not set on action");
	
}
