/* === S Y N F I G ========================================================= */
/*!	\file canvasremove.cpp
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

#include "canvasremove.h"
#include <synfigapp/canvasinterface.h>

#include <synfigapp/localization.h>

#endif

using namespace etl;
using namespace synfig;
using namespace synfigapp;
using namespace Action;

/* === M A C R O S ========================================================= */

ACTION_INIT(Action::CanvasRemove);
ACTION_SET_NAME(Action::CanvasRemove,"CanvasRemove");
ACTION_SET_LOCAL_NAME(Action::CanvasRemove,N_("Remove Canvas"));
ACTION_SET_TASK(Action::CanvasRemove,"remove");
ACTION_SET_CATEGORY(Action::CanvasRemove,Action::CATEGORY_CANVAS);
ACTION_SET_PRIORITY(Action::CanvasRemove,0);
ACTION_SET_VERSION(Action::CanvasRemove,"0.0");

/* === G L O B A L S ======================================================= */

/* === P R O C E D U R E S ================================================= */

/* === M E T H O D S ======================================================= */

Action::CanvasRemove::CanvasRemove()
{
}

Action::ParamVocab
Action::CanvasRemove::get_param_vocab()
{
	ParamVocab ret(Action::CanvasSpecific::get_param_vocab());

	return ret;
}

bool
Action::CanvasRemove::is_candidate(const ParamList &x)
{
	if(candidate_check(get_param_vocab(),x))
	{
		Canvas::Handle canvas=x.find("canvas")->second.get_canvas();
		assert(canvas);
		// We cannot remove the root canvas.
		if(canvas->is_root())
			return false;

		return true;
	}
	return false;
}

bool
Action::CanvasRemove::set_param(const synfig::String& name, const Action::Param &param)
{
	return Action::CanvasSpecific::set_param(name,param);
}

bool
Action::CanvasRemove::is_ready()const
{
	return Action::CanvasSpecific::is_ready();
}

void
Action::CanvasRemove::perform()
{
	// We cannot remove the root canvas.
	if(get_canvas()->is_root())
		throw Error(_("You cannot remove the root canvas!"));

	if(get_canvas()->is_inline())
		throw Error(_("You cannot remove a canvas from a Group!"));

	parent_canvas=get_canvas()->parent();
	canvas_id=get_canvas()->get_id();

	assert(parent_canvas);

	parent_canvas->remove_child_canvas(get_canvas());

	if(get_canvas_interface())
	{
		get_canvas_interface()->signal_canvas_removed()(get_canvas());
	}
	else synfig::warning("CanvasInterface not set on action");
}

void
Action::CanvasRemove::undo()
{
	parent_canvas->add_child_canvas(get_canvas(), canvas_id);

	if(get_canvas_interface())
	{
		get_canvas_interface()->signal_canvas_added()(get_canvas());
	}
	else synfig::warning("CanvasInterface not set on action");
}
