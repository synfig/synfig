/*! ========================================================================
** Extended Template and Library
** Mutex Abstraction Class Implementation
** $Id$
**
** Copyright (c) 2002 Robert B. Quattlebaum Jr.
**
** This package is free software; you can redistribute it and/or
** modify it under the terms of the GNU General Public License as
** published by the Free Software Foundation; either version 2 of
** the License, or (at your option) any later version.
**
** This package is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
** General Public License for more details.
**
** === N O T E S ===========================================================
**
** This is an internal header file, included by other ETL headers.
** You should not attempt to use it directly.
**
** ========================================================================= */

/* === S T A R T =========================================================== */

#ifndef __ETL__MUTEX_PTHREADS_H_
#define __ETL__MUTEX_PTHREADS_H_

/* === H E A D E R S ======================================================= */

#define __USE_GNU

#include <pthread.h>

#ifdef HAVE_SCHED_H
# include <sched.h>
#endif

/* === M A C R O S ========================================================= */

/* === C L A S S E S & S T R U C T S ======================================= */

namespace etl {

class mutex
{
	pthread_mutex_t mtx;
	pthread_t locker;
	int depth;
public:

	mutex()
	{
		pthread_mutexattr_t attr;
		pthread_mutexattr_init(&attr);
		//#ifdef PTHREAD_PRIO_INHERIT
		//pthread_mutexattr_setprioceiling(&attr,PTHREAD_PRIO_INHERIT);
		//#endif
		#ifdef PTHREAD_MUTEX_RECURSIVE
		pthread_mutexattr_settype(&attr,PTHREAD_MUTEX_RECURSIVE);
		#endif
		pthread_mutex_init(&mtx,&attr);
		pthread_mutexattr_destroy(&attr);
		locker=0;
		depth=0;
	}

	~mutex()
	{ pthread_mutex_destroy(&mtx); }


	//! Exception-safe mutex lock class
	class lock
	{
		mutex *_mtx;
	public:
		lock(mutex &x):_mtx(&x) { _mtx->lock_mutex(); }
		~lock() { _mtx->unlock_mutex(); }
		mutex &get() { return *_mtx; }
	};

	void lock_mutex(void)
	{
		if(!locker || locker!=pthread_self())
		{
			pthread_mutex_lock(&mtx);
			locker=pthread_self();
			depth=0;
			return;
		}
		depth++;
	}

	bool try_lock_mutex(void)
	{ return !(bool) pthread_mutex_trylock(&mtx); }

	void unlock_mutex(void)
	{
		if(depth)
		{
			depth--;
			return;
		}
		pthread_mutex_unlock(&mtx);
		locker=0;
	}
};

};

/* === E X T E R N S ======================================================= */

/* === E N D =============================================================== */

#endif
