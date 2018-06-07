/* === S Y N F I G ========================================================= */
/*!	\file synfig/rendering/renderqueue.cpp
**	\brief RenderQueue
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

#ifndef _WIN32
#include <unistd.h>
#include <sys/types.h>
#include <signal.h>
#endif

#include <cstdlib>
#include <climits>

#include <typeinfo>

#include <synfig/general.h>
#include <synfig/localization.h>
#include <synfig/debug/debugsurface.h>
#include <synfig/debug/log.h>
#include <synfig/debug/measure.h>

#include "renderqueue.h"
#include "renderer.h"

#endif

using namespace synfig;
using namespace rendering;


#define SYNFIG_RENDERING_MAX_THREADS 256


#ifndef NDEBUG
//#define DEBUG_THREAD_TASK
//#define DEBUG_THREAD_WAIT
//#define DEBUG_TASK_SURFACE
#endif


/* === M A C R O S ========================================================= */

/* === G L O B A L S ======================================================= */

/* === P R O C E D U R E S ================================================= */

/* === M E T H O D S ======================================================= */

namespace {

class TaskSubQueue: public Task
{
public:
	typedef etl::handle<TaskSubQueue> Handle;
	static Token token;
	virtual Token::Handle get_token() const { return token.handle(); }
	const Task::Handle& sub_task() const { return Task::sub_task(0); }
	Task::Handle& sub_task() { return Task::sub_task(0); }
	virtual bool run(RunParams&) const { return true; }
};

Task::Token TaskSubQueue::token(
	DescSpecial<TaskSubQueue>("SubQueue") );

} // end of anonimous namespace


RenderQueue::RenderQueue(): started(false) { start(); }
RenderQueue::~RenderQueue() { stop(); }

void
RenderQueue::start()
{
	Glib::Threads::Mutex::Lock lock(mutex);
	if (started) return;

	// one thread reserved for non-multithreading tasks (OpenGL)
	// also this thread almost don't use CPU time
	// so we have ~50% of one core for GUI
	int count = g_get_num_processors();

	#ifdef DEBUG_TASK_SURFACE
	count = 2;
	#endif

	if (const char *s = getenv("SYNFIG_RENDERING_THREADS"))
		count = atoi(s) + 1;

	if (count > SYNFIG_RENDERING_MAX_THREADS) count = SYNFIG_RENDERING_MAX_THREADS;
	if (count < 2) count = 2;

	for(int i = 0; i < count; ++i)
		threads.push_back(
			Glib::Threads::Thread::create(
				sigc::bind(sigc::mem_fun(*this, &RenderQueue::process), i) ));
	info("rendering threads %d", count);
	started = true;
}

void
RenderQueue::stop()
{
	{
		Glib::Threads::Mutex::Lock lock(mutex);
		started = false;
		cond.broadcast();
		single_cond.broadcast();
	}
	while(!threads.empty())
		{ threads.front()->join(); threads.pop_front(); }
}

void
RenderQueue::process(int thread_index)
{
	while(Task::Handle task = get(thread_index))
	{
		#ifdef DEBUG_THREAD_TASK
		info( "thread %d: begin task #%05d-%04d '%s'",
			  thread_index,
			  task->renderer_data.batch_index,
			  task->renderer_data.index,
			  task->get_token()->name.c_str() );
		#endif

		if (TaskSubQueue::Handle task_sub_queue = TaskSubQueue::Handle::cast_dynamic(task))
		{
			done(thread_index, task_sub_queue->sub_task());
			done(thread_index, task_sub_queue);
			continue;
		}

		if (!task->run(task->renderer_data.params))
			task->renderer_data.success = false;

		#ifdef DEBUG_TASK_SURFACE
		debug::DebugSurface::save_to_file(
			task->target_surface,
			etl::strprintf(
				"task-%05d-%04d-%05d",
				task->renderer_data.batch_index,
				task->renderer_data.index,
				task->target_surface ? task->target_surface->get_id() : 0 ));
		#endif

		#ifdef DEBUG_THREAD_TASK
		info( "thread %d: end task #%05d-%04d '%s'",
			  thread_index,
			  task->renderer_data.batch_index,
			  task->renderer_data.index,
			  task->get_token()->name.c_str() );
		#endif

		if (!task->renderer_data.params.sub_queue.empty())
		{
			if (task->renderer_data.params.renderer)
			{
				TaskSubQueue::Handle task_sub_queue(new TaskSubQueue());
				task_sub_queue->sub_task() = task;
				task->renderer_data.params.renderer->enqueue(task->renderer_data.params.sub_queue, task_sub_queue);
				continue;
			}
			task->renderer_data.success = false;
		}

		done(thread_index, task);
	}
}

