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
	typedef unsigned int Category;
	typedef unsigned int Mode;

	enum CategoryId
	{
		CATEGORY_ID_COMMON,		     // common optimizations of task-tree
		CATEGORY_ID_SPECIALIZE,      // renderer-specified optimizations of task-tree
		CATEGORY_ID_POST_SPECIALIZE, // optimizations of task-tree, which required assigned surfaces
		CATEGORY_ID_CONVERT,	     // OptimizerSurfaceConvert
		CATEGORY_ID_LINEAR,		     // OptimizerLinear
		CATEGORY_ID_LIST		     // optimizations of plain (linear) list of tasks
	};

	enum
	{
		CATEGORY_ID_COUNT = CATEGORY_ID_LIST + 1,
		CATEGORY_COMMON     = 1 << CATEGORY_ID_COMMON,		// --
		CATEGORY_SPECIALIZE = 1 << CATEGORY_ID_SPECIALIZE,	// --
		CATEGORY_CONVERT    = 1 << CATEGORY_ID_CONVERT,		// --
		CATEGORY_LINEAR     = 1 << CATEGORY_ID_LINEAR,		// --
		CATEGORY_LIST       = 1 << CATEGORY_ID_LIST,		// --
		CATEGORY_TREE       = CATEGORY_LINEAR - 1, 			// optimizations of task-tree
		CATEGORY_ALL        = (1 << CATEGORY_ID_COUNT) -1		// all optimizations
	};

	enum
	{
		MODE_NONE = 0,
		MODE_REPEAT_LAST = 1,
		MODE_REPEAT_BRUNCH = 2,
		MODE_REPEAT = MODE_REPEAT_LAST | MODE_REPEAT_BRUNCH
	};

	struct CategoryInfo
	{
		bool simultaneous_run;
		CategoryInfo(bool simultaneous_run): simultaneous_run(simultaneous_run) { }
	};

	struct RunParams
	{
		const Renderer &renderer;
		Task::List &list;
		const Category depends_from;
		const RunParams * const parent;

		mutable Task::Handle ref_task;
		mutable Category ref_affects_to;
		mutable Mode ref_mode;

		RunParams(
			const Renderer &renderer,
			Task::List &list,
			Category depends_from,
			const Task::Handle &task = Task::Handle(),
			const RunParams *parent = NULL
		):
			renderer(renderer),
			list(list),
			depends_from(depends_from),
			parent(parent),
			ref_task(task),
			ref_affects_to(),
			ref_mode()
		{ }

		RunParams(const RunParams &other):
			renderer(other.renderer),
			list(other.list),
			depends_from(other.depends_from),
			parent(other.parent),
			ref_task(other.ref_task),
			ref_affects_to(),
			ref_mode()
		{ }

		RunParams sub(const Task::Handle &task) const
			{ return RunParams(renderer, list, depends_from, task, this); }
	};

	static const CategoryInfo categories_info[CATEGORY_ID_COUNT];

	CategoryId category_id;
	Category depends_from;
	Category affects_to;
	Mode mode;
	bool for_list;
	bool for_task;
	bool for_root_task;

	Optimizer(): category_id(), depends_from(), affects_to(), mode(), for_list(), for_task(), for_root_task() { }
	virtual ~Optimizer();

	virtual void run(const RunParams &params) const = 0;

	void apply(const RunParams &params) const
	{
		params.ref_affects_to |= affects_to;
		params.ref_mode |= mode;
	}

	void apply(const RunParams &params, const Task::Handle &task) const
	{
		apply(params);
		params.ref_task = task;
	}

	void apply_clone(const RunParams &params) const
	{
		apply(params, params.ref_task->clone());
	}

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
