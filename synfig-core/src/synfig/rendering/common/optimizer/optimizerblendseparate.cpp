/* === S Y N F I G ========================================================= */
/*!	\file synfig/rendering/common/optimizer/optimizerblendseparate.cpp
**	\brief OptimizerBlendSeparate
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

#include "optimizerblendseparate.h"

#include "../task/taskblend.h"
#include "../task/tasklist.h"
#include "../task/tasksurface.h"
#include "../task/tasksurfaceempty.h"

#endif

using namespace synfig;
using namespace rendering;

/* === M A C R O S ========================================================= */

/* === G L O B A L S ======================================================= */

/* === P R O C E D U R E S ================================================= */

/* === M E T H O D S ======================================================= */

void
OptimizerBlendSeparate::run(const RunParams& params) const
{
	TaskBlend::Handle blend = TaskBlend::Handle::cast_dynamic(params.ref_task);
	if ( blend
	  && blend->target_surface
	  && blend->sub_task_a()
	  && blend->sub_task_a()->target_surface == blend->target_surface
	  && !blend->sub_task_a().type_is<TaskSurface>()
	  && !blend->sub_task_a().type_is<TaskSurfaceEmpty>()
	  && blend->sub_task_b()
	  && blend->sub_task_b()->target_surface )
	{
		TaskBlend::Handle new_blend = TaskBlend::Handle::cast_dynamic(blend->clone());
		new_blend->sub_task_a() = new TaskSurface();
		assign(new_blend->sub_task_a(), Task::Handle(blend->sub_task_a()));
		new_blend->sub_task_a()->sub_tasks.clear();

		RectInt blend_rect = blend->get_target_rect();
		if (!Color::is_straight(blend->blend_method))
			etl::set_intersect(
				blend_rect, blend_rect,
				blend->sub_task_b()->get_target_rect()
				+ blend->get_target_offset()
				+ blend->offset_b );
		if (Color::is_onto(blend->blend_method))
			etl::set_intersect(blend_rect, blend_rect, blend->sub_task_a()->get_target_rect());
		if (blend_rect != blend->get_target_rect())
		{
			new_blend->trunc_target_rect(blend_rect);
			new_blend->offset_a = -blend_rect.get_min();
			new_blend->offset_b += blend->get_target_offset()
					             - blend_rect.get_min();
		}

		//new_blend->sub_task_a()->trunc_target_rect( new_blend->get_target_rect() );

		// TODO: we can truncate task_a when blend straight used,
		// but in current code organization we cannot change target_rects of
		// unknown tasks.
		// For example, to change tagret_rect of TaskBlend we should to change
		// fields TaskBlend::offset_a and TaskBlend::offset_b
		// May be we can use TaskSplittable interface?
		// I need to think about it

		TaskList::Handle list;
		list = new TaskList();
		assign(list, Task::Handle(blend));
		list->sub_tasks.clear();

		list->sub_tasks.push_back(blend->sub_task_a());
		list->sub_tasks.push_back(new_blend);

		apply(params, list);
	}
}

/* === E N T R Y P O I N T ================================================= */
