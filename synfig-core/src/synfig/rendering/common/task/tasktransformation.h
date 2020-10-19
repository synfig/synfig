/* === S Y N F I G ========================================================= */
/*!	\file synfig/rendering/common/task/tasktransformation.h
**	\brief TaskTransformation Header
**
**	$Id$
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

#ifndef __SYNFIG_RENDERING_TASKTRANSFORMATION_H
#define __SYNFIG_RENDERING_TASKTRANSFORMATION_H

/* === H E A D E R S ======================================================= */

#include "../../task.h"
#include "../../primitive/transformationaffine.h"
#include <synfig/synfig_export.h>

/* === M A C R O S ========================================================= */

/* === T Y P E D E F S ===================================================== */

/* === C L A S S E S & S T R U C T S ======================================= */

namespace synfig
{
namespace rendering
{


//! Interface for tasks which works identically while placed before or after transformation
class TaskInterfaceTransformationPass
{
public:
	virtual ~TaskInterfaceTransformationPass() { }
};


class TaskInterfaceTransformation
{
public:
	virtual ~TaskInterfaceTransformation() { }
	virtual Transformation::Handle get_transformation() const
		{ return Transformation::Handle(); }
};


class TaskTransformation: public Task, public TaskInterfaceTransformation
{
public:
	typedef etl::handle<TaskTransformation> Handle;
	static Token token;
	virtual Token::Handle get_token() const { return token.handle(); }

	Color::Interpolation interpolation;
	Vector supersample;

	TaskTransformation();

	//! returns true then task just do transformation of first sub-task only
	//! without supersampling and any drawings.
	//! so this task may be fully merged into other suitable transformation task
	virtual bool is_simple() const;

	virtual int get_pass_subtask_index() const;

	const Task::Handle& sub_task() const { return Task::sub_task(0); }
	Task::Handle& sub_task() { return Task::sub_task(0); }

	virtual Rect calc_bounds() const;
	virtual void set_coords_sub_tasks();
};


class TaskTransformationAffine: public TaskTransformation
{
public:
	typedef etl::handle<TaskTransformationAffine> Handle;
	SYNFIG_EXPORT static Token token;
	virtual Token::Handle get_token() const { return token.handle(); }

	Holder<TransformationAffine> transformation;

	virtual Transformation::Handle get_transformation() const
		{ return transformation.handle(); }

	virtual int get_pass_subtask_index() const;
};


} /* end namespace rendering */
} /* end namespace synfig */

/* -- E N D ----------------------------------------------------------------- */

#endif
