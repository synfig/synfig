/* === S Y N F I G ========================================================= */
/*!	\file synfig/rendering/renderer.cpp
**	\brief Renderer
**
**	$Id$
**
**	\legal
**	......... ... 2015-2018 Ivan Mahonin
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

/* === H E A D E R S ======================================================= */

#ifdef USING_PCH
#	include "pch.h"
#else
#ifdef HAVE_CONFIG_H
#	include <config.h>
#endif

#include <cstdlib>
#include <climits>

#include <typeinfo>

#include <synfig/general.h>
#include <synfig/localization.h>
#include <synfig/threadpool.h>
#include <synfig/debug/debugsurface.h>
#include <synfig/debug/log.h>
#include <synfig/debug/measure.h>

#include "renderer.h"
#include "renderqueue.h"

#include "software/renderersw.h"
#include "software/rendererdraftsw.h"
#include "software/rendererpreviewsw.h"
#include "software/rendererlowressw.h"
#include "software/renderersafe.h"
#ifdef WITH_OPENGL
#include "opengl/renderergl.h"
#include "opengl/task/taskgl.h"
#endif

#endif

using namespace synfig;
using namespace rendering;


//#define DEBUG_TASK_MEASURE
//#define DEBUG_OPTIMIZATION_MEASURE
//#define DEBUG_OPTIMIZATION_COUNTERS

#ifndef NDEBUG
//#define DEBUG_TASK_LIST
//#define DEBUG_OPTIMIZATION
//#define DEBUG_OPTIMIZATION_EACH_CHANGE
#endif


/* === M A C R O S ========================================================= */

/* === G L O B A L S ======================================================= */

/* === P R O C E D U R E S ================================================= */

/* === M E T H O D S ======================================================= */

Renderer::Handle Renderer::blank;
std::map<String, Renderer::Handle> *Renderer::renderers;
RenderQueue *Renderer::queue;
Renderer::DebugOptions Renderer::debug_options;
long long Renderer::last_registered_optimizer_index = 0;
long long Renderer::last_batch_index = 0;


void
Renderer::initialize_renderers()
{
	// initialize renderers
	RendererSW::initialize();
#ifdef WITH_OPENGL
	RendererGL::initialize();
#endif

	// register renderers
	register_renderer("software", new RendererSW());
	register_renderer("software-preview", new RendererPreviewSW());
	register_renderer("software-draft", new RendererDraftSW());
	register_renderer("software-low2",  new RendererLowResSW(2));
	register_renderer("software-low4",  new RendererLowResSW(4));
	register_renderer("software-low8",  new RendererLowResSW(8));
	register_renderer("software-low16", new RendererLowResSW(16));
	register_renderer("safe", new RendererSafe());
#ifdef WITH_OPENGL
	register_renderer("gl", new RendererGL());
#endif
}

void
Renderer::deinitialize_renderers()
{
#ifdef WITH_OPENGL
	RendererGL::deinitialize();
#endif
	RendererSW::deinitialize();
}



Renderer::~Renderer() { }

int
Renderer::get_max_simultaneous_threads() const
{
	assert(queue);
	return queue->get_threads_count() - 1;
}

bool
Renderer::is_optimizer_registered(const Optimizer::Handle &optimizer) const
{
	if (!optimizer)
		for(Optimizer::List::const_iterator i = optimizers[optimizer->category_id].begin(); i != optimizers[optimizer->category_id].end(); ++i)
			if (*i == optimizer) return true;
	return false;
}

void
Renderer::register_optimizer(Real order, const Optimizer::Handle &optimizer)
{
	if (optimizer) {
		assert(!is_optimizer_registered(optimizer));
		optimizer->order = order;
		optimizer->index = ++last_registered_optimizer_index;
		Optimizer::List &list = optimizers[optimizer->category_id];
		list.push_back(optimizer);
		std::sort(list.begin(), list.end(), Optimizer::less);
	}
}

void
Renderer::register_optimizer(const Optimizer::Handle &optimizer)
{
	if (optimizer)
		register_optimizer(optimizer->order, optimizer);
}

void
Renderer::unregister_optimizer(const Optimizer::Handle &optimizer)
{
	for(Optimizer::List::iterator i = optimizers[optimizer->category_id].begin(); i != optimizers[optimizer->category_id].end();)
		if (*i == optimizer) i = optimizers[optimizer->category_id].erase(i); else ++i;
}

