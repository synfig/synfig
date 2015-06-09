/* === S Y N F I G ========================================================= */
/*!	\file synfig/rendering/common/optimizer/optimizerblur.cpp
**	\brief OptimizerBlur
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

#include "optimizerblur.h"
#include "../task/taskblur.h"
#include "../task/tasktransformation.h"
#include "../../primitive/affinetransformation.h"

#endif

using namespace synfig;
using namespace rendering;

/* === M A C R O S ========================================================= */

/* === G L O B A L S ======================================================= */

/* === P R O C E D U R E S ================================================= */

/* === M E T H O D S ======================================================= */

bool
OptimizerBlur::run(const RunParams& params) const
{
	if (TaskBlur::Handle blur = TaskBlur::Handle::cast_dynamic(params.task))
	{
		if (!blur->sub_task_resized
		 && blur->target_surface)
		{
			if (blur->target_surface->empty())
			{
				params.out_task.reset();
				return true;
			}
			else
			{
				int width = 1;
				int height = 1;
				blur->blur.get_surface_extra_size(
					0.5 * (Real)blur->target_surface->get_width(),
					0.5 * (Real)blur->target_surface->get_height(),
					width, height );
				if (width < 1) width = 1;
				if (height < 1) height = 1;

				AffineTransformation::Handle affine_transfromation(new AffineTransformation());
				affine_transfromation->matrix.set_scale(
					(Real)blur->target_surface->get_width()/(Real)width,
					(Real)blur->target_surface->get_height()/(Real)height );

				TaskTransformation::Handle transfromation(new TaskTransformation());
				transfromation->transformation = affine_transfromation;
				transfromation->sub_task() = blur->sub_task();

				TaskBlur::Handle blur_prepared(Task::clone(blur));
				blur_prepared->sub_task() = transfromation;
				blur_prepared->sub_task_resized = true;

				params.out_task = blur_prepared;
				return true;
			}
		}
	}
	return false;
}

/* === E N T R Y P O I N T ================================================= */
