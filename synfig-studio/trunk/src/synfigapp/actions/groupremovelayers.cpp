/* === S Y N F I G ========================================================= */
/*!	\file action_layerremove.cpp
**	\brief Template File
**
**	$Id: groupremovelayers.cpp,v 1.1.1.1 2005/01/07 03:34:37 darco Exp $
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

#include "groupremovelayers.h"
#include <synfigapp/canvasinterface.h>

#endif

using namespace std;
using namespace etl;
using namespace synfig;
using namespace synfigapp;
using namespace Action;

/* === M A C R O S ========================================================= */

ACTION_INIT(Action::GroupRemoveLayers);
ACTION_SET_NAME(Action::GroupRemoveLayers,"group_remove_layers");
ACTION_SET_LOCAL_NAME(Action::GroupRemoveLayers,"Remove Layers from a Group");
ACTION_SET_TASK(Action::GroupRemoveLayers,"remove");
ACTION_SET_CATEGORY(Action::GroupRemoveLayers,Action::CATEGORY_LAYER|Action::CATEGORY_GROUP);
ACTION_SET_PRIORITY(Action::GroupRemoveLayers,0);
ACTION_SET_VERSION(Action::GroupRemoveLayers,"0.0");
ACTION_SET_CVS_ID(Action::GroupRemoveLayers,"$Id: groupremovelayers.cpp,v 1.1.1.1 2005/01/07 03:34:37 darco Exp $");

/* === G L O B A L S ======================================================= */

/* === P R O C E D U R E S ================================================= */

/* === M E T H O D S ======================================================= */

Action::GroupRemoveLayers::GroupRemoveLayers()
{
}

Action::ParamVocab
Action::GroupRemoveLayers::get_param_vocab()
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
Action::GroupRemoveLayers::is_canidate(const ParamList &x)
{
	return canidate_check(get_param_vocab(),x);
}

bool
Action::GroupRemoveLayers::set_param(const synfig::String& name, const Action::Param &param)
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
Action::GroupRemoveLayers::is_ready()const
{
	if(layer_list.empty() || group.empty())
		return false;
	return Action::CanvasSpecific::is_ready();
}

void
Action::GroupRemoveLayers::perform()
{
	std::list<std::pair<synfig::Layer::Handle,String> >::iterator iter;
	for(iter=layer_list.begin();iter!=layer_list.end();++iter)
	{
		Layer::Handle layer(iter->first);
		iter->second=layer->get_group();
		
		layer->remove_from_group(group);
	}
}

void
Action::GroupRemoveLayers::undo()
{
	std::list<std::pair<synfig::Layer::Handle,String> >::iterator iter;
	for(iter=layer_list.begin();iter!=layer_list.end();++iter)
	{
		Layer::Handle layer(iter->first);
		
		layer->add_to_group(iter->second);
	}
}
