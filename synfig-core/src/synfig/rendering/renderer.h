/* === S Y N F I G ========================================================= */
/*!	\file synfig/rendering/renderer.h
**	\brief Renderer Header
**
**	$Id$
**
**	\legal
**	......... ... 2015 Ivan Mahonin
**
**	This file is part of Synfig.
**
**	Synfig is free software: you can redistribute it and/or modify
**	it under the terms of the GNU General Public License as published by
**	the Free Software Foundation, either version 2 of the License, or
**	(at your option) any later version.
**
**	Synfig is distributed in the hope that it will be useful,
**	but WITHOUT ANY WARRANTY; without even the implied warranty of
**	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
**	GNU General Public License for more details.
**
**	You should have received a copy of the GNU General Public License
**	along with Synfig.  If not, see <https://www.gnu.org/licenses/>.
**	\endlegal
*/
/* ========================================================================= */

/* === S T A R T =========================================================== */

#ifndef __SYNFIG_RENDERING_RENDERER_H
#define __SYNFIG_RENDERING_RENDERER_H

/* === H E A D E R S ======================================================= */

#include <cstdio>

#include <map>
#include <atomic>

#include "optimizer.h"

/* === M A C R O S ========================================================= */

/* === T Y P E D E F S ===================================================== */

/* === C L A S S E S & S T R U C T S ======================================= */

namespace synfig
{
namespace rendering
{

class RenderQueue;

class Renderer: public etl::shared_object
{
public:
	typedef etl::handle<Renderer> Handle;
	typedef std::multimap<Real, ModeToken::Handle> ModeMap;

	struct DebugOptions {
		String task_list_log;
		String task_list_optimized_log;
		String result_image;
	};

private:
	static Handle blank;
	static std::map<String, Handle> *renderers;
	static RenderQueue *queue;
	static DebugOptions debug_options;
	static long long last_registered_optimizer_index;
	static long long last_batch_index; // TODO: atomic

	ModeList modes;
	Optimizer::List optimizers[Optimizer::CATEGORIES_COUNT];

public:

	virtual ~Renderer();

	virtual String get_name() const = 0;

	const Optimizer::List& get_optimizers(Optimizer::CategoryId category_id) const
		{ return optimizers[category_id]; }
	bool is_optimizer_registered(const Optimizer::Handle &optimizer) const;
	void register_optimizer(Real order, const Optimizer::Handle &optimizer);
	void register_optimizer(const Optimizer::Handle &optimizer);
	void unregister_optimizer(const Optimizer::Handle &optimizer);

	const ModeList& get_modes() const
		{ return modes; }
	void register_mode(int index, const ModeToken::Handle &mode);
	void register_mode(const ModeToken::Handle &mode);
	void unregister_mode(const ModeToken::Handle &mode);

private:
	int count_tasks_recursive(Task::List &list) const;
	int count_tasks(Task::List &list) const;
	void calc_coords(const Task::List &list) const;
	void specialize_recursive(Task::List &list) const;
	void specialize(Task::List &list) const;
	void remove_dummy(Task::List &list) const;
	void linearize(Task::List &list) const;

	int subtasks_count(const Task::Handle &task, int max_count) const;

	bool
	call_optimizers(
		const Optimizer::List &optimizers,
		const Optimizer::RunParams& params,
		std::atomic<int> *calls_count,
		std::atomic<int> *optimizations_count,
		bool deep_first ) const;

	void optimize_recursive(
		const Optimizer::List *optimizers,  // pass by pointer for use with sigc::bind
		const Optimizer::RunParams *params, // pass by pointer for use with sigc::bind
		std::atomic<int> *calls_count,
		std::atomic<int> *optimizations_count,
		int max_level ) const;

	void optimize(Optimizer::Category category, Task::List &list) const;

	void log(
		const String &logfile,
		const Task::Handle &task,
		const Optimizer::RunParams* optimization_stack = NULL,
		int level = 0 ) const;
	void log(
		const String &logfile,
		const Task::List &list,
		const String &name = String(),
		const Optimizer::RunParams* optimization_stack = NULL ) const;

	static void initialize_renderers();
	static void deinitialize_renderers();

	typedef std::pair<Surface::Handle, int>             DepTargetKey;
	typedef std::pair<Task::Handle, Task::Set>          DepTargetValue;
	typedef std::multimap<DepTargetKey, DepTargetValue> DepTargetMap;
	typedef DepTargetMap::value_type                    DepTargetPair;

	void find_deps(const Task::List &list, long long batch_index) const;

public:
	int get_max_simultaneous_threads() const;
	void optimize(Task::List &list) const;

	bool run(
		const Task::List &list,
		bool quiet = false ) const;
	bool run(
		const Task::Handle &task,
		bool quiet = false ) const
			{ return run(Task::List(1, task), quiet); }

	void enqueue(
		const Task::List &list,
		const TaskEvent::Handle &finish_event_task,
		bool quiet = false ) const;
	void enqueue(
		const Task::Handle &task,
		const TaskEvent::Handle &finish_event_task,
		bool quiet = false ) const
			{ return enqueue(Task::List(1, task), finish_event_task, quiet); }

	static void cancel(const Task::Handle &task);
	static void cancel(const Task::List &list);

	// function to use in signals
	static void enqueue_task_func(Renderer::Handle renderer, Task::Handle task, TaskEvent::Handle finish_event_task, bool quiet)
		{ renderer->enqueue(task, finish_event_task, quiet); }
	static void enqueue_list_func(Renderer::Handle renderer, Task::List list, TaskEvent::Handle finish_event_task, bool quiet)
		{ renderer->enqueue(list, finish_event_task, quiet); }  // list will passed as copy
	static void cancel_task_func(Task::Handle task)
		{ cancel(task); }
	static void cancel_list_func(Task::List list)
		{ cancel(list); } // list will passed as copy

	static void initialize();
	static void deinitialize();
	static void register_renderer(const String &name, const Renderer::Handle &renderer);
	static void unregister_renderer(const String &name);
	static const Renderer::Handle& get_renderer(const String &name);
	static const std::map<String, Handle>& get_renderers();

	static const DebugOptions& get_debug_options()
		{ return debug_options; }

	static bool subsys_init()
		{ initialize(); return true; }
	static bool subsys_stop()
		{ deinitialize(); return false; }
};

} /* end namespace rendering */
} /* end namespace synfig */

/* -- E N D ----------------------------------------------------------------- */

#endif
