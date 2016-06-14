/* === S Y N F I G ========================================================= */
/*!	\file synfig/rendering/common/task/taskpixelprocessor.h
**	\brief TaskPixelProcessor Header
**
**	$Id$
**
**	\legal
**	......... ... 2016 Ivan Mahonin
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

/* === S T A R T =========================================================== */

#ifndef __SYNFIG_RENDERING_TASKPIXELPROCESSOR_H
#define __SYNFIG_RENDERING_TASKPIXELPROCESSOR_H

/* === H E A D E R S ======================================================= */

#include "../../task.h"
#include "tasktransformationpass.h"

/* === M A C R O S ========================================================= */

/* === T Y P E D E F S ===================================================== */

/* === C L A S S E S & S T R U C T S ======================================= */

namespace synfig
{
namespace rendering
{

class TaskPixelProcessor: public Task, public TaskTransformationPass
{
public:
	typedef etl::handle<TaskPixelProcessor> Handle;

	TaskPixelProcessor() { }
	Task::Handle clone() const { return clone_pointer(this); }

	const Task::Handle& sub_task() const { return Task::sub_task(0); }
	Task::Handle& sub_task() { return Task::sub_task(0); }

	VectorInt get_offset() const;

	virtual Rect calc_bounds() const;
	virtual bool is_transparent() const { return false; }
	virtual bool is_constant() const { return false; }
	virtual bool is_affects_transparent() const { return false; }
};

} /* end namespace rendering */
} /* end namespace synfig */

/* -- E N D ----------------------------------------------------------------- */

#endif
