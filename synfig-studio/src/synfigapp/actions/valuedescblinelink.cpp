/* === S Y N F I G ========================================================= */
/*!	\file valuedescblinelink.cpp
**	\brief Template File
**
**	\legal
**	Copyright (c) 2002-2005 Robert B. Quattlebaum Jr., Adrian Bentley
**	Copyright (c) 2007, 2008 Chris Moore
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

#include "layerparamconnect.h"
#include "valuenodelinkconnect.h"
#include "valuenodereplace.h"
#include "valuedescblinelink.h"
#include "valuedescset.h"

#include <synfigapp/canvasinterface.h>
#include <synfig/valuenodes/valuenode_const.h>
#include <synfig/valuenodes/valuenode_composite.h>
#include <synfig/valuenodes/valuenode_blinecalctangent.h>
#include <synfig/valuenodes/valuenode_blinecalcvertex.h>
#include <synfig/valuenodes/valuenode_blinecalcwidth.h>
#include <synfig/valuenodes/valuenode_bline.h>

#include <synfigapp/localization.h>

#endif

using namespace synfig;
using namespace synfigapp;
using namespace Action;

/* === M A C R O S ========================================================= */

ACTION_INIT(Action::ValueDescBLineLink);
ACTION_SET_NAME(Action::ValueDescBLineLink,"ValueDescBLineLink");
ACTION_SET_LOCAL_NAME(Action::ValueDescBLineLink,N_("Link to Spline"));
ACTION_SET_TASK(Action::ValueDescBLineLink,"connect");
ACTION_SET_CATEGORY(Action::ValueDescBLineLink,Action::CATEGORY_BEZIER);
ACTION_SET_PRIORITY(Action::ValueDescBLineLink,0);
ACTION_SET_VERSION(Action::ValueDescBLineLink,"0.0");

/* === G L O B A L S ======================================================= */

/* === P R O C E D U R E S ================================================= */

/* === M E T H O D S ======================================================= */

Action::ValueDescBLineLink::ValueDescBLineLink():
	origin(),
	index()
{ }

Action::ParamVocab
Action::ValueDescBLineLink::get_param_vocab()
{
	ParamVocab ret(Action::CanvasSpecific::get_param_vocab());

	ret.push_back(ParamDesc("selected_value_desc",Param::TYPE_VALUEDESC)
		.set_local_name(_("ValueDesc to link"))
		.set_supports_multiple()
	);
	ret.push_back(ParamDesc("value_desc",Param::TYPE_VALUEDESC)
		.set_local_name(_("ValueDesc on Spline to link to"))
	);
	ret.push_back(ParamDesc("time",Param::TYPE_TIME)
		.set_local_name(_("Time"))
		.set_optional()
	);
	ret.push_back(ParamDesc("origin",Param::TYPE_REAL)
		.set_local_name(_("Origin"))
		.set_optional()
	);

	return ret;
}

bool
Action::ValueDescBLineLink::is_candidate(const ParamList &x)
{
	if (!candidate_check(get_param_vocab(),x))
		return false;

	ValueDesc value_desc(x.find("value_desc")->second.get_value_desc());

	if (!value_desc.parent_is_value_node())
		return false;

	// We need a dynamic list.
	ValueNode::Handle value_desc_valuenode = value_desc.get_parent_value_node();
	if (!ValueNode_DynamicList::Handle::cast_dynamic(value_desc_valuenode))
		return false;

	// if any of the selected valuedesc belongs to the spline, can't link.
	const auto range = x.equal_range("selected_value_desc");
	for (auto it = range.first; it != range.second; ++it) {
		const auto& selected_value_desc = it->second.get_value_desc();
		if (selected_value_desc.parent_is_value_node() && value_desc_valuenode == selected_value_desc.get_parent_value_node())
			return false;
	}

	return true;
}

bool
Action::ValueDescBLineLink::set_param(const synfig::String& name, const Action::Param &param)
{
	if (name == "time" && param.get_type() == Param::TYPE_TIME)
	{
		time = param.get_time();
		return true;
	}

	if (name == "value_desc" && param.get_type() == Param::TYPE_VALUEDESC)
	{
		value_desc = param.get_value_desc();
		index = value_desc.get_index();
		return true;
	}

	if (name == "selected_value_desc" && param.get_type() == Param::TYPE_VALUEDESC)
	{
		ValueDesc value_desc(param.get_value_desc());
		value_desc_list.push_back(value_desc);
		return true;
	}

	if (name == "origin" && param.get_type() == Param::TYPE_REAL)
	{
		origin = param.get_real();
		return true;
	}

	return Action::CanvasSpecific::set_param(name,param);
}

bool
Action::ValueDescBLineLink::is_ready()const
{
	if (value_desc_list.size()<1)
		return false;
	if (!value_desc)
		return false;
	return Action::CanvasSpecific::is_ready();
}

