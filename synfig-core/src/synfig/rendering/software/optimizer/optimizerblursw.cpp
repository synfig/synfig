/* === S Y N F I G ========================================================= */
/*!	\file synfig/rendering/software/optimizer/optimizerblurpreparedsw.cpp
**	\brief OptimizerBlurPreparedSW
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

#include <synfig/rendering/software/optimizer/optimizerblursw.h>

#include "../surfacesw.h"
#include "../task/taskblursw.h"
#include "../function/blur.h"
#include "../../common/task/taskblur.h"

#include <synfig/blur.h>
#include <synfig/general.h>

#endif

using namespace synfig;
using namespace rendering;

/* === M A C R O S ========================================================= */

/* === G L O B A L S ======================================================= */

/* === P R O C E D U R E S ================================================= */

/* === M E T H O D S ======================================================= */

void
OptimizerBlurSW::run(const RunParams& params) const
{
	TaskBlur::Handle blur = TaskBlur::Handle::cast_dynamic(params.ref_task);
	if ( blur
	  && blur->target_surface
	  && blur.type_equal<TaskBlur>() )
	{
		TaskBlurSW::Handle blur_sw;
		init_and_assign_all<SurfaceSW>(blur_sw, blur);

		if ( blur_sw->sub_task()
		  && blur_sw->sub_task()->target_surface
		  && blur_sw->sub_task()->target_surface->is_temporary )
		{
			Vector s = blur_sw->blur.size;
			Vector pixels_per_unit = blur_sw->get_pixels_per_unit();
			s[0] *= pixels_per_unit[0];
			s[1] *= pixels_per_unit[1];

			VectorInt extra_size = software::Blur::get_extra_size(blur_sw->blur.type, s);
			VectorInt size = blur_sw->get_target_rect().get_max()
						   - blur_sw->get_target_rect().get_min()
						   + extra_size*2;
			blur_sw->sub_task()->target_surface->set_size(size);

			RectInt rect = blur_sw->get_target_rect();
			Vector lt = blur_sw->get_source_rect_lt();
			Vector rb = blur_sw->get_source_rect_rb();
			Vector k( (rb[0] - lt[0])/(rect.maxx - rect.minx),
					  (rb[1] - lt[1])/(rect.maxy - rect.miny) );
			Vector nlt( lt[0] - k[0]*extra_size[0],
						lt[1] - k[1]*extra_size[1] );
			Vector nrb( rb[0] + k[0]*extra_size[0],
						rb[1] + k[1]*extra_size[1] );
			blur_sw->sub_task()->init_target_rect(RectInt(VectorInt::zero(), size), nlt, nrb);
			blur_sw->sub_task()->trunc_target_by_bounds();
			assert( blur_sw->sub_task()->check() );
		}

		apply(params, blur_sw);
	}
}

/* === E N T R Y P O I N T ================================================= */
