/* === S Y N F I G ========================================================= */
/*!	\file synfig/rendering/common/optimizer/optimizerblendzero.cpp
**	\brief OptimizerBlendZero
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

/* === H E A D E R S ======================================================= */

#ifdef USING_PCH
#	include "pch.h"
#else
#ifdef HAVE_CONFIG_H
#	include <config.h>
#endif

#ifndef WIN32
#include <unistd.h>
#include <sys/types.h>
#include <signal.h>
#endif

#include <synfig/general.h>
#include <synfig/localization.h>

#include "optimizerblendzero.h"

#include "../task/taskblend.h"
#include "../task/tasksurfaceempty.h"

#endif

using namespace synfig;
using namespace rendering;

/* === M A C R O S ========================================================= */

/* === G L O B A L S ======================================================= */

/* === P R O C E D U R E S ================================================= */

/* === M E T H O D S ======================================================= */

void
OptimizerBlendZero::apply_zero(const RunParams &params, const TaskBlend::Handle &blend, const Task::Handle &task) const
{
	if (!task || !task->valid_target())
	{
		Task::Handle empty = new TaskSurfaceEmpty();
		empty->target_surface = params.ref_task->target_surface;
		apply(params, empty);
		return;
	}

	if (task->target_surface == blend->target_surface)
	{
		apply(params, task);
		return;
	}

	apply(params, task->clone());
	params.ref_task->target_surface = blend->target_surface;
	params.ref_task->move_target_rect(
		  blend->get_target_offset()
		- task->get_target_offset()
		+ (task == blend->sub_task_a() ? blend->offset_a : blend->offset_b) );
	assert( params.ref_task->check() );
}

void
OptimizerBlendZero::run(const RunParams& params) const
{
	TaskBlend::Handle blend = TaskBlend::Handle::cast_dynamic(params.ref_task);
	if ( blend
	  && blend->target_surface
	  && blend->sub_task_a()
	  && blend->sub_task_a()->target_surface
	  && blend->sub_task_b()
	  && blend->sub_task_a()->target_surface)
	{
		bool zero_amount = fabsf(blend->amount) <= 1e-6;

		if (zero_amount)
			{ apply_zero(params, blend, blend->sub_task_a()); return; }

		bool valid_a = blend->sub_task_a()->valid_target();
		bool valid_b = blend->sub_task_b()->valid_target();

		if (!valid_b && !valid_a)
			{ apply_zero(params, blend, Task::Handle()); return; }

		bool one_amount = fabsf(blend->amount - 1.f) <= 1e-6;
		bool intertsects = valid_a && valid_b
						&& etl::intersect(blend->sub_task_a()->get_target_rect(), blend->sub_task_b()->get_target_rect());

		if (one_amount && !intertsects)
		{
			bool onto = Color::is_onto(blend->blend_method);
			bool straight = Color::is_straight(blend->blend_method);

			if ( onto && straight)
				{ apply_zero(params, blend, Task::Handle()); return; }
			if ( onto )
				{ apply_zero(params, blend, blend->sub_task_a()); return; }
			//if ( straight )
			//	{ apply_zero(params, blend, blend->sub_task_b()); return; }
		}
	}
}

/* === E N T R Y P O I N T ================================================= */
