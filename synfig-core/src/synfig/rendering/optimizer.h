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
		CATEGORY_ID_COMMON,		     //! common optimizations of task-tree
		CATEGORY_ID_PRE_SPECIALIZE,  //! common optimizations, when transformations already applied
		CATEGORY_ID_SPECIALIZE,      //! renderer-specified optimizations of task-tree
		CATEGORY_ID_POST_SPECIALIZE, //! optimizations of task-tree, which required assigned surfaces
		CATEGORY_ID_CONVERT,	     //! OptimizerSurfaceConvert
		CATEGORY_ID_LINEAR,		     //! OptimizerLinear
		CATEGORY_ID_LIST		     //! optimizations of plain (linear) list of tasks
	};

	enum
	{
		CATEGORY_ID_COUNT        = CATEGORY_ID_LIST + 1,
		CATEGORY_COMMON          = 1 << CATEGORY_ID_COMMON,          //! --
		CATEGORY_PRE_SPECIALIZE  = 1 << CATEGORY_ID_PRE_SPECIALIZE,  //! --
		CATEGORY_SPECIALIZE      = 1 << CATEGORY_ID_SPECIALIZE,      //! --
		CATEGORY_POST_SPECIALIZE = 1 << CATEGORY_ID_POST_SPECIALIZE, //! --
		CATEGORY_CONVERT         = 1 << CATEGORY_ID_CONVERT,         //! --
		CATEGORY_LINEAR          = 1 << CATEGORY_ID_LINEAR,          //! --
		CATEGORY_LIST            = 1 << CATEGORY_ID_LIST,            //! --
		CATEGORY_TREE            = CATEGORY_LINEAR - 1,              //! optimizations of task-tree
		CATEGORY_ALL             = (1 << CATEGORY_ID_COUNT) -1       //! all optimizations
	};

	enum
	{
		MODE_NONE          = 0,	//! do nothing
		MODE_REPEAT_LAST   = 1, //! repeat optimization for current task
		MODE_REPEAT_PARENT = 3, //! repeat optimization for parent task (includes MODE_REPEAT_LAST)
		MODE_REPEAT_BRUNCH = 7, //! repeat optimization for each of parent tasks (includes MODE_REPEAT_PARENT and MODE_REPEAT_LAST)
								//! in sequence: current, parent, parent-of-parent, etc
		MODE_RECURSIVE     = 8,	//! uses with MODE_REPEAT_XXX, and tells what
		                        //! repeating should be done with recursive call of subtasks
	};

	struct CategoryInfo
	{
		//! if set then run all optimizers for each task, else run each optimizer for all tasks
		//! true:  optimizer1(taskA), optimizer2(taskA), optimizer1(taskB), optimizer2(taskB)
		//! false: optimizer1(taskA), optimizer1(taskB), optimizer2(taskA), optimizer2(taskB)
		bool simultaneous_run;
		CategoryInfo(bool simultaneous_run): simultaneous_run(simultaneous_run) { }
	};

	struct RunParams
	{
		//! current renderer
		const Renderer &renderer;

		//! List of tasks for optimization,
		//! (see Optimizer::for_list)
		Task::List &list;

		const Category depends_from;

		//! Parent optimization params.
		//! Optimizer can read parent tasks via this field
		const RunParams * const parent;

		const Task::Handle orig_task;

		//! Task for optimization, optimizer may replace or remove (make null) it,
		//! (see Optimizer::for_task and Optimizer::for_root_task)
		mutable Task::Handle ref_task;
		//! Optimizer may mark dirty some set of categories, these categories should be reran
		mutable Category ref_affects_to;
		//! Optimizer may affect to optimization sequence.
		//! For example, initiate reoptimization of current task.
		//! (see Optimizater::MODE_XXX)
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
			orig_task(task),
			ref_task(task),
			ref_affects_to(),
			ref_mode()
		{ }

		RunParams(const RunParams &other):
			renderer(other.renderer),
			list(other.list),
			depends_from(other.depends_from),
			parent(other.parent),
			orig_task(other.orig_task),
			ref_task(other.ref_task),
			ref_affects_to(),
			ref_mode()
		{ }

		//! Creates RunParams structure for sub-task
		RunParams sub(const Task::Handle &task) const
			{ return RunParams(renderer, list, depends_from, task, this); }

		const RunParams& root() const
			{ return parent ? parent->root() : *this; }
		int get_current_level_index() const
			{ return parent ? parent->get_current_level_index() + 1 : 0; }
		const RunParams* get_parent(int index) const
		{
			return index == 0 ? this
				 : index < 0 || !parent ? NULL
				 : parent->get_parent(index - 1);
		}
		const RunParams* get_level(int index) const
			{ return get_parent(get_current_level_index() - index); }
	};

	static const CategoryInfo categories_info[CATEGORY_ID_COUNT];

	//! Category of this optimizer,
	//! see enum Optimizer::CategoryId (CATEGORY_ID_XXX)
	CategoryId category_id;
	//! Set of categories of optimizers which should be complete before run this optimizer,
	//! see Optimizer::CATEGORY_XXX enumerations
	Category depends_from;
	//! Set of categories of optimizers which should be processed again when this optimizer applied,
	//! see Optimizer::CATEGORY_XXX enumerations
	Category affects_to;
	//! Mode determines what shoud do optimization system when this optimizer applied,
	//! see Optimizer::MODE_XXXX enumerations
	Mode mode;
	//! Optimizer uses for list of tasks
	bool for_list;
	//! Optimizer uses for individual tasks
	bool for_task;
	//! Optimizer uses for individual tasks, but root nodes of the list only
	bool for_root_task;
	//! Optimizer runs for task after all of sub-tasks are processed
	bool deep_first;


	Optimizer(): category_id(), depends_from(), affects_to(), mode(), for_list(), for_task(), for_root_task(), deep_first() { }
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
		const Vector& rect_lt, const Vector& rect_rb,
		const RectInt &target_rect )
	{
		if (task && !task->target_surface)
		{
			task = task->clone();
			task->target_surface = new T();
			task->target_surface->is_temporary = true;
			task->target_surface->set_size(width, height);
			task->init_target_rect(target_rect, rect_lt, rect_rb);
			assert( task->check() );
			task->trunc_target_by_bounds();
		}
	}

	template<typename T>
	static void assign_surface(Task::Handle &task, const Task::Handle &parent)
	{
		if (task && parent && parent->target_surface)
			assign_surface<T>(
				task,
				parent->get_target_rect().maxx - parent->get_target_rect().minx,
				parent->get_target_rect().maxy - parent->get_target_rect().miny,
				parent->get_source_rect_lt(),
				parent->get_source_rect_rb(),
				RectInt( 0, 0,
					parent->get_target_rect().maxx - parent->get_target_rect().minx,
					parent->get_target_rect().maxy - parent->get_target_rect().miny ));
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
		{ *(TT*)dest.get() = *src; dest->trunc_target_by_bounds(); }

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
