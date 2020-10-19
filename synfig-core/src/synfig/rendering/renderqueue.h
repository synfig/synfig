/* === S Y N F I G ========================================================= */
/*!	\file synfig/rendering/renderqueue.h
**	\brief RenderQueue Header
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

#ifndef __SYNFIG_RENDERING_RENDERQUEUE_H
#define __SYNFIG_RENDERING_RENDERQUEUE_H

/* === H E A D E R S ======================================================= */

#include <map>

#include <mutex>
#include <condition_variable>
#include <thread>

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
	typedef std::list<std::thread> ThreadList;
	typedef std::map<int, Task::Handle> ThreadTaskMap;
	typedef std::set<Task::Handle> TaskSet;
	typedef std::list<Task::Handle> TaskQueue;

private:
	static int last_batch_index;

	std::mutex mutex;
	std::mutex threads_mutex;
	std::condition_variable cond;
	std::condition_variable single_cond;

	TaskQueue ready_tasks;
	TaskQueue single_ready_tasks;
	TaskSet not_ready_tasks;
	TaskSet single_not_ready_tasks;

	bool started;

	ThreadList threads;
	ThreadTaskMap tasks_in_process;

	void start();
	void stop();

	void process(int thread_index);
	void done(int thread_index, const Task::Handle &task);
	Task::Handle get(int thread_index);

	static void fix_task(const Task &task, const Task::RunParams &params);
	bool remove_if_orphan(const Task::Handle &task, bool in_queue);
	void remove_orphans();
	bool remove_task(const Task::Handle &task);

public:
	RenderQueue();
	~RenderQueue();

	int get_threads_count() const;
	void enqueue(const Task::Handle &task, const Task::RunParams &params);
	void enqueue(const Task::List &tasks, const Task::RunParams &params);
	void cancel(const Task::Handle &task);
	void cancel(const Task::List &list);
	void clear();
};

} /* end namespace rendering */
} /* end namespace synfig */

/* -- E N D ----------------------------------------------------------------- */

#endif
