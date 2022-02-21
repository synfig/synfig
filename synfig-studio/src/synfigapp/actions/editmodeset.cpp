/* === S Y N F I G ========================================================= */
/*!	\file editmodeset.cpp
**	\brief Template File
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

#include "editmodeset.h"
#include <synfigapp/canvasinterface.h>

#include <synfigapp/localization.h>

#endif

using namespace synfig;
using namespace synfigapp;
using namespace Action;

/* === M A C R O S ========================================================= */

ACTION_INIT(Action::EditModeSet);
ACTION_SET_NAME(Action::EditModeSet,"EditModeSet");
ACTION_SET_LOCAL_NAME(Action::EditModeSet,N_("Set Edit Mode"));
ACTION_SET_TASK(Action::EditModeSet,"set");
ACTION_SET_CATEGORY(Action::EditModeSet,Action::CATEGORY_OTHER);
ACTION_SET_PRIORITY(Action::EditModeSet,0);
ACTION_SET_VERSION(Action::EditModeSet,"0.0");

/* === G L O B A L S ======================================================= */

/* === P R O C E D U R E S ================================================= */

/* === M E T H O D S ======================================================= */

Action::EditModeSet::EditModeSet():
	old_edit_mode()
{ }

Action::ParamVocab
Action::EditModeSet::get_param_vocab()
{
	ParamVocab ret(Action::CanvasSpecific::get_param_vocab());

	ret.push_back(ParamDesc("edit_mode",Param::TYPE_EDITMODE)
		.set_local_name(_("New Edit Mode"))
	);

	return ret;
}

bool
Action::EditModeSet::is_candidate(const ParamList &x)
{
	return candidate_check(get_param_vocab(),x);
}

bool
Action::EditModeSet::set_param(const synfig::String& name, const Action::Param &param)
{
/*
	if(name=="edit_mode" && param.get_type()==Param::TYPE_EDITMODE)
	{
		set_edit_mode(param.get_edit_mode());

		return true;
	}
*/

	return Action::CanvasSpecific::set_param(name,param);
}

bool
Action::EditModeSet::is_ready()const
{
	return Action::CanvasSpecific::is_ready() && get_canvas_interface();
}

void
Action::EditModeSet::perform()
{
	set_dirty(false);

	old_edit_mode=get_canvas_interface()->get_mode();

	if(old_edit_mode==get_edit_mode())
		return;

	get_canvas_interface()->mode_=get_edit_mode();

	get_canvas_interface()->signal_mode_changed_(get_edit_mode());
}

void
Action::EditModeSet::undo()
{
	set_dirty(false);

	if(old_edit_mode==get_edit_mode())
		return;

	get_canvas_interface()->mode_=old_edit_mode;

	get_canvas_interface()->signal_mode_changed_(old_edit_mode);
}
