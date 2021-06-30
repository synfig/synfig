/* === S Y N F I G ========================================================= */
/*!	\file canvasrenddescset.cpp
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

#include "canvasrenddescset.h"
#include <synfigapp/canvasinterface.h>

#include <synfigapp/localization.h>

#endif

using namespace etl;
using namespace synfig;
using namespace synfigapp;
using namespace Action;

/* === M A C R O S ========================================================= */

ACTION_INIT(Action::CanvasRendDescSet);
ACTION_SET_NAME(Action::CanvasRendDescSet,"CanvasRendDescSet");
ACTION_SET_LOCAL_NAME(Action::CanvasRendDescSet,N_("Set Canvas RendDesc"));
ACTION_SET_TASK(Action::CanvasRendDescSet,"set");
ACTION_SET_CATEGORY(Action::CanvasRendDescSet,Action::CATEGORY_CANVAS);
ACTION_SET_PRIORITY(Action::CanvasRendDescSet,0);
ACTION_SET_VERSION(Action::CanvasRendDescSet,"0.0");

/* === G L O B A L S ======================================================= */

/* === P R O C E D U R E S ================================================= */

/* === M E T H O D S ======================================================= */

Action::CanvasRendDescSet::CanvasRendDescSet()
{
	set_dirty(true);
}

Action::ParamVocab
Action::CanvasRendDescSet::get_param_vocab()
{
	ParamVocab ret(Action::CanvasSpecific::get_param_vocab());

	ret.push_back(ParamDesc("rend_desc",Param::TYPE_RENDDESC)
		.set_local_name(_("RendDesc"))
	);

	return ret;
}

bool
Action::CanvasRendDescSet::is_candidate(const ParamList &x)
{
	return candidate_check(get_param_vocab(),x);
}

bool
Action::CanvasRendDescSet::set_param(const synfig::String& name, const Action::Param &param)
{
	if(name=="rend_desc" && param.get_type()==Param::TYPE_RENDDESC)
	{
		new_rend_desc=param.get_rend_desc();

		return true;
	}

	return Action::CanvasSpecific::set_param(name,param);
}

bool
Action::CanvasRendDescSet::is_ready()const
{
	return Action::CanvasSpecific::is_ready();
}

void
Action::CanvasRendDescSet::perform()
{
	old_rend_desc=get_canvas()->rend_desc();

	get_canvas()->rend_desc()=new_rend_desc;

	if(get_canvas_interface())
	{
		get_canvas_interface()->signal_rend_desc_changed()();
	}
	else synfig::warning("CanvasInterface not set on action");
}

void
Action::CanvasRendDescSet::undo()
{
	get_canvas()->rend_desc()=old_rend_desc;

	if(get_canvas_interface())
	{
		get_canvas_interface()->signal_rend_desc_changed()();
	}
	else synfig::warning("CanvasInterface not set on action");
}