void
Renderer::register_mode(int index, const ModeToken::Handle &mode)
	{ modes.insert(modes.begin() + index, mode); }

void
Renderer::register_mode(const ModeToken::Handle &mode)
	{ modes.push_back(mode); }

void
Renderer::unregister_mode(const ModeToken::Handle &mode)
{
	for(ModeList::iterator i = modes.begin(); i != modes.end();)
	if (*i == mode) i = modes.erase(i); else ++i;
}

int
Renderer::count_tasks_recursive(Task::List &list) const {
	int count = 0;
	for(Task::List::const_iterator i = list.begin(); i != list.end(); ++i)
		if (*i) count += 1 + count_tasks_recursive((*i)->sub_tasks);
	return count;
}

int
Renderer::count_tasks(Task::List &list) const
{
	#ifdef DEBUG_OPTIMIZATION_MEASURE
	debug::Measure t("count tasks");
	#endif
	return count_tasks_recursive(list);
}

void
Renderer::calc_coords(const Task::List &list) const
{
	#ifdef DEBUG_OPTIMIZATION_MEASURE
	debug::Measure t("calc coords");
	#endif
	for(Task::List::const_iterator i = list.begin(); i != list.end(); ++i)
		if (*i) (*i)->touch_coords();
}

void
Renderer::specialize_recursive(Task::List &list) const
{
	for(Task::List::iterator i = list.begin(); i != list.end(); ++i)
		if (*i) {
			Task::Handle task;
			for(ModeList::const_iterator j = modes.begin(); j != modes.end() && !task; ++j)
				task = (*i)->convert_to(*j);
			if (!task)
				task = (*i)->convert_to_any();
			if (!task)
				task = (*i)->clone();
			*i = task;
			specialize_recursive((*i)->sub_tasks);
		}
}

void
Renderer::specialize(Task::List &list) const
{
	#ifdef DEBUG_OPTIMIZATION_MEASURE
	debug::Measure t("specialize");
	#endif
	specialize_recursive(list);
}

void
Renderer::remove_dummy(Task::List &list) const
{
	// remove dummy tasks
	for(Task::List::iterator i = list.begin(); i != list.end();)
		if ( !*i
		  || i->type_is<TaskSurface>()
		  || i->type_is<TaskList>() )
			i = list.erase(i); else ++i;
}

void
Renderer::linearize(Task::List &list) const
{
	#ifdef DEBUG_OPTIMIZATION_MEASURE
	debug::Measure t("linearize");
	#endif

	// convert task-tree to linear list
	for(Task::List::iterator i = list.begin(); i != list.end();)
	{
		if (*i && !i->type_is<TaskSurface>())
		{
			// remember current position
			int index = i - list.begin();
			bool found = false;

			for(Task::List::iterator j = (*i)->sub_tasks.begin(); j != (*i)->sub_tasks.end(); ++j)
			{
				if ( *j
				  && !TaskSurface::Handle::cast_dynamic(*j) )
				{
					i = list.insert(i, *j);
					++i;

					if (!found)
					{
						// clone task
						int index = j - (*i)->sub_tasks.begin();
						*i = (*i)->clone();
						j = (*i)->sub_tasks.begin() + index;
						found = true;
					}

					// replace sub_task by TaskSurface
					Task::Handle surface(new TaskSurface());
					*surface = **j;
					surface->sub_tasks.clear();
					*j = surface;
				}
			}

			// if changed then go back to check inserted tasks
			if (found)
				{ i = list.begin() + index; continue; }
		}
		++i;
	}

	remove_dummy(list);
}

int
Renderer::subtasks_count(const Task::Handle &task, int max_count) const {
	int count = 1 + (int)task->sub_tasks.size();
	if (count >= max_count) return count;

	for(Task::List::const_iterator i = task->sub_tasks.begin(); i != task->sub_tasks.end(); ++i)
		if (*i) {
			count += subtasks_count(*i, max_count - count);
			if (count >= max_count) return count;
		}

	return count;
}