void
Action::ValueDescBLineLink::prepare()
{
	if (value_desc_list.empty())
		throw Error(Error::TYPE_NOTREADY);

	clear();

	ValueNode_DynamicList::Handle bline_value_node(ValueNode_DynamicList::Handle::cast_dynamic(value_desc.get_parent_value_node()));
	bool loop(bline_value_node->get_loop());
	int loop_adjust(loop ? 0 : -1);
	const std::vector<ValueBase> bline((*bline_value_node)(time).get_list());
	int size = bline.size();
	Real amount = (index - 1 + origin) / (size + loop_adjust);
	// This is the standard amount, let's calculate the homogeneous amount
	// since by default, homogeneous is 'on' for new BLineLink
	// Note: if bline is looped, then consider the loop option of
	// BLineLink looped too.
	amount=std_to_hom(ValueBase(bline), amount, loop, loop);
	LinkableValueNode::Handle calculated_value_node;
	Action::Handle action;

	ValueNode::Handle loop_value_node(ValueNode_Const::create(loop));
	ValueNode::Handle amount_value_node(ValueNode_Const::create(amount));
	ValueNode::Handle homogeneous_value_node(ValueNode_Const::create(true));

	for (std::list<ValueDesc>::iterator iter = value_desc_list.begin(); iter != value_desc_list.end(); ++iter)
	{
		ValueDesc& value_desc(*iter);

		// parent is BLINEPOINT ValueNode
		if (value_desc.parent_is_linkable_value_node() &&
			value_desc.get_parent_value_node()->get_type() == type_bline_point &&
			ValueNode_Composite::Handle::cast_dynamic(value_desc.get_parent_value_node()))
		{
			ValueNode_Composite::Handle composite = ValueNode_Composite::Handle::cast_dynamic(value_desc.get_parent_value_node());
			String link_name(composite->link_name(value_desc.get_index()));

			if (link_name == "t1" || link_name == "t2")
			{
				action = ValueDescSet::create();
				action->set_param("canvas",get_canvas());
				action->set_param("canvas_interface",get_canvas_interface());
				action->set_param("value_desc",ValueDesc(composite,composite->get_link_index_from_name("split_radius")));
				action->set_param("time",time);
				action->set_param("new_value",synfig::ValueBase(true));
				assert(action->is_ready());
				if(!action->is_ready()) throw Error(Error::TYPE_NOTREADY);
				add_action(action);

				action = ValueDescSet::create();
				action->set_param("canvas",get_canvas());
				action->set_param("canvas_interface",get_canvas_interface());
				action->set_param("value_desc",ValueDesc(composite,composite->get_link_index_from_name("split_angle")));
				action->set_param("time",time);
				action->set_param("new_value",synfig::ValueBase(true));
				assert(action->is_ready());
				if(!action->is_ready()) throw Error(Error::TYPE_NOTREADY);
				add_action(action);

				calculated_value_node = ValueNode_BLineCalcTangent::create(type_vector);
			}
			else
			if (link_name == "width")
				calculated_value_node = ValueNode_BLineCalcWidth::create(type_real);
			else
			if (link_name == "point")
				calculated_value_node = ValueNode_BLineCalcVertex::create(type_vector);
			else
			{
				synfig::warning("can't link '%s'", link_name.c_str());
				continue;
			}

			action = ValueNodeLinkConnect::create();
			action->set_param("parent_value_node", value_desc.get_parent_value_node());
			action->set_param("index", value_desc.get_index());
		}
		// BLINEPOINT ValueNode - link its vertex
		else if (value_desc.is_value_node() &&
				 value_desc.get_value_type() == type_bline_point &&
				 ValueNode_Composite::Handle::cast_dynamic(value_desc.get_value_node()))
		{
			ValueNode_Composite::Handle composite = ValueNode_Composite::Handle::cast_dynamic(value_desc.get_value_node());
			String link_name(value_desc.get_sub_name());
			int index = composite->get_link_index_from_name(link_name);
			calculated_value_node.reset();

			if (link_name == "t1" || link_name == "t2")
			{
				action = ValueDescSet::create();
				action->set_param("canvas",get_canvas());
				action->set_param("canvas_interface",get_canvas_interface());
				action->set_param("value_desc",ValueDesc(composite,composite->get_link_index_from_name("split_radius")));
				action->set_param("time",time);
				action->set_param("new_value",synfig::ValueBase(true));
				assert(action->is_ready());
				if(!action->is_ready()) throw Error(Error::TYPE_NOTREADY);
				add_action(action);

				action = ValueDescSet::create();
				action->set_param("canvas",get_canvas());
				action->set_param("canvas_interface",get_canvas_interface());
				action->set_param("value_desc",ValueDesc(composite,composite->get_link_index_from_name("split_angle")));
				action->set_param("time",time);
				action->set_param("new_value",synfig::ValueBase(true));
				assert(action->is_ready());
				if(!action->is_ready()) throw Error(Error::TYPE_NOTREADY);
				add_action(action);

				calculated_value_node = ValueNode_BLineCalcTangent::create(type_vector);
			}
			else
			if (link_name == "width")
				calculated_value_node = ValueNode_BLineCalcWidth::create(type_real);
			else
			if (link_name == "point")
				calculated_value_node = ValueNode_BLineCalcVertex::create(type_vector);

			if (index < 0 || !calculated_value_node)
			{
				synfig::warning("can't link '%s'", link_name.c_str());
				continue;
			}

			action = ValueNodeLinkConnect::create();
			action->set_param("parent_value_node", value_desc.get_value_node());
			action->set_param("index", index);
		}
		// exported ValueNode
		else if (value_desc.parent_is_canvas())
		{
			if (value_desc.get_value_type() == type_vector)
				calculated_value_node = ValueNode_BLineCalcVertex::create(type_vector);
			else if (value_desc.get_value_type() == type_real)
				calculated_value_node = ValueNode_BLineCalcWidth::create(type_real);
			else
				continue;

			calculated_value_node->set_link("bline",  bline_value_node);
			calculated_value_node->set_link("loop",   ValueNode_Const::create(loop));
			calculated_value_node->set_link("amount", ValueNode_Const::create(amount));
			calculated_value_node->set_link("homogeneous", ValueNode_Const::create(true));

			action = ValueNodeReplace::create();
			action->set_param("canvas", get_canvas());
			action->set_param("canvas_interface", get_canvas_interface());
			action->set_param("src", ValueNode::Handle(calculated_value_node));
			action->set_param("dest", value_desc.get_value_node());

			assert(action->is_ready());
			if (!action->is_ready()) throw Error(Error::TYPE_NOTREADY);
			add_action_front(action);

			continue;
		}
		else if (value_desc.parent_is_layer())
		{
			// VECTOR layer parameter
			if (value_desc.get_value_type() == type_vector)
				calculated_value_node = ValueNode_BLineCalcVertex::create(type_vector);
			// REAL layer parameter
			else if (value_desc.get_value_type() == type_real)
				calculated_value_node = ValueNode_BLineCalcWidth::create(type_real);
			// ANGLE layer parameter
			else if (value_desc.get_value_type() == type_angle)
				calculated_value_node = ValueNode_BLineCalcTangent::create(type_angle);
			// TRANSFORMATION layer parameter
			else if (value_desc.get_value_type() == type_transformation)
			{
				LinkableValueNode::Handle composite_node = ValueNode_Composite::create(value_desc.get_value(time), get_canvas());
				LinkableValueNode::Handle offset_node = ValueNode_BLineCalcVertex::create(type_vector);
				LinkableValueNode::Handle angle_node = ValueNode_BLineCalcTangent::create(type_angle);
				composite_node->set_link("offset", offset_node);
				composite_node->set_link("angle", angle_node);

				offset_node->set_link("bline",  bline_value_node );
				offset_node->set_link("loop",   loop_value_node  );
				offset_node->set_link("amount", amount_value_node);
				offset_node->set_link("homogeneous", homogeneous_value_node);

				angle_node->set_link("bline",  bline_value_node );
				angle_node->set_link("loop",   loop_value_node  );
				angle_node->set_link("amount", amount_value_node);
				angle_node->set_link("homogeneous", homogeneous_value_node);

				action = LayerParamConnect::create();
				action->set_param("layer", value_desc.get_layer());
				action->set_param("param", value_desc.get_param_name());
				action->set_param("canvas", get_canvas());
				action->set_param("canvas_interface", get_canvas_interface());
				action->set_param("value_node", ValueNode::Handle(composite_node));

				assert(action->is_ready());
				if (!action->is_ready()) throw Error(Error::TYPE_NOTREADY);
				add_action_front(action);

				continue;
			}
			else
				continue;

			action = LayerParamConnect::create();
			action->set_param("layer", value_desc.get_layer());
			action->set_param("param", value_desc.get_param_name());
		}
		else
			continue;

		calculated_value_node->set_link("bline",  bline_value_node );
		calculated_value_node->set_link("loop",   loop_value_node  );
		calculated_value_node->set_link("amount", amount_value_node);
		calculated_value_node->set_link("homogeneous", homogeneous_value_node);

		action->set_param("canvas", get_canvas());
		action->set_param("canvas_interface", get_canvas_interface());
		action->set_param("value_node", ValueNode::Handle(calculated_value_node));

		assert(action->is_ready());
		if (!action->is_ready()) throw Error(Error::TYPE_NOTREADY);
		add_action(action);
	}
}
