/* === S Y N F I G ========================================================= */
/*!	\file synfig/rendering/common/optimizer/optimizersplit.cpp
**	\brief OptimizerSplit
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

#include "optimizersplit.h"

#endif

using namespace synfig;
using namespace rendering;

/* === M A C R O S ========================================================= */

/* === G L O B A L S ======================================================= */

/* === P R O C E D U R E S ================================================= */

/* === M E T H O D S ======================================================= */

OptimizerSplit::OptimizerSplit()
{
	category_id = CATEGORY_ID_LIST;
	depends_from = CATEGORY_SPECIALIZED;
	for_list = true;
}

void
OptimizerSplit::run(const RunParams &params) const
{
	if (!params.list) return;
	const int min_area = 256*256;
	for(Task::List::iterator i = params.list->begin(); i != params.list->end(); ++i)
	{
		if (TaskInterfaceSplit *split = i->type_pointer<TaskInterfaceSplit>())
		if (split->is_splittable())
		{
			RectInt r = (*i)->target_rect;
			int w = r.maxx - r.minx;
			int h = r.maxy - r.miny;
			int t = std::min(h/10, w*h/min_area);
			if (t >= 2)
			{
				int hh = h/t;
				int y = r.miny;
				for(int j = 1; j < t; ++j, y += hh)
				{
					Task::Handle task = (*i)->clone();
					task->trunc_target_rect( RectInt(r.minx, y, r.maxx, y + hh) );
					i = params.list->insert(i, task);
					++i;
				}
				*i = (*i)->clone();
				(*i)->trunc_target_rect( RectInt(r.minx, y, r.maxx, r.maxy) );
				apply(params);
			}
		}
	}
}

/* === E N T R Y P O I N T ================================================= */