bool
Renderer::call_optimizers(
	const Optimizer::List &optimizers,
	const Optimizer::RunParams& params,
	std::atomic<int> *calls_count,
	std::atomic<int> *optimizations_count,
	bool deep_first ) const
{
	for(Optimizer::List::const_iterator i = optimizers.begin(); i != optimizers.end(); ++i)
	{
		if ((*i)->deep_first == deep_first)
		{
			if ((*i)->for_task || ((*i)->for_root_task && !params.parent))
			{
				// run
				Optimizer::RunParams p(params);
				p.ref_mode = 0;
				p.ref_affects_to = 0;
				(*i)->run(p);

				if (calls_count) ++(*calls_count);
				if (params.ref_task != p.ref_task)
				{
					if (optimizations_count) ++(*optimizations_count);
					#ifdef DEBUG_OPTIMIZATION_EACH_CHANGE
					log("", *params.list, (typeid(**i).name() + 19), &p);
					#endif
				}

				// apply params
				params.ref_affects_to |= p.ref_affects_to;
				params.ref_mode |= p.ref_mode;
				params.ref_task = p.ref_task;

				// return when current task removed
				if (!params.ref_task) return false;

				// check dependencies
				if (params.ref_affects_to & params.depends_from) return false;

				// return when recursive re-optimization required
				// anyway function will re-called again in the same mode
				if ( (params.ref_mode & Optimizer::MODE_REPEAT_BRANCH)
				  && (params.ref_mode & Optimizer::MODE_RECURSIVE) )
					return false;
			}
		}
	}
	return true;
}

void
Renderer::optimize_recursive(
	const Optimizer::List *optimizers,  // pass by pointer for use with sigc::bind
	const Optimizer::RunParams *params, // pass by pointer for use with sigc::bind
	std::atomic<int> *calls_count,
	std::atomic<int> *optimizations_count,
	int max_level ) const
{
	if (!params || !params->ref_task) return;

	// run all non-deep-first optimizers for current task
	// before processing of sub-tasks (see Optimizer::deep_first)
	if (!call_optimizers(*optimizers, *params, calls_count, optimizations_count, false))
		return;

	// process sub-tasks, only for non-for-root-task optimizers (see Optimizer::for_root_task)
	int count = max_level > 0 ? params->ref_task->sub_tasks.size() : 0;
	if (count > 0)
	{
		int counter_limit = 32;
		int counter_prophecy = 256;
		Real counter_weight = (Real)optimizers->size()/(Real)1024;
		if (counter_prophecy * counter_weight < 0.75)
			{ counter_limit = 0; counter_prophecy = 0; }

		// prepare params
		bool task_clonned = false;
		std::vector<Optimizer::RunParams> sub_params(count);
		int jumps[count+1]; // initial jump stored after last element
		jumps[count] = 0;
		for(int i = 0; i < count; ++i) {
			sub_params[i] = params->sub( params->ref_task->sub_task(i) );
			jumps[i] = i+1;
		}
		for(int j = count, i = jumps[j]; i < count; i = jumps[i])
			if (sub_params[i].ref_task) j = i; else jumps[j] = jumps[i];

		// run optimizers
		bool first_pass = true;
		ThreadPool::Group group;
		while (jumps[count] != count) {
			for(int j = count, i = jumps[j]; i < count; i = jumps[i]) {
				Optimizer::RunParams &sp = sub_params[i];

				int sub_level = first_pass ? max_level-1
							  : (sp.ref_mode & Optimizer::MODE_RECURSIVE) ? INT_MAX : 0;
				sp.ref_mode = 0;

				int cnt = subtasks_count(sp.ref_task, counter_limit);
				if (cnt >= counter_limit && cnt < counter_prophecy) cnt = counter_prophecy;
				Real weight = (Real)cnt*counter_weight;

				group.enqueue( sigc::bind( sigc::mem_fun(this, &Renderer::optimize_recursive),
					optimizers,
					&sp,
					calls_count,
					optimizations_count,
					sub_level ), weight );
			}
			group.run();

			// merge results
			for(int j = count, i = jumps[j]; i < count; i = jumps[i]) {
				Optimizer::RunParams &sp = sub_params[i];

				if (Task::Handle &sub_task = params->ref_task->sub_task(i))
				if (sp.ref_task != sub_task) {
					if (task_clonned) {
						sub_task = sp.ref_task;
					} else {
						params->ref_task = params->ref_task->clone();
						params->ref_task->sub_task(i) = sp.ref_task;
						task_clonned = true;
					}
				}

				// apply affected categories
				params->ref_affects_to |= sp.ref_affects_to;

				// if mode is REPEAT_BRUNCH then provide this flag to result
				if ((sp.ref_mode & Optimizer::MODE_REPEAT_BRANCH) == Optimizer::MODE_REPEAT_BRANCH) {
					params->ref_mode |= Optimizer::MODE_REPEAT_BRANCH;
					params->ref_mode |= (sp.ref_mode & Optimizer::MODE_RECURSIVE);
				} else
				// if mode is REPEAT_PARENT then provide flag REPEAT_LAST to result (repeat only one upper level)
				if ((sp.ref_mode & Optimizer::MODE_REPEAT_PARENT) == Optimizer::MODE_REPEAT_PARENT) {
					params->ref_mode |= Optimizer::MODE_REPEAT_LAST;
					params->ref_mode |= (sp.ref_mode & Optimizer::MODE_RECURSIVE);
				}

				// reset ref_affects_to flags
				// and keep sp.ref_mode we still need MODE_RECURSIVE flag
				sp.ref_affects_to = 0;
				sp.orig_task = sp.ref_task;

				// mark to skip
				if (!sp.ref_task || !(sp.ref_mode & Optimizer::MODE_REPEAT_LAST))
					jumps[j] = jumps[i]; else j = i;
			}

			// check dependencies
			if (params->ref_affects_to & params->depends_from) return;

			// return when recursive re-optimization required
			// anyway function will re-called again in the same mode
			if ( (params->ref_mode & Optimizer::MODE_REPEAT_BRANCH)
			  && (params->ref_mode & Optimizer::MODE_RECURSIVE) )
				return;

			first_pass = false;
		}
	}

	// run deep-first optimizers for current task
	// when all sub-tasks are processed (see Optimizer::deep_first)
	if (!call_optimizers(*optimizers, *params, calls_count, optimizations_count, true))
		return;
}

