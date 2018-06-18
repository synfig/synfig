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
#include <glibmm/threads.h>

/* === M A C R O S ========================================================= */

/* === T Y P E D E F S ===================================================== */

/* === C L A S S E S & S T R U C T S ======================================= */

namespace synfig {

class ThreadPool {
public:
	typedef sigc::slot<void> Slot;

	class Group {
	private:
		bool multithreading;
		std::atomic<int> running_threads;
		Glib::Threads::Mutex mutex;
		Glib::Threads::Cond cond;

		void wrap(const Slot &slot) {
			try { slot(); } catch(...) { }
			Glib::Threads::Mutex::Lock lock(mutex);
			if (!--running_threads) cond.signal();
		}

	public:
		Group(): multithreading(), running_threads(0) { }
		~Group() { wait(); }

		void enqueue(const Slot &slot) {
			multithreading = true;
			++running_threads;
			instance.enqueue(sigc::bind( sigc::mem_fun(this, &Group::wrap), slot ));
		}

		void call(const Slot &slot) {
			if (instance.pre_push()) {
				multithreading = true;
				++running_threads;
				instance.push(sigc::bind( sigc::mem_fun(this, &Group::wrap), slot ));
			} else {
				slot();
			}
		}

		void wait() {
			if (multithreading) {
				Glib::Threads::Mutex::Lock lock(mutex);
				while(running_threads) instance.wait(cond, mutex);
			}
		}
	};

private:
	Glib::Threads::Mutex mutex;
	Glib::Threads::Cond cond;
	int max_running_threads;
	std::atomic<int> running_threads;
	std::atomic<int> ready_threads;
	std::queue<Slot> queue;
	std::list<Glib::Threads::Thread*> threads;
	bool stopped;

	void thread_loop();
	void stop();

	bool pre_push() {
		if (++running_threads <= max_running_threads)
			return true;
		--running_threads;
		return false;
	}

	void push(const Slot &slot) {
		Glib::Threads::Mutex::Lock lock(mutex);
		queue.push(slot);
		if (ready_threads < (int)queue.size())
			threads.push_back(
				Glib::Threads::Thread::create(
						sigc::mem_fun(this, &ThreadPool::thread_loop) ));
		cond.signal();
	}

	ThreadPool();
	ThreadPool(const ThreadPool&);

public:
	static ThreadPool instance;

	~ThreadPool();

	void enqueue(const Slot &slot)
		{ ++running_threads; push(slot); }
	void call(const Slot &slot)
		{ if (pre_push()) push(slot); else slot(); }

	int get_max_threads() const
		{ return max_running_threads; }
	int get_running_threads() const
		{ return running_threads; }

	void wait(Glib::Threads::Cond &cond, Glib::Threads::Mutex &mutex) {
		--instance.running_threads;
		cond.wait(mutex);
		++instance.running_threads;
	}
};

}; // END of namespace synfig

/* === E N D =============================================================== */

#endif
