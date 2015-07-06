/* === S Y N F I G ========================================================= */
/*!	\file synfig/rendering/common/optimizer/optimizersurfacecreate.cpp
**	\brief OptimizerSurfaceCreate
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

#include "optimizersurfacecreate.h"

#include "../task/tasksurfacecreate.h"

#endif

using namespace synfig;
using namespace rendering;

/* === M A C R O S ========================================================= */

/* === G L O B A L S ======================================================= */

/* === P R O C E D U R E S ================================================= */

/* === M E T H O D S ======================================================= */

bool
OptimizerSurfaceCreate::run(const RunParams& params) const
{
	if (!params.task)
	{
		for(Task::List::iterator i = params.list.begin(); i != params.list.end();)
		{
			if (*i && (*i)->target_surface)
			{
				bool created = (*i)->target_surface->is_created();
				if (!created)
					for(Task::List::const_iterator j = params.list.begin(); j != i; ++j)
						if ( TaskSurfaceCreate::Handle::cast_dynamic(*j)
						  && (*j)->target_surface == (*i)->target_surface )
							{ created = true; break; }

				if (TaskSurfaceCreate::Handle::cast_dynamic(*i))
				{
					// Remove unneeded TaskSurfaceCreate
					if (created)
					{
						i = params.list.erase(i);
						continue;
					}
				}
				else
				{
					// Insert TaskSurfaceCreate when target_surface used first time
					if (!created)
					{
						TaskSurfaceCreate::Handle surface_create(new TaskSurfaceCreate());
						surface_create->target_surface = (*i)->target_surface;
						i = params.list.insert(i, surface_create);
						++i; ++i;
						continue;
					}
				}

			}
			++i;
		}

	}
	return false;
}

/* === E N T R Y P O I N T ================================================= */
