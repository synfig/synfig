/* === S Y N F I G ========================================================= */
/*!	\file threadpool.cpp
**	\brief ThreadPool File
**
**	$Id$
**
**	\legal
**	......... ... 2018 Ivan Mahonin
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

#include <synfig/localization.h>
#include <synfig/general.h>

#include "threadpool.h"

#endif

/* === U S I N G =========================================================== */

using namespace std;
using namespace synfig;

/* === M A C R O S ========================================================= */

/* === G L O B A L S ======================================================= */

/* === M E T H O D S ======================================================= */

ThreadPool ThreadPool::instance;

ThreadPool::ThreadPool():
	max_running_threads(0),
	running_threads(0),
	ready_threads(0),
	stopped(false)
{
	max_running_threads = g_get_num_processors();
	if (max_running_threads < 1) max_running_threads = 1;
	if (max_running_threads > 2) --max_running_threads;
	++running_threads;
}

ThreadPool::ThreadPool(const ThreadPool&):
	max_running_threads(0),
	running_threads(0),
	ready_threads(0),
	stopped(false) { }


ThreadPool::~ThreadPool() { stop(); }

void
ThreadPool::thread_loop() {
	info("started new thread in ThreadPool");
	while(true) {
		Slot slot;
		{
			Glib::Threads::Mutex::Lock lock(mutex);
			while (queue.empty()) {
				if (stopped) return;
				++ready_threads;
				wait(cond, mutex);
				--ready_threads;
			}
			slot = queue.front();
			queue.pop();
		}
		slot();
		--running_threads;
	}
	info("thread in ThreadPool stopped");
}

void
ThreadPool::stop() {
	{
		Glib::Threads::Mutex::Lock lock(mutex);
		stopped = true;
		cond.broadcast();
	}

	while(true) {
		Glib::Threads::Thread *thread = 0;
		{
			Glib::Threads::Mutex::Lock lock(mutex);
			if (threads.empty()) break;
			thread = threads.front();
			threads.pop_front();
		}
		thread->join();
	}
}


