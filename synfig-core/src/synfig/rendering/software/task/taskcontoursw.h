/* === S Y N F I G ========================================================= */
/*!	\file synfig/rendering/software/task/taskcontoursw.h
**	\brief TaskContourSW Header
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

#ifndef __SYNFIG_RENDERING_TASKCONTOURSW_H
#define __SYNFIG_RENDERING_TASKCONTOURSW_H

/* === H E A D E R S ======================================================= */

#include <synfig/surface.h>

#include "tasksw.h"
#include "../../common/task/taskcontour.h"
#include "../../common/task/taskcomposite.h"
#include "../../common/task/tasksplittable.h"
#include "../../primitive/contour.h"
#include "../../primitive/polyspan.h"

/* === M A C R O S ========================================================= */

/* === T Y P E D E F S ===================================================== */

/* === C L A S S E S & S T R U C T S ======================================= */

namespace synfig
{
namespace rendering
{

class TaskContourSW: public TaskContour, public TaskSW, public TaskComposite, public TaskSplittable
{
public:
	typedef etl::handle<TaskContourSW> Handle;
	Task::Handle clone() const { return clone_pointer(this); }
	virtual void split(const RectInt &sub_target_rect);
	virtual bool run(RunParams &params) const;

	virtual Rect calc_bounds() const { return Rect::infinite(); }

	virtual Color::BlendMethodFlags get_supported_blend_methods() const
		{ return Color::BLEND_METHODS_ALL & ~Color::BLEND_METHODS_STRAIGHT; }
};

} /* end namespace rendering */
} /* end namespace synfig */

/* -- E N D ----------------------------------------------------------------- */

#endif
