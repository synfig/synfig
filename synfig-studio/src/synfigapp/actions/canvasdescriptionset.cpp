/* === S Y N F I G ========================================================= */
/*!	\file canvasdescriptionset.cpp
**	\brief Template File
**
**	$Id$
**
**	\legal
**	Copyright (c) 2002-2005 Robert B. Quattlebaum Jr., Adrian Bentley
**	Copyright (c) 2008 Chris Moore
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

#include "canvasdescriptionset.h"
#include <synfigapp/canvasinterface.h>

#include <synfigapp/localization.h>

#endif

using namespace std;
using namespace etl;
using namespace synfig;
using namespace synfigapp;
using namespace Action;

/* === M A C R O S ========================================================= */

ACTION_INIT_NO_GET_LOCAL_NAME(Action::CanvasDescriptionSet);
ACTION_SET_NAME(Action::CanvasDescriptionSet,"CanvasDescriptionSet");
ACTION_SET_LOCAL_NAME(Action::CanvasDescriptionSet,N_("Set Canvas Description"));
ACTION_SET_TASK(Action::CanvasDescriptionSet,"set");
ACTION_SET_CATEGORY(Action::CanvasDescriptionSet,Action::CATEGORY_CANVAS);
ACTION_SET_PRIORITY(Action::CanvasDescriptionSet,0);
ACTION_SET_VERSION(Action::CanvasDescriptionSet,"0.0");

/* === G L O B A L S ======================================================= */

/* === P R O C E D U R E S ================================================= */

/* === M E T H O D S ======================================================= */

Action::CanvasDescriptionSet::CanvasDescriptionSet()
{
}

synfig::String
Action::CanvasDescriptionSet::get_local_name()const
{
	// TRANSLATORS: This is used in the 'history' dialog when a Canvas has its description changed.
	return strprintf(_("Change canvas description from '%s' to '%s'"),
					 old_description.c_str(),
					 new_description.c_str());
}

Action::ParamVocab
Action::CanvasDescriptionSet::get_param_vocab()
{
	ParamVocab ret(Action::CanvasSpecific::get_param_vocab());

	ret.push_back(ParamDesc("description",Param::TYPE_STRING)
		.set_local_name(_("Description"))
	);

	return ret;
}

bool
Action::CanvasDescriptionSet::is_candidate(const ParamList &x)
{
	return candidate_check(get_param_vocab(),x);
}

bool
Action::CanvasDescriptionSet::set_param(const synfig::String& description, const Action::Param &param)
{
	if(description=="description" && param.get_type()==Param::TYPE_STRING)
	{
		new_description=param.get_string();

		return true;
	}

	return Action::CanvasSpecific::set_param(description,param);
}

bool
Action::CanvasDescriptionSet::is_ready()const
{
	if(new_description.empty())
	{
		synfig::error("Action::CanvasDescriptionSet::is_ready(): Description not set!");
		return false;
	}

	return Action::CanvasSpecific::is_ready();
}

void
Action::CanvasDescriptionSet::perform()
{
	old_description=get_canvas()->get_description();

	get_canvas()->set_description(new_description);

	if(get_canvas_interface())
		get_canvas_interface()->signal_id_changed()();
	else
		synfig::warning("CanvasInterface not set on action");
}

void
Action::CanvasDescriptionSet::undo()
{
	get_canvas()->set_description(old_description);

	if(get_canvas_interface())
		get_canvas_interface()->signal_id_changed()();
	else
		synfig::warning("CanvasInterface not set on action");
}
