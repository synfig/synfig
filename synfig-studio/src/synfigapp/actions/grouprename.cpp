/* === S Y N F I G ========================================================= */
/*!	\file grouprename.cpp
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

#include "grouprename.h"
#include <synfigapp/canvasinterface.h>

#include <synfigapp/localization.h>

#endif

using namespace std;
using namespace etl;
using namespace synfig;
using namespace synfigapp;
using namespace Action;

/* === M A C R O S ========================================================= */

ACTION_INIT(Action::GroupRename);
ACTION_SET_NAME(Action::GroupRename,"GroupRename");
ACTION_SET_LOCAL_NAME(Action::GroupRename,N_("Rename Set"));
ACTION_SET_TASK(Action::GroupRename,"rename");
ACTION_SET_CATEGORY(Action::GroupRename,Action::CATEGORY_GROUP);
ACTION_SET_PRIORITY(Action::GroupRename,0);
ACTION_SET_VERSION(Action::GroupRename,"0.0");

/* === G L O B A L S ======================================================= */

/* === P R O C E D U R E S ================================================= */

/* === M E T H O D S ======================================================= */

Action::GroupRename::GroupRename()
{
}

Action::ParamVocab
Action::GroupRename::get_param_vocab()
{
	ParamVocab ret(Action::CanvasSpecific::get_param_vocab());

	ret.push_back(ParamDesc("group",Param::TYPE_STRING)
		.set_local_name(_("Old Set"))
		.set_desc(_("Name of the Set to rename"))
	);

	ret.push_back(ParamDesc("new_group",Param::TYPE_STRING)
		.set_local_name(_("New Set"))
		.set_desc(_("New name for Set"))
	);

	return ret;
}

bool
Action::GroupRename::is_candidate(const ParamList &x)
{
	return candidate_check(get_param_vocab(),x);
}

bool
Action::GroupRename::set_param(const synfig::String& name, const Action::Param &param)
{
	if(name=="group" && param.get_type()==Param::TYPE_STRING)
	{
		old_group_name=param.get_string();

		return true;
	}

	if(name=="new_group" && param.get_type()==Param::TYPE_STRING)
	{
		new_group_name=param.get_string();

		return true;
	}

	return Action::CanvasSpecific::set_param(name,param);
}

bool
Action::GroupRename::is_ready()const
{
	if(old_group_name.empty()||new_group_name.empty())
		return false;
	return Action::CanvasSpecific::is_ready();
}

void
Action::GroupRename::perform()
{
	if(get_canvas()->get_groups().count(new_group_name)!=0)
	{
		throw Error(_("A set with the name \"%s\" already exists!"),new_group_name.c_str());
	}
	get_canvas()->rename_group(old_group_name,new_group_name);
}

void
Action::GroupRename::undo()
{
	get_canvas()->rename_group(new_group_name,old_group_name);
}
