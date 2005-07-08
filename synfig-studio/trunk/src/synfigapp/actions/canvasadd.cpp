/* === S Y N F I G ========================================================= */
/*!	\file canvasadd.cpp
**	\brief Template File
**
**	$Id: canvasadd.cpp,v 1.1.1.1 2005/01/07 03:34:37 darco Exp $
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

#include "canvasadd.h"
#include <synfigapp/canvasinterface.h>

#endif

using namespace std;
using namespace etl;
using namespace synfig;
using namespace synfigapp;
using namespace Action;

/* === M A C R O S ========================================================= */

ACTION_INIT(Action::CanvasAdd);
ACTION_SET_NAME(Action::CanvasAdd,"canvas_add");
ACTION_SET_LOCAL_NAME(Action::CanvasAdd,"Add Child Canvas");
ACTION_SET_TASK(Action::CanvasAdd,"add");
ACTION_SET_CATEGORY(Action::CanvasAdd,Action::CATEGORY_CANVAS);
ACTION_SET_PRIORITY(Action::CanvasAdd,0);
ACTION_SET_VERSION(Action::CanvasAdd,"0.0");
ACTION_SET_CVS_ID(Action::CanvasAdd,"$Id: canvasadd.cpp,v 1.1.1.1 2005/01/07 03:34:37 darco Exp $");

/* === G L O B A L S ======================================================= */

/* === P R O C E D U R E S ================================================= */

/* === M E T H O D S ======================================================= */

Action::CanvasAdd::CanvasAdd()
{
	set_dirty(true);
}

Action::ParamVocab
Action::CanvasAdd::get_param_vocab()
{
	ParamVocab ret(Action::CanvasSpecific::get_param_vocab());
	
	ret.push_back(ParamDesc("src",Param::TYPE_CANVAS)
		.set_local_name(_("New Canvas"))
		.set_optional()
	);

	ret.push_back(ParamDesc("id",Param::TYPE_STRING)
		.set_local_name(_("ID"))
		.set_desc(_("The name that you want this canvas to be"))
		.set_user_supplied()
	);
	
	return ret;
}

bool
Action::CanvasAdd::is_canidate(const ParamList &x)
{
	return canidate_check(get_param_vocab(),x);
}

bool
Action::CanvasAdd::set_param(const synfig::String& name, const Action::Param &param)
{
	if(name=="src" && param.get_type()==Param::TYPE_CANVAS)
	{
		new_canvas=param.get_canvas();
		
		return true;
	}
	if(name=="id" && param.get_type()==Param::TYPE_STRING)
	{
		id=param.get_string();
		
		return true;
	}

	return Action::CanvasSpecific::set_param(name,param);
}

bool
Action::CanvasAdd::is_ready()const
{
	return Action::CanvasSpecific::is_ready();
}

void
Action::CanvasAdd::perform()
{
	if(!new_canvas)
	{
		new_canvas=get_canvas()->new_child_canvas(id);
	}
	else
	{
		if(new_canvas->is_inline())
		{
			inline_parent=new_canvas->parent();
		}
		get_canvas()->add_child_canvas(new_canvas,id);
	}

	if(get_canvas_interface())
	{
		get_canvas_interface()->signal_canvas_added()(new_canvas);
	}
	else synfig::warning("CanvasInterface not set on action");
}

void
Action::CanvasAdd::undo()
{
	get_canvas()->remove_child_canvas(new_canvas);

	if(inline_parent)
		new_canvas->set_inline(inline_parent);		
	
	if(get_canvas_interface())
	{
		get_canvas_interface()->signal_canvas_removed()(new_canvas);
	}
	else synfig::warning("CanvasInterface not set on action");
}
