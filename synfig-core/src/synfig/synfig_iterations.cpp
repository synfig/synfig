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
#include <synfig/general.h>
#include <synfig/context.h>

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