void
Renderer::optimize(Task::List &list) const
{
	#ifdef DEBUG_TASK_MEASURE
	debug::Measure t("Renderer::optimize");
	#endif

	#ifdef DEBUG_OPTIMIZATION_COUNTERS
	debug::Log::info("", "optimize %d tasks", count_tasks(list));
	#endif

	int current_category_id = 0;
	int prepared_category_id = 0;
	int current_optimizer_index = 0;
	Optimizer::Category current_affected = 0;
	Optimizer::Category categories_to_process = Optimizer::CATEGORY_ALL;
	Optimizer::List single(1);

	while(categories_to_process &= Optimizer::CATEGORY_ALL)
	{
		while (prepared_category_id < current_category_id) {
			switch (++prepared_category_id) {
			case Optimizer::CATEGORY_ID_COORDS:
				calc_coords(list); break;
			case Optimizer::CATEGORY_ID_SPECIALIZED:
				specialize(list); break;
			case Optimizer::CATEGORY_ID_LIST:
				linearize(list); break;
			default:
				break;
			}
			#ifdef DEBUG_OPTIMIZATION
			log("", list, etl::strprintf("after category %d prepared", prepared_category_id));
			#endif
		}
		prepared_category_id = current_category_id;

		if (current_category_id >= Optimizer::CATEGORIES_COUNT)
		{
			current_category_id = 0;
			current_optimizer_index = 0;
			current_affected = 0;
			continue;
		}

		if (!((1 << current_category_id) & categories_to_process))
		{
			++current_category_id;
			current_optimizer_index = 0;
			current_affected = 0;
			continue;
		}

		if (current_optimizer_index >= (int)optimizers[current_category_id].size())
		{
			categories_to_process &= ~(1 << current_category_id);
			categories_to_process |= current_affected;
			++current_category_id;
			current_optimizer_index = 0;
			current_affected = 0;
			continue;
		}

		bool simultaneous_run = Optimizer::categories_info[current_category_id].simultaneous_run;
		const Optimizer::List &current_optimizers = simultaneous_run ? optimizers[current_category_id] : single;
		if (!simultaneous_run) {
			single.front() = optimizers[current_category_id][current_optimizer_index];
			Optimizer::Category depends_from_self = (1 << current_category_id) & single.front()->depends_from;
			if (current_affected & depends_from_self)
			{
				current_category_id = 0;
				current_optimizer_index = 0;
				current_affected = 0;
				continue;
			}
		}

		Optimizer::Category depends_from = 0;
		bool for_list = false;
		bool for_task = false;
		bool for_root_task = false;
		for(Optimizer::List::const_iterator i = current_optimizers.begin(); i != current_optimizers.end(); ++i)
		{
			depends_from |= ((1 << current_category_id) - 1) & (*i)->depends_from;
			if ((*i)->for_list) for_list = true;
			if ((*i)->for_task) for_task = true;
			if ((*i)->for_root_task) for_root_task = true;
		}

		#ifdef DEBUG_OPTIMIZATION
		log("", list, etl::strprintf("before optimize category %d index %d", current_category_id, current_optimizer_index));
		#endif

		#ifdef DEBUG_OPTIMIZATION_MEASURE
		debug::Measure t(etl::strprintf("optimize category %d index %d", current_category_id, current_optimizer_index));
		#endif

		#ifdef DEBUG_OPTIMIZATION_COUNTERS
		std::atomic<int> calls_count(0), *calls_count_ptr = &calls_count;
		std::atomic<int> optimizations_count(0), *optimizations_count_ptr = &optimizations_count;
		#else
		std::atomic<int> *calls_count_ptr = NULL;
		std::atomic<int> *optimizations_count_ptr = NULL;
		#endif

		if (for_list)
		{
			for(Optimizer::List::const_iterator i = current_optimizers.begin(); !(categories_to_process & depends_from) && i != current_optimizers.end(); ++i)
			{
				if ((*i)->for_list)
				{
					Optimizer::RunParams params(depends_from, list);
					(*i)->run(params);
					categories_to_process |= current_affected |= params.ref_affects_to;

					if (calls_count_ptr) ++(*calls_count_ptr);
					if (optimizations_count_ptr && params.ref_affects_to) ++(*optimizations_count_ptr);
				}
			}
		}

		if (for_task || for_root_task)
		{
			bool nonrecursive = false;
			for(Task::List::iterator j = list.begin(); !(categories_to_process & depends_from) && j != list.end();)
			{
				if (*j)
				{
					Optimizer::RunParams params(depends_from, *j, &list);
					Renderer::optimize_recursive(
						&current_optimizers,
						&params,
						calls_count_ptr,
						optimizations_count_ptr,
						!for_task ? 0 : nonrecursive ? 1 : INT_MAX );
					nonrecursive = false;

					if (*j != params.ref_task)
					{

						if (params.ref_task)
						{
							*j = params.ref_task;
							// go to next sub-task if we don't need to repeat optimization (see Optimizer::MODE_REPEAT)
							if ((params.ref_mode & Optimizer::MODE_REPEAT_LAST) == Optimizer::MODE_REPEAT_LAST)
							{
								// check non-recursive flag (see Optimizer::MODE_RECURSIVE)
								if (!(params.ref_mode & Optimizer::MODE_RECURSIVE))
									nonrecursive = true;
							}
							else ++j;
						}
						else
							j = list.erase(j);
					} else ++j;
					categories_to_process |= current_affected |= params.ref_affects_to;
				}
				else
				{
					j = list.erase(j);
				}
			}
		}

		#ifdef DEBUG_OPTIMIZATION_COUNTERS
		debug::Log::info("", "optimize category %d index %d: calls %d, changes %d",
			current_category_id, current_optimizer_index, (int)calls_count, (int)optimizations_count );
		#endif

		#ifdef DEBUG_OPTIMIZATION
		log("", list, etl::strprintf("after optimize category %d index %d", current_category_id, current_optimizer_index));
		#endif

		if (categories_to_process & depends_from)
		{
			current_category_id = 0;
			current_optimizer_index = 0;
			current_affected = 0;
			continue;
		}

		current_optimizer_index += current_optimizers.size();
	}

	remove_dummy(list);
}

