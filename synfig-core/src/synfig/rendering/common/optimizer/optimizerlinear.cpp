/* === S Y N F I G ========================================================= */
/*!	\file synfig/rendering/common/optimizer/optimizerlinear.cpp
**	\brief OptimizerLinear
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

#include "optimizerlinear.h"

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
OptimizerLinear::run(const RunParams& params) const
{
	// convert task-tree to linear list
	for(Task::List::iterator i = params.list.begin(); i != params.list.end();)
	{
		if (!i->type_is<TaskSurface>())
		{
			// remember current position
			int index = i - params.list.begin();
			bool found = false;

			for(Task::List::iterator j = (*i)->sub_tasks.begin(); j != (*i)->sub_tasks.end(); ++j)
			{
				if ( *j
				  && !TaskSurface::Handle::cast_dynamic(*j)
				  && !TaskSurfaceEmpty::Handle::cast_dynamic(*j) )
				{
					i = params.list.insert(i, *j);
					++i;

					if (!found)
					{
						// clone task
						int index = j - (*i)->sub_tasks.begin();
						*i = (*i)->clone();
						j = (*i)->sub_tasks.begin() + index;
						found = true;
					}

					// replace sub_task by TaskSurface
					Task::Handle surface(new TaskSurface());
					assign(surface, *j);
					surface->sub_tasks.clear();
					*j = surface;
				}
			}

			// if changed then go back to check inserted tasks
			if (found)
				{ i = params.list.begin() + index; continue; }
		}
		++i;
	}

	// remove dummy tasks
	for(Task::List::iterator i = params.list.begin(); i != params.list.end();)
		if ( i->type_is<TaskList>()
		  || i->type_is<TaskSurfaceEmpty>()
		  /*|| i->type_is<TaskSurface>()*/ )
			i = params.list.erase(i); else ++i;
}

/* === E N T R Y P O I N T ================================================= */
