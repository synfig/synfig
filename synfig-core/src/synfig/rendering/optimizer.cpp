/* === S Y N F I G ========================================================= */
/*!	\file synfig/rendering/optimizer.cpp
**	\brief Optimizer
**
**	$Id$
**
**	\legal
**	......... ... 2015 Ivan Mahonin
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

#include "optimizer.h"

#endif

using namespace synfig;
using namespace rendering;

/* === M A C R O S ========================================================= */

/* === G L O B A L S ======================================================= */

/* === P R O C E D U R E S ================================================= */

/* === M E T H O D S ======================================================= */

const Optimizer::CategoryInfo Optimizer::categories_info[CATEGORIES_COUNT] = {
	CategoryInfo(false),   // CATEGORY_ID_BEGIN
	CategoryInfo(false),   // CATEGORY_ID_COORDS
	CategoryInfo(true),    // CATEGORY_ID_SPECIALIZED
	CategoryInfo(false) }; // CATEGORY_ID_LIST

Optimizer::~Optimizer() { }

Task::Handle
Optimizer::replace_target(
	const Task::Handle &parent,
	const SurfaceResource::Handle &surface,
	const Task::Handle &task )
{
	if (!task) return Task::Handle();

	Task::Handle new_task = task;
	if (task->target_surface == surface && task->is_valid()) {
		new_task = task->clone();
		new_task->target_rect -= TaskList::calc_target_offset(*parent, *new_task);
		new_task->trunc_target_rect(parent->target_rect);
		new_task->target_surface = parent->target_surface;
	}

	int index = 0;
	for(Task::List::iterator i = new_task->sub_tasks.begin(); i != new_task->sub_tasks.end(); ++i, ++index)
		if (new_task != task) {
			*i = replace_target(parent, surface, *i);
		} else {
			Task::Handle sub_task = replace_target(parent, surface, *i);
			if (sub_task != *i) {
				new_task = task->clone();
				i = new_task->sub_tasks.begin() + (i - task->sub_tasks.begin());
				*i = sub_task;
			}
		}

	return new_task;
}


/* === E N T R Y P O I N T ================================================= */
