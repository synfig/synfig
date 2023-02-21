/* === S Y N F I G ========================================================= */
/*!	\file synfig_iterations.h
**	\brief Header for Layer and ValueNode iteration helpers
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

#ifndef SYNFIG_ITERATIONS_H
#define SYNFIG_ITERATIONS_H

#include <set>
#include <vector>

#include <synfig/layer.h>
#include <synfig/valuenode.h>

namespace synfig
{

struct TraverseLayerSettings
{
	/// Should traverse into static layer parameter values of canvas type and they are not inline
	bool traverse_static_non_inline_canvas = false;
	/// Should traverse into layer parameter valuenodes of canvas type and they are inline
	bool traverse_dynamic_inline_canvas = true;
	/// Should traverse into layer parameter valuenodes of canvas type and they are not inline
	bool traverse_dynamic_non_inline_canvas = false;
};

struct TraverseLayerStatus
{
	TraverseLayerSettings settings;
	// - STATUS -
	/// Tracks the index of each recursive iteration. The last element is the current level. Its size is, then, the real depth
	std::vector<int> depth = {-1};
	/// Is inside a canvas of a layer dynamic parameter?
	bool is_dynamic_canvas = false;
	/// List of visited layers (avoid infinite loop)
	std::set<synfig::Layer::LooseHandle> visited_layers;
};

/// Used in traverse_layers()
typedef std::function<void(Layer::LooseHandle, const TraverseLayerStatus&)> TraverseLayerCallback;

/// Search for layers listed/pointed by another (via Canvas-type parameters)
/// Normally used for Layer_PasteCanvas, but another ones may have canvases
///
/// \param layer The starting point to scanning
/// \param callback A functor called at each layer found
void traverse_layers(Layer::Handle layer, TraverseLayerCallback callback, TraverseLayerSettings settings = TraverseLayerSettings());

/// Return of TraverseCallback
/// TRAVERSE_CALLBACK_SKIP means it should not enter in valuenode recursively (if linkable valuenode)
/// TRAVERSE_CALLBACK_RECURSIVE means the callback will be called for current valuenode as well as for each of its links
/// TRAVERSE_CALLBACK_ABORT means to stop any other for-each iteration (e.g. exiting from the traverse_valuenodes() call)
enum TraverseCallbackAction {
	TRAVERSE_CALLBACK_SKIP,
	TRAVERSE_CALLBACK_RECURSIVE,
	TRAVERSE_CALLBACK_ABORT,
};

/// Walks into valuenode tree performing valuenode_callback() on each valuenode
/// If value_node is LinkableValueNode, runs on each of its links too, according to return of the valuenode_callback (see TraverseCallbackAction)
///
/// \param value_node The starting point to scannning
/// \param valuenode_callback A functor called at each valuenode found
void traverse_valuenodes(ValueNode::Handle value_node, std::function<TraverseCallbackAction(ValueNode::Handle)> valuenode_callback);

/// Useful for parameter fetch_replacement_for of replace_value_nodes()
/// When you have a simple std::map that maps source-value-node to replacement-value-node
/// instead of a function to map this relationship.
struct SimpleValueNodeReplaceFunctor {
	SimpleValueNodeReplaceFunctor(const std::map<ValueNode::LooseHandle, ValueNode::LooseHandle>& replacer_map);
	ValueNode::LooseHandle operator()(const ValueNode::LooseHandle&);
private:
	const std::map<ValueNode::LooseHandle, ValueNode::LooseHandle>& replacer_map;
};

/// Scans for value nodes in value_node to replace them according to fetch_replacement_for functor.
/// It is useful, for example, for fixing links after cloning value nodes.
/// Concrete example: skeleton-type layers need it after cloning.
/// \param value_node starting point to scan
/// \param fetch_replacement_for functor to return the value-node replacement for a given value node
void
replace_value_nodes(ValueNode::LooseHandle value_node, std::function<ValueNode::LooseHandle(const ValueNode::LooseHandle&)> fetch_replacement_for);

/// Scans for value nodes in layer parameters to replace them according to fetch_replacement_for functor.
/// It is useful, for example, for fixing links after cloning a layer.
/// \param layer Where to look for replaceable value nodes
/// \param fetch_replacement_for functor to return the value-node replacement for a given value node
void
replace_value_nodes(Layer::LooseHandle layer, std::function<ValueNode::LooseHandle(const ValueNode::LooseHandle&)> fetch_replacement_for);

}; // namespace synfig

#endif // SYNFIG_ITERATIONS_H
