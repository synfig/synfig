/* === S Y N F I G ========================================================= */
/*!	\file synfig_iterations.cpp
**	\brief Layer and ValueNode iteration helpers
**
**	\legal
**	Copyright (c) 2002-2005 Robert B. Quattlebaum Jr., Adrian Bentley
**	Copyright (c) 2021      Rodolfo Ribeiro Gomes
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

#include "synfig_iterations.h"

#include <synfig/canvas.h>
#include <synfig/context.h>
#include <synfig/general.h>
#include <synfig/localization.h>
#include <synfig/valuenodes/valuenode_animated.h>
#include <synfig/valuenodes/valuenode_bone.h>
#include <synfig/valuenodes/valuenode_const.h>

#endif

using namespace synfig;

/* === P R O C E D U R E S ================================================= */

static void
do_traverse_layers(Layer::Handle layer, TraverseLayerStatus& status, TraverseLayerCallback& callback)
{
	if (!layer) {
		synfig::warning("%s:%d null layer?!\n", __FILE__, __LINE__);
		assert(false);
		return;
	}

	if (status.visited_layers.count(layer))
		return;
	status.visited_layers.insert(layer);

	++status.depth.back();
	callback(layer, status);

	Layer::ParamList param_list(layer->get_param_list());
	for (Layer::ParamList::const_iterator iter(param_list.begin())
			 ; iter != param_list.end()
			 ; ++iter)
	{
		if (layer->dynamic_param_list().count(iter->first)==0 && iter->second.get_type()==type_canvas)
		{
			bool previous_is_dynamic = status.is_dynamic_canvas;
			/*status.is_dynamic_canvas = false;*/
			status.depth.push_back(-1);
			Canvas::Handle subcanvas(iter->second.get(Canvas::Handle()));
			if (subcanvas && subcanvas->is_inline())
				for (IndependentContext iter = subcanvas->get_independent_context(); iter != subcanvas->end(); iter++)
					do_traverse_layers(*iter, status, callback);
			status.depth.pop_back();
			status.is_dynamic_canvas = previous_is_dynamic;
		}
	}

	if (!status.traverse_dynamic_inline_canvas && !status.traverse_dynamic_non_inline_canvas)
		return;

	for (Layer::DynamicParamList::const_iterator iter(layer->dynamic_param_list().begin())
			 ; iter != layer->dynamic_param_list().end()
			 ; ++iter)
	{
		if (iter->second->get_type()==type_canvas)
		{
			std::set<ValueBase> values;
			iter->second->get_values(values);
			for (const ValueBase& value : values)
			{
				bool previous_is_dynamic = status.is_dynamic_canvas;
				status.is_dynamic_canvas = true;
				status.depth.push_back(-1);

				Canvas::Handle subcanvas(value.get(Canvas::Handle()));
				if (subcanvas && subcanvas->is_inline()) {
					if (status.traverse_dynamic_inline_canvas)
						for (IndependentContext iter = subcanvas->get_independent_context(); iter != subcanvas->end(); ++iter)
							do_traverse_layers(*iter, status, callback);
				} else {
					//! \todo do we need to implement this?
					if (status.traverse_dynamic_non_inline_canvas)
						warning("%s:%d not yet implemented - do we need to traverse non-inline canvases in layer dynamic parameters?", __FILE__, __LINE__);
				}

				status.depth.pop_back();
				status.is_dynamic_canvas = previous_is_dynamic;
			}
		}
	}
}

void
synfig::traverse_layers(Layer::Handle layer, TraverseLayerCallback callback)
{
	TraverseLayerStatus status;
	do_traverse_layers(layer, status, callback);
}


struct ReplaceValueNodeStatus {
	std::set<ValueNode::LooseHandle> visited_value_nodes;
};

