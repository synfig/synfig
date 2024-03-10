/* === S Y N F I G ========================================================= */
/*!	\file synfig/rendering/common/task/taskdistort.h
**	\brief TaskDistort Interface Header
**
**	\legal
**	Copyright (c) 2002-2005 Robert B. Quattlebaum Jr., Adrian Bentley
**	......... ... 2024 Synfig Contributors
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

#ifndef SYNFIG_RENDERING_TASKDISTORT_H
#define SYNFIG_RENDERING_TASKDISTORT_H

/* === H E A D E R S ======================================================= */

#include "../../task.h"

#include <synfig/matrix.h>

/* === M A C R O S ========================================================= */

/* === T Y P E D E F S ===================================================== */

/* === C L A S S E S & S T R U C T S ======================================= */

namespace synfig
{
namespace rendering
{

/**
 * A base for tasks that remaps pixels from source (context) to target surface.
 *
 * It isn't a task by itself, but acts like a base class for task that just
 * adds a method to compute what area from source surface ("context") is required
 * to desired target surface region:
 * @see compute_required_source_rect()
 * THe computed area is then stored in @c required_source_rect property.
 *
 * Maybe this could be based on TaskTransformation, but it adds complexity due
 * to extra methods.
 * If the distortion can be represented by a matrix, it's recommended to use
 * TaskTransformation or TaskTransformationAffine instead of this one.
 */
class TaskDistort
	: public Task
{
public:
	/**
	 * Distort tasks may need pixels from Source regions that were not displayed
	 * on Target if it Source was not distorted.
	 * This should be filled in set_coords_sub_tasks() with the (probably)
	 * expanded area needed from Source vector-space surface.
	 */
	Rect required_source_rect;

	void set_coords_sub_tasks() override;

	const Task::Handle& sub_task() const { return Task::sub_task(0); }
	Task::Handle& sub_task() { return Task::sub_task(0); }

protected:
	/**
	 * Compute the required area for current target surface raster area.
	 *
	 * When distorting an image, original pixels are often remapped or
	 * @param source_rect The original source surface rectangle coordinates (in vector units)
	 * @param inv_matrix A matrix to transform Target pixel coordinates to Source vector unit coordinates
	 * @return The needed source surface dimensions in vector units
	 */
	virtual Rect compute_required_source_rect(const Rect& source_rect, const Matrix& inv_matrix) const = 0;

};

} /* end namespace rendering */
} /* end namespace synfig */

/* -- E N D ----------------------------------------------------------------- */

#endif // SYNFIG_RENDERING_TASKDISTORT_H
