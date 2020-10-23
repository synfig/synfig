/* === S Y N F I G ========================================================= */
/*!	\file valuenodelinkdisconnect.cpp
**	\brief Template File
**
**	$Id$
**
**	\legal
**	Copyright (c) 2002-2005 Robert B. Quattlebaum Jr., Adrian Bentley
**
**	This file is part of Synfig.
**
**	Synfig is free software: you can redistribute it and/or modify
**	it under the terms of the GNU General Public License as published by
**	the Free Software Foundation, either version 2 of the License, or
**	(at your option) any later version.
**
**	Synfig is distributed in the hope that it will be useful,
**	but WITHOUT ANY WARRANTY; without even the implied warranty of
**	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
**	GNU General Public License for more details.
**
**	You should have received a copy of the GNU General Public License
**	along with Synfig.  If not, see <https://www.gnu.org/licenses/>.
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

#include "valuenodelinkdisconnect.h"
#include <synfigapp/canvasinterface.h>
#include <synfig/valuenodes/valuenode_const.h>

#include <synfigapp/localization.h>

#endif

using namespace etl;
using namespace synfig;
using namespace synfigapp;
using namespace Action;

/* === M A C R O S ========================================================= */

ACTION_INIT(Action::ValueNodeLinkDisconnect);
ACTION_SET_NAME(Action::ValueNodeLinkDisconnect,"ValueNodeLinkDisconnect");
ACTION_SET_LOCAL_NAME(Action::ValueNodeLinkDisconnect,N_("Disconnect ValueNode Link"));
ACTION_SET_TASK(Action::ValueNodeLinkDisconnect,"disconnect");
ACTION_SET_CATEGORY(Action::ValueNodeLinkDisconnect,Action::CATEGORY_VALUENODE);
ACTION_SET_PRIORITY(Action::ValueNodeLinkDisconnect,0);
ACTION_SET_VERSION(Action::ValueNodeLinkDisconnect,"0.0");

/* === G L O B A L S ======================================================= */

/* === P R O C E D U R E S ================================================= */

/* === M E T H O D S ======================================================= */

Action::ValueNodeLinkDisconnect::ValueNodeLinkDisconnect():
	index(-1),	// Initially set it to negative one so that we know when it has changed
	time(0)
{
}

Action::ParamVocab
Action::ValueNodeLinkDisconnect::get_param_vocab()
{
	ParamVocab ret(Action::CanvasSpecific::get_param_vocab());

	ret.push_back(ParamDesc("parent_value_node",Param::TYPE_VALUENODE)
		.set_local_name(_("Parent ValueNode"))
	);

	ret.push_back(ParamDesc("index",Param::TYPE_INTEGER)
		.set_local_name(_("Index"))
	);

	ret.push_back(ParamDesc("time",Param::TYPE_TIME)
		.set_local_name(_("Time"))
		.set_optional()
	);

	return ret;
}

bool
Action::ValueNodeLinkDisconnect::is_candidate(const ParamList &x)
{
	return candidate_check(get_param_vocab(),x);
}

bool
Action::ValueNodeLinkDisconnect::set_param(const synfig::String& name, const Action::Param &param)
{
	if(name=="parent_value_node" && param.get_type()==Param::TYPE_VALUENODE)
	{
		parent_value_node=LinkableValueNode::Handle::cast_dynamic(param.get_value_node());

		return static_cast<bool>(parent_value_node);
	}

	if(name=="index" && param.get_type()==Param::TYPE_INTEGER)
	{
		index=param.get_integer();

		return true;
	}

	if(name=="time" && param.get_type()==Param::TYPE_TIME)
	{
		time=param.get_time();

		return true;
	}

	return Action::CanvasSpecific::set_param(name,param);
}

bool
Action::ValueNodeLinkDisconnect::is_ready()const
{
	if(!parent_value_node || index==-1)
		return false;
	return Action::CanvasSpecific::is_ready();
}

void
Action::ValueNodeLinkDisconnect::perform()
{
	if(parent_value_node->link_count()<=index)
		throw Error(_("Bad index, too big. LinkCount=%d, Index=%d"),parent_value_node->link_count(),index);

	old_value_node=parent_value_node->get_link(index);

	if(!parent_value_node->set_link(index,ValueNode_Const::create((*old_value_node)(time))))
		throw Error(_("Parent would not accept link"));

	/*
	if(get_canvas()->get_time()!=time)
		set_dirty(true);
	else
		set_dirty(false);

	if(get_canvas_interface())
	{
		get_canvas_interface()->signal_value_node_changed()(parent_value_node);
	}
	*/
}

void
Action::ValueNodeLinkDisconnect::undo()
{
	if(parent_value_node->link_count()<=index)
		throw Error(_("Bad index, too big. LinkCount=%d, Index=%d"),parent_value_node->link_count(),index);

	if(!parent_value_node->set_link(index,old_value_node))
		throw Error(_("Parent would not accept old link"));

	/*if(get_canvas()->get_time()!=time)
		set_dirty(true);
	else
		set_dirty(false);

	if(get_canvas_interface())
	{
		get_canvas_interface()->signal_value_node_changed()(parent_value_node);
	}*/
}
