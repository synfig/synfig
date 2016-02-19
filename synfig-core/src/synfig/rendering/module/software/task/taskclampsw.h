/* === S Y N F I G ========================================================= */
/*!	\file synfig/rendering/module/software/task/taskclampsw.h
**	\brief TaskClampSW Header
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

#ifndef __SYNFIG_RENDERING_MODULE_TASKBLENDSW_H
#define __SYNFIG_RENDERING_MODULE_TASKBLENDSW_H

/* === H E A D E R S ======================================================= */

#include "../../common/task/taskclamp.h"

#include <synfig/rendering/software/task/tasksw.h>
#include <synfig/rendering/common/task/tasksplittable.h>

/* === M A C R O S ========================================================= */

/* === T Y P E D E F S ===================================================== */

/* === C L A S S E S & S T R U C T S ======================================= */

namespace synfig
{
namespace rendering
{

class TaskClampSW: public TaskClamp, public TaskSW, public TaskSplittable
{
private:
	void clamp_pixel(Color &dst, const Color &src) const;

public:
	typedef etl::handle<TaskClampSW> Handle;
	Task::Handle clone() const { return clone_pointer(this); }
	virtual void split(const RectInt &sub_target_rect);
	virtual bool run(RunParams &params) const;
};

} /* end namespace rendering */
} /* end namespace synfig */

/* -- E N D ----------------------------------------------------------------- */

#endif
