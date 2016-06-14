/* === S Y N F I G ========================================================= */
/*!	\file synfig/rendering/common/optimizer/optimizerpixelprocessorsplit.cpp
**	\brief OptimizerPixelProcessorSplit
**
**	$Id$
**
**	\legal
**	......... ... 2016 Ivan Mahonin
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

#include "optimizerpixelprocessorsplit.h"

#include "../task/taskpixelprocessor.h"
#include "../task/tasklist.h"

#endif

using namespace synfig;
using namespace rendering;

/* === M A C R O S ========================================================= */

/* === G L O B A L S ======================================================= */

/* === P R O C E D U R E S ================================================= */

/* === M E T H O D S ======================================================= */

void
OptimizerPixelProcessorSplit::run(const RunParams& params) const
{
	TaskPixelProcessor::Handle pixelprocessor = TaskPixelProcessor::Handle::cast_dynamic(params.ref_task);
	if ( pixelprocessor
	  && pixelprocessor->target_surface
	  && pixelprocessor->sub_task()
	  && pixelprocessor->sub_task()->target_surface
	  && !pixelprocessor->is_affects_transparent() )
	{
		if (TaskList::Handle list = TaskList::Handle::cast_dynamic(pixelprocessor->sub_task()))
		{
			// try to find dedicated groups
			std::vector<RectInt> groups;
			for(Task::List::const_iterator i = list->sub_tasks.begin(); i != list->sub_tasks.end(); ++i)
				if (*i && (*i)->valid_target())
					groups.push_back((*i)->get_target_rect());

			#ifndef	NDEBUG
			int sub_tasks_count = (int)groups.size();
			#endif

			bool retry = true;
			while(retry)
			{
				retry = false;
				for(int i = 0; i < (int)groups.size(); ++i)
				{
					for(int j = i+1; j < (int)groups.size(); ++j)
					{
						if (etl::intersect(groups[i], groups[j]))
						{
							etl::set_union(groups[i], groups[i], groups[j]);
							groups.erase(groups.begin() + j);
							--j;
							retry = true;
						}
					}
				}
			}

			// split task
			if (groups.size() > 1)
			{
				// create list
				TaskList::Handle list;
				list = new TaskList();
				assign(list, Task::Handle(pixelprocessor));
				list->sub_tasks.clear();

				#ifndef	NDEBUG
				Task::Set processed;
				#endif

				// fill list
				for(int j = 0; j < (int)groups.size(); ++j)
				{
					// create sub-pixelprocessor
					TaskList::Handle sub_list = TaskList::Handle::cast_dynamic(pixelprocessor->sub_task()->clone());
					sub_list->sub_tasks.clear();
					sub_list->trunc_target_rect(groups[j]);

					RectInt rect = groups[j]
								 + pixelprocessor->get_target_offset()
								 + pixelprocessor->get_offset();
					TaskPixelProcessor::Handle sub_pixelprocesor = TaskPixelProcessor::Handle::cast_dynamic(pixelprocessor->clone());
					sub_pixelprocesor->trunc_target_rect(rect);
					sub_pixelprocesor->sub_task() = sub_list;

					list->sub_tasks.push_back(sub_pixelprocesor);

					// fill list of sub-pixelprocesor
					for(Task::List::const_iterator i = list->sub_tasks.begin(); i != list->sub_tasks.end(); ++i)
						if (*i && (*i)->valid_target() && etl::contains(groups[j], (*i)->get_target_rect()))
						{
							#ifndef	NDEBUG
							assert(processed.count(*i) == 0);
							processed.insert(*i);
							#endif
							sub_list->sub_tasks.push_back(*i);
						}

					// optimization for list with single task
					if (sub_list->sub_tasks.size() == 1)
						sub_pixelprocesor->sub_task() = sub_list->sub_tasks[0];
				}

				#ifndef	NDEBUG
				assert((int)processed.size() == sub_tasks_count);
				#endif

				apply(params, list);
			}
		}
	}
}

/* === E N T R Y P O I N T ================================================= */