static void
do_replace_value_nodes(ValueNode::LooseHandle value_node, ReplaceValueNodeStatus& status, std::function<ValueNode::LooseHandle(ValueNode::LooseHandle)> fetch_replacement_for)
{
	if (!value_node) {
		synfig::warning("%s:%d null valuenode?!\n", __FILE__, __LINE__);
		assert(false);
		return;
	}

	// avoid loops
	if (status.visited_value_nodes.count(value_node))
		return;
	status.visited_value_nodes.insert(value_node);

	// search for replaceable value nodes

	if (auto linkable_vn = LinkableValueNode::Handle::cast_dynamic(value_node)) {
		for (int i=0; i < linkable_vn->link_count(); i++) {
			auto ith_link = linkable_vn->get_link(i);
			if (auto replacement = fetch_replacement_for(ith_link)) {
				linkable_vn->set_link(i, replacement);
			} else {
				replace_value_nodes(ith_link, fetch_replacement_for);
			}
		}
	} else if (auto const_vn = ValueNode_Const::Handle::cast_dynamic(value_node)) {
		if (const_vn->get_type() == type_bone_valuenode) {
			ValueNode_Bone::Handle bone_vn = const_vn->get_value().get(ValueNode_Bone::Handle());
			if (auto replacement = fetch_replacement_for(bone_vn.get())) {
				ValueBase vb(ValueNode_Bone::Handle::cast_dynamic(replacement));
				vb.copy_properties_of(bone_vn);
				const_vn->set_value(vb);
			} else {
				replace_value_nodes(bone_vn.get(), fetch_replacement_for);
			}
		}
	} else if (auto animated_vn = ValueNode_Animated::Handle::cast_dynamic(value_node)) {
		ValueNode_Animated::WaypointList& list(animated_vn->editable_waypoint_list());
		for (ValueNode_Animated::WaypointList::iterator iter = list.begin(); iter != list.end(); ++iter) {
			ValueNode::Handle vn = iter->get_value_node();
			if (auto replacement = fetch_replacement_for(vn))
				iter->set_value_node(replacement);
			else
				replace_value_nodes(vn, fetch_replacement_for);
		}
	} else {
		// actually there is a known case: PlaceholderValueNode
		// but maybe user has custom valuenode modules...
		synfig::warning(_("Unknown value node type (%s) to search/replace into it. Ignoring it."), value_node->get_local_name().c_str());
	}
}

void
synfig::replace_value_nodes(ValueNode::LooseHandle value_node, std::function<ValueNode::LooseHandle(const ValueNode::LooseHandle&)> fetch_replacement_for)
{
	ReplaceValueNodeStatus status;
	do_replace_value_nodes(value_node, status, fetch_replacement_for);
}

void
synfig::replace_value_nodes(Layer::LooseHandle layer, std::function<ValueNode::LooseHandle(const ValueNode::LooseHandle&)> fetch_replacement_for)
{
	auto replace_value_nodes_from_layer = [fetch_replacement_for](Layer::LooseHandle layer, const TraverseLayerStatus& /*status*/) {
		const auto dyn_param_list = layer->dynamic_param_list();
		for (auto dyn_param : dyn_param_list) {
			if (auto new_vn = fetch_replacement_for(dyn_param.second)) {
				layer->disconnect_dynamic_param(dyn_param.first);
				layer->connect_dynamic_param(dyn_param.first, new_vn);
			} else {
				replace_value_nodes(dyn_param.second, fetch_replacement_for);
			}
		}
	};
	traverse_layers(layer, replace_value_nodes_from_layer);
}

SimpleValueNodeReplaceFunctor::SimpleValueNodeReplaceFunctor(const std::map<ValueNode::LooseHandle, ValueNode::LooseHandle> &replacer_map)
	: replacer_map(replacer_map)
{
}

ValueNode::LooseHandle
SimpleValueNodeReplaceFunctor::operator()(const ValueNode::LooseHandle& vn)
{
	auto found = replacer_map.find(vn);
	if (found == replacer_map.end()) {
		return nullptr;
	}
	return found->second;
}
