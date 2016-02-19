/* === S Y N F I G ========================================================= */
/*!	\file synfig/rendering/module/common/task/taskclamp.h
**	\brief TaskClamp Header
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

#ifndef __SYNFIG_RENDERING_MODULE_TASKCLAMP_H
#define __SYNFIG_RENDERING_MODULE_TASKCLAMP_H

/* === H E A D E R S ======================================================= */

#include <synfig/rendering/common/task/taskpixelprocessor.h>

/* === M A C R O S ========================================================= */

/* === T Y P E D E F S ===================================================== */

/* === C L A S S E S & S T R U C T S ======================================= */

namespace synfig
{
namespace rendering
{

class TaskClamp: public TaskPixelProcessor
{
public:
	typedef etl::handle<TaskClamp> Handle;

	bool invert_negative;
	bool clamp_floor;
	bool clamp_ceiling;
	Real floor;
	Real ceiling;

	TaskClamp():
		invert_negative(false),
		clamp_floor(true),
		clamp_ceiling(true),
		floor(0.0),
		ceiling(1.0) { }
	Task::Handle clone() const { return clone_pointer(this); }
};

} /* end namespace rendering */
} /* end namespace synfig */

/* -- E N D ----------------------------------------------------------------- */

#endif
