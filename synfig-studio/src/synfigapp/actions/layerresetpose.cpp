/* === S Y N F I G ========================================================= */
/*!	\file layerresetpose.cpp
**	\brief Template File
**
**	$Id$
**
**	\legal
**	......... ... 2014 Ivan Mahonin
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

#include "layerresetpose.h"
#include <synfigapp/canvasinterface.h>
#include <synfigapp/localization.h>

#include <synfig/valuenodes/valuenode_bone.h>
#include <synfig/valuenodes/valuenode_composite.h>

#endif

using namespace etl;
using namespace synfig;
using namespace synfigapp;
using namespace Action;

/* === M A C R O S ========================================================= */

ACTION_INIT(Action::LayerResetPose);
ACTION_SET_NAME(Action::LayerResetPose,"LayerResetPose");
ACTION_SET_LOCAL_NAME(Action::LayerResetPose,N_("Reset Pose"));
ACTION_SET_TASK(Action::LayerResetPose,"layer_reset_pose");
ACTION_SET_CATEGORY(Action::LayerResetPose,Action::CATEGORY_LAYER);
ACTION_SET_PRIORITY(Action::LayerResetPose,0);
ACTION_SET_VERSION(Action::LayerResetPose,"0.0");

/* === G L O B A L S ======================================================= */

/* === P R O C E D U R E S ================================================= */

/* === M E T H O D S ======================================================= */

Action::LayerResetPose::LayerResetPose():
	time(0)
{
}

Action::ParamVocab
Action::LayerResetPose::get_param_vocab()
{
	ParamVocab ret(Action::CanvasSpecific::get_param_vocab());

	ret.push_back(ParamDesc("layer",Param::TYPE_LAYER)
		.set_local_name(_("Layer"))
		.set_desc(_("Layer to reset"))
		.set_supports_multiple()
	);
	ret.push_back(ParamDesc("time",Param::TYPE_TIME)
		.set_local_name(_("Time"))
		.set_optional()
	);

	return ret;
}

bool
Action::LayerResetPose::is_candidate(const ParamList &x)
{
	if(!candidate_check(get_param_vocab(),x))
		return false;

	for(ParamList::const_iterator i = x.find("layer"); i != x.end() && i->first == "layer"; ++i)
		if (i->second.get_type()==Param::TYPE_LAYER
		 && i->second.get_layer()->get_name() == "skeleton_deformation")
			return true;

	return false;
}

bool
Action::LayerResetPose::set_param(const synfig::String& name, const Action::Param &param)
{
	if (name=="layer"
	 && param.get_type()==Param::TYPE_LAYER
	 && param.get_layer()->get_name() == "skeleton_deformation" )
	{
		layers.push_back(param.get_layer());
		return true;
	}

	if(name=="time" && param.get_type()==Param::TYPE_TIME)
	{
		time=param.get_time();
		return true;
	}

	return Action::CanvasSpecific::set_param(name,param);
}

bool
Action::LayerResetPose::is_ready()const
{
	if(layers.empty())
		return false;
	return Action::CanvasSpecific::is_ready();
}

void
Action::LayerResetPose::prepare()
{
	clear();

	for(std::list<synfig::Layer::Handle>::const_iterator i=layers.begin(); i!=layers.end(); ++i)
	{
		Layer::Handle layer(*i);
		Canvas::Handle subcanvas(layer->get_canvas());

		Layer::DynamicParamList::const_iterator j = layer->dynamic_param_list().find("bones");
		if (j == layer->dynamic_param_list().end()) continue;

		LinkableValueNode::Handle bones_node = LinkableValueNode::Handle::cast_dynamic(j->second);
		if (!bones_node) continue;

		for(int k = 0; k < bones_node->link_count(); ++k)
		{
			ValueNode_Composite::Handle pair_node =
				ValueNode_Composite::Handle::cast_dynamic(
					bones_node->get_link(k) );
			if (!pair_node) continue;

			ValueNode_Bone::Handle bone_node =
				ValueNode_Bone::Handle::cast_dynamic(
					pair_node->get_link("first") );
			if (!bone_node) continue;

			Action::Handle action(Action::create("ValueDescResetPose"));
			action->set_param("canvas", get_canvas());
			action->set_param("canvas_interface", get_canvas_interface());
			action->set_param(
				"value_desc",
				ValueDesc(
					bone_node,
					bone_node->get_link_index_from_name("origin"),
					ValueDesc(pair_node, pair_node->get_link_index_from_name("first")) ));
			action->set_param("time", time);
			add_action_front(action);
		}
	}
}
