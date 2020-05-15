/* === S Y N F I G ========================================================= */
/*!	\file keyframetoggl.cpp
**	\brief Template File
**
**	$Id$
**
**	\legal
**	Copyright (c) 2002-2005 Robert B. Quattlebaum Jr., Adrian Bentley
**	Copyright (c) 2012-2013 Konstantin Dmitriev
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

#include "keyframetoggl.h"
#include <synfigapp/canvasinterface.h>
#include <synfig/valuenodes/valuenode_dynamiclist.h>
#include <synfig/valuenodes/valuenode_animated.h>
#include "activepointremove.h"
#include "waypointremove.h"

#include <synfigapp/localization.h>

#endif

using namespace std;
using namespace etl;
using namespace synfig;
using namespace synfigapp;
using namespace Action;

/* === M A C R O S ========================================================= */

ACTION_INIT_NO_GET_LOCAL_NAME(Action::KeyframeToggl);
ACTION_SET_NAME(Action::KeyframeToggl,"KeyframeToggl");
ACTION_SET_LOCAL_NAME(Action::KeyframeToggl,N_("Activate/Deactivate Keyframe"));
ACTION_SET_TASK(Action::KeyframeToggl,"disconnect");
ACTION_SET_CATEGORY(Action::KeyframeToggl,Action::CATEGORY_KEYFRAME);
ACTION_SET_PRIORITY(Action::KeyframeToggl,0);
ACTION_SET_VERSION(Action::KeyframeToggl,"0.0");
ACTION_SET_CVS_ID(Action::KeyframeToggl,"$Id$");

/* === G L O B A L S ======================================================= */

/* === P R O C E D U R E S ================================================= */

/* === M E T H O D S ======================================================= */

Action::KeyframeToggl::KeyframeToggl():
	new_status()
{
	keyframe.set_time(Time::begin()-1);
	set_dirty(true);
}

synfig::String
Action::KeyframeToggl::get_local_name()const
{
	if(keyframe.get_time()==(Time::begin()-1))
		return _("Activate Keyframe");

	return strprintf(_("%s at %s"),
					 new_status
					 ? _("Activate Keyframe")
					 : _("Deactivate Keyframe"),
					 keyframe.get_time().get_string(get_canvas()->rend_desc().get_frame_rate(), Time::FORMAT_FRAMES).c_str());
}

Action::ParamVocab
Action::KeyframeToggl::get_param_vocab()
{
	ParamVocab ret(Action::CanvasSpecific::get_param_vocab());

	ret.push_back(ParamDesc("keyframe",Param::TYPE_KEYFRAME)
		.set_local_name(_("Keyframe"))
		.set_desc(_("Keyframe to be activated or deactivated"))
	);

	ret.push_back(ParamDesc("new_status",Param::TYPE_BOOL)
		.set_local_name(_("New Status"))
		.set_desc(_("The new status of the keyframe"))
	);

	return ret;
}

bool
Action::KeyframeToggl::is_candidate(const ParamList &x)
{
	return candidate_check(get_param_vocab(),x);
}

bool
Action::KeyframeToggl::set_param(const synfig::String& name, const Action::Param &param)
{
	if(name=="keyframe" && param.get_type()==Param::TYPE_KEYFRAME)
	{
		keyframe=param.get_keyframe();

		return true;
	}

	if(name=="new_status" && param.get_type()==Param::TYPE_BOOL)
	{
		new_status=param.get_bool();

		return true;
	}

	return Action::CanvasSpecific::set_param(name,param);
}

bool
Action::KeyframeToggl::is_ready()const
{
	if(keyframe.get_time()==(Time::begin()-1))
		return false;
	return Action::CanvasSpecific::is_ready();
}

void
Action::KeyframeToggl::prepare()
{
	clear();

	KeyframeList::iterator iter;
	//try { get_canvas()->keyframe_list().find(keyframe);}
	//catch(synfig::Exception::NotFound)
	if (!get_canvas()->keyframe_list().find(keyframe, iter)) {
		throw Error(_("Unable to find the given keyframe"));
	}
}


void
Action::KeyframeToggl::perform()
{
	Action::Super::perform();
	
	keyframe.set_active(new_status);
	
	KeyframeList::iterator iter;
	//*get_canvas()->keyframe_list().find(keyframe)=keyframe;
	if (get_canvas()->keyframe_list().find(keyframe, iter)) {
		*iter = keyframe;
		get_canvas()->keyframe_list().sync();
	}

	if(get_canvas_interface())
	{
		get_canvas_interface()->signal_keyframe_changed()(keyframe);
	}
	else synfig::warning("CanvasInterface not set on action");
}

void
Action::KeyframeToggl::undo()
{
	//try { get_canvas()->keyframe_list().find(keyframe.get_time()); throw Error(_("A Keyframe already exists at this point in time"));}
	//catch(synfig::Exception::NotFound) { }

	//try { get_canvas()->keyframe_list().find(keyframe); throw Error(_("This keyframe is already in the ValueNode"));}
	//catch(synfig::Exception::NotFound) { }

	Action::Super::undo();
	
	keyframe.set_active(!new_status);
	
	//*get_canvas()->keyframe_list().find(keyframe)=keyframe;
	KeyframeList::iterator iter;
	if (get_canvas()->keyframe_list().find(keyframe, iter)) *iter = keyframe;

	get_canvas()->keyframe_list().sync();

	if(get_canvas_interface())
	{
		get_canvas_interface()->signal_keyframe_changed()(keyframe);
	}
	else synfig::warning("CanvasInterface not set on action");
}