void
RenderQueue::done(int thread_index, const Task::Handle &task)
{
	assert(task);
	int single_signals = 0;
	int signals = 0;
	Glib::Threads::Mutex::Lock lock(mutex);
	for(Task::Set::iterator i = task->renderer_data.back_deps.begin(); i != task->renderer_data.back_deps.end(); ++i)
	{
		assert(*i);
		--(*i)->renderer_data.deps_count;
		if ((*i)->renderer_data.deps_count == 0)
		{
			bool mt = (*i)->get_allow_multithreading();
			TaskQueue &queue = mt ? ready_tasks     : single_ready_tasks;
			TaskSet   &wait  = mt ? not_ready_tasks : single_not_ready_tasks;
			wait.erase(*i);
			queue.push_back(*i);
			++(mt ? signals : single_signals);
		}
	}
	task->renderer_data.back_deps.clear();
	assert( tasks_in_process.count(thread_index) == 1 );
	tasks_in_process.erase(thread_index);
	//info("rendering threads used %d", tasks_in_process.size());

	// limit signals count
	int threads = get_threads_count() - 1;
	if (signals > threads) signals = threads;
	if (single_signals > 1) single_signals = 1;

	// we don't need to wakeup the current thread
	--(thread_index ? signals : single_signals);

	// wake up
	while(signals-- > 0) cond.signal();
	while(single_signals-- > 0) single_cond.signal();
}

Task::Handle
RenderQueue::get(int thread_index)
{
	Glib::Threads::Mutex::Lock lock(mutex);

	TaskQueue &queue  = thread_index == 0 ? single_ready_tasks     : ready_tasks;
	TaskQueue &queue2 = thread_index != 0 ? single_ready_tasks     : ready_tasks;
	TaskSet   &wait   = thread_index == 0 ? single_not_ready_tasks : not_ready_tasks;
	while(started)
	{
		if (!queue.empty())
		{
			Task::Handle task = queue.front();
			queue.pop_front();
			assert( tasks_in_process.count(thread_index) == 0 );
			tasks_in_process[thread_index] = task;
			//info("rendering threads used %d", tasks_in_process.size());
			return task;
		}

		#ifdef DEBUG_THREAD_WAIT
		if (!wait.empty())
			info("thread %d: rendering wait for task", thread_index);
		#endif

		assert( wait.empty() || !tasks_in_process.empty() || !queue2.empty() );

		(thread_index ? cond : single_cond).wait(mutex);
	}
	return Task::Handle();
}

void
RenderQueue::fix_task(const Task &task, const Task::RunParams &params)
{
	//for(Task::List::iterator i = task.back_deps.begin(); i != task.back_deps.end();)
	//	if (*i) ++i; else i = (*i)->back_deps.erase(i);
	task.renderer_data.params = params;
	task.renderer_data.params.sub_queue.clear();
	task.renderer_data.success = true;
}

int
RenderQueue::get_threads_count() const
{
	return threads.size();
}

void
RenderQueue::enqueue(const Task::Handle &task, const Task::RunParams &params)
{
	if (!task) return;
	fix_task(*task, params);
	Glib::Threads::Mutex::Lock lock(mutex);

	bool mt = task->get_allow_multithreading();
	TaskQueue &queue = mt ? ready_tasks     : single_ready_tasks;
	TaskSet   &wait  = mt ? not_ready_tasks : single_not_ready_tasks;
	if (task->renderer_data.deps_count == 0) {
		queue.push_back(task);
		(mt ? cond : single_cond).signal();
	}
	else
	{
		wait.insert(task);
	}
}

void
RenderQueue::enqueue(const Task::List &tasks, const Task::RunParams &params)
{
	Task::RunParams p(params);
	p.sub_queue.clear();
	int count = 0;
	for(Task::List::const_iterator i = tasks.begin(); i != tasks.end(); ++i)
		if (*i) { fix_task(**i, p); ++count; }
	if (!count) return;

	Glib::Threads::Mutex::Lock lock(mutex);
	int single_signals = 0;
	int signals = 0;
	for(Task::List::const_iterator i = tasks.begin(); i != tasks.end(); ++i)
	{
		if (*i)
		{
			bool mt = (*i)->get_allow_multithreading();
			TaskQueue &queue = mt ? ready_tasks     : single_ready_tasks;
			TaskSet   &wait  = mt ? not_ready_tasks : single_not_ready_tasks;
			if ((*i)->renderer_data.deps_count == 0) {
				queue.push_back(*i);
				++(mt ? signals : single_signals);
			} else {
				wait.insert(*i);
			}
		}
	}

	// limit signals count
	int threads = get_threads_count() - 1;
	if (signals > threads) signals = threads;
	if (single_signals > 1) single_signals = 1;

	// wake up
	while(signals-- > 0) cond.signal();
	while(single_signals-- > 0) single_cond.signal();
}

void
RenderQueue::clear()
{
	Glib::Threads::Mutex::Lock lock(mutex);
	ready_tasks.clear();
	single_ready_tasks.clear();
	not_ready_tasks.clear();
	single_not_ready_tasks.clear();
}

/* === E N T R Y P O I N T ================================================= */