void
Renderer::find_deps(const Task::List &list, long long batch_index) const
{
	#ifdef DEBUG_TASK_MEASURE
	debug::Measure t("Renderer::find_deps");
	#endif

	typedef std::map<SurfaceResource::Handle, Task::Handle> DepTargetPrevMap;
	DepTargetPrevMap target_prev_map;
	Task::Set tasks_to_process;
	const int max_iterations = 50000;

	// find dependencies by target surface
	for(Task::List::const_iterator i = list.begin(); i != list.end(); ++i)
	{
		Task::Handle task = *i;
		Task::RendererData &task_rd = task->renderer_data;

		assert(task_rd.index == 0);
		assert(task_rd.batch_index == 0);
		task_rd.batch_index = batch_index;
		task_rd.index = i - list.begin() + 1;

		task_rd.deps.clear();
		task_rd.back_deps.clear();

		if ((*i)->is_valid()) {
			for(Task::List::const_iterator j = (*i)->sub_tasks.begin(); j != (*i)->sub_tasks.end(); ++j)
				if (*j && (*j)->is_valid())
					if (Task::Handle dep = target_prev_map[(*j)->target_surface]) {
						if (task_rd.tmp_deps.empty()) tasks_to_process.insert(*i);
						task_rd.tmp_deps.insert(dep);
						dep->renderer_data.tmp_back_deps.insert(*i);
					}
			Task::Handle &dep = target_prev_map[(*i)->target_surface];
			if (dep) {
				if (task_rd.tmp_deps.empty()) tasks_to_process.insert(*i);
				task_rd.tmp_deps.insert(dep);
				dep->renderer_data.tmp_back_deps.insert(*i);
			}
			dep = *i;
		}
	}

	// fix dependencies for tasks with non-intersected areas
	int iterations = 0;
	while(iterations < max_iterations && !tasks_to_process.empty())
	{
		for(Task::Set::iterator i = tasks_to_process.begin(); i != tasks_to_process.end();)
		{
			Task::Handle task = *i;
			Task::RendererData &task_rd = task->renderer_data;
			assert(!task_rd.tmp_deps.empty());

			Task::Handle dep = *task_rd.tmp_deps.begin();
			Task::RendererData &dep_rd = dep->renderer_data;
			task_rd.tmp_deps.erase( task_rd.tmp_deps.begin() );
			dep_rd.tmp_back_deps.erase(task);

			if (!task->allow_run_before(*dep)) {
				task_rd.deps.insert(dep);
				dep_rd.back_deps.insert(task);
				++iterations;
			} else {
				iterations += dep_rd.deps.size() + dep_rd.tmp_deps.size() + task_rd.back_deps.size() + task_rd.tmp_back_deps.size();
				for(Task::Set::iterator j = dep_rd.deps.begin(); j != dep_rd.deps.end(); ++j)
					if (task_rd.deps.count(*j) == 0) {
						task_rd.tmp_deps.insert(*j);
						(*j)->renderer_data.tmp_back_deps.insert(task);
					}
				for(Task::Set::iterator j = dep_rd.tmp_deps.begin(); j != dep_rd.tmp_deps.end(); ++j)
					if (task_rd.deps.count(*j) == 0) {
						task_rd.tmp_deps.insert(*j);
						(*j)->renderer_data.tmp_back_deps.insert(task);
					}
				for(Task::Set::iterator j = task_rd.back_deps.begin(); j != task_rd.back_deps.end(); ++j)
					if ((*j)->renderer_data.deps.count(dep) == 0) {
						if ((*j)->renderer_data.tmp_deps.empty()) tasks_to_process.insert(*j);
						(*j)->renderer_data.tmp_deps.insert(dep);
						dep_rd.tmp_back_deps.insert(*j);
					}
				for(Task::Set::iterator j = task_rd.tmp_back_deps.begin(); j != task_rd.tmp_back_deps.end(); ++j)
					if ((*j)->renderer_data.deps.count(dep) == 0) {
						(*j)->renderer_data.tmp_deps.insert(dep);
						dep_rd.tmp_back_deps.insert(*j);
					}
			}

			Task::Set::iterator j = i;
			++i;
			++iterations;

			if (task_rd.tmp_deps.empty())
				tasks_to_process.erase(j);
		}
	}

	#ifdef DEBUG_TASK_MEASURE
	info("find deps iterations: %d (%d from %d tasks not optimized)", iterations, (int)tasks_to_process.size(), (int)list.size());
	#endif

	// copy tmp_deps to deps
	for(Task::List::const_iterator i = list.begin(); i != list.end(); ++i) {
		Task::Handle task = *i;
		Task::RendererData &task_rd = task->renderer_data;

		task_rd.deps     .insert(task_rd.tmp_deps     .begin(), task_rd.tmp_deps     .end());
		task_rd.back_deps.insert(task_rd.tmp_back_deps.begin(), task_rd.tmp_back_deps.end());

		task_rd.tmp_deps.clear();
		task_rd.tmp_back_deps.clear();
	}
}

