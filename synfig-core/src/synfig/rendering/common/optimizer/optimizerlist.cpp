/* === S Y N F I G ========================================================= */
/*!	\file synfig/rendering/common/optimizer/optimizerlist.cpp
**	\brief OptimizerList
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

#include "optimizerlist.h"

#endif

using namespace synfig;
using namespace rendering;

/* === M A C R O S ========================================================= */

/* === G L O B A L S ======================================================= */

/* === P R O C E D U R E S ================================================= */

/* === M E T H O D S ======================================================= */

// these tasks should be removed from list
static bool
can_be_skipped(const Task::Handle &task)
	{ return !task || task.type_is<TaskSurface>(); }

// can we make list with two or more elements?
static bool
can_build_list(const Task::Handle &task)
{
	const TaskInterfaceTargetAsSource *interface = task.type_pointer<TaskInterfaceTargetAsSource>();
	return interface
		&& interface->is_allowed_target_as_source()
	    && !can_be_skipped( interface->target_subtask() );
}

static bool
can_be_modified(const Task::Handle &task)
{
	if (can_be_skipped(task)) return false; // don't try to modify empty tasks
	if (task.type_is<TaskList>())
	{ // can we modify any task in list?
		if ((int)task->sub_tasks.size() < 50)
			for(Task::List::const_iterator i = task->sub_tasks.begin(); i != task->sub_tasks.end(); ++i)
				if (can_be_skipped(*i) || can_build_list(*i))
					return true;
		return false;
	}
	return can_build_list(task); // or can we build a new list?
}

static void
add_task(
	const TaskList::Handle &list,
	const Task::Handle &task )
{
	if (task.type_is<TaskList>()) {
		for(Task::List::const_iterator i = task->sub_tasks.begin(); i != task->sub_tasks.end(); ++i)
			if (!can_be_skipped(*i)) add_task(list, *i);
		return;
	}

	Task::Handle new_task = Optimizer::replace_target(list, task);
	if (can_build_list(task))
	{
		if (new_task == task) new_task = task->clone();
		TaskInterfaceTargetAsSource *new_interface = new_task.type_pointer<TaskInterfaceTargetAsSource>();
		assert(new_interface);

		Task::Handle &target_subtask = new_interface->target_subtask();
		add_task(list, target_subtask);

		target_subtask = new TaskSurface();
		target_subtask->assign_target(*new_task);
		new_interface->on_target_set_as_source();
	}
	list->sub_tasks.push_back(new_task);
}

OptimizerList::OptimizerList()
{
	category_id = CATEGORY_ID_SPECIALIZED;
	depends_from = CATEGORY_COORDS;
	mode = MODE_REPEAT_LAST | MODE_RECURSIVE;
	for_task = true;
}

void
OptimizerList::run(const RunParams& params) const
{
	//
	// task    - any Task
	// surface - TaskSurface
	// list    - TaskList
	// tas     - Task derived from TaskInterfaceTargetAsSource
	//
	//  tasA(targetA)
	//  - tasB(targetB)
	//    - listC(targetC)
	//      - tasD(targetC)
	//      - tasE(targetC)
	//       - surfaceF(targetC)
	//       - taskG(targetG)
	//         - taskH(targetH)
	//      - taskI(targetI)
	//  - taskJ(targetJ)
	//
	// converts to:
	//
	//  list(targetA)
	//  - tasD(targetA)
	//  - tasE(targetA)
	//   - surfaceF(targetA)
	//   - taskG(targetG)
	//     - taskH(targetH)
	//  - taskI(targetA)
	//  - tasB(targetA)
	//    - surface(targetA)
	//  - tasA(targetA)
	//    - surface(targetA)
	//    - taskJ(targetJ)
	//

	const Task::Handle &task = params.ref_task;
	if (can_be_modified(task))
	{
		TaskList::Handle list = new TaskList();
		list->assign_target(*task);
		add_task(list, task);
		apply(params, list);
	}
}

/* === E N T R Y P O I N T ================================================= */
