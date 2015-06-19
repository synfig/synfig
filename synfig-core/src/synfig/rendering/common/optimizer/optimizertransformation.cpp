/* === S Y N F I G ========================================================= */
/*!	\file synfig/rendering/common/optimizer/optimizertransformation.cpp
**	\brief OptimizerTransformation
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

#include "optimizertransformation.h"

#include "../task/tasktransformation.h"
#include "../task/taskmesh.h"

#endif

using namespace synfig;
using namespace rendering;

/* === M A C R O S ========================================================= */

/* === G L O B A L S ======================================================= */

/* === P R O C E D U R E S ================================================= */

/* === M E T H O D S ======================================================= */

bool
OptimizerTransformation::run(const RunParams& params) const
{
	// TODO: Optimize affine transformation
	// TODO: Optimize transformation to transformation
	if (TaskTransformation::Handle transformation = TaskTransformation::Handle::cast_dynamic(params.task))
	{
		if ( transformation->target_surface
		  && !transformation->target_surface->empty() )
		{
			// TODO: custom parameter
			Real precision_pixels = 5.0;

			TaskMesh::Handle mesh(new TaskMesh());
			*((Task*)(mesh)) = *((Task*)(transformation));
			mesh->mesh = transformation->transformation->build_mesh(
				transformation->rect_lt,
				transformation->rect_rb,
				transformation->get_utits_per_pixel() * precision_pixels );

			params.out_task = mesh;
			return true;
		}
	}
	return false;
}

/* === E N T R Y P O I N T ================================================= */
