/* === S Y N F I G ========================================================= */
/*!	\file synfig/rendering/common/optimizer/optimizerdraft.cpp
**	\brief Draft Optimizers
**
**	$Id$
**
**	\legal
**	......... ... 2017-2018 Ivan Mahonin
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


// OptimizerDraft

OptimizerDraft::OptimizerDraft()
{
	category_id = CATEGORY_ID_BEGIN;
	for_task = true;
}


// OptimizerDraftLowRes

OptimizerDraftLowRes::OptimizerDraftLowRes(Real scale): scale(scale)
{
	for_root_task = true;
	for_task = false;
}

void
OptimizerDraftLowRes::run(const RunParams &params) const
{
	if (!params.parent && params.ref_task && !std::dynamic_pointer_cast<TaskSurface>(params.ref_task))
	{
		Task::Handle sub_task = params.ref_task->clone();

		TaskTransformationAffine::Handle affine = std::make_shared<TaskTransformationAffine>();
		affine->sub_task() = sub_task;
		affine->supersample = Vector(1.0/scale, 1.0/scale);
		affine->interpolation = Color::INTERPOLATION_NEAREST;
		affine->sub_task() = sub_task;

		// swap target
		affine->assign_target(*sub_task);
		sub_task->target_surface.reset();
		sub_task->source_rect = Rect::infinite();
		sub_task->target_rect = RectInt::zero();

		apply(params, affine);
	}
}


// OptimizerDraftTransformation

void
OptimizerDraftTransformation::run(const RunParams &params) const
{
	if (TaskTransformation::Handle transformation = std::dynamic_pointer_cast<TaskTransformation>(params.ref_task))
	{
		const bool affine = (bool)std::dynamic_pointer_cast<TransformationAffine>(transformation->get_transformation());
		const Real supersample_max = affine ? 1 : 0.5;
		if ( transformation->interpolation != Color::INTERPOLATION_NEAREST
		  || approximate_greater_lp(transformation->supersample[0], supersample_max)
		  || approximate_greater_lp(transformation->supersample[1], supersample_max) )
		{
			transformation = std::dynamic_pointer_cast<TaskTransformation>( transformation->clone() );
			transformation->interpolation = Color::INTERPOLATION_NEAREST;
			transformation->supersample[0] = std::min(transformation->supersample[0], supersample_max);
			transformation->supersample[1] = std::min(transformation->supersample[1], supersample_max);
			apply(params, transformation);
		}
	}
}


// OptimizerDraftContour

OptimizerDraftContour::OptimizerDraftContour(Real detail, bool antialias):
	detail(detail), antialias(antialias) { }

void
OptimizerDraftContour::run(const RunParams &params) const
{
	if (TaskContour::Handle contour = std::dynamic_pointer_cast<TaskContour>(params.ref_task))
	{
		if ( approximate_less_lp(contour->detail, detail)
		 || ( !antialias
		   && contour->contour
		   && contour->contour->antialias
		   && contour->allow_antialias ))
		{
			contour = std::dynamic_pointer_cast<TaskContour>(contour->clone());
			if (contour->detail < detail) contour->detail = detail;
			if (!antialias) contour->allow_antialias = false;
			apply(params, contour);
		}
	}
}


// OptimizerDraftBlur

void
OptimizerDraftBlur::run(const RunParams &params) const
{
	if (TaskBlur::Handle blur = std::dynamic_pointer_cast<TaskBlur>(params.ref_task))
	{
		if (blur->blur.type != Blur::BOX && blur->blur.type != Blur::CROSS)
		{
			blur = std::dynamic_pointer_cast<TaskBlur>(blur->clone());
			blur->blur.type = Blur::BOX;
			apply(params, blur);
		}
	}
}


// OptimizerDraftLayerRemove

OptimizerDraftLayerRemove::OptimizerDraftLayerRemove(const String &layername):
	layername(layername) { }

void
OptimizerDraftLayerRemove::run(const RunParams &params) const
{
	if (TaskLayer::Handle layer = std::dynamic_pointer_cast<TaskLayer>(params.ref_task))
		if (layer->layer && layer->layer->get_name() == layername)
			apply(params, Task::Handle());
}


// OptimizerDraftLayerSkip

OptimizerDraftLayerSkip::OptimizerDraftLayerSkip(const String &layername):
	layername(layername)
	{ mode |= MODE_REPEAT_LAST; }

void
OptimizerDraftLayerSkip::run(const RunParams &params) const
{
	if (TaskLayer::Handle layer = std::dynamic_pointer_cast<TaskLayer>(params.ref_task))
		if (layer->layer && layer->layer->get_name() == layername)
			apply(params, layer->sub_task());
}


/* === E N T R Y P O I N T ================================================= */
