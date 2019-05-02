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
		CATEGORY_ID_BEGIN,		 //! optimizations of pure input tasks-tree
		CATEGORY_ID_COORDS,      //! optimizations of tasks-tree with calculated coordinates and resolution
		CATEGORY_ID_SPECIALIZED, //! optimizations of tree of specialized tasks
		CATEGORY_ID_LIST		 //! optimizations of plain (linear) list of tasks
	};

	enum
	{
		CATEGORIES_COUNT         = CATEGORY_ID_LIST + 1,

		CATEGORY_BEGIN           = 1 << CATEGORY_ID_BEGIN,
		CATEGORY_COORDS          = 1 << CATEGORY_ID_COORDS,
		CATEGORY_SPECIALIZED     = 1 << CATEGORY_ID_SPECIALIZED,
		CATEGORY_LIST            = 1 << CATEGORY_ID_LIST,

		CATEGORY_ALL_TREE        = CATEGORY_BEGIN
		                         | CATEGORY_COORDS
								 | CATEGORY_SPECIALIZED,
		CATEGORY_ALL_SPECIALIZED = CATEGORY_SPECIALIZED
								 | CATEGORY_LIST,
		CATEGORY_ALL             = (1 << CATEGORIES_COUNT) - 1
	};

	enum
	{
		MODE_NONE          = 0,	//! do nothing
		MODE_REPEAT_LAST   = 1, //! repeat optimization for current task
		MODE_REPEAT_PARENT = 3, //! repeat optimization for parent task (includes MODE_REPEAT_LAST)
		MODE_REPEAT_BRANCH = 7, //! repeat optimization for each of parent tasks (includes MODE_REPEAT_PARENT and MODE_REPEAT_LAST)
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
		Category depends_from;

		//! List of tasks for optimization,
		//! (see Optimizer::for_list)
		Task::List *list;

		//! Parent optimization params.
		//! Optimizer can read parent tasks via this field
		const RunParams * parent;

		Task::Handle orig_task;

		//! Task for optimization, optimizer may replace or remove (make null) it,
		//! (see Optimizer::for_task and Optimizer::for_root_task)
		mutable Task::Handle ref_task;
		//! Optimizer may mark dirty some set of categories, these categories should be reran
		mutable Category ref_affects_to;
		//! Optimizer may affect to optimization sequence.
		//! For example, initiate reoptimization of current task.
		//! (see Optimizater::MODE_XXX)
		mutable Mode ref_mode;

		RunParams():
			depends_from(),
			list(),
			parent(),
			ref_affects_to(),
			ref_mode()
		{ }

		// for list optimizers
		RunParams(
			Category depends_from,
			Task::List &list
		):
			depends_from(depends_from),
			list(&list),
			parent(),
			ref_affects_to(),
			ref_mode()
		{ }

		// for task optimizers
		RunParams(
			Category depends_from,
			const Task::Handle &task,
			Task::List *list
		):
			depends_from(depends_from),
			list(list),
			parent(),
			orig_task(task),
			ref_task(task),
			ref_affects_to(),
			ref_mode()
		{ }

		// sub-params for task optimizers
		RunParams(
			const RunParams *parent,
			const Task::Handle &task
		):
			depends_from(parent->depends_from),
			list(parent->list),
			parent(parent),
			orig_task(task),
			ref_task(task),
			ref_affects_to(),
			ref_mode()
		{ }

		//! Creates RunParams structure for sub-task
		RunParams sub(const Task::Handle &task) const
			{ return RunParams(this, task); }

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


	static const CategoryInfo categories_info[CATEGORIES_COUNT];

	//! Category of this optimizer,
	//! see enum Optimizer::CategoryId (CATEGORY_ID_XXX)
	CategoryId category_id;
	//! Determines the order of optimizers execution inside category
	Real order;
	//! Determines the order of optimizers execution inside category when order fields are equal
	long long index;
	//! Set of categories of optimizers which should be complete before run this optimizer,
	//! see Optimizer::CATEGORY_XXX enumerations
	Category depends_from;
	//! Set of categories of optimizers which should be processed again when this optimizer applied,
	//! see Optimizer::CATEGORY_XXX enumerations
	Category affects_to;
	//! Mode determines what should do optimization system when this optimizer applied,
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


	Optimizer(): category_id(), order(), index(), depends_from(), affects_to(), mode(), for_list(), for_task(), for_root_task(), deep_first() { }
	virtual ~Optimizer();

	static bool less(const Handle &a, const Handle &b)
	{
		return !b ? false
			 : !a ? true
			 : a->order < b->order ? true
			 : b->order < a->order ? false
			 : a->index < b->index;
	}

	static Task::Handle replace_target(
		const Task::Handle &parent,
		const SurfaceResource::Handle &surface,
		const Task::Handle &task );
	static Task::Handle replace_target(
		const Task::Handle &parent,
		const Task::Handle &task )
			{ return replace_target(parent, task->target_surface, task); }

	virtual void run(const RunParams &params) const = 0;

	void apply(const RunParams &params) const
	{
		params.ref_affects_to |= affects_to;
		params.ref_mode |= mode;
	}

	void apply(const RunParams &params, const Task::Handle &task) const
		{ apply(params); params.ref_task = task; }
};

} /* end namespace rendering */
} /* end namespace synfig */

/* -- E N D ----------------------------------------------------------------- */

#endif