bool
Renderer::run(const Task::List &list, bool quiet) const
{
	//if (!quiet) info("renderer.debug.result_image: %s", get_debug_options().result_image.c_str());

	#ifdef DEBUG_TASK_MEASURE
	if (!quiet) debug::Measure t("Renderer::run");
	#endif

	TaskEvent::Handle task_event = new TaskEvent();
	enqueue(list, task_event, quiet);

	{
		#ifdef DEBUG_TASK_MEASURE
		if (!quiet) debug::Measure t("run tasks");
		#endif

		task_event->wait();
	}

	if (!quiet && !get_debug_options().result_image.empty())
		debug::DebugSurface::save_to_file(
			!list.empty() && list.back()
				? list.back()->target_surface
				: SurfaceResource::Handle(),
			get_debug_options().result_image,
			true );

	return task_event->is_done();
}

void
Renderer::enqueue(const Task::List &list, const TaskEvent::Handle &finish_event_task, bool quiet) const
{
	assert(finish_event_task);
	if (!finish_event_task || finish_event_task->is_finished()) return;

	//if (!quiet) info("renderer: %s", get_name().c_str());
	//if (!quiet) info("renderer.debug.task_list_log: %s", get_debug_options().task_list_log.c_str());
	//if (!quiet) info("renderer.debug.task_list_optimized_log: %s", get_debug_options().task_list_optimized_log.c_str());

	#ifdef DEBUG_TASK_MEASURE
	if (!quiet) debug::Measure t("Renderer::enqueue");
	#endif

	#ifdef DEBUG_TASK_LIST
	if (!quiet) log("", list, "input list");
	#endif

	if (!quiet && !get_debug_options().task_list_log.empty())
		log(get_debug_options().task_list_log, list, "input list");

	Task::List optimized_list(list);
	optimize(optimized_list);
	find_deps(optimized_list, ++last_batch_index);

	#ifdef DEBUG_TASK_LIST
	if (!quiet) log("", optimized_list, "optimized list");
	#endif

	if (!quiet && !get_debug_options().task_list_optimized_log.empty())
		log(get_debug_options().task_list_optimized_log, optimized_list, "optimized list");

	if (finish_event_task)
	{
		finish_event_task->renderer_data.deps.insert(optimized_list.begin(), optimized_list.end());
		for(Task::List::const_iterator i = optimized_list.begin(); i != optimized_list.end(); ++i)
			(*i)->renderer_data.back_deps.insert(finish_event_task);
		optimized_list.push_back(finish_event_task);
	}

	// try to find existing handle to this renderer instead,
	// because creation and destruction of handle may cause destruction of renderer
	// if it never stored in handles before
	queue->enqueue(optimized_list, Task::RunParams( get_renderer(get_name()) ));
}

