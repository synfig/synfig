/* === S Y N F I G ========================================================= */
/*!	\file synfig/rendering/common/task/taskpixelprocessor.h
**	\brief TaskPixelProcessor Header
**
**	\legal
**	......... ... 2016-2018 Ivan Mahonin
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

#ifndef __SYNFIG_RENDERING_TASKPIXELPROCESSOR_H
#define __SYNFIG_RENDERING_TASKPIXELPROCESSOR_H

/* === H E A D E R S ======================================================= */

#include <synfig/color/colormatrix.h>

#include "../../task.h"
#include "tasktransformation.h"

/* === M A C R O S ========================================================= */

/* === T Y P E D E F S ===================================================== */

/* === C L A S S E S & S T R U C T S ======================================= */

namespace synfig
{
namespace rendering
{


class TaskPixelProcessor: public Task,
	public TaskInterfaceTransformationPass,
	public TaskInterfaceConstant,
	public TaskInterfaceSplit
{
public:
	typedef etl::handle<TaskPixelProcessor> Handle;
	SYNFIG_EXPORT static Token token;
	virtual Token::Handle get_token() const { return token.handle(); }

	const Task::Handle& sub_task() const { return Task::sub_task(0); }
	Task::Handle& sub_task() { return Task::sub_task(0); }

	VectorInt get_offset() const;

	virtual Rect calc_bounds() const;

	virtual int get_pass_subtask_index() const
	{
		if (is_zero())
			return PASSTO_NO_TASK;
		if (!is_affects_transparent() && !sub_task())
			return PASSTO_NO_TASK;
		if (is_transparent())
			return sub_task() ? 0 : PASSTO_NO_TASK;
		if (is_constant())
			return PASSTO_THIS_TASK_WITHOUT_SUBTASKS;
		return PASSTO_THIS_TASK;
	}

	virtual bool is_zero() const
		{ return false; }
	/// It does not do/affect anything. Every pixel keeps the same after running this task
	virtual bool is_transparent() const
		{ return false; }
	/// It produces the same color for each pixel
	virtual bool is_constant() const
		{ return false; }
	virtual bool is_affects_transparent() const
		{ return false; }
};


class TaskPixelGamma: public TaskPixelProcessor
{
public:
	typedef etl::handle<TaskPixelGamma> Handle;
	SYNFIG_EXPORT static Token token;
	virtual Token::Handle get_token() const { return token.handle(); }

	Gamma gamma;
	TaskPixelGamma() { }

	virtual bool is_transparent() const
	{
		return approximate_equal_lp(gamma.get_r(), ColorReal(1.0))
			&& approximate_equal_lp(gamma.get_g(), ColorReal(1.0))
			&& approximate_equal_lp(gamma.get_b(), ColorReal(1.0));
	}
};


class TaskPixelColorMatrix: public TaskPixelProcessor
{
public:
	typedef etl::handle<TaskPixelColorMatrix> Handle;
	SYNFIG_EXPORT static Token token;
	virtual Token::Handle get_token() const { return token.handle(); }

	ColorMatrix matrix;

	virtual bool is_zero() const
		{ return matrix.is_transparent(); }
	virtual bool is_transparent() const
		{ return matrix.is_identity(); }
	virtual bool is_constant() const
		{ return matrix.is_constant(); }
	virtual bool is_affects_transparent() const
		{ return matrix.is_affects_transparent(); }
};


} /* end namespace rendering */
} /* end namespace synfig */

/* -- E N D ----------------------------------------------------------------- */

#endif
