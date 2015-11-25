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

		//! Creates RunParams structure for sub-task
		RunParams sub(const Task::Handle &task) const
			{ return RunParams(renderer, list, depends_from, task, this); }
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

	static void apply_target_bounds(Task &task, const RectInt &target_bounds)
	{
		RectInt tr = task.target_rect;
		if (tr.valid())
		{
			if (target_bounds.valid())
			{
				RectInt ntr = target_bounds;
				etl::set_intersect(ntr, ntr, tr);
				if (ntr.valid())
				{
					Vector lt = task.source_rect_lt;
					Vector rb = task.source_rect_rb;
					Vector k( (rb[0] - lt[0])/(Real)(tr.maxx - tr.minx),
							  (rb[1] - lt[1])/(Real)(tr.maxy - tr.miny) );
					task.source_rect_lt[0] = (Real)(ntr.minx - tr.minx)*k[0] + lt[0];
					task.source_rect_lt[1] = (Real)(ntr.miny - tr.miny)*k[1] + lt[1];
					task.source_rect_rb[0] = (Real)(ntr.maxx - tr.minx)*k[0] + lt[0];
					task.source_rect_rb[1] = (Real)(ntr.maxy - tr.miny)*k[1] + lt[1];
					task.target_rect = ntr;
					return;
				}
			}
			task.source_rect_lt = task.source_rect_rb = Vector::zero();
			task.target_rect = RectInt::zero();
		}
	}

	static void apply_source_bounds(Task &task, const Rect &source_bounds)
	{
		RectInt tr = task.target_rect;
		if (tr.valid())
		{
			Rect nsb = source_bounds;
			if (nsb.valid())
			{
				Vector lt = task.source_rect_lt;
				Vector rb = task.source_rect_rb;
				Vector nlt( std::min(std::max(lt[0], nsb.minx), nsb.maxx),
							std::min(std::max(lt[1], nsb.miny), nsb.maxy) );
				Vector nrb( std::min(std::max(rb[0], nsb.minx), nsb.maxx),
							std::min(std::max(rb[1], nsb.miny), nsb.maxy) );
				if (nlt[0] != nrb[0] && nlt[1] != nrb[1])
				{
					Vector k(  (Real)(tr.maxx - tr.minx)/(rb[0] - lt[0]),
							   (Real)(tr.maxy - tr.miny)/(rb[1] - lt[1]) );
					Vector t0( (nlt[0] - lt[0])*k[0] + tr.minx,
							   (nlt[1] - lt[1])*k[1] + tr.miny );
					Vector t1( (nrb[0] - lt[0])*k[0] + tr.minx,
							   (nrb[1] - lt[1])*k[1] + tr.miny );
					if (t1[0] < t0[0]) std::swap(t1[0], t0[0]);
					if (t1[1] < t0[1]) std::swap(t1[1], t0[1]);

					const Real e = 1e-6;
					RectInt ntr( (int)floor(t0[0] + e),
							     (int)floor(t0[1] + e),
								 (int)ceil (t1[0] - e),
								 (int)ceil (t1[1] - e) );
					apply_target_bounds(task, ntr);
					return;
				}
			}
			task.source_rect_lt = task.source_rect_rb = Vector::zero();
			task.target_rect = RectInt::zero();
		}
	}

	static void apply_source_bounds(Task &task)
		{ apply_source_bounds(task, task.bounds); }

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
			task->source_rect_lt = rect_lt;
			task->source_rect_rb = rect_rb;
			task->target_rect = target_rect;
			apply_source_bounds(*task);
		}
	}

	template<typename T>
	static void assign_surface(Task::Handle &task, const Task::Handle &parent)
	{
		if (task && parent && parent->target_surface)
			assign_surface<T>(
				task,
				parent->target_rect.maxx - parent->target_rect.minx,
				parent->target_rect.maxy - parent->target_rect.miny,
				parent->source_rect_lt,
				parent->source_rect_rb,
				RectInt( 0, 0,
					parent->target_rect.maxx - parent->target_rect.minx,
					parent->target_rect.maxy - parent->target_rect.miny ));
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
		{ *(TT*)dest.get() = *src; apply_source_bounds(*dest); }

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
