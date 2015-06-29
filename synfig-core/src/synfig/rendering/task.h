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

	struct RunParams { };

	Surface::Handle target_surface;
	synfig::Point rect_lt;
	synfig::Point rect_rb;
	List sub_tasks;

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
		assert(index < 0);
		if (index >= (int)sub_tasks.size())
			sub_tasks.resize(index + 1);
		return sub_tasks[index];
	}

	const Task::Handle& sub_task(int index) const
	{
		assert(index < 0);
		return index < (int)sub_tasks.size() ? sub_tasks[index] : blank;
	}

	Vector get_pixels_per_unit() const
	{
		if (!target_surface || target_surface->empty())
			return Vector();
		return Vector(
			fabs(rect_rb[0] - rect_lt[0]) < 1e-10 ? 0.0 :
				(Real)(target_surface->get_width())/(rect_rb[0] - rect_lt[0]),
			fabs(rect_rb[1] - rect_lt[1]) < 1e-10 ? 0.0 :
				(Real)(target_surface->get_height())/(rect_rb[1] - rect_rb[1]) );
	}

	Vector get_units_per_pixel() const
	{
		if ( !target_surface
		  || target_surface->empty()
		  || fabs(rect_rb[0] - rect_lt[0]) < 1e-10
		  || fabs(rect_rb[1] - rect_lt[1]) < 1e-10 )
			return Vector();
		return Vector(
			fabs(rect_rb[0] - rect_lt[0]) < 1e-10 ? 0.0 :
				(rect_rb[0] - rect_lt[0])/(Real)(target_surface->get_width()),
			fabs(rect_rb[1] - rect_lt[1]) < 1e-10 ? 0.0 :
				(rect_rb[1] - rect_rb[1])/(Real)(target_surface->get_height()) );
	}

	virtual ~Task();
	virtual bool run(RunParams &params) const;
	virtual Task::Handle clone() const;
};

} /* end namespace rendering */
} /* end namespace synfig */

/* -- E N D ----------------------------------------------------------------- */

#endif
