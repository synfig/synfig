/* === S Y N F I G ========================================================= */
/*!	\file canvasmetadataset.cpp
**	\brief Template File
**
**	$Id$
**
**	\legal
**	Copyright (c) 2002-2005 Robert B. Quattlebaum Jr., Adrian Bentley
**	Copyright (c) 2008 Chris Moore
**	Copyright (c) 2011 Nikita Kitaev
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

#include "canvasmetadataset.h"
#include <synfigapp/canvasinterface.h>

#include <synfigapp/localization.h>

#endif

using namespace std;
using namespace etl;
using namespace synfig;
using namespace synfigapp;
using namespace Action;

/* === M A C R O S ========================================================= */

ACTION_INIT_NO_GET_LOCAL_NAME(Action::CanvasMetadataSet);
ACTION_SET_NAME(Action::CanvasMetadataSet,"CanvasMetadataSet");
ACTION_SET_LOCAL_NAME(Action::CanvasMetadataSet,N_("Set Canvas Metadata"));
ACTION_SET_TASK(Action::CanvasMetadataSet,"set");
ACTION_SET_CATEGORY(Action::CanvasMetadataSet,Action::CATEGORY_CANVAS);
ACTION_SET_PRIORITY(Action::CanvasMetadataSet,0);
ACTION_SET_VERSION(Action::CanvasMetadataSet,"0.0");

/* === G L O B A L S ======================================================= */

/* === P R O C E D U R E S ================================================= */

/* === M E T H O D S ======================================================= */

Action::CanvasMetadataSet::CanvasMetadataSet()
{
}

synfig::String
Action::CanvasMetadataSet::get_local_name()const
{
	// TRANSLATORS: This is used in the 'history' dialog when a Canvas has its name changed.
	return _("Edit canvas metadata");
}

Action::ParamVocab
Action::CanvasMetadataSet::get_param_vocab()
{
	ParamVocab ret(Action::CanvasSpecific::get_param_vocab());

	ret.push_back(ParamDesc("key",Param::TYPE_STRING)
		.set_local_name(_("Key"))
	);
	ret.push_back(ParamDesc("value",Param::TYPE_STRING)
		.set_local_name(_("Value"))
	);

	return ret;
}

bool
Action::CanvasMetadataSet::is_candidate(const ParamList &x)
{
	return candidate_check(get_param_vocab(),x);
}

bool
Action::CanvasMetadataSet::set_param(const synfig::String& name, const Action::Param &param)
{
	if(name=="key" && param.get_type()==Param::TYPE_STRING)
	{
		key=param.get_string();

		return true;
	}

	if(name=="value" && param.get_type()==Param::TYPE_STRING)
	{
		new_value=param.get_string();

		return true;
	}

	return Action::CanvasSpecific::set_param(name,param);
}

bool
Action::CanvasMetadataSet::is_ready()const
{
	if(key.empty())
	{
		synfig::error("Action::CanvasMetadataSet::is_ready(): Metadata Key not specified!");
		return false;
	}

	if(new_value.empty())
	{
		synfig::error("Action::CanvasMetadataSet::is_ready(): Metadata new value not set!");
		return false;
	}

	return Action::CanvasSpecific::is_ready();
}

void
Action::CanvasMetadataSet::perform()
{
	old_value=get_canvas()->get_meta_data(key);

	get_canvas()->set_meta_data(key,new_value);
}

void
Action::CanvasMetadataSet::undo()
{
	get_canvas()->set_meta_data(key,old_value);
}
