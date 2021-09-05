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

namespace synfig
{

struct TraverseLayerStatus
{
	// - SETTINGS -
	/// Should traverse into layer paramenter valuenodes of canvas type and they are inline
	bool traverse_dynamic_inline_canvas = true;
	/// Should traverse into layer paramenter valuenodes of canvas type and they are not inline
	bool traverse_dynamic_non_inline_canvas = false;
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
/// \param layer The starting point to scannning
/// \param callback A functor called at each layer found
void traverse_layers(Layer::Handle layer, TraverseLayerCallback callback);

}; // namespace synfig

#endif // SYNFIG_ITERATIONS_H
