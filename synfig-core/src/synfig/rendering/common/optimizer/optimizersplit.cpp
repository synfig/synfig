/* === S Y N F I G ========================================================= */
/*!	\file synfig/rendering/common/optimizer/optimizersplit.cpp
**	\brief OptimizerSplit
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

#include "optimizersplit.h"

#include "../task/tasksplittable.h"
#include "../../renderer.h"

#endif

using namespace synfig;
using namespace rendering;

/* === M A C R O S ========================================================= */

/* === G L O B A L S ======================================================= */

/* === P R O C E D U R E S ================================================= */

/* === M E T H O D S ======================================================= */

void
OptimizerSplit::run(const RunParams& /* params */) const
{
	// TODO: this version of split don't keep coordinates
	/*
	const int min_area = 256*256;
	int threads = params.renderer.get_max_simultaneous_threads();
	for(Task::List::iterator i = params.list.begin(); i != params.list.end(); ++i)
	{
		if ( i->type_is<TaskSplittable>()
		 &&  !i->type_pointer<TaskSplittable>()->splitted )
		{
			RectInt r = (*i)->get_target_rect();
			int w = r.maxx - r.minx;
			int h = r.maxy - r.miny;
			int t = std::min(h, std::min(w*h/min_area, threads));
			if (t >= 2)
			{
				int hh = h/t;
				int offset = r.miny;
				for(int j = 1; j < t; ++j)
				{
					Task::Handle task = (*i)->clone();
					task.type_pointer<TaskSplittable>()->split(
						RectInt(r.minx, offset, r.maxx, offset + hh) );
					task.type_pointer<TaskSplittable>()->splitted = true;
					i = params.list.insert(i, task);
					++i;
					offset += hh;
				}
				*i = (*i)->clone();
				i->type_pointer<TaskSplittable>()->split(
					RectInt(r.minx, offset, r.maxx, r.maxy) );
				i->type_pointer<TaskSplittable>()->splitted = true;
			}
		}
	}
	*/
}

/* === E N T R Y P O I N T ================================================= */
