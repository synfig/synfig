/* === S Y N F I G ========================================================= */
/*!	\file valuenodeadd.cpp
**	\brief Template File
**
**	$Id: valuenodeadd.cpp,v 1.1.1.1 2005/01/07 03:34:37 darco Exp $
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

#include "valuenodeadd.h"
#include <synfigapp/canvasinterface.h>

#endif

using namespace std;
using namespace etl;
using namespace synfig;
using namespace synfigapp;
using namespace Action;

/* === M A C R O S ========================================================= */

ACTION_INIT(Action::ValueNodeAdd);
ACTION_SET_NAME(Action::ValueNodeAdd,"value_node_add");
ACTION_SET_LOCAL_NAME(Action::ValueNodeAdd,"Add ValueNode");
ACTION_SET_TASK(Action::ValueNodeAdd,"add");
ACTION_SET_CATEGORY(Action::ValueNodeAdd,Action::CATEGORY_VALUENODE);
ACTION_SET_PRIORITY(Action::ValueNodeAdd,0);
ACTION_SET_VERSION(Action::ValueNodeAdd,"0.0");
ACTION_SET_CVS_ID(Action::ValueNodeAdd,"$Id: valuenodeadd.cpp,v 1.1.1.1 2005/01/07 03:34:37 darco Exp $");

/* === G L O B A L S ======================================================= */

/* === P R O C E D U R E S ================================================= */

/* === M E T H O D S ======================================================= */

Action::ValueNodeAdd::ValueNodeAdd()
{
}

Action::ParamVocab
Action::ValueNodeAdd::get_param_vocab()
{
	ParamVocab ret(Action::CanvasSpecific::get_param_vocab());
	
	ret.push_back(ParamDesc("new",Param::TYPE_VALUENODE)
		.set_local_name(_("New ValueNode"))
		.set_desc(_("ValueNode to be added"))
	);

	ret.push_back(ParamDesc("name",Param::TYPE_STRING)
		.set_local_name(_("Name"))
	);
	
	return ret;
}

bool
Action::ValueNodeAdd::is_canidate(const ParamList &x)
{
	return canidate_check(get_param_vocab(),x);
}

bool
Action::ValueNodeAdd::set_param(const synfig::String& param_name, const Action::Param &param)
{
	if(param_name=="new" && param.get_type()==Param::TYPE_VALUENODE)
	{
		value_node=param.get_value_node();
		
		return true;
	}

	if(param_name=="name" && param.get_type()==Param::TYPE_STRING)
	{
		name=param.get_string();
		
		return true;
	}

	return Action::CanvasSpecific::set_param(param_name,param);
}

bool
Action::ValueNodeAdd::is_ready()const
{
	if(!value_node || name.empty())
		return false;
	return Action::CanvasSpecific::is_ready();
}

void
Action::ValueNodeAdd::perform()
{
	if(value_node->is_exported())
	{
		throw Error(_("Parameter appears to already be exported"));
	}
	
	try
	{
		get_canvas()->add_value_node(value_node,name);
	}
	catch(Exception::IDAlreadyExists)
	{
		throw Error(_("Another exported ValueBase with this name already exists"));
	}
	catch(...)
	{
		throw Error(_("Exception caught on Add ValueNode."));
	}

	set_dirty(false);
	
	// Signal that a layer has been inserted
	if(get_canvas_interface())
	{
		get_canvas_interface()->signal_value_node_added()(value_node);
	}
	else synfig::warning("CanvasInterface not set on action");
}

void
Action::ValueNodeAdd::undo()
{
	try { get_canvas()->remove_value_node(value_node); }
	catch(...)
	{
		throw Error(_("Exception caught on Remove ValueNode."));
	}

	set_dirty(false);
	
	// Signal that a layer has been inserted
	if(get_canvas_interface())
	{
		get_canvas_interface()->signal_value_node_deleted()(value_node);
	}
	else synfig::warning("CanvasInterface not set on action");
}
