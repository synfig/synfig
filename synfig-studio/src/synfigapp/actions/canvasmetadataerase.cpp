/* === S Y N F I G ========================================================= */
/*!	\file canvasmetadataerase.cpp
**	\brief Template File
**
**	\legal
**	Copyright (c) 2002-2005 Robert B. Quattlebaum Jr., Adrian Bentley
**	Copyright (c) 2008 Chris Moore
**	Copyright (c) 2011 Nikita Kitaev
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

#include "canvasmetadataerase.h"
#include <synfigapp/canvasinterface.h>
#include <synfigapp/localization.h>

#endif

using namespace synfig;
using namespace synfigapp;
using namespace Action;

/* === M A C R O S ========================================================= */

ACTION_INIT_NO_GET_LOCAL_NAME(Action::CanvasMetadataErase);
ACTION_SET_NAME(Action::CanvasMetadataErase,"CanvasMetadataErase");
ACTION_SET_LOCAL_NAME(Action::CanvasMetadataErase,N_("Erase Canvas Metadata"));
ACTION_SET_TASK(Action::CanvasMetadataErase,"set");
ACTION_SET_CATEGORY(Action::CanvasMetadataErase,Action::CATEGORY_CANVAS);
ACTION_SET_PRIORITY(Action::CanvasMetadataErase,0);
ACTION_SET_VERSION(Action::CanvasMetadataErase,"0.0");

/* === G L O B A L S ======================================================= */

/* === P R O C E D U R E S ================================================= */

/* === M E T H O D S ======================================================= */

Action::CanvasMetadataErase::CanvasMetadataErase()
{
}

synfig::String
Action::CanvasMetadataErase::get_local_name()const
{
	// TRANSLATORS: This is used in the 'history' dialog when a Canvas has its name changed.
	return _("Erase canvas metadata");
}

Action::ParamVocab
Action::CanvasMetadataErase::get_param_vocab()
{
	ParamVocab ret(Action::CanvasSpecific::get_param_vocab());

	ret.push_back(ParamDesc("key",Param::TYPE_STRING)
		.set_local_name(_("Key"))
	);

	return ret;
}

bool
Action::CanvasMetadataErase::is_candidate(const ParamList &x)
{
	return candidate_check(get_param_vocab(),x);
}

bool
Action::CanvasMetadataErase::set_param(const synfig::String& name, const Action::Param &param)
{
	if(name=="key" && param.get_type()==Param::TYPE_STRING)
	{
		key=param.get_string();

		return true;
	}

	return Action::CanvasSpecific::set_param(name,param);
}

bool
Action::CanvasMetadataErase::is_ready()const
{
	if(key.empty())
	{
		synfig::error("Action::CanvasMetadataErase::is_ready(): Metadata Key not specified!");
		return false;
	}

	return Action::CanvasSpecific::is_ready();
}

void
Action::CanvasMetadataErase::perform()
{
	old_value=get_canvas()->get_meta_data(key);

	get_canvas()->erase_meta_data(key);
}

void
Action::CanvasMetadataErase::undo()
{
	get_canvas()->set_meta_data(key,old_value);
}
