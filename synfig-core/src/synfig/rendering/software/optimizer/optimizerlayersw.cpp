/* === S Y N F I G ========================================================= */
/*!	\file synfig/rendering/software/optimizer/optimizerlayersw.cpp
**	\brief OptimizerLayerSW
**
**	$Id$
**
**	\legal
**	......... ... 2015 Ivan Mahonin
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

#ifndef _WIN32
#include <unistd.h>
#include <sys/types.h>
#include <signal.h>
#endif

#include <algorithm>

#include "optimizerlayersw.h"

#include "../surfacesw.h"
#include "../task/tasklayersw.h"

#include <synfig/renddesc.h>

#endif

using namespace synfig;
using namespace rendering;

/* === M A C R O S ========================================================= */

/* === G L O B A L S ======================================================= */

/* === P R O C E D U R E S ================================================= */

/* === M E T H O D S ======================================================= */

bool
OptimizerLayerSW::renddesc_less(const RendDesc &a, const RendDesc &b)
{
	return fabs(a.get_pw()*a.get_ph()) > fabs(b.get_pw()*b.get_ph());
}

void
OptimizerLayerSW::run(const RunParams& params) const
{
	TaskLayer::Handle layer = TaskLayer::Handle::cast_dynamic(params.ref_task);
	if ( layer
	  && layer->target_surface
	  && layer.type_equal<TaskLayer>() )
	{
		if (!layer->layer) { apply(params, Task::Handle()); return; }

		TaskLayerSW::Handle layer_sw = create_and_assign<TaskLayerSW>(layer);
		layer_sw->sub_tasks.clear();

		if (layer->sub_task())
		{
			VectorInt size = layer_sw->get_target_rect().get_size();

			RendDesc desc;
			desc.set_wh(size[0], size[1]);
			desc.set_tl(layer_sw->get_source_rect_lt());
			desc.set_br(layer_sw->get_source_rect_rb());

			std::vector<RendDesc> descs;
			layer_sw->layer->get_sub_renddesc(desc, descs);
			sort(descs.begin(), descs.end(), renddesc_less);

			for(std::vector<RendDesc>::const_iterator i = descs.begin(); i != descs.end(); ++i)
			{
				Task::Handle task = layer->sub_task()->clone();
				assign_surface<SurfaceSW>(task, i->get_w(), i->get_h(), i->get_tl(), i->get_br(), RectInt(0, 0, i->get_w(), i->get_h()));
				layer_sw->sub_tasks.push_back(task);
			}
		}

		apply(params, layer_sw);
	}
}

/* === E N T R Y P O I N T ================================================= */
