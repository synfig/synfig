/* === S I N F G =========================================================== */
/*!	\file canvasremove.cpp
**	\brief Template File
**
**	$Id: canvasremove.cpp,v 1.1.1.1 2005/01/07 03:34:37 darco Exp $
**
**	\legal
**	Copyright (c) 2002 Robert B. Quattlebaum Jr.
**
**	This software and associated documentation
**	are CONFIDENTIAL and PROPRIETARY property of
**	the above-mentioned copyright holder.
**
**	You may not copy, print, publish, or in any
**	other way distribute this software without
**	a prior written agreement with
**	the copyright holder.
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

#include "canvasremove.h"
#include <sinfgapp/canvasinterface.h>

#endif

using namespace std;
using namespace etl;
using namespace sinfg;
using namespace sinfgapp;
using namespace Action;

/* === M A C R O S ========================================================= */

ACTION_INIT(Action::CanvasRemove);
ACTION_SET_NAME(Action::CanvasRemove,"canvas_remove");
ACTION_SET_LOCAL_NAME(Action::CanvasRemove,"Remove Canvas");
ACTION_SET_TASK(Action::CanvasRemove,"remove");
ACTION_SET_CATEGORY(Action::CanvasRemove,Action::CATEGORY_CANVAS);
ACTION_SET_PRIORITY(Action::CanvasRemove,0);
ACTION_SET_VERSION(Action::CanvasRemove,"0.0");
ACTION_SET_CVS_ID(Action::CanvasRemove,"$Id: canvasremove.cpp,v 1.1.1.1 2005/01/07 03:34:37 darco Exp $");

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
Action::CanvasRemove::is_canidate(const ParamList &x)
{
	if(canidate_check(get_param_vocab(),x))
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
Action::CanvasRemove::set_param(const sinfg::String& name, const Action::Param &param)
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
		throw Error(_("You cannot remove an inline canvas!"));
	
	parent_canvas=get_canvas()->parent();
	canvas_id=get_canvas()->get_id();
	
	assert(parent_canvas);
	
	parent_canvas->remove_child_canvas(get_canvas());
	
	if(get_canvas_interface())
	{
		get_canvas_interface()->signal_canvas_removed()(get_canvas());
	}
	else sinfg::warning("CanvasInterface not set on action");
}

void
Action::CanvasRemove::undo()
{
	parent_canvas->add_child_canvas(get_canvas(), canvas_id);

	if(get_canvas_interface())
	{
		get_canvas_interface()->signal_canvas_added()(get_canvas());
	}
	else sinfg::warning("CanvasInterface not set on action");
}
