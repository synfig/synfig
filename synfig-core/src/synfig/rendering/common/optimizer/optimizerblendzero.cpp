/* === S Y N F I G ========================================================= */
/*!	\file synfig/rendering/common/optimizer/optimizerblendzero.cpp
**	\brief OptimizerBlendZero
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

#include <synfig/general.h>
#include <synfig/localization.h>

#include "optimizerblendzero.h"

#include "../task/taskblend.h"
#include "../task/taskcomposite.h"
#include "../task/tasksurfaceempty.h"

#endif

using namespace synfig;
using namespace rendering;

/* === M A C R O S ========================================================= */

/* === G L O B A L S ======================================================= */

/* === P R O C E D U R E S ================================================= */

/* === M E T H O D S ======================================================= */

void
OptimizerBlendZero::run(const RunParams& params) const
{
	TaskBlend::Handle blend = TaskBlend::Handle::cast_dynamic(params.ref_task);
	if ( blend
	  && blend->target_surface
	  && blend->sub_task_a()
	  && blend->sub_task_a()->target_surface )
	{
		// remove blend if amount is zero
		if (fabsf(blend->amount) <= 1e-6)
		{
			apply(params, blend->sub_task_a());
			if (blend->offset_a[0] || blend->offset_a[1])
			{
				params.ref_task = params.ref_task->clone();
				params.ref_task->target_surface = blend->target_surface;
				params.ref_task->target_rect += VectorInt(blend->target_rect.minx, blend->target_rect.miny) + blend->offset_a;
				apply_target_bounds(*params.ref_task, RectInt(0, 0, blend->target_surface->get_width(), blend->target_surface->get_height()));
			}
			return;
		}
	}
}

/* === E N T R Y P O I N T ================================================= */
