/* === S Y N F I G ========================================================= */
/*!	\file synfig/rendering/common/task/taskblur.h
**	\brief TaskBlur Header
**
**	\legal
**	......... ... 2015-2018 Ivan Mahonin
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

#ifndef __SYNFIG_RENDERING_TASKBLUR_H
#define __SYNFIG_RENDERING_TASKBLUR_H

/* === H E A D E R S ======================================================= */

#include "../../task.h"
#include "../../primitive/blur.h"
#include "./tasktransformation.h"

/* === M A C R O S ========================================================= */

/* === T Y P E D E F S ===================================================== */

/* === C L A S S E S & S T R U C T S ======================================= */

namespace synfig
{
namespace rendering
{

// Blur works identically after or before transformation, so we make it a
// member of TaskInterfaceTransformationPass to avoid unnecessarily resampling
class TaskBlur: public Task, public TaskInterfaceTransformationPass
{
public:
	typedef etl::handle<TaskBlur> Handle;
	SYNFIG_EXPORT static Token token;
	virtual Token::Handle get_token() const { return token.handle(); }

	Blur blur;

	virtual int get_pass_subtask_index() const
		{ return sub_task() ? PASSTO_THIS_TASK : PASSTO_NO_TASK; }

	const Task::Handle& sub_task() const { return Task::sub_task(0); }
	Task::Handle& sub_task() { return Task::sub_task(0); }

	virtual Rect calc_bounds() const;
	virtual void set_coords_sub_tasks();
};

} /* end namespace rendering */
} /* end namespace synfig */

/* -- E N D ----------------------------------------------------------------- */

#endif