void Renderer::cancel(const Task::Handle &task)
	{ if (queue) queue->cancel(task); }

void Renderer::cancel(const Task::List &list)
	{ if (queue) queue->cancel(list); }

void
Renderer::log(
	const String &logfile,
	const Task::Handle &task,
	const Optimizer::RunParams* optimization_stack,
	int level ) const
{
	bool use_stack = false;
	const Optimizer::RunParams* p = optimization_stack ? optimization_stack->get_level(level) : NULL;
	Task::Handle t = task;
	if (p && p->orig_task == t)
		{ use_stack = true; t = p->ref_task; }

	if (t)
	{
		const Task::RendererData &trd = t->renderer_data;

		String deps;
		if (!trd.deps.empty())
		{
			std::multiset<int> deps_set;
			for(Task::Set::const_iterator i = trd.deps.begin(); i != trd.deps.end(); ++i)
				deps_set.insert((*i)->renderer_data.index);
			for(std::multiset<int>::const_iterator i = deps_set.begin(); i != deps_set.end(); ++i)
				deps += etl::strprintf("%d ", *i);
			deps = "(" + deps.substr(0, deps.size()-1) + ") ";
		}

		String back_deps;
		if (!trd.back_deps.empty())
		{
			std::multiset<int> back_deps_set;
			for(Task::Set::const_iterator i = trd.back_deps.begin(); i != trd.back_deps.end(); ++i)
				back_deps_set.insert((*i)->renderer_data.index);
			for(std::multiset<int>::const_iterator i = back_deps_set.begin(); i != back_deps_set.end(); ++i)
				back_deps += etl::strprintf("%d ", *i);
			back_deps = "(" + back_deps.substr(0, back_deps.size()-1) + ") ";
		}

		String surfaces;
		if (t->target_surface) {
			std::vector<Surface::Token::Handle> tokens;
			t->target_surface->get_tokens(tokens);

			std::set<String> names;
			for(std::vector<Surface::Token::Handle>::const_iterator i = tokens.begin(); i != tokens.end(); ++i)
				names.insert( (*i)->name );

			for(std::set<String>::const_iterator i = names.begin(); i != names.end(); ++i) {
				if (!surfaces.empty()) surfaces += ", ";
				surfaces += *i;
			}
		}

		debug::Log::info(logfile,
		      String(level*2, ' ')
			+ (use_stack ? "*" : "")
			+ (trd.index ? etl::strprintf("#%05d-%04d ", trd.batch_index, trd.index): "")
			+ deps
			+ back_deps
			+ t->get_token()->name
			+ ( t->get_bounds().valid()
			  ? etl::strprintf(" bounds (%f, %f)-(%f, %f)",
				t->get_bounds().minx, t->get_bounds().miny,
				t->get_bounds().maxx, t->get_bounds().maxy )
			  : "" )
			+ ( t->target_surface
              ? etl::strprintf(" source (%f, %f)-(%f, %f) target (%d, %d)-(%d, %d) surface [%s] (%dx%d) id %d",
				t->source_rect.minx, t->source_rect.miny,
				t->source_rect.maxx, t->source_rect.maxy,
				t->target_rect.minx, t->target_rect.miny,
				t->target_rect.maxx, t->target_rect.maxy,
				surfaces.c_str(),
				t->target_surface->get_width(),
				t->target_surface->get_height(),
				t->target_surface->get_id() )
		      : "" ));
		for(Task::List::const_iterator i = t->sub_tasks.begin(); i != t->sub_tasks.end(); ++i)
			log(logfile, *i, use_stack ? optimization_stack : NULL, level+1);
	}
	else
	{
		debug::Log::info(logfile,
			String(level*2, ' ')
			+ (use_stack ? "*" : "")
			+ "NULL" );
	}
}

