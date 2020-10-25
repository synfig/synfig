/* === S Y N F I G ========================================================= */
/*!	\file synfig/rendering/common/task/tasklayer.h
**	\brief TaskLayer Header
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

/* === S T A R T =========================================================== */

#ifndef __SYNFIG_RENDERING_TASKLAYER_H
#define __SYNFIG_RENDERING_TASKLAYER_H

/* === H E A D E R S ======================================================= */

#include "../../task.h"

#include <synfig/layer.h>

/* === M A C R O S ========================================================= */

/* === T Y P E D E F S ===================================================== */

/* === C L A S S E S & S T R U C T S ======================================= */

namespace synfig
{
namespace rendering
{

class TaskLayer: public Task
{
public:
	typedef etl::handle<TaskLayer> Handle;
	static Token token;
	virtual Token::Handle get_token() const { return token.handle(); }

	Layer::Handle layer;

	const Task::Handle& sub_task() const { return Task::sub_task(0); }
	Task::Handle& sub_task() { return Task::sub_task(0); }

	virtual Rect calc_bounds() const;
	virtual void set_coords_sub_tasks();

private:
	static bool renddesc_less(const RendDesc &a, const RendDesc &b);
};

} /* end namespace rendering */
} /* end namespace synfig */

/* -- E N D ----------------------------------------------------------------- */

#endif
