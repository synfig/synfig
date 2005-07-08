/* === S Y N F I G ========================================================= */
/*!	\file action_layerremove.cpp
**	\brief Template File
**
**	$Id: groupaddlayers.cpp,v 1.1.1.1 2005/01/07 03:34:37 darco Exp $
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

#include "groupaddlayers.h"
#include <synfigapp/canvasinterface.h>

#endif

using namespace std;
using namespace etl;
using namespace synfig;
using namespace synfigapp;
using namespace Action;

/* === M A C R O S ========================================================= */

ACTION_INIT(Action::GroupAddLayers);
ACTION_SET_NAME(Action::GroupAddLayers,"group_add_layers");
ACTION_SET_LOCAL_NAME(Action::GroupAddLayers,"Add Layers to Group");
ACTION_SET_TASK(Action::GroupAddLayers,"add");
ACTION_SET_CATEGORY(Action::GroupAddLayers,Action::CATEGORY_LAYER|Action::CATEGORY_GROUP);
ACTION_SET_PRIORITY(Action::GroupAddLayers,0);
ACTION_SET_VERSION(Action::GroupAddLayers,"0.0");
ACTION_SET_CVS_ID(Action::GroupAddLayers,"$Id: groupaddlayers.cpp,v 1.1.1.1 2005/01/07 03:34:37 darco Exp $");

/* === G L O B A L S ======================================================= */

/* === P R O C E D U R E S ================================================= */

/* === M E T H O D S ======================================================= */

Action::GroupAddLayers::GroupAddLayers()
{
}

Action::ParamVocab
Action::GroupAddLayers::get_param_vocab()
{
	ParamVocab ret(Action::CanvasSpecific::get_param_vocab());
	
	ret.push_back(ParamDesc("layer",Param::TYPE_LAYER)
		.set_local_name(_("Layer"))
		.set_desc(_("Layer to be added to group"))
		.set_supports_multiple()
	);

	ret.push_back(ParamDesc("group",Param::TYPE_STRING)
		.set_local_name(_("Group"))
		.set_desc(_("Name of the Group to add the Layers to"))
		.set_user_supplied()
	);
	
	return ret;
}

bool
Action::GroupAddLayers::is_canidate(const ParamList &x)
{
	return canidate_check(get_param_vocab(),x);
}

bool
Action::GroupAddLayers::set_param(const synfig::String& name, const Action::Param &param)
{
	if(name=="layer" && param.get_type()==Param::TYPE_LAYER)
	{
		std::pair<synfig::Layer::Handle,String> layer_pair;
		layer_pair.first=param.get_layer();
		layer_list.push_back(layer_pair);
		
		return true;
	}

	if(name=="group" && param.get_type()==Param::TYPE_STRING)
	{
		group=param.get_string();
		
		return true;
	}

	return Action::CanvasSpecific::set_param(name,param);
}

bool
Action::GroupAddLayers::is_ready()const
{
	if(layer_list.empty() || group.empty())
		return false;
	return Action::CanvasSpecific::is_ready();
}

void
Action::GroupAddLayers::perform()
{
	std::list<std::pair<synfig::Layer::Handle,String> >::iterator iter;
	for(iter=layer_list.begin();iter!=layer_list.end();++iter)
	{
		Layer::Handle layer(iter->first);
		iter->second=layer->get_group();
		
		layer->add_to_group(group);
	}
}

void
Action::GroupAddLayers::undo()
{
	std::list<std::pair<synfig::Layer::Handle,String> >::iterator iter;
	for(iter=layer_list.begin();iter!=layer_list.end();++iter)
	{
		Layer::Handle layer(iter->first);
		
		layer->remove_from_group(group);
		
		layer->add_to_group(iter->second);
	}
}
