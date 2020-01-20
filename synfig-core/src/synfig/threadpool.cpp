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


//#define DEBUG_PTHREAD_MEASURE

#ifdef DEBUG_PTHREAD_MEASURE
#include <pthread.h>
#include <time.h>
#endif

#include <cassert>

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

ThreadPool* ThreadPool::instance_ = 0;


// ThreadPool::Group

ThreadPool::Group::Group():
	multithreading(), running_threads(0), sum_weight() { }

ThreadPool::Group::~Group()
	{ run(); }

void
ThreadPool::Group::process(int begin, int end) {
	for(int i = begin; i < end; ++i)
		try { tasks[i].second(); } catch(...) { }
	Glib::Threads::Mutex::Lock lock(mutex);
	if (!--running_threads) cond.signal();
}

void
ThreadPool::Group::enqueue(const Slot &slot, Real weight) {
	weight = std::max(real_precision<Real>(), std::min(1.0/real_precision<Real>(), weight));
	tasks.push_back(Entry(weight, slot));
	sum_weight += weight;
}

void
ThreadPool::Group::run(bool force_thread) {
	// enqueue parallel tasks
	Real sum = 0.0;
	int begin = 0;
	int end = 0;

	for(int i = 0; i < (int)tasks.size(); ++i) {
		end = i + 1;
		sum += tasks[i].first;
		if (sum_weight - sum < 0.75) break;
		if (sum >= 0.75) {
			multithreading = true;
			++running_threads;
			instance().enqueue( sigc::bind( sigc::mem_fun(this, &Group::process), begin, end ));
			sum_weight -= sum;
			sum = 0.0;
			begin = end;
		}
	}
	end = (int)tasks.size();

	// run in current thread
	if (begin < end) {
		if (force_thread) {
			multithreading = true;
			++running_threads;
			instance().enqueue( sigc::bind( sigc::mem_fun(this, &Group::process), begin, end ));
		} else {
			for(int i = begin; i < end; ++i)
				tasks[i].second();
		}
	}

	// wait
	if (multithreading) {
		Glib::Threads::Mutex::Lock lock(mutex);
		while(running_threads > 0) instance().wait(cond, mutex);
	}

	// reset
	multithreading = false;
	tasks.clear();
	sum_weight = 0.0;
}


// ThreadPool

ThreadPool::ThreadPool():
	max_running_threads(0),
	last_thread_id(0),
	running_threads(0),
	ready_threads(0),
	stopped(false)
{
	max_running_threads = g_get_num_processors();

	if (const char *s = getenv("SYNFIG_GENERIC_THREADS"))
		max_running_threads = atoi(s) + 1;

	if (max_running_threads < 2) max_running_threads = 2;
	if (max_running_threads > 2) --max_running_threads;
	++running_threads;

	#ifdef DEBUG_PTHREAD_MEASURE
	info("ThreadPool created with max running threads: %d", max_running_threads - 1);
	#endif
}

ThreadPool::ThreadPool(const ThreadPool&):
	max_running_threads(0),
	last_thread_id(0),
	running_threads(0),
	ready_threads(0),
	queue_size(0),
	stopped(false) { }

ThreadPool::~ThreadPool() {
	#ifdef DEBUG_PTHREAD_MEASURE
	info("ThreadPool destroying with tasks in queue: %d, and tasks in process: %d", (int)queue_size, (int)running_threads);
	#endif

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
			stopped = true;
			cond.broadcast();
			thread = threads.back();
			threads.pop_back();
		}
		thread->join();
	}

	{
		#ifdef DEBUG_PTHREAD_MEASURE
		Glib::Threads::Mutex::Lock lock(mutex);
		info("ThreadPool destroyed with unprocessed tasks in queue: %d", queue.size());
		#endif
	}
}

void
ThreadPool::thread_loop(int
	#ifdef DEBUG_PTHREAD_MEASURE
	id
	#endif
) {
	++running_threads;

	#ifdef DEBUG_PTHREAD_MEASURE
	info("started new thread #%d in ThreadPool", id);

	clockid_t clock_id;
	pthread_getcpuclockid(pthread_self(), &clock_id);
	#endif

	while(true) {
		Slot slot;
		{
			Glib::Threads::Mutex::Lock lock(mutex);
			while(!stopped && (queue.empty() || running_threads > max_running_threads)) {
				++ready_threads;
				--running_threads;
				cond.wait(mutex);
				++running_threads;
				--ready_threads;
			}
			if (stopped) break;
			slot = queue.front();
			queue.pop();
			--queue_size;
		}

		#ifdef DEBUG_PTHREAD_MEASURE
		struct timespec spec;
		clock_gettime(clock_id, &spec);
		long long time0 = spec.tv_sec*1000000000ll + spec.tv_nsec;
		long long rtime0 = g_get_monotonic_time();

		info( "ThreadPool thread #%d: begin, running threads: %d, ready threads: %d, queue size: %d",
				id,
				(int)running_threads,
				(int)ready_threads,
				(int)queue_size );
		#endif

		slot();

		#ifdef DEBUG_PTHREAD_MEASURE
		clock_gettime(clock_id, &spec);
		long long time1 = spec.tv_sec*1000000000ll + spec.tv_nsec;
		long long rtime1 = g_get_monotonic_time();

		info( "ThreadPool thread #%d: processed task for %.6f (real %.6f)",
				id,
				(double)(time1 - time0)*1e-9,
				(double)(rtime1 - rtime0)*1e-6 );
		#endif
	}

	#ifdef DEBUG_PTHREAD_MEASURE
	info("thread #%d in ThreadPool stopped", id);
	#endif

	--running_threads;
}

void
ThreadPool::wakeup() {
	int to_wakeup = std::max(0, std::min((int)queue_size, max_running_threads - (int)running_threads));
	int to_create = std::max(0, to_wakeup - (int)ready_threads);
	to_wakeup     = std::max(0, to_wakeup - to_create);
	while(to_create-- > 0)
		threads.push_back(
			Glib::Threads::Thread::create(
				sigc::bind( sigc::mem_fun(this, &ThreadPool::thread_loop), ++last_thread_id )));
	while(to_wakeup-- > 0)
		cond.signal();
}

void
ThreadPool::enqueue(const Slot &slot) {
	Glib::Threads::Mutex::Lock lock(mutex);
	++queue_size;
	queue.push(slot);
	wakeup();
}

void
ThreadPool::wait(Glib::Threads::Cond &cond, Glib::Threads::Mutex &mutex) {
	if (--running_threads < max_running_threads)
		if (queue_size) // wakeup or create ready thread if we have tasks in queue
			{ Glib::Threads::Mutex::Lock lock(this->mutex); wakeup(); }
	cond.wait(mutex);
	++running_threads;
}

ThreadPool&
ThreadPool::instance() {
	assert(instance_);
	return *instance_;
}

bool
ThreadPool::subsys_init() {
	assert(!instance_);
	if (!instance_) instance_ = new ThreadPool();
	return true;
}

bool
ThreadPool::subsys_stop() {
	assert(instance_);
	if (instance_) delete instance_;
	instance_ = 0;
	return true;
}
