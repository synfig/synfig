/* === S Y N F I G ========================================================= */
/*!	\file valuenoderename.cpp
**	\brief Template File
**
**	$Id: valuenoderename.cpp,v 1.1.1.1 2005/01/07 03:34:37 darco Exp $
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

#include "valuenoderename.h"
#include <synfigapp/canvasinterface.h>

#endif

using namespace std;
using namespace etl;
using namespace synfig;
using namespace synfigapp;
using namespace Action;

/* === M A C R O S ========================================================= */

ACTION_INIT(Action::ValueNodeRename);
ACTION_SET_NAME(Action::ValueNodeRename,"value_node_rename");
ACTION_SET_LOCAL_NAME(Action::ValueNodeRename,_("Rename ValueNode"));
ACTION_SET_TASK(Action::ValueNodeRename,"rename");
ACTION_SET_CATEGORY(Action::ValueNodeRename,Action::CATEGORY_VALUENODE);
ACTION_SET_PRIORITY(Action::ValueNodeRename,0);
ACTION_SET_VERSION(Action::ValueNodeRename,"0.0");
ACTION_SET_CVS_ID(Action::ValueNodeRename,"$Id: valuenoderename.cpp,v 1.1.1.1 2005/01/07 03:34:37 darco Exp $");

/* === G L O B A L S ======================================================= */

/* === P R O C E D U R E S ================================================= */

/* === M E T H O D S ======================================================= */

Action::ValueNodeRename::ValueNodeRename()
{
}

Action::ParamVocab
Action::ValueNodeRename::get_param_vocab()
{
	ParamVocab ret(Action::CanvasSpecific::get_param_vocab());
	
	ret.push_back(ParamDesc("value_node",Param::TYPE_VALUENODE)
		.set_local_name(_("ValueNode_Const"))
	);

	ret.push_back(ParamDesc("name",Param::TYPE_STRING)
		.set_local_name(_("Name"))
		.set_desc(_("The new name of the ValueNode"))
		.set_user_supplied()
	);
	
	return ret;
}

bool
Action::ValueNodeRename::is_canidate(const ParamList &x)
{
	if(canidate_check(get_param_vocab(),x))
	{
		if(x.find("value_node")->second.get_value_node()->is_exported())
			return true;
	}
	return false;
}

bool
Action::ValueNodeRename::set_param(const synfig::String& name, const Action::Param &param)
{
	if(name=="value_node" && param.get_type()==Param::TYPE_VALUENODE)
	{
		value_node=param.get_value_node();
		
		if(value_node && !value_node->is_exported())
		{
			synfig::error("Action::ValueNodeRename::set_param(): ValueBase node not exported!");
			value_node=0;
		}
		
		return (bool)value_node;
	}

	if(name=="name" && param.get_type()==Param::TYPE_STRING)
	{
		new_name=param.get_string();
		
		return true;
	}

	return Action::CanvasSpecific::set_param(name,param);
}

bool
Action::ValueNodeRename::is_ready()const
{
	if(!value_node)
		synfig::error("Action::ValueNodeRename::is_ready(): ValueNode not set!");

	if(new_name.empty())
		synfig::error("Action::ValueNodeRename::is_ready(): ValueNode not set!");

	if(!value_node || new_name.empty())
		return false;
	return Action::CanvasSpecific::is_ready();
}

void
Action::ValueNodeRename::perform()
{	
	assert(value_node->is_exported());

	if(get_canvas()->value_node_list().count(new_name))
		throw Error(_("A ValueNode with this ID already exists in this canvas"));
	
	old_name=value_node->get_id();

	value_node->set_id(new_name);	
	
	if(get_canvas_interface())
	{
		get_canvas_interface()->signal_value_node_changed()(value_node);
	}
}

void
Action::ValueNodeRename::undo()
{
	assert(value_node->is_exported());

	if(get_canvas()->value_node_list().count(old_name))
		throw Error(_("A ValueNode with the old ID already exists in this canvas (BUG)"));
	
	value_node->set_id(old_name);	
	
	if(get_canvas_interface())
	{
		get_canvas_interface()->signal_value_node_changed()(value_node);
	}
}
