/* === S Y N F I G ========================================================= */
/*!	\file synfig/rendering/common/task/taskdistortsw.h
**	\brief TaskDistortSW Interface Header
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

#ifndef SYNFIG_TASKDISTORTSW_H
#define SYNFIG_TASKDISTORTSW_H

/* === H E A D E R S ======================================================= */

#include "tasksw.h"
#include "../../common/task/taskdistort.h"

/* === M A C R O S ========================================================= */

/* === T Y P E D E F S ===================================================== */

/* === C L A S S E S & S T R U C T S ======================================= */

namespace synfig
{
namespace rendering
{

/**
 * Software implementation of TaskDistort base.
 *
 * This is a base/interface class to tasks that remap pixels from source surface
 * to another coordinates in target surface.
 *
 * The final task class must implement point_vfunc() that has this role of remapping.
 *
 * The final task class should call run_task() in its run() method, that actually
 * calls point_vfunc() pixel by pixel of target surface.
 */
class TaskDistortSW
	: public synfig::rendering::TaskSW
{
protected:
	/**
	 * Convert @a point coordinates in target vectorial region to the vectorial coordinates in source region.
	 *
	 * @param point The transformed vectorial coordinates in target region
	 * @return From where in source region should take the color (in vectorial coordinates)
	 */
	virtual Point point_vfunc(const Point &point) const = 0;

public:
	/**
	 * Scan the target surface and fill each pixel according to point_vfunc().
	 * @param task the TaskDistort object
	 * @return true, if successful
	 */
	bool run_task(const rendering::TaskDistort& task) const;

	/**
	 * Helper to compute info useful to loop the task target_rect raster surface.
	 * @see get_loop_info()
	 */
	struct LoopInfo {
		bool should_abort = false; /**< If true, it should quit run() immediatly with error flag (return false) */
		bool may_end = false; /**< If true, it should quit run() immediatly as successful (return true) */

		Point initial_p; /**< The initial value of point p (vector coordinates) */
		Point p_dy; /**< The increase rate of point p after each line/row */
		Point p_dx; /**< The increase rate of point p after each column */

		PointInt pen_dy; /**< The increase rate of the pen in target_rect surface after each line/row */

		Matrix sub_world_to_raster_transformation; /**< COmputes the conversion from vector to raster coordinates in sub_task surface */
	};

	static LoopInfo get_loop_info(const rendering::Task& task);
};

}
}

#endif // SYNFIG_TASKDISTORTSW_H
