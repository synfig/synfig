/* === S Y N F I G ========================================================= */
/*!	\file synfig/rendering/common/optimizer/optimizerlist.h
**	\brief OptimizerList Header
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

/* === S T A R T =========================================================== */

#ifndef __SYNFIG_RENDERING_OPTIMIZERLIST_H
#define __SYNFIG_RENDERING_OPTIMIZERLIST_H

/* === H E A D E R S ======================================================= */

#include "../../optimizer.h"

/* === M A C R O S ========================================================= */

/* === T Y P E D E F S ===================================================== */

/* === C L A S S E S & S T R U C T S ======================================= */

namespace synfig
{
namespace rendering
{

class TaskList;

//! Optimizer runs for task.
//! OptimizerList transforms TaskList, which contains other TaskList
//! into linear (single level) list.
class OptimizerList: public Optimizer
{
private:
	void clone_list(const RunParams &params, Task::List::iterator &i, etl::handle<TaskList> &list) const;

public:
	OptimizerList()
	{
		category_id = CATEGORY_ID_POST_SPECIALIZE;
		depends_from = CATEGORY_SPECIALIZE;
		mode = MODE_REPEAT_PARENT;
		deep_first = true;
		for_task = true;
	}

	virtual void run(const RunParams &params) const;
};

} /* end namespace rendering */
} /* end namespace synfig */

/* -- E N D ----------------------------------------------------------------- */

#endif
