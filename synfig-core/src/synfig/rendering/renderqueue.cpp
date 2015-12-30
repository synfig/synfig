/* === S Y N F I G ========================================================= */
/*!	\file synfig/rendering/renderqueue.cpp
**	\brief RenderQueue
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

/* === H E A D E R S ======================================================= */

#ifdef USING_PCH
#	include "pch.h"
#else
#ifdef HAVE_CONFIG_H
#	include <config.h>
#endif

#ifndef WIN32
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
#ifdef WITH_OPENGL
#include "opengl/task/taskgl.h"
#endif

#endif

using namespace synfig;
using namespace rendering;


#define SYNFIG_RENDERING_MAX_THREADS 256


#ifdef _DEBUG
//#define DEBUG_THREAD_TASK
//#define DEBUG_THREAD_WAIT
#endif


/* === M A C R O S ========================================================= */

/* === G L O B A L S ======================================================= */

/* === P R O C E D U R E S ================================================= */

/* === M E T H O D S ======================================================= */

RenderQueue::RenderQueue(): started(false) { start(); }
RenderQueue::~RenderQueue() { stop(); }

void
RenderQueue::start()
{
	Glib::Threads::Mutex::Lock lock(mutex);
	if (started) return;

	// one thread reserved for OpenGL
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
		condgl.broadcast();
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
		info("thread %d: begin task #%d '%s'", thread_index, task->index, typeid(*task).name());
		#endif

		assert( task->check() );

		if (!task->run(task->params))
			task->success = false;

		#ifdef DEBUG_TASK_SURFACE
		debug::DebugSurface::save_to_file(task->target_surface, etl::strprintf("task%d", task->index));
		#endif

		#ifdef DEBUG_THREAD_TASK
		info("thread %d: end task #%d '%s'", thread_index, task->index, typeid(*task).name());
		#endif

		done(thread_index, task);
	}
}

void
RenderQueue::done(int thread_index, const Task::Handle &task)
{
	assert(task);
	bool found = false;
	Glib::Threads::Mutex::Lock lock(mutex);
	for(Task::Set::iterator i = task->back_deps.begin(); i != task->back_deps.end(); ++i)
	{
		assert(*i);
		--(*i)->deps_count;
		if ((*i)->deps_count == 0)
		{
#ifdef WITH_OPENGL
			bool gl = i->type_is<TaskGL>();
#else
			bool gl = false;
#endif
			TaskQueue &queue = gl ? gl_ready_tasks     : ready_tasks;
			TaskSet   &wait  = gl ? gl_not_ready_tasks : not_ready_tasks;
			wait.erase(*i);
			queue.push_back(*i);

			// current process will take one task,
			// so we don't need to call signal by first time
			if (!found)
				found = true;
			else
				(gl ? condgl : cond).signal();
		}
	}
	task->back_deps.clear();
	assert( tasks_in_process.count(thread_index) == 1 );
	tasks_in_process.erase(thread_index);
	//info("rendering threads used %d", tasks_in_process.size());
}

Task::Handle
RenderQueue::get(int thread_index)
{
	Glib::Threads::Mutex::Lock lock(mutex);

	TaskQueue &queue = thread_index == 0 ? gl_ready_tasks     : ready_tasks;
	TaskSet   &wait  = thread_index == 0 ? gl_not_ready_tasks : not_ready_tasks;
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

		assert( wait.empty() || !tasks_in_process.empty() );

		(thread_index ? cond : condgl).wait(mutex);
	}
	return Task::Handle();
}

void
RenderQueue::fix_task(const Task &task, const Task::RunParams &params)
{
	//for(Task::List::iterator i = task.back_deps.begin(); i != task.back_deps.end();)
	//	if (*i) ++i; else i = (*i)->back_deps.erase(i);
	task.params = params;
	task.success = true;
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

#ifdef WITH_OPENGL
	bool gl = task.type_is<TaskGL>();
#else
	bool gl = false;
#endif
	TaskQueue &queue = gl ? gl_ready_tasks     : ready_tasks;
	TaskSet   &wait  = gl ? gl_not_ready_tasks : not_ready_tasks;
	if (task->deps_count == 0) {
		queue.push_back(task);
		(gl ? condgl : cond).signal();
	}
	else
	{
		wait.insert(task);
	}
}

void
RenderQueue::enqueue(const Task::List &tasks, const Task::RunParams &params)
{
	int count = 0;
	for(Task::List::const_iterator i = tasks.begin(); i != tasks.end(); ++i)
		if (*i) { fix_task(**i, params); ++count; }
	if (!count) return;

	Glib::Threads::Mutex::Lock lock(mutex);
	int glsignals = 0;
	int signals = 0;
	int threads = get_threads_count() - 1;
	for(Task::List::const_iterator i = tasks.begin(); i != tasks.end(); ++i)
	{
		if (*i)
		{
#ifdef WITH_OPENGL
			bool gl = i->type_is<TaskGL>();
#else
			bool gl = false;
#endif
			TaskQueue &queue = gl ? gl_ready_tasks     : ready_tasks;
			TaskSet   &wait  = gl ? gl_not_ready_tasks : not_ready_tasks;
			if ((*i)->deps_count == 0) {
				queue.push_back(*i);
				if (gl)
				{
					if (glsignals < 1) { condgl.signal(); ++glsignals; }
				}
				else
				{
					if (signals < threads) { cond.signal(); ++signals; }
				}
			}
			else
			{
				wait.insert(*i);
			}
		}
	}
}

void
RenderQueue::clear()
{
	Glib::Threads::Mutex::Lock lock(mutex);
	ready_tasks.clear();
	gl_ready_tasks.clear();
	not_ready_tasks.clear();
	gl_not_ready_tasks.clear();
}

/* === E N T R Y P O I N T ================================================= */
