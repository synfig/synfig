/* === S I N F G =========================================================== */
/*!	\file activepointremove.cpp
**	\brief Template File
**
**	$Id: activepointremove.cpp,v 1.1.1.1 2005/01/07 03:34:37 darco Exp $
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

#include "activepointremove.h"
#include <sinfgapp/canvasinterface.h>

#endif

using namespace std;
using namespace etl;
using namespace sinfg;
using namespace sinfgapp;
using namespace Action;

/* === M A C R O S ========================================================= */

ACTION_INIT(Action::ActivepointRemove);
ACTION_SET_NAME(Action::ActivepointRemove,"activepoint_remove");
ACTION_SET_LOCAL_NAME(Action::ActivepointRemove,"Remove Activepoint");
ACTION_SET_TASK(Action::ActivepointRemove,"remove");
ACTION_SET_CATEGORY(Action::ActivepointRemove,Action::CATEGORY_ACTIVEPOINT);
ACTION_SET_PRIORITY(Action::ActivepointRemove,0);
ACTION_SET_VERSION(Action::ActivepointRemove,"0.0");
ACTION_SET_CVS_ID(Action::ActivepointRemove,"$Id: activepointremove.cpp,v 1.1.1.1 2005/01/07 03:34:37 darco Exp $");

/* === G L O B A L S ======================================================= */

/* === P R O C E D U R E S ================================================= */

/* === M E T H O D S ======================================================= */

Action::ActivepointRemove::ActivepointRemove()
{
	activepoint.set_time(Time::begin()-1);
	set_dirty(true);
}

Action::ParamVocab
Action::ActivepointRemove::get_param_vocab()
{
	ParamVocab ret(Action::CanvasSpecific::get_param_vocab());
	
	ret.push_back(ParamDesc("value_desc",Param::TYPE_VALUEDESC)
		.set_local_name(_("ValueDesc"))
	);

	ret.push_back(ParamDesc("activepoint",Param::TYPE_ACTIVEPOINT)
		.set_local_name(_("Activepoint"))
		.set_desc(_("Activepoint to be changed"))
	);

	return ret;
}

bool
Action::ActivepointRemove::is_canidate(const ParamList &x)
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
Action::ActivepointRemove::set_param(const sinfg::String& name, const Action::Param &param)
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
	if(name=="activepoint" && param.get_type()==Param::TYPE_ACTIVEPOINT)
	{
		activepoint=param.get_activepoint();
		
		return true;
	}

	return Action::CanvasSpecific::set_param(name,param);
}

bool
Action::ActivepointRemove::is_ready()const
{
	if(!value_node || activepoint.get_time()==(Time::begin()-1))
		return false;
	return Action::CanvasSpecific::is_ready();
}

void
Action::ActivepointRemove::perform()
{	
	ValueNode_DynamicList::ListEntry::ActivepointList::iterator iter;
	
	try { iter=value_node->list[index].find(activepoint); }
	catch(sinfg::Exception::NotFound)
	{
		throw Error(_("Unable to find activepoint"));
	}	

	value_node->list[index].erase(activepoint);
	value_node->changed();
	
	/*
	// Signal that a layer has been inserted
	if(get_canvas_interface())
	{
		get_canvas_interface()->signal_value_node_changed()(value_node);
	}
	else sinfg::warning("CanvasInterface not set on action");
	*/
}

void
Action::ActivepointRemove::undo()
{
	try { value_node->list[index].find(activepoint.get_time()); throw Error(_("A Activepoint already exists at this point in time"));}
	catch(sinfg::Exception::NotFound) { }	

	try { if(value_node->list[index].find(activepoint)!=value_node->list[index].timing_info.end()) throw Error(_("This activepoint is already in the ValueNode"));}
	catch(sinfg::Exception::NotFound) { }	
	
	value_node->list[index].add(activepoint);
	value_node->changed();
	/*
	// Signal that a layer has been inserted
	if(get_canvas_interface())
	{
		get_canvas_interface()->signal_value_node_changed()(value_node);
	}
	else sinfg::warning("CanvasInterface not set on action");
	*/
}
