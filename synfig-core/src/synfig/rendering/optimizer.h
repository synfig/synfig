/* === S Y N F I G ========================================================= */
/*!	\file synfig/rendering/optimizer.h
**	\brief Optimizer Header
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

#ifndef __SYNFIG_RENDERING_OPTIMIZER_H
#define __SYNFIG_RENDERING_OPTIMIZER_H

#include "task.h"

/* === M A C R O S ========================================================= */

/* === T Y P E D E F S ===================================================== */

/* === C L A S S E S & S T R U C T S ======================================= */

namespace synfig
{
namespace rendering
{

class Renderer;

class Optimizer: public etl::shared_object
{
public:
	etl::handle<Optimizer> Handle;
	typedef std::vector<Handle> List;

	struct RunParams {
		const Renderer &renderer;
		Task::List &list;
		Task::Handle task;
		RunParams(
			const Renderer &renderer,
			Task::List &list,
			const Task::Handle &task = Task::Handle()
		): renderer(renderer), list(list), task(task) { }
	};

	virtual ~Optimizer();
	virtual bool run(const RunParams &params) const = 0;
};

} /* end namespace rendering */
} /* end namespace synfig */

/* -- E N D ----------------------------------------------------------------- */

#endif
