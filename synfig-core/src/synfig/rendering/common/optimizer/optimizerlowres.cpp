/* === S Y N F I G ========================================================= */
/*!	\file synfig/rendering/common/optimizer/optimizerlowres.cpp
**	\brief OptimizerLowRes
**
**	$Id$
**
**	\legal
**	......... ... 2017 Ivan Mahonin
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

#include "optimizerlowres.h"

#include "../task/tasksurfaceresample.h"

#endif

using namespace synfig;
using namespace rendering;

/* === M A C R O S ========================================================= */

/* === G L O B A L S ======================================================= */

/* === P R O C E D U R E S ================================================= */

/* === M E T H O D S ======================================================= */

void
OptimizerLowRes::run(const RunParams& params) const
{
	if (params.ref_task && !params.parent)
	{
		Task::Handle sub_task = params.ref_task->clone();

		TaskSurfaceResample::Handle resample = new TaskSurfaceResample();
		assign(resample, sub_task);
		resample->init_target_rect(
			sub_task->get_target_rect(),
			sub_task->get_source_rect_lt(),
			sub_task->get_source_rect_rb() );
		resample->supersample = Vector(1.0/scale, 1.0/scale);
		resample->interpolation = Color::INTERPOLATION_NEAREST;
		resample->sub_task() = sub_task;

		sub_task->target_surface.reset();

		apply(params, resample);
	}
}

/* === E N T R Y P O I N T ================================================= */
