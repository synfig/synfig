/* === S I N F G =========================================================== */
/*!	\file action_layerremove.cpp
**	\brief Template File
**
**	$Id: groupremove.cpp,v 1.1.1.1 2005/01/07 03:34:37 darco Exp $
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

#include "groupremove.h"
#include <sinfgapp/canvasinterface.h>

#endif

using namespace std;
using namespace etl;
using namespace sinfg;
using namespace sinfgapp;
using namespace Action;

/* === M A C R O S ========================================================= */

ACTION_INIT(Action::GroupRemove);
ACTION_SET_NAME(Action::GroupRemove,"group_remove");
ACTION_SET_LOCAL_NAME(Action::GroupRemove,"Remove Group");
ACTION_SET_TASK(Action::GroupRemove,"remove");
ACTION_SET_CATEGORY(Action::GroupRemove,Action::CATEGORY_GROUP);
ACTION_SET_PRIORITY(Action::GroupRemove,0);
ACTION_SET_VERSION(Action::GroupRemove,"0.0");
ACTION_SET_CVS_ID(Action::GroupRemove,"$Id: groupremove.cpp,v 1.1.1.1 2005/01/07 03:34:37 darco Exp $");

/* === G L O B A L S ======================================================= */

/* === P R O C E D U R E S ================================================= */

/* === M E T H O D S ======================================================= */

Action::GroupRemove::GroupRemove()
{
}

Action::ParamVocab
Action::GroupRemove::get_param_vocab()
{
	ParamVocab ret(Action::CanvasSpecific::get_param_vocab());
	
	ret.push_back(ParamDesc("group",Param::TYPE_STRING)
		.set_local_name(_("Group"))
		.set_desc(_("Name of the Group to remove"))
	);
	
	return ret;
}

bool
Action::GroupRemove::is_canidate(const ParamList &x)
{
	bool ret(canidate_check(get_param_vocab(),x));
	if(!ret)
	{
		sinfg::info("Action::GroupRemove::is_canidate(): failed canidate check");
		ParamList::const_iterator iter;
		for(iter=x.begin();iter!=x.end();++iter)
		{
			sinfg::info("PARAM: %s",iter->first.c_str());
		}
	}
	return ret;
}

bool
Action::GroupRemove::set_param(const sinfg::String& name, const Action::Param &param)
{
	if(name=="group" && param.get_type()==Param::TYPE_STRING)
	{
		group=param.get_string();
		
		return true;
	}

	return Action::CanvasSpecific::set_param(name,param);
}

bool
Action::GroupRemove::is_ready()const
{
	if(group.empty())
		return false;
	return Action::CanvasSpecific::is_ready();
}

void
Action::GroupRemove::perform()
{
	layer_list=get_canvas()->get_layers_in_group(group);
	
	std::set<sinfg::Layer::Handle>::iterator iter;
	for(iter=layer_list.begin();iter!=layer_list.end();++iter)
	{
		(*iter)->remove_from_group(group);
	}
}

void
Action::GroupRemove::undo()
{
	std::set<sinfg::Layer::Handle>::iterator iter;
	for(iter=layer_list.begin();iter!=layer_list.end();++iter)
	{
		(*iter)->add_to_group(group);
	}
}
