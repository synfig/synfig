/* === S Y N F I G ========================================================= */
/*!	\file synfig/rendering/common/task/taskblend.h
**	\brief TaskBlend Header
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

#ifndef __SYNFIG_RENDERING_TASKBLEND_H
#define __SYNFIG_RENDERING_TASKBLEND_H

/* === H E A D E R S ======================================================= */

#include "../../task.h"
#include "tasktransformation.h"

/* === M A C R O S ========================================================= */

/* === T Y P E D E F S ===================================================== */

/* === C L A S S E S & S T R U C T S ======================================= */

namespace synfig
{
namespace rendering
{


//! Tasks with this interface can paint at target with selected blending mode.
//! In this case target of sub_task(0) should be equal the task target
class TaskInterfaceBlendToTarget: public TaskInterfaceTargetAsSource
{
public:
	bool blend;
	Color::BlendMethod blend_method;
	Color::value_type amount;

	TaskInterfaceBlendToTarget():
		blend(),
		blend_method(Color::BLEND_COMPOSITE),
		amount() { }
	virtual Color::BlendMethodFlags get_supported_blend_methods() const
		{ return 0; }
	bool is_blend_method_supported(Color::BlendMethod blend_method)
		{ return get_supported_blend_methods() & (1 << blend_method); }
};


class TaskBlend: public Task,
	public TaskInterfaceTransformationPass,
	public TaskInterfaceSplit
{
public:
	typedef etl::handle<TaskBlend> Handle;
	SYNFIG_EXPORT static Token token;
	virtual Token::Handle get_token() const { return token.handle(); }

	Color::BlendMethod blend_method;
	Color::value_type amount;

	TaskBlend():
		blend_method(Color::BLEND_COMPOSITE), amount(1.0) { }

	virtual int get_pass_subtask_index() const;

	const Task::Handle& sub_task_a() const { return sub_task(0); }
	Task::Handle& sub_task_a() { return sub_task(0); }

	const Task::Handle& sub_task_b() const { return sub_task(1); }
	Task::Handle& sub_task_b() { return sub_task(1); }

	VectorInt get_offset_a() const
		{ return sub_task_a() ? TaskList::calc_target_offset(*this, *sub_task_a()) : VectorInt(); }
	VectorInt get_offset_b() const
		{ return sub_task_b() ? TaskList::calc_target_offset(*this, *sub_task_b()) : VectorInt(); }

	virtual Rect calc_bounds() const;
};


} /* end namespace rendering */
} /* end namespace synfig */

/* -- E N D ----------------------------------------------------------------- */

#endif
