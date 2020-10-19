/* === S Y N F I G ========================================================= */
/*!	\file synfig/rendering/common/optimizer/optimizerblendtotarget.cpp
**	\brief OptimizerBlendToTarget
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

/* === H E A D E R S ======================================================= */

#ifdef USING_PCH
#	include "pch.h"
#else
#ifdef HAVE_CONFIG_H
#	include <config.h>
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
			|| blend->sub_task_a()->target_surface == blend->target_surface ))
		{
			TaskInterfaceBlendToTarget *interface = blend->sub_task_b().type_pointer<TaskInterfaceBlendToTarget>();
			if ( interface
			  && interface->is_blend_method_supported(blend->blend_method)
			  && (!interface->target_subtask())
			  && ( !interface->blend
				|| (blend->blend_method == Color::BLEND_COMPOSITE && interface->blend_method == blend->blend_method) ))
			{
				Task::Handle new_task = blend->sub_task_b()->clone();

				interface = new_task.type_pointer<TaskInterfaceBlendToTarget>();
				assert(interface);
				if (!interface->blend) interface->amount = 1.0;
				interface->amount *= blend->amount;
				interface->blend_method = blend->blend_method;
				interface->blend = true;
				interface->target_subtask() = blend->sub_task_a();
				new_task->assign_target(*blend);

				interface->on_target_set_as_source();

				apply(params, new_task);
			}
		}
	}
}

/* === E N T R Y P O I N T ================================================= */
