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
	typedef etl::handle<Optimizer> Handle;
	typedef std::vector<Handle> List;

	struct RunParams {
		const Renderer &renderer;
		Task::List &list;
		const Task::Handle task;
		const RunParams * const parent;

		mutable Task::Handle out_task;

		RunParams(
			const Renderer &renderer,
			Task::List &list,
			const Task::Handle &task = Task::Handle(),
			const RunParams *parent = NULL
		): renderer(renderer), list(list), task(task), parent(parent) { }

		RunParams sub(const Task::Handle &task) const
			{ return RunParams(renderer, list, task, this); }
	};

	virtual ~Optimizer();
	virtual bool run(const RunParams &params) const = 0;

	template<typename T>
	static void assign_surface(
		Task::Handle &task,
		int width, int height,
		const Vector& rect_lt, const Vector& rect_rb )
	{
		if (task && !task->target_surface)
		{
			task = task->clone();
			task->target_surface = new T();
			task->target_surface->is_temporary = true;
			task->target_surface->set_size(width, height);
			task->rect_lt = rect_lt;
			task->rect_rb = rect_rb;
		}
	}

	template<typename T>
	static void assign_surface(
		Task::Handle &task,
		const std::pair<int, int> &size,
		const Vector& rect_lt, const Vector& rect_rb )
	{
		assign_surface<T>(task, size.first, size.second, rect_lt, rect_rb);
	}

	template<typename T>
	static void assign_surface(Task::Handle &task, const Task::Handle &parent)
	{
		if (task && parent && parent->target_surface)
			assign_surface<T>(
				task,
				parent->target_surface->get_size(),
				parent->rect_lt,
				parent->rect_rb );
	}

	template<typename T>
	static void assign_surfaces(const Task::Handle &parent)
	{
		if (parent && parent->target_surface)
		{
			for(Task::List::iterator i = parent->sub_tasks.begin(); i != parent->sub_tasks.end(); ++i)
				assign_surface<T>(*i, parent);
		}
	}

	template<typename T, typename TT>
	static void assign(const etl::handle<T> &dest, const etl::handle<TT> &src)
		{ *(TT*)dest.get() = *src; }

	template<typename SurfaceType, typename T, typename TT>
	static void assign_all(const etl::handle<T> &dest, const etl::handle<TT> &src)
		{ assign(dest, src); assign_surfaces<SurfaceType>(dest); }

	template<typename T, typename TT>
	static void init_and_assign(etl::handle<T> &dest, const etl::handle<TT> &src)
		{ dest = new T(); assign(dest, src); }

	template<typename SurfaceType, typename T, typename TT>
	static void init_and_assign_all(etl::handle<T> &dest, const etl::handle<TT> &src)
		{ dest = new T(); assign_all<SurfaceType>(dest, src); }

	template<typename T, typename TT>
	static const etl::handle<T> create_and_assign(const etl::handle<TT> &src)
		{ const etl::handle<T> dest = new T(); assign(dest, src); return dest; }

	template<typename SurfaceType, typename T, typename TT>
	static const etl::handle<T> create_and_assign_all(const etl::handle<TT> &src)
		{ const etl::handle<T> dest = new T(); assign_all<SurfaceType>(dest, src); return dest; }
};

} /* end namespace rendering */
} /* end namespace synfig */

/* -- E N D ----------------------------------------------------------------- */

#endif
