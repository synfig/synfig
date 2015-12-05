/* === S Y N F I G ========================================================= */
/*!	\file synfig/rendering/renderqueue.h
**	\brief RenderQueue Header
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

#ifndef __SYNFIG_RENDERING_RENDERQUEUE_H
#define __SYNFIG_RENDERING_RENDERQUEUE_H

/* === H E A D E R S ======================================================= */

#include <cstdio>

#include <map>

#include <glibmm/threads.h>

#include "task.h"

/* === M A C R O S ========================================================= */

/* === T Y P E D E F S ===================================================== */

/* === C L A S S E S & S T R U C T S ======================================= */

namespace synfig
{
namespace rendering
{

class RenderQueue
{
public:
	typedef std::list<Glib::Threads::Thread*> ThreadList;
	typedef std::map<int, Task::Handle> ThreadTaskMap;
	typedef std::set<Task::Handle> TaskSet;
	typedef std::list<Task::Handle> TaskQueue;

private:
	Glib::Threads::Mutex mutex;
	Glib::Threads::Mutex threads_mutex;
	Glib::Threads::Cond cond;
	Glib::Threads::Cond condgl;

	TaskQueue ready_tasks;
	TaskQueue gl_ready_tasks;
	TaskSet not_ready_tasks;
	TaskSet gl_not_ready_tasks;

	bool started;

	ThreadList threads;
	ThreadTaskMap tasks_in_process;

	void start();
	void stop();

	void process(int thread_index);
	void done(int thread_index, const Task::Handle &task);
	Task::Handle get(int thread_index);

	static void fix_task(const Task &task, const Task::RunParams &params);

public:
	RenderQueue();
	~RenderQueue();

	int get_threads_count() const;
	void enqueue(const Task::Handle &task, const Task::RunParams &params);
	void enqueue(const Task::List &tasks, const Task::RunParams &params);
	void clear();
};

} /* end namespace rendering */
} /* end namespace synfig */

/* -- E N D ----------------------------------------------------------------- */

#endif
