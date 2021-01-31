/* === S Y N F I G ========================================================= */
/*!	\file threadpool.h
**	\brief ThreadPool Header
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

/* === S T A R T =========================================================== */

#ifndef __SYNFIG_THREADPOOL_H
#define __SYNFIG_THREADPOOL_H

/* === H E A D E R S ======================================================= */

#include <atomic>
#include <queue>

#include <sigc++/signal.h>
#include <mutex>
#include <condition_variable>
#include <thread>

#include "real.h"

/* === M A C R O S ========================================================= */

/* === T Y P E D E F S ===================================================== */

/* === C L A S S E S & S T R U C T S ======================================= */

namespace synfig {

class ThreadPool {
public:
	typedef sigc::slot<void> Slot;

	class Group {
	public:
	typedef std::pair<Real, Slot> Entry;
	typedef std::vector<Entry> List;

	private:
		bool multithreading;
		std::atomic<int> running_threads;
		std::mutex mutex;
		std::condition_variable cond;

		List tasks;
		Real sum_weight;

		void process(int begin, int end);
	public:
		Group();
		~Group();

		void enqueue(const Slot &slot, Real weight = 1.0);
		void run(bool force_thread = false);
	};

private:
	std::mutex mutex;
	std::condition_variable cond;
	int max_running_threads;
	int last_thread_id;
	std::atomic<int> running_threads;
	std::atomic<int> ready_threads;
	std::atomic<int> queue_size;
	std::queue<Slot> queue;
	std::vector<std::thread*> threads;
	bool stopped;

	static ThreadPool *instance_;

	void thread_loop(int id);
	void wakeup();

	ThreadPool();
	ThreadPool(const ThreadPool&) = delete;

public:
	~ThreadPool();

	void enqueue(const Slot &slot);
	void wait(std::condition_variable &cond, std::unique_lock<std::mutex>& lock);

	void set_num_threads(int num_threads);

	int get_max_threads() const
		{ return max_running_threads; }
	int get_running_threads() const
		{ return running_threads; }
	int get_queue_size() const
		{ return queue_size + running_threads; }

	static ThreadPool& instance();
	static bool subsys_init();
	static bool subsys_stop();
};

}; // END of namespace synfig

/* === E N D =============================================================== */

#endif
