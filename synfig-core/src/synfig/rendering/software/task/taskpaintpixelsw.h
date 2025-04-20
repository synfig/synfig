/* === S Y N F I G ========================================================= */
/*!	\file synfig/rendering/software/task/taskpaintpixelsw.h
**	\brief TaskPaintPixelSW Header
**
**	\legal
**	Copyright (c) 2002-2005 Robert B. Quattlebaum Jr., Adrian Bentley
**	......... ... 2022 Synfig Contributors
**
**	This file is part of Synfig.
**
**	Synfig is free software: you can redistribute it and/or modify
**	it under the terms of the GNU General Public License as published by
**	the Free Software Foundation, either version 2 of the License, or
**	(at your option) any later version.
**
**	Synfig is distributed in the hope that it will be useful,
**	but WITHOUT ANY WARRANTY; without even the implied warranty of
**	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
**	GNU General Public License for more details.
**
**	You should have received a copy of the GNU General Public License
**	along with Synfig.  If not, see <https://www.gnu.org/licenses/>.
**	\endlegal
*/
/* ========================================================================= */

/* === S T A R T =========================================================== */

#ifndef __SYNFIG_RENDERING_TASKPAINTPIXELSW_H
#define __SYNFIG_RENDERING_TASKPAINTPIXELSW_H

/* === H E A D E R S ======================================================= */

#include "tasksw.h"
#include <synfig/rendering/task.h>
#include <synfig/rendering/common/task/taskblend.h>
#include <synfig/rendering/common/task/taskpixelprocessor.h>

/* === M A C R O S ========================================================= */

/* === T Y P E D E F S ===================================================== */

/* === C L A S S E S & S T R U C T S ======================================= */

namespace synfig
{
namespace rendering
{

/**
 * Paint each pixel depending on its position.
 *
 * The color of each pixel is defined by get_color() calls.
 *
 * To use this abstract class, call run_task() inside of your implementation of Task::run().
 *
 */
class TaskPaintPixelSW :
		public TaskSW,
		public TaskInterfaceBlendToTarget,
		public TaskInterfaceSplit
{
public:
	//! Called inside run() right before start iterating over each pixel.
	//! Useful for computing some parameters that are constant for all iterations.
	//!
	//! \param world_to_raster full transformation matrix mixing transformation of vector to raster coords and task affine transformation
	//! \param raster_to_world the inverse of world_to_raster
	virtual void pre_run(const Matrix3& /*world_to_raster*/, const Matrix3& /*raster_to_world*/) const {}

	//! Fetch color at position p (in synfig units) when antialias is false
	virtual Color get_color(const Vector& p) const = 0;

	//! Call this method from run() method of the real task implementation
	virtual bool run_task() const;

	void on_target_set_as_source() override;

	Color::BlendMethodFlags get_supported_blend_methods() const override;
};

/**
 * Paint each pixel depending on its position and the context color at that point.
 *
 * The color of each pixel is defined by get_color() calls.
 *
 * To use this abstract class, call run_task() inside of your implementation of Task::run().
 *
 */
class TaskFilterPixelSW :
		public TaskSW
{
public:
	//! Called inside run() right before iterating over each pixel.
	//! Useful for computing some parameters that are constant for all iterations.
	//!
	//! \param raster_to_world transformation matrix : raster coordinates to world coordinates
	virtual void pre_run(const Matrix3& /*raster_to_world*/) const {}

	/** Fetch color at position @a p (in synfig units) given a previous Color @a c at same point */
	virtual Color get_color(const Vector& /*p*/, const Color& c) const = 0;

	//! Call this method from run() method of the real task implementation
	virtual bool run_task() const;
};


} /* end namespace rendering */
} /* end namespace synfig */

#endif // __SYNFIG_RENDERING_TASKPAINTPIXELSW_H
