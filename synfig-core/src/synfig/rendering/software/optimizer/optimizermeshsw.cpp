/* === S Y N F I G ========================================================= */
/*!	\file synfig/rendering/software/optimizer/optimizermeshsw.cpp
**	\brief OptimizerMeshSW
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

#ifndef WIN32
#include <unistd.h>
#include <sys/types.h>
#include <signal.h>
#endif

#include "optimizermeshsw.h"

#include "../surfacesw.h"
#include "../../common/task/taskmesh.h"
#include "../task/taskmeshsw.h"

#endif

using namespace synfig;
using namespace rendering;

/* === M A C R O S ========================================================= */

/* === G L O B A L S ======================================================= */

/* === P R O C E D U R E S ================================================= */

/* === M E T H O D S ======================================================= */

void
OptimizerMeshSW::run(const RunParams& params) const
{
	TaskMesh::Handle mesh = TaskMesh::Handle::cast_dynamic(params.ref_task);
	if ( mesh
	  && mesh->target_surface
	  && mesh->sub_task()
	  && mesh->mesh )
	{
		TaskMeshSW::Handle mesh_sw(new TaskMeshSW());
		*((Task*)(mesh_sw.get())) = *((Task*)(mesh.get()));
		mesh_sw->mesh = mesh->mesh;

		if ( !mesh_sw->sub_task()->target_surface )
		{
			Vector resolution = mesh->mesh->get_resolution_transfrom()
				.get_transformed(
					Vector(
						(Real)mesh->target_surface->get_width(),
						(Real)mesh->target_surface->get_height() ));
			int width, height;
			if (resolution.is_valid())
			{
				width = std::max(1, (int)round(fabs(resolution[0])));
				height = std::max(1, (int)round(fabs(resolution[0])));
			}
			else
			{
				width = 1;
				height = 1;
			}

			mesh_sw->sub_task() = mesh->sub_task()->clone();
			mesh_sw->sub_task()->target_surface = new SurfaceSW();
			mesh_sw->sub_task()->target_surface->set_size(width, height);
			mesh_sw->sub_task()->init_target_rect(
				RectInt(0, 0, width, height),
				mesh->mesh->get_source_rectangle().get_min(),
				mesh->mesh->get_source_rectangle().get_max() );
			assert( mesh_sw->check() );
			mesh_sw->sub_task()->trunc_target_by_bounds();
		}
		apply(params, mesh_sw);
	}
}

/* === E N T R Y P O I N T ================================================= */
