/* === S I N F G =========================================================== */
/*!	\file blinepointtangentsplit.cpp
**	\brief Template File
**
**	$Id: blinepointtangentsplit.cpp,v 1.1.1.1 2005/01/07 03:34:37 darco Exp $
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

#include "blinepointtangentsplit.h"
#include "valuedescset.h"

#include "activepointset.h"
#include "activepointadd.h"

#include "valuedescconnect.h"
#include <sinfgapp/canvasinterface.h>

#endif

using namespace std;
using namespace etl;
using namespace sinfg;
using namespace sinfgapp;
using namespace Action;

/* === M A C R O S ========================================================= */

ACTION_INIT(Action::BLinePointTangentSplit);
ACTION_SET_NAME(Action::BLinePointTangentSplit,"bline_point_tangent_split");
ACTION_SET_LOCAL_NAME(Action::BLinePointTangentSplit,_("Split Tangents"));
ACTION_SET_TASK(Action::BLinePointTangentSplit,"split");
ACTION_SET_CATEGORY(Action::BLinePointTangentSplit,Action::CATEGORY_VALUENODE);
ACTION_SET_PRIORITY(Action::BLinePointTangentSplit,0);
ACTION_SET_VERSION(Action::BLinePointTangentSplit,"0.0");
ACTION_SET_CVS_ID(Action::BLinePointTangentSplit,"$Id: blinepointtangentsplit.cpp,v 1.1.1.1 2005/01/07 03:34:37 darco Exp $");

/* === G L O B A L S ======================================================= */

/* === P R O C E D U R E S ================================================= */

/* === M E T H O D S ======================================================= */

Action::BLinePointTangentSplit::BLinePointTangentSplit()
{
	time=(Time::begin()-1);
	set_dirty(true);
}

Action::ParamVocab
Action::BLinePointTangentSplit::get_param_vocab()
{
	ParamVocab ret(Action::CanvasSpecific::get_param_vocab());
	
	ret.push_back(ParamDesc("value_node",Param::TYPE_VALUENODE)
		.set_local_name(_("ValueNode of BLinePoint"))
	);

	ret.push_back(ParamDesc("time",Param::TYPE_TIME)
		.set_local_name(_("Time"))
	);
	
	return ret;
}

bool
Action::BLinePointTangentSplit::is_canidate(const ParamList &x)
{
	if(canidate_check(get_param_vocab(),x))
	{
		ValueNode_Composite::Handle value_node;
		value_node=ValueNode_Composite::Handle::cast_dynamic(x.find("value_node")->second.get_value_node());
		if(!value_node || value_node->get_type()!=ValueBase::TYPE_BLINEPOINT)
			return false;
		sinfg::Time time(x.find("time")->second.get_time());
		if((*value_node->get_link("split"))(time).get(bool())==true)
			return false;
		return true;
	}
	return false;
}

bool
Action::BLinePointTangentSplit::set_param(const sinfg::String& name, const Action::Param &param)
{
	if(name=="value_node" && param.get_type()==Param::TYPE_VALUENODE)
	{
		value_node=value_node.cast_dynamic(param.get_value_node());
		
		return (bool)(value_node);
	}
	if(name=="time" && param.get_type()==Param::TYPE_TIME)
	{
		time=param.get_time();
		
		return true;
	}

	return Action::CanvasSpecific::set_param(name,param);
}

bool
Action::BLinePointTangentSplit::is_ready()const
{
	if(!value_node)
		sinfg::error("Missing or bad value_node");

	if(time==(Time::begin()-1))
		sinfg::error("Missing time");
	
	if(!value_node || time==(Time::begin()-1))
		return false;
	return Action::CanvasSpecific::is_ready();
}

void
Action::BLinePointTangentSplit::prepare()
{
	clear();

	Action::Handle action;
	
	{
		action=Action::create("value_desc_set");
		if(!action)
			throw Error(_("Couldn't find action \"value_desc_set\""));
		
		action->set_param("canvas",get_canvas());
		action->set_param("canvas_interface",get_canvas_interface());
		action->set_param("value_desc",ValueDesc(value_node,3));
		action->set_param("time",time);
		action->set_param("new_value",sinfg::ValueBase(true));
	
		assert(action->is_ready());
		if(!action->is_ready())
			throw Error(Error::TYPE_NOTREADY);
	
		add_action(action);
	}
	{
		action=Action::create("value_desc_set");
		if(!action)
			throw Error(_("Couldn't find action \"value_desc_set\""));
		
		action->set_param("canvas",get_canvas());
		action->set_param("canvas_interface",get_canvas_interface());
		action->set_param("value_desc",ValueDesc(value_node,5));
		action->set_param("time",time);
		action->set_param("new_value",(*value_node->get_link("t1"))(time));
	
		assert(action->is_ready());
		if(!action->is_ready())
			throw Error(Error::TYPE_NOTREADY);
	
		add_action(action);
	}
	
}
