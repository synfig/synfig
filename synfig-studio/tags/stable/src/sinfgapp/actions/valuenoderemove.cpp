/* === S I N F G =========================================================== */
/*!	\file valuenoderemove.cpp
**	\brief Template File
**
**	$Id: valuenoderemove.cpp,v 1.1.1.1 2005/01/07 03:34:37 darco Exp $
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

#include "valuenoderemove.h"
#include <sinfgapp/canvasinterface.h>

#endif

using namespace std;
using namespace etl;
using namespace sinfg;
using namespace sinfgapp;
using namespace Action;

/* === M A C R O S ========================================================= */

ACTION_INIT(Action::ValueNodeRemove);
ACTION_SET_NAME(Action::ValueNodeRemove,"value_node_remove");
ACTION_SET_LOCAL_NAME(Action::ValueNodeRemove,_("Unexport"));
ACTION_SET_TASK(Action::ValueNodeRemove,"remove");
ACTION_SET_CATEGORY(Action::ValueNodeRemove,Action::CATEGORY_VALUENODE);
ACTION_SET_PRIORITY(Action::ValueNodeRemove,0);
ACTION_SET_VERSION(Action::ValueNodeRemove,"0.0");
ACTION_SET_CVS_ID(Action::ValueNodeRemove,"$Id: valuenoderemove.cpp,v 1.1.1.1 2005/01/07 03:34:37 darco Exp $");

/* === G L O B A L S ======================================================= */

/* === P R O C E D U R E S ================================================= */

/* === M E T H O D S ======================================================= */

Action::ValueNodeRemove::ValueNodeRemove()
{
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
Action::ValueNodeRemove::is_canidate(const ParamList &x)
{
	if(canidate_check(get_param_vocab(),x))
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
Action::ValueNodeRemove::set_param(const sinfg::String& name, const Action::Param &param)
{
	if(name=="value_node" && param.get_type()==Param::TYPE_VALUENODE)
	{
		value_node=param.get_value_node();
		
		if(value_node && !value_node->is_exported())
		{
			sinfg::error("Action::ValueNodeRemove::set_param(): ValueBase node not exported!");
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
		sinfg::error("Action::ValueNodeRemove::is_ready(): ValueNode not set!");

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
	parent_canvas->remove_value_node(value_node);

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
