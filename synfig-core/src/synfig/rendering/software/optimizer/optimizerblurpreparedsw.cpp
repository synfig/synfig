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

#include "optimizerblurpreparedsw.h"

#include "../surfacesw.h"
#include "../../common/task/taskblur.h"
#include "../task/taskblurpreparedsw.h"
#include "../task/taskexpandsurfacesw.h"

#include <synfig/blur.h>

#endif

using namespace synfig;
using namespace rendering;

/* === M A C R O S ========================================================= */

/* === G L O B A L S ======================================================= */

/* === P R O C E D U R E S ================================================= */

/* === M E T H O D S ======================================================= */

bool
OptimizerBlurPreparedSW::run(const RunParams& params) const
{
	TaskBlur::Handle blur = TaskBlur::Handle::cast_dynamic(params.task);
	if ( blur
	  && blur->target_surface
	  && blur->sub_task() )
	{
		Vector pixels_per_unit = blur->get_pixels_per_unit();
		Vector units_per_pixel = blur->get_units_per_pixel();
		int dw = 0;
		int dh = 0;

		::Blur b;
		b.set_type(blur->blur.type);
		b.set_size(blur->blur.size);
		b.get_surface_extra_size(
			pixels_per_unit[0], pixels_per_unit[1], dw, dh );
		Vector d((Real)dw*units_per_pixel[0], (Real)dh*units_per_pixel[1]);
		int width = blur->target_surface->get_width() + 2*dw;
		int height = blur->target_surface->get_height() + 2*dh;

		TaskBlurPreparedSW::Handle blur_prepared_sw(new TaskBlurPreparedSW());
		*((Task*)(blur_prepared_sw.get())) = *((Task*)(blur.get()));
		blur_prepared_sw->blur = blur->blur;
		blur_prepared_sw->rect_lt -= d;
		blur_prepared_sw->rect_rb += d;

		if (blur_prepared_sw->sub_task()->target_surface)
		{
			// task to expand surface
			TaskExpandSurfaceSW::Handle expand(new TaskExpandSurfaceSW());
			*((Task*)(expand.get())) = *((Task*)(blur.get()));
			expand->target_surface = new SurfaceSW();
			expand->target_surface->set_size(width, height);
			expand->rect_lt -= d;
			expand->rect_rb += d;
			blur_prepared_sw->sub_task() = expand;
		}
		else
		{
			Task::Handle sub = blur_prepared_sw->sub_task()->clone();
			sub->target_surface = new SurfaceSW();
			sub->target_surface->set_size(width, height);
			sub->rect_lt -= d;
			sub->rect_rb += d;
			blur_prepared_sw->sub_task() = sub;
		}

		params.out_task = blur_prepared_sw;
		return true;
	}
	return false;
}

/* === E N T R Y P O I N T ================================================= */
