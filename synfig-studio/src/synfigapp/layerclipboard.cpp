// SPDX-License-Identifier: GPL-2.0-or-later
#include "layerclipboard.h"
#include <synfig/synfig_iterations.h>
#include <synfig/valuenodes/valuenode_animated.h>
#include <synfig/valuenodes/valuenode_bone.h>
#include <synfig/valuenodes/valuenode_const.h>
#include <functional>
#include <map>
#include <set>

using namespace synfig;

std::list<Layer::Handle>
synfigapp::clone_layers_for_clipboard(const std::list<Layer::Handle>& layers,
	Canvas::LooseHandle canvas, const GUID& guid)
{
	std::list<Layer::Handle> clones;
	for (const auto& layer : layers)
		clones.push_back(layer->clone(canvas, guid));
	relink_cloned_bones(layers, guid);
	return clones;
}

void
synfigapp::relink_cloned_bones(const std::list<Layer::Handle>& layers, const GUID& guid)
{
	std::map<ValueNode::LooseHandle, ValueNode_Bone::Handle> bone_clones;
	std::set<ValueNode::Handle> owned_nodes;
	// Layer::clone gives each copied node a GUID derived from its source.
	// Collect only nodes actually cloned by this operation. Exported nodes and
	// external bone references remain shared and must never be edited in place.
	for (const auto& layer : layers)
		traverse_layers(layer, [&](Layer::LooseHandle nested, const TraverseLayerStatus&) {
			for (const auto& param : nested->dynamic_param_list())
				traverse_valuenodes(param.second, [&](ValueNode::Handle source) {
					ValueNode::Handle clone = find_value_node(source->get_guid() ^ guid);
					if (clone) {
						owned_nodes.insert(clone);
						if (ValueNode_Bone::Handle::cast_dynamic(source)) {
							auto bone = ValueNode_Bone::Handle::cast_dynamic(clone);
							if (bone)
								bone_clones[source.get()] = bone;
						}
					}
					return TRAVERSE_CALLBACK_RECURSIVE;
				});
		});

	std::set<ValueNode::Handle> visited;
	std::function<void(ValueNode::Handle)> repair = [&](ValueNode::Handle node) {
		if (!node || !owned_nodes.count(node) || !visited.insert(node).second)
			return;
		if (auto linkable = LinkableValueNode::Handle::cast_dynamic(node)) {
			for (int i = 0; i < linkable->link_count(); ++i)
				repair(linkable->get_link(i));
		} else if (auto constant = ValueNode_Const::Handle::cast_dynamic(node)) {
			if (constant->get_type() == type_bone_valuenode) {
				auto bone = constant->get_value().get(ValueNode_Bone::Handle());
				auto replacement = bone_clones.find(bone.get());
				if (replacement != bone_clones.end()) {
					ValueBase value(replacement->second);
					value.copy_properties_of(constant->get_value());
					constant->set_value(value);
				}
				// A bone held as a value is a reference, not an owned subtree.
			}
		} else if (auto animated = ValueNode_Animated::Handle::cast_dynamic(node)) {
			for (auto& waypoint : animated->editable_waypoint_list())
				repair(waypoint.get_value_node());
		}
	};
	for (const auto& node : owned_nodes)
		repair(node);
}
