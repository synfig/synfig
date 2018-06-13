/* === S Y N F I G ========================================================= */
/*!	\file synfig/rendering/common/optimizer/optimizerblendtotarget.cpp
**	\brief OptimizerBlendToTarget
**
**	$Id$
**
**	\legal
**	......... ... 2015-2018 Ivan Mahonin
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

#ifndef _WIN32
#include <unistd.h>
#include <sys/types.h>
#include <signal.h>
#endif

#include <synfig/general.h>
#include <synfig/localization.h>

#include "optimizerblendtotarget.h"

#include "../task/taskblend.h"

#endif

using namespace synfig;
using namespace rendering;

/* === M A C R O S ========================================================= */

/* === G L O B A L S ======================================================= */

/* === P R O C E D U R E S ================================================= */

/* === M E T H O D S ======================================================= */

OptimizerBlendToTarget::OptimizerBlendToTarget()
{
	category_id = CATEGORY_ID_SPECIALIZED;
	depends_from = CATEGORY_COORDS;
	mode = MODE_REPEAT_PARENT;
	deep_first = true;
	for_task = true;
}

void
OptimizerBlendToTarget::run(const RunParams& params) const
{
	//
	// merge TaskBlend with TaskInterfaceBlendToTarget (only for BLEND_COMPOSITE)
	// or just set blending for TaskInterfaceBlendToTarget
	//
	//  blendA(targetA)
	//  - taskB(targetA) - the parent target
	//  - compositeC(targetC)
	//
	// converts to:
	//
	//  compositeAC(targetA)
	//  - taskB(targetA)
	//

	if (TaskBlend::Handle blend = TaskBlend::Handle::cast_dynamic(params.ref_task))
	if (TaskInterfaceTargetAsSource *sourcetarget = blend.type_pointer<TaskInterfaceTargetAsSource>())
	{
		if ( sourcetarget->get_target_subtask_index() == 0
		  && ( !blend->sub_task_a()
		    || blend->sub_task_a().type_is<TaskNone>()
			|| blend->sub_task_a()->target_surface == blend->target_surface ))
		{
			TaskInterfaceBlendToTarget *composite = blend->sub_task_b().type_pointer<TaskInterfaceBlendToTarget>();
			if ( composite
			  && composite->is_blend_method_supported(blend->blend_method)
			  && (!composite->target_subtask() || composite->target_subtask().type_is<TaskNone>())
			  && ( !composite->blend
				|| (blend->blend_method == Color::BLEND_COMPOSITE && composite->blend_method == blend->blend_method) ))
			{
				Task::Handle new_task = blend->sub_task_b()->clone();

				composite = new_task.type_pointer<TaskInterfaceBlendToTarget>();
				assert(composite);
				if (!composite->blend) composite->amount = 1.0;
				composite->amount *= blend->amount;
				composite->blend_method = blend->blend_method;
				composite->blend = true;
				composite->target_subtask() = blend->sub_task_a();
				new_task->assign_target(*blend);

				composite->on_target_set_as_source();

				apply(params, new_task);
			}
		}
	}
}

/* === E N T R Y P O I N T ================================================= */
