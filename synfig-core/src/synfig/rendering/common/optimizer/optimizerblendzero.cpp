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
	if (!task || !task->target_rect.valid())
	{
		apply(params, Task::Handle());
		return;
	}


	if (task->target_surface == blend->target_surface)
	{
		apply(params, blend->sub_task_a());
		return;
	}

	apply(params, task->clone());
	params.ref_task->target_surface = blend->target_surface;
	params.ref_task->target_rect +=
		  blend->target_rect.get_min()
		- task->target_rect.get_min()
		+ blend->offset_a;
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

		bool valid_a = blend->sub_task_a()->target_rect.is_valid();
		bool valid_b = blend->sub_task_b()->target_rect.is_valid();

		if (!valid_b && !valid_a)
			{ apply_zero(params, blend, Task::Handle()); return; }

		bool one_amount = fabsf(blend->amount - 1.f) <= 1e-6;
		bool intertsects = valid_a && valid_b
						&& etl::intersect(blend->sub_task_a()->target_rect, blend->sub_task_b()->target_rect);

		if (one_amount && !intertsects)
		{
			bool onto = Color::is_straight(blend->blend_method);
			bool straight = Color::is_straight(blend->blend_method);

			if ( onto && straight)
				{ apply_zero(params, blend, Task::Handle()); return; }
			if ( onto )
				{ apply_zero(params, blend, blend->sub_task_a()); return; }
			if ( straight )
				{ apply_zero(params, blend, blend->sub_task_b()); return; }
		}
	}
}

/* === E N T R Y P O I N T ================================================= */
