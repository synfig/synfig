/* === S Y N F I G ========================================================= */
/*!	\file synfig/rendering/opengl/optimizer/optimizersurfaceresamplesw.cpp
**	\brief OptimizerSurfaceResampleSW
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

#include "optimizersurfaceresamplesw.h"

#include "../task/tasksurfaceresamplesw.h"
#include "../surfacesw.h"

#endif

using namespace synfig;
using namespace rendering;

/* === M A C R O S ========================================================= */

/* === G L O B A L S ======================================================= */

/* === P R O C E D U R E S ================================================= */

/* === M E T H O D S ======================================================= */

void
OptimizerSurfaceResampleSW::run(const RunParams& params) const
{
	TaskSurfaceResample::Handle resample = TaskSurfaceResample::Handle::cast_dynamic(params.ref_task);
	if ( resample
	  && resample->target_surface
	  && resample.type_equal<TaskSurfaceResample>() )
	{
		TaskSurfaceResampleSW::Handle resample_sw;
		init_and_assign_all<SurfaceSW>(resample_sw, resample);

		// init target of sub-task
		if (resample->valid_target_rect()
		 && resample->sub_task()
		 && !resample->sub_task()->target_surface )
		{
			const Real precision = 1e-10;
			const int border_size = 4;

			RectInt target = resample->get_target_rect();
			Vector src_lt = resample->get_source_rect_lt();
			Vector src_rb = resample->get_source_rect_rb();
			Matrix matrix = resample->transformation;
			Matrix inv_matrix = matrix;
			inv_matrix.invert();

			// calculate source-rect of sub-task
			Vector corners[] = {
					inv_matrix.get_transformed( src_lt ),
					inv_matrix.get_transformed( Vector(src_rb[0], src_lt[1]) ),
					inv_matrix.get_transformed( Vector(src_lt[0], src_rb[1]) ),
					inv_matrix.get_transformed( src_rb ) };

			Rect sub_src = Rect( corners[0] )
					    .expand( corners[1] )
					    .expand( corners[2] )
					    .expand( corners[3] );

			// calculate size of surface of sub-task
			Real lx = (corners[1] - corners[0]).mag();
			Real ly = (corners[2] - corners[0]).mag();
			if (lx > precision && ly > precision)
			{
				int sub_w = (int)ceil((target.maxx - target.minx)*(sub_src.maxx - sub_src.minx)/lx - precision);
				int sub_h = (int)ceil((target.maxy - target.miny)*(sub_src.maxy - sub_src.miny)/ly - precision);

				// add border
				/*
				Vector border( (sub_src.maxx - sub_src.minx)/Real(sub_w),
						       (sub_src.maxy - sub_src.miny)/Real(sub_h) );
				border *= Real(border_size);
				sub_src.minx -= border[0];
				sub_src.miny -= border[0];
				sub_src.maxx += border[0];
				sub_src.maxy += border[0];
				sub_w += 2*border_size;
				sub_h += 2*border_size;
				*/

				// set target
				resample_sw->sub_task()->target_surface->set_size(sub_w, sub_h);
				resample_sw->sub_task()->init_target_rect(RectInt(0, 0, sub_w, sub_h), sub_src.get_min(), sub_src.get_max());
				assert(resample_sw->sub_task()->check());
				resample_sw->sub_task()->trunc_target_by_bounds();
			}
			else
			{
				// reset target
				resample_sw->sub_task()->target_surface->set_size(0, 0);
				resample_sw->sub_task()->clear_target_rect();
			}
		}

		apply(params, resample_sw);
	}
}

/* === E N T R Y P O I N T ================================================= */
