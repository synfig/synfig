/* === S I N F G =========================================================== */
/*!	\file keyframeadd.cpp
**	\brief Template File
**
**	$Id: keyframeadd.cpp,v 1.1.1.1 2005/01/07 03:34:37 darco Exp $
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

#include "keyframeadd.h"
#include <sinfgapp/canvasinterface.h>

#endif

using namespace std;
using namespace etl;
using namespace sinfg;
using namespace sinfgapp;
using namespace Action;

/* === M A C R O S ========================================================= */

ACTION_INIT(Action::KeyframeAdd);
ACTION_SET_NAME(Action::KeyframeAdd,"keyframe_add");
ACTION_SET_LOCAL_NAME(Action::KeyframeAdd,"Add Keyframe");
ACTION_SET_TASK(Action::KeyframeAdd,"add");
ACTION_SET_CATEGORY(Action::KeyframeAdd,Action::CATEGORY_KEYFRAME);
ACTION_SET_PRIORITY(Action::KeyframeAdd,0);
ACTION_SET_VERSION(Action::KeyframeAdd,"0.0");
ACTION_SET_CVS_ID(Action::KeyframeAdd,"$Id: keyframeadd.cpp,v 1.1.1.1 2005/01/07 03:34:37 darco Exp $");

/* === G L O B A L S ======================================================= */

/* === P R O C E D U R E S ================================================= */

/* === M E T H O D S ======================================================= */

Action::KeyframeAdd::KeyframeAdd()
{
	keyframe.set_time(Time::begin()-1);
	set_dirty(false);
}

Action::ParamVocab
Action::KeyframeAdd::get_param_vocab()
{
	ParamVocab ret(Action::CanvasSpecific::get_param_vocab());
	
	ret.push_back(ParamDesc("keyframe",Param::TYPE_KEYFRAME)
		.set_local_name(_("New Keyframe"))
		.set_desc(_("Keyframe to be added"))
	);

	return ret;
}

bool
Action::KeyframeAdd::is_canidate(const ParamList &x)
{
	if(!canidate_check(get_param_vocab(),x))
		return false;
	
	return true;
}

bool
Action::KeyframeAdd::set_param(const sinfg::String& name, const Action::Param &param)
{
	if(name=="keyframe" && param.get_type()==Param::TYPE_KEYFRAME)
	{
		keyframe=param.get_keyframe();
		
		return true;
	}

	return Action::CanvasSpecific::set_param(name,param);
}

bool
Action::KeyframeAdd::is_ready()const
{
	if(keyframe.get_time()==(Time::begin()-1))
		return false;
	return Action::CanvasSpecific::is_ready();
}

void
Action::KeyframeAdd::perform()
{	
	try { get_canvas()->keyframe_list().find(keyframe.get_time()); throw Error(_("A Keyframe already exists at this point in time"));}
	catch(sinfg::Exception::NotFound) { }	

	try { get_canvas()->keyframe_list().find(keyframe); throw Error(_("This keyframe is already in the ValueNode"));}
	catch(sinfg::Exception::NotFound) { }	
	
	get_canvas()->keyframe_list().add(keyframe);
	
	if(get_canvas_interface())
	{
		get_canvas_interface()->signal_keyframe_added()(keyframe);
	}
	else sinfg::warning("CanvasInterface not set on action");
}

void
Action::KeyframeAdd::undo()
{
	if(get_canvas_interface())
	{
		get_canvas_interface()->signal_keyframe_removed()(keyframe);
	}
	else sinfg::warning("CanvasInterface not set on action");

	get_canvas()->keyframe_list().erase(keyframe);	
}
