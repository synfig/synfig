/* === S I N F G =========================================================== */
/*!	\file valuenodeconstset.cpp
**	\brief Template File
**
**	$Id: valuenodeconstset.cpp,v 1.1.1.1 2005/01/07 03:34:37 darco Exp $
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

#include "valuenodeconstset.h"
#include <sinfgapp/canvasinterface.h>

#endif

using namespace std;
using namespace etl;
using namespace sinfg;
using namespace sinfgapp;
using namespace Action;

/* === M A C R O S ========================================================= */

ACTION_INIT(Action::ValueNodeConstSet);
ACTION_SET_NAME(Action::ValueNodeConstSet,"value_node_const_set");
ACTION_SET_LOCAL_NAME(Action::ValueNodeConstSet,_("Set ValueNode_Const"));
ACTION_SET_TASK(Action::ValueNodeConstSet,"set");
ACTION_SET_CATEGORY(Action::ValueNodeConstSet,Action::CATEGORY_VALUENODE);
ACTION_SET_PRIORITY(Action::ValueNodeConstSet,0);
ACTION_SET_VERSION(Action::ValueNodeConstSet,"0.0");
ACTION_SET_CVS_ID(Action::ValueNodeConstSet,"$Id: valuenodeconstset.cpp,v 1.1.1.1 2005/01/07 03:34:37 darco Exp $");

/* === G L O B A L S ======================================================= */

/* === P R O C E D U R E S ================================================= */

/* === M E T H O D S ======================================================= */

Action::ValueNodeConstSet::ValueNodeConstSet()
{
	set_dirty(true);
}

Action::ParamVocab
Action::ValueNodeConstSet::get_param_vocab()
{
	ParamVocab ret(Action::CanvasSpecific::get_param_vocab());
	
	ret.push_back(ParamDesc("value_node",Param::TYPE_VALUENODE)
		.set_local_name(_("ValueNode_Const"))
	);

	ret.push_back(ParamDesc("new_value",Param::TYPE_VALUE)
		.set_local_name(_("ValueBase"))
	);
	
	return ret;
}

bool
Action::ValueNodeConstSet::is_canidate(const ParamList &x)
{
	if(canidate_check(get_param_vocab(),x))
	{
		if(ValueNode_Const::Handle::cast_dynamic(x.find("value_node")->second.get_value_node()))
			return true;
	}
	return false;
}

bool
Action::ValueNodeConstSet::set_param(const sinfg::String& name, const Action::Param &param)
{
	if(name=="value_node" && param.get_type()==Param::TYPE_VALUENODE)
	{
		value_node=ValueNode_Const::Handle::cast_dynamic(param.get_value_node());
		
		return (bool)value_node;
	}

	if(name=="new_value" && param.get_type()==Param::TYPE_VALUE)
	{
		new_value=param.get_value();
		
		return true;
	}

	return Action::CanvasSpecific::set_param(name,param);
}

bool
Action::ValueNodeConstSet::is_ready()const
{
	if(!value_node || !new_value.is_valid())
		return false;
	return Action::CanvasSpecific::is_ready();
}

void
Action::ValueNodeConstSet::perform()
{
	//set_dirty(true);
	
	old_value=value_node->get_value();

	value_node->set_value(new_value);	
	
	// Signal that a layer has been inserted
	/*if(get_canvas_interface())
	{
		get_canvas_interface()->signal_value_node_changed()(value_node);
	}*/
}

void
Action::ValueNodeConstSet::undo()
{
	//set_dirty(true);

	value_node->set_value(old_value);	
	
	// Signal that a layer has been inserted
	/*if(get_canvas_interface())
	{
		get_canvas_interface()->signal_value_node_changed()(value_node);
	}*/
}
