/* === S Y N F I G ========================================================= */
/*!	\file synfig/rendering/common/task/taskblend.h
**	\brief TaskBlend Header
**
**	$Id$
**
**	\legal
**	......... ... 2015 Ivan Mahonin
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

#ifndef __SYNFIG_RENDERING_TASKBLEND_H
#define __SYNFIG_RENDERING_TASKBLEND_H

/* === H E A D E R S ======================================================= */

#include "../../task.h"

/* === M A C R O S ========================================================= */

/* === T Y P E D E F S ===================================================== */

/* === C L A S S E S & S T R U C T S ======================================= */

namespace synfig
{
namespace rendering
{

class TaskBlend: public Task
{
public:
	typedef etl::handle<TaskBlend> Handle;

	Color::BlendMethod blend_method;
	Color::value_type amount;
	VectorInt offset_a;
	VectorInt offset_b;

	TaskBlend():
		blend_method(Color::BLEND_COMPOSITE), amount(1.0) { }
	Task::Handle clone() const { return clone_pointer(this); }

	const Task::Handle& sub_task_a() const { return sub_task(0); }
	Task::Handle& sub_task_a() { return sub_task(0); }

	const Task::Handle& sub_task_b() const { return sub_task(1); }
	Task::Handle& sub_task_b() { return sub_task(1); }

	virtual Rect calc_bounds() const;
};

} /* end namespace rendering */
} /* end namespace synfig */

/* -- E N D ----------------------------------------------------------------- */

#endif
