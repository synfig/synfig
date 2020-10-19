/* === S Y N F I G ========================================================= */
/*!	\file groupremove.cpp
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

#include "groupremove.h"
#include <synfigapp/canvasinterface.h>

#include <synfigapp/localization.h>

#endif

using namespace std;
using namespace etl;
using namespace synfig;
using namespace synfigapp;
using namespace Action;

/* === M A C R O S ========================================================= */

ACTION_INIT(Action::GroupRemove);
ACTION_SET_NAME(Action::GroupRemove,"GroupRemove");
ACTION_SET_LOCAL_NAME(Action::GroupRemove,N_("Remove Set"));
ACTION_SET_TASK(Action::GroupRemove,"remove");
ACTION_SET_CATEGORY(Action::GroupRemove,Action::CATEGORY_GROUP);
ACTION_SET_PRIORITY(Action::GroupRemove,0);
ACTION_SET_VERSION(Action::GroupRemove,"0.0");

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
		.set_local_name(_("Set"))
		.set_desc(_("Name of the Set to remove"))
	);

	return ret;
}

bool
Action::GroupRemove::is_candidate(const ParamList &x)
{
	return candidate_check(get_param_vocab(),x);
}

bool
Action::GroupRemove::set_param(const synfig::String& name, const Action::Param &param)
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

	std::set<synfig::Layer::Handle>::iterator iter;
	for(iter=layer_list.begin();iter!=layer_list.end();++iter)
	{
		(*iter)->remove_from_group(group);
	}
}

void
Action::GroupRemove::undo()
{
	std::set<synfig::Layer::Handle>::iterator iter;
	for(iter=layer_list.begin();iter!=layer_list.end();++iter)
	{
		(*iter)->add_to_group(group);
	}
}
