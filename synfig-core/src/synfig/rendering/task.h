/* === S Y N F I G ========================================================= */
/*!	\file synfig/rendering/task.h
**	\brief Task Header
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

#ifndef __SYNFIG_RENDERING_TASK_H
#define __SYNFIG_RENDERING_TASK_H

/* === H E A D E R S ======================================================= */

#include <vector>
#include <set>

#include <synfig/rect.h>
#include <synfig/vector.h>

#include "surface.h"

/* === M A C R O S ========================================================= */

/* === T Y P E D E F S ===================================================== */

/* === C L A S S E S & S T R U C T S ======================================= */

namespace synfig
{
namespace rendering
{

class Task: public etl::shared_object
{
private:
	static const etl::handle<Task> blank;

public:
	typedef etl::handle<Task> Handle;
	typedef std::vector<Handle> List;
	typedef std::set<Handle> Set;

	struct RunParams { };

	Surface::Handle target_surface;
	synfig::Point source_rect_lt;
	synfig::Point source_rect_rb;
	RectInt target_rect;
	List sub_tasks;

	mutable Rect bounds;
	mutable Set back_deps;
	mutable RunParams params;
	mutable int index;
	mutable int deps_count;
	mutable bool success;

	Task(): target_rect(0, 0), bounds(0.0, 0.0), index(), deps_count(0), success(true) { }

	template<typename T>
	static T* clone_pointer(const T *task)
	{
		if (!task) return NULL;
		T *t(new T());
		*t = *task;
		return t;
	}

	template<typename T>
	static etl::handle<T> clone(const etl::handle<T> &task)
		{ return clone_pointer(task.get()); }

	Task::Handle& sub_task(int index)
	{
		assert(index >= 0);
		if (index >= (int)sub_tasks.size())
			sub_tasks.resize(index + 1);
		return sub_tasks[index];
	}

	const Task::Handle& sub_task(int index) const
	{
		assert(index >= 0);
		return index < (int)sub_tasks.size() ? sub_tasks[index] : blank;
	}

	Vector get_pixels_per_unit() const
	{
		if (!target_rect.valid())
			return Vector();
		return Vector(
			fabs(source_rect_rb[0] - source_rect_lt[0]) < 1e-10 ? 0.0 :
				(Real)(target_rect.maxx - target_rect.minx)/(source_rect_rb[0] - source_rect_lt[0]),
			fabs(source_rect_rb[1] - source_rect_lt[1]) < 1e-10 ? 0.0 :
				(Real)(target_rect.maxy - target_rect.miny)/(source_rect_rb[1] - source_rect_lt[1]) );
	}

	Vector get_units_per_pixel() const
	{
		if ( !target_rect.valid()
		  || fabs(source_rect_rb[0] - source_rect_lt[0]) < 1e-10
		  || fabs(source_rect_rb[1] - source_rect_lt[1]) < 1e-10 )
			return Vector();
		return Vector(
			fabs(source_rect_rb[0] - source_rect_lt[0]) < 1e-10 ? 0.0 :
				(source_rect_rb[0] - source_rect_lt[0])/(Real)(target_rect.maxx - target_rect.minx),
			fabs(source_rect_rb[1] - source_rect_lt[1]) < 1e-10 ? 0.0 :
				(source_rect_rb[1] - source_rect_lt[1])/(Real)(target_rect.maxy - target_rect.miny) );
	}

	virtual ~Task();
	virtual bool run(RunParams &params) const;
	virtual Task::Handle clone() const { return clone_pointer(this); }

	// use OptimizerCalcBounds and field 'bounds'
	// to avoid multiple calculation of bounds of same task
	virtual Rect calc_bounds() const { return Rect::infinite(); }
};

} /* end namespace rendering */
} /* end namespace synfig */

/* -- E N D ----------------------------------------------------------------- */

#endif
