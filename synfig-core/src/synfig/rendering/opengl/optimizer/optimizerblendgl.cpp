/* === S Y N F I G ========================================================= */
/*!	\file synfig/rendering/opengl/optimizer/optimizerblendgl.cpp
**	\brief OptimizerBlendGL
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

#include "optimizerblendgl.h"

#include "../task/taskblendgl.h"
#include "../surfacegl.h"

#endif

using namespace synfig;
using namespace rendering;

/* === M A C R O S ========================================================= */

/* === G L O B A L S ======================================================= */

/* === P R O C E D U R E S ================================================= */

/* === M E T H O D S ======================================================= */

void
OptimizerBlendGL::run(const RunParams& params) const
{
	TaskBlend::Handle blend = TaskBlend::Handle::cast_dynamic(params.ref_task);
	if ( blend
	  && blend->target_surface
	  && blend.type_equal<TaskBlend>() )
	{
		TaskBlendGL::Handle blend_gl;
		init_and_assign_all<SurfaceGL>(blend_gl, blend);

		if ( blend_gl->target_surface->is_temporary
		  && blend_gl->sub_task_a()->target_surface )
			blend_gl->target_surface = blend_gl->sub_task_a()->target_surface;

		apply(params, blend_gl);
	}
}

/* === E N T R Y P O I N T ================================================= */
