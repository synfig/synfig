/* === S Y N F I G ========================================================= */
/*!	\file layersetdesc.cpp
**	\brief Template File
**
**	$Id: layersetdesc.cpp,v 1.1.1.1 2005/01/07 03:34:37 darco Exp $
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

#include "layersetdesc.h"
#include <synfigapp/canvasinterface.h>

#endif

using namespace std;
using namespace etl;
using namespace synfig;
using namespace synfigapp;
using namespace Action;

/* === M A C R O S ========================================================= */

ACTION_INIT(Action::LayerSetDesc);
ACTION_SET_NAME(Action::LayerSetDesc,"layer_set_desc");
ACTION_SET_LOCAL_NAME(Action::LayerSetDesc,_("Set Layer Description"));
ACTION_SET_TASK(Action::LayerSetDesc,"set_desc");
ACTION_SET_CATEGORY(Action::LayerSetDesc,Action::CATEGORY_LAYER);
ACTION_SET_PRIORITY(Action::LayerSetDesc,0);
ACTION_SET_VERSION(Action::LayerSetDesc,"0.0");
ACTION_SET_CVS_ID(Action::LayerSetDesc,"$Id: layersetdesc.cpp,v 1.1.1.1 2005/01/07 03:34:37 darco Exp $");

/* === G L O B A L S ======================================================= */

static const int nindex=-1;

/* === P R O C E D U R E S ================================================= */

/* === M E T H O D S ======================================================= */

Action::LayerSetDesc::LayerSetDesc()
{
}

Action::ParamVocab
Action::LayerSetDesc::get_param_vocab()
{
	ParamVocab ret(Action::CanvasSpecific::get_param_vocab());
	
	ret.push_back(ParamDesc("layer",Param::TYPE_LAYER)
		.set_local_name(_("Layer"))
		.set_desc(_("Layer to be moved"))
	);

	ret.push_back(ParamDesc("new_description",Param::TYPE_STRING)
		.set_local_name(_("New Description"))
		.set_local_name(_("Enter a new description for this layer"))
		.set_user_supplied()
	);
	
	return ret;
}

bool
Action::LayerSetDesc::is_canidate(const ParamList &x)
{
	return canidate_check(get_param_vocab(),x);
}

bool
Action::LayerSetDesc::set_param(const synfig::String& name, const Action::Param &param)
{
	if(name=="layer" && param.get_type()==Param::TYPE_LAYER)
	{
		layer=param.get_layer();
		
		return true;
	}

	if(name=="new_description" && param.get_type()==Param::TYPE_STRING)
	{
		new_description=param.get_string();
		
		return true;
	}

	return Action::CanvasSpecific::set_param(name,param);
}

bool
Action::LayerSetDesc::is_ready()const
{
	if(!layer)
		return false;
	return Action::CanvasSpecific::is_ready();
}

void
Action::LayerSetDesc::perform()
{
	old_description=layer->get_description();
	layer->set_description(new_description);
	set_dirty(false);
	if(get_canvas_interface())
	{
		get_canvas_interface()->signal_layer_new_description()(layer,new_description);
	}
	else synfig::warning("CanvasInterface not set on action");
}

void
Action::LayerSetDesc::undo()
{
	layer->set_description(old_description);
	set_dirty(false);
	if(get_canvas_interface())
	{
		get_canvas_interface()->signal_layer_new_description()(layer,old_description);
	}
	else synfig::warning("CanvasInterface not set on action");
}
