/* === S Y N F I G ========================================================= */
/*!	\file canvasidset.cpp
**	\brief Template File
**
**	$Id$
**
**	\legal
**	Copyright (c) 2002-2005 Robert B. Quattlebaum Jr., Adrian Bentley
**	Copyright (c) 2008 Chris Moore
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

#include "canvasidset.h"
#include <synfigapp/canvasinterface.h>
#include <synfigapp/localization.h>

#endif

using namespace etl;
using namespace synfig;
using namespace synfigapp;
using namespace Action;

/* === M A C R O S ========================================================= */

ACTION_INIT_NO_GET_LOCAL_NAME(Action::CanvasIdSet);
ACTION_SET_NAME(Action::CanvasIdSet,"CanvasIdSet");
ACTION_SET_LOCAL_NAME(Action::CanvasIdSet,N_("Set Canvas Id"));
ACTION_SET_TASK(Action::CanvasIdSet,"set");
ACTION_SET_CATEGORY(Action::CanvasIdSet,Action::CATEGORY_CANVAS);
ACTION_SET_PRIORITY(Action::CanvasIdSet,0);
ACTION_SET_VERSION(Action::CanvasIdSet,"0.0");

/* === G L O B A L S ======================================================= */

/* === P R O C E D U R E S ================================================= */

/* === M E T H O D S ======================================================= */

Action::CanvasIdSet::CanvasIdSet()
{
}

synfig::String
Action::CanvasIdSet::get_local_name()const
{
	// TRANSLATORS: This is used in the 'history' dialog when a Canvas has its id changed.
	return strprintf(_("Change canvas id from '%s' to '%s'"),
					 old_id.c_str(),
					 new_id.c_str());
}

Action::ParamVocab
Action::CanvasIdSet::get_param_vocab()
{
	ParamVocab ret(Action::CanvasSpecific::get_param_vocab());

	ret.push_back(ParamDesc("id",Param::TYPE_STRING)
		.set_local_name(_("Id"))
	);

	return ret;
}

bool
Action::CanvasIdSet::is_candidate(const ParamList &x)
{
	return candidate_check(get_param_vocab(),x);
}

bool
Action::CanvasIdSet::set_param(const synfig::String& id, const Action::Param &param)
{
	if(id=="id" && param.get_type()==Param::TYPE_STRING)
	{
		new_id=param.get_string();

		return true;
	}

	return Action::CanvasSpecific::set_param(id,param);
}

bool
Action::CanvasIdSet::is_ready()const
{
	if(new_id.empty())
	{
		synfig::error("Action::CanvasIdSet::is_ready(): Id not set!");
		return false;
	}

	return Action::CanvasSpecific::is_ready();
}

void
Action::CanvasIdSet::perform()
{
	old_id=get_canvas()->get_id();

	get_canvas()->set_id(new_id);

	if(get_canvas_interface())
		get_canvas_interface()->signal_id_changed()();
	else
		synfig::warning("CanvasInterface not set on action");
}

void
Action::CanvasIdSet::undo()
{
	get_canvas()->set_id(old_id);

	if(get_canvas_interface())
		get_canvas_interface()->signal_id_changed()();
	else
		synfig::warning("CanvasInterface not set on action");
}
