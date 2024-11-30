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

	/**
	 * If the coordinates distorted by the task should be clamped to the pixel-surface coordinates
	 */
	bool should_clamp_coordinates = false;

public:
	/**
	 * Scan the target surface and fill each pixel according to point_vfunc().
	 * @param task the TaskDistort object
	 * @return true, if successful
	 */
	bool run_task(const rendering::TaskDistort& task) const;
};

}
}

#endif // SYNFIG_TASKDISTORTSW_H
