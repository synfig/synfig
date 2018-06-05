/* === S Y N F I G ========================================================= */
/*!	\file synfig/rendering/common/optimizer/optimizerdraft.cpp
**	\brief Draft Optimizers
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

#include "optimizerdraft.h"

#include "../task/taskcontour.h"
#include "../task/taskblur.h"
#include "../task/tasklayer.h"
#include "../task/tasktransformation.h"

#endif

using namespace synfig;
using namespace rendering;

/* === M A C R O S ========================================================= */

/* === G L O B A L S ======================================================= */

/* === P R O C E D U R E S ================================================= */

/* === M E T H O D S ======================================================= */


void
OptimizerDraftLowRes::run(const RunParams& /*params*/) const
{
	/*
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
	*/
}


void
OptimizerDraftResample::run(const RunParams& /*params*/) const
{
	/*
	if (TaskSurfaceResample::Handle resample = TaskSurfaceResample::Handle::cast_dynamic(params.ref_task))
	{
		if ( resample->interpolation != Color::INTERPOLATION_NEAREST
		  || approximate_greater_lp(resample->supersample[0], 1.0)
		  || approximate_greater_lp(resample->supersample[1], 1.0) )
		{
			resample = TaskSurfaceResample::Handle::cast_dynamic(resample->clone());
			resample->interpolation = Color::INTERPOLATION_NEAREST;
			resample->supersample[0] = std::min(resample->supersample[0], 1.0);
			resample->supersample[1] = std::min(resample->supersample[1], 1.0);
			apply(params, resample);
		}
	}
	*/
}


void
OptimizerDraftContour::run(const RunParams& /*params*/) const
{
	/*
	if (TaskContour::Handle contour = TaskContour::Handle::cast_dynamic(params.ref_task))
	{
		if ( approximate_less_lp(contour->detail, 4.0)
		 || (contour->contour && contour->contour->antialias && contour->allow_antialias) )
		{
			contour = TaskContour::Handle::cast_dynamic(contour->clone());
			contour->detail = std::max(4.0, contour->detail);
			contour->allow_antialias = false;
			apply(params, contour);
		}
	}
	*/
}


void
OptimizerDraftBlur::run(const RunParams& /*params*/) const
{
	/*
	if (TaskBlur::Handle blur = TaskBlur::Handle::cast_dynamic(params.ref_task))
	{
		if (blur->blur.type != Blur::BOX)
		{
			blur = TaskBlur::Handle::cast_dynamic(blur->clone());
			blur->blur.type = Blur::BOX;
			apply(params, blur);
		}
	}
	*/
}


void
OptimizerDraftLayerRemove::run(const RunParams& /*params*/) const
{
	/*
	if (TaskLayer::Handle layer = TaskLayer::Handle::cast_dynamic(params.ref_task))
		if (layer->layer && layer->layer->get_name() == layername)
			apply(params, NULL);
	*/
}


void
OptimizerDraftLayerSkip::run(const RunParams& /*params*/) const
{
	/*
	if (TaskLayer::Handle layer = TaskLayer::Handle::cast_dynamic(params.ref_task))
		if (layer->layer && layer->layer->get_name() == layername)
			apply(params, layer->sub_task());
	*/
}


/* === E N T R Y P O I N T ================================================= */