void
Renderer::log(
	const String &logfile,
	const Task::List &list,
	const String &name,
	const Optimizer::RunParams* optimization_stack ) const
{
	String line = "-------------------------------------------";
	String n = "    " + name;
	n.resize(line.size(), ' ');
	for(int i = 0; i < (int)line.size(); ++i)
		if (n[i] == ' ') n[i] = line[i];
	debug::Log::info(logfile, n);
	for(Task::List::const_iterator i = list.begin(); i != list.end(); ++i)
		log(logfile, *i, optimization_stack);
	debug::Log::info(logfile, line);
}

void
Renderer::initialize()
{
	if (renderers != NULL || queue != NULL)
		synfig::error("rendering::Renderer already initialized");

	// init debug options
	if (const char *s = getenv("SYNFIG_RENDERING_DEBUG_TASK_LIST_LOG"))
		debug_options.task_list_log = s;
	if (const char *s = getenv("SYNFIG_RENDERING_DEBUG_TASK_LIST_OPTIMIZED_LOG"))
		debug_options.task_list_optimized_log = s;
	if (const char *s = getenv("SYNFIG_RENDERING_DEBUG_RESULT_IMAGE"))
		debug_options.result_image = s;

	renderers = new std::map<String, Handle>();
	queue = new RenderQueue();

	initialize_renderers();
}

void
Renderer::deinitialize()
{
	if (renderers == NULL || queue == NULL)
		synfig::error("rendering::Renderer not initialized");

	while(!get_renderers().empty())
		unregister_renderer(get_renderers().begin()->first);

	deinitialize_renderers();

	delete renderers;
	delete queue;
}

void
Renderer::register_renderer(const String &name, const Renderer::Handle &renderer)
{
	if (renderers->count(name))
		synfig::error("rendering::Renderer renderer '%s' already registered", name.c_str());
	(*renderers)[name] = renderer;
}

void
Renderer::unregister_renderer(const String &name)
{
	if (!renderers->count(name))
		synfig::error("rendering::Renderer renderer '%s' not registered", name.c_str());
	renderers->erase(name);
}

const Renderer::Handle&
Renderer::get_renderer(const String &name)
{
	static const char default_engine[] = "software";
	return get_renderers().count(name) > 0           ? get_renderers().find(name)->second
		 : get_renderers().count(default_engine) > 0 ? get_renderers().find(default_engine)->second
		 : blank;
}

const std::map<String, Renderer::Handle>&
Renderer::get_renderers()
{
	if (renderers == NULL)
		synfig::error("rendering::Renderer not initialized");
	return *renderers;
}

/* === E N T R Y P O I N T ================================================= */
