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

private:
	mutable Rect bounds;

	Point source_rect_lt;
	Point source_rect_rb;
	RectInt target_rect;

protected:
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

public:
	Surface::Handle target_surface;
	List sub_tasks;

	mutable int index;
	mutable int deps_count;
	mutable Set back_deps;

	mutable RunParams params;
	mutable bool success;


	Task(): index(), deps_count(0), success(true) { }
	virtual ~Task();


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


	//! calls from update_bounds()
	virtual Rect calc_bounds() const { return Rect::infinite(); }
	//! use OptimizerCalcBounds and to avoid multiple calculation of bounds of same task
	void update_bounds() const { bounds = calc_bounds(); }
	const Rect& get_bounds() const { return bounds; }


	void init_target_rect(const RectInt &target_rect, const Point &source_rect_lt, const Point &source_rect_rb);
	void clear_target_rect();


	const RectInt& get_target_rect() const { return target_rect; }
	const VectorInt get_target_offset() const { return target_rect.get_min(); }
	const Point& get_source_rect_lt() const { return source_rect_lt; }
	const Point& get_source_rect_rb() const { return source_rect_rb; }
	Vector get_pixels_per_unit() const;
	Vector get_units_per_pixel() const;

	void trunc_target_rect(const RectInt &rect);
	void trunc_source_rect(const Rect &rect);
	void trunc_source_rect(const Point &lt, const Point &rb);
	void trunc_target_by_bounds();

	void move_target_rect(const VectorInt &offset);
	void set_target_origin(const VectorInt &origin);

	bool valid_target_surface() const
	{
		return target_surface
			&& !target_surface->empty();
	}

	bool valid_target_rect() const
	{
		return target_rect.valid()
			&& !source_rect_lt.is_nan_or_inf()
			&& !source_rect_rb.is_nan_or_inf()
			&& fabs(source_rect_rb[0] - source_rect_lt[0]) >= 1e-10
			&& fabs(source_rect_rb[1] - source_rect_lt[1]) >= 1e-10;
	}

	bool valid_target() const
	{
		return valid_target_surface()
			&& valid_target_rect()
			&& etl::contains(RectInt(VectorInt::zero(), target_surface->get_size()), target_rect);
	}

	bool check() const
	{
		if ( valid_target_surface()
		  && valid_target_rect()
		  && !etl::contains(RectInt(VectorInt::zero(), target_surface->get_size()), target_rect) )
			return false;

		return true;
	}

	virtual bool run(RunParams &params) const;
	virtual Task::Handle clone() const { return clone_pointer(this); }

	Task::Handle clone_recursive() const;
};

} /* end namespace rendering */
} /* end namespace synfig */

/* -- E N D ----------------------------------------------------------------- */

#endif
