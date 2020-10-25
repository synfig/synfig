/* === S Y N F I G ========================================================= */
/*!	\file synfig/rendering/common/optimizer/optimizerpass.cpp
**	\brief OptimizerPass
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

#include "optimizerpass.h"

#include "../task/taskblend.h"

#endif

using namespace synfig;
using namespace rendering;

/* === M A C R O S ========================================================= */

/* === G L O B A L S ======================================================= */

/* === P R O C E D U R E S ================================================= */

/* === M E T H O D S ======================================================= */

OptimizerPass::OptimizerPass(bool deep_first)
{
	category_id = CATEGORY_ID_SPECIALIZED;
	depends_from = CATEGORY_COORDS;
	mode = MODE_REPEAT_PARENT;
	this->deep_first = deep_first;
	for_task = true;
}

void
OptimizerPass::run(const RunParams& params) const
{
	if ( !params.ref_task->is_valid() )
		{ apply(params, Task::Handle()); return; }

	int index = params.ref_task->get_pass_subtask_index();
	if (!deep_first && index >= 0) return;

	// keep unchanged
	if (index == Task::PASSTO_THIS_TASK)
		return;

	// remove
	if (index == Task::PASSTO_NO_TASK)
		{ apply(params, Task::Handle()); return; }

	// remove sub-tasks
	if (index == Task::PASSTO_THIS_TASK_WITHOUT_SUBTASKS)
	{
		for(Task::List::const_iterator i = params.ref_task->sub_tasks.begin(); i != params.ref_task->sub_tasks.end(); ++i)
			if (*i) {
				Task::Handle new_task = params.ref_task->clone();
				new_task->sub_tasks.clear();
				apply(params, new_task);
				return;
			}
		return;
	}

	// replace to sub-task

	if (index < 0) return;

	const Task::Handle &task = params.ref_task.get()->sub_task(index);
	if (!task || !task->is_valid())
		{ apply(params, Task::Handle()); return; }

	apply(params, replace_target(params.ref_task, task));
}

/* === E N T R Y P O I N T ================================================= */
