/* === S I N F G =========================================================== */
/*!	\file editmodeset.cpp
**	\brief Template File
**
**	$Id: editmodeset.cpp,v 1.1.1.1 2005/01/07 03:34:37 darco Exp $
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

#include "editmodeset.h"
#include <sinfgapp/canvasinterface.h>

#endif

using namespace std;
using namespace etl;
using namespace sinfg;
using namespace sinfgapp;
using namespace Action;

/* === M A C R O S ========================================================= */

ACTION_INIT(Action::EditModeSet);
ACTION_SET_NAME(Action::EditModeSet,"edit_mode_set");
ACTION_SET_LOCAL_NAME(Action::EditModeSet,"Set Edit Mode");
ACTION_SET_TASK(Action::EditModeSet,"set");
ACTION_SET_CATEGORY(Action::EditModeSet,Action::CATEGORY_OTHER);
ACTION_SET_PRIORITY(Action::EditModeSet,0);
ACTION_SET_VERSION(Action::EditModeSet,"0.0");
ACTION_SET_CVS_ID(Action::EditModeSet,"$Id: editmodeset.cpp,v 1.1.1.1 2005/01/07 03:34:37 darco Exp $");

/* === G L O B A L S ======================================================= */

/* === P R O C E D U R E S ================================================= */

/* === M E T H O D S ======================================================= */

Action::EditModeSet::EditModeSet()
{
}

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
Action::EditModeSet::is_canidate(const ParamList &x)
{
	return canidate_check(get_param_vocab(),x);
}

bool
Action::EditModeSet::set_param(const sinfg::String& name, const Action::Param &param)
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
