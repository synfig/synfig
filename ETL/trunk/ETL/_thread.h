/*! ========================================================================
** Extended Template and Library
** Thread Abstraction Class Implementation
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

#ifndef __ETL__THREAD_H_
#define __ETL__THREAD_H_

/* === H E A D E R S ======================================================= */

#define __USE_GNU

#ifdef HAVE_PTHREAD_H
# include <pthread.h>
#endif

#ifdef HAVE_SCHED_H
# include <sched.h>
#endif

#ifdef HAVE_CREATETHREAD
# include <windows.h>
#endif

/* === M A C R O S ========================================================= */

#if ( defined (HAVE_PTHREAD_CREATE) || defined (HAVE_CLONE) || defined (HAVE_CREATETHREAD) ) && !defined (NO_THREADS)
# define CALLISTO_THREADS
#endif

#define THREAD_ENTRYPOINT

/* === C L A S S E S & S T R U C T S ======================================= */

#if defined(CALLISTO_THREADS) && defined(HAVE_PTHREAD_CREATE)
static inline void Yield(void)
{
	sched_yield();
	pthread_testcancel();
}
#else
#ifdef Yield
	#undef Yield
#endif
inline void Yield(void) { }
#endif

#ifdef CALLISTO_THREADS

#ifdef HAVE_PTHREAD_CREATE

class Thread
{
public:
	typedef void* entrypoint_return;
private:

	pthread_t thread;
	int *references;
	entrypoint_return (*entrypoint)(void *);
	void *context;
public:
	Thread(void *(*ep)(void *)=NULL,void *context=NULL):
		references(NULL),entrypoint(ep),context(context) { }
	Thread(const Thread &t)
	{
		thread=t.thread;
		references=t.references;
		entrypoint=t.entrypoint;
		context=t.context;
		if(references)
			(*references)++;
	}
	const Thread &operator=(const Thread &rhs)
	{
		if(references)
		{
			(*references)--;
			if(*references==0)
				stop();
		}
		thread=rhs.thread;
		references=rhs.references;
		entrypoint=rhs.entrypoint;
		context=rhs.context;
		if(references)
			(*references)++;
		return *this;
	}

	void start(void)
	{
		references = new int;
		*references = 1;
		pthread_create(&thread,NULL,entrypoint,context);
//		pthread_detach(thread);
	}

	void stop(void)
	{
		delete references;
		references=NULL;
		void *exit_status;
		pthread_cancel(thread);
		pthread_join(thread,&exit_status);
	}

	static void TestStop()
	{
		pthread_testcancel();
	}

	static void SyncStop()
	{
		int i;
		pthread_setcanceltype(PTHREAD_CANCEL_DEFERRED,&i);
	}

	static void AsyncStop()
	{
		int i;
		pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS,&i);
	}

	~Thread()
	{
		if(references)
		{
			(*references)--;
			if(*references==0)
				stop();
		}
	}
};

class Mutex
{
	pthread_mutex_t mutex;
	pthread_t locker;
	int depth;
public:

	Mutex()
	{
		pthread_mutexattr_t attr;
		pthread_mutexattr_init(&attr);
		//#ifdef PTHREAD_PRIO_INHERIT
		//pthread_mutexattr_setprioceiling(&attr,PTHREAD_PRIO_INHERIT);
		//#endif
		#ifdef PTHREAD_MUTEX_RECURSIVE
		pthread_mutexattr_settype(&attr,PTHREAD_MUTEX_RECURSIVE);
		#endif
		pthread_mutex_init(&mutex,&attr);
		pthread_mutexattr_destroy(&attr);
		locker=0;
		depth=0;
	}

	~Mutex()
	{ pthread_mutex_destroy(&mutex); }

	void Lock(void)
	{
		if(!locker || locker!=pthread_self())
		{
			pthread_mutex_lock(&mutex);
			locker=pthread_self();
			depth=0;
			return;
		}
		depth++;
	}

	bool TryLock(void)
	{ return !(bool) pthread_mutex_trylock(&mutex); }

	void UnLock(void)
	{
		if(depth)
		{
			depth--;
			return;
		}
		pthread_mutex_unlock(&mutex);
		locker=0;
	}
};

#ifdef HAVE_PTHREAD_RW_LOCK_INIT
class ReadWriteLock
{
	pthread_rwlock_t rwlock;
public:

	ReadWriteLock()
	{ pthread_rwlock_init(&rwlock,NULL); }

	~ReadWriteLock()
	{ pthread_rwlock_destroy(&rwlock); }

	void LockRead(void)
	{ pthread_rwlock_rdlock(&rwlock); }

	void LockWrite(void)
	{ pthread_rwlock_wrlock(&rwlock); }

	bool TryLockRead(void)
	{ return !(bool)pthread_rwlock_tryrdlock(&rwlock); }

	bool TryLockWrite(void)
	{ return !(bool)pthread_rwlock_trywrlock(&rwlock); }

	void UnLockWrite(void)
	{ pthread_rwlock_unlock(&rwlock); }

	void UnLockRead(void)
	{ pthread_rwlock_unlock(&rwlock); }
};
#else
//*
class ReadWriteLock : public Mutex
{
public:

	ReadWriteLock()
	{  }

	~ReadWriteLock()
	{  }

	void LockRead(void)
	{ Lock(); }

	void LockWrite(void)
	{ Lock(); }

	bool TryLockRead(void)
	{ return TryLock(); }

	bool TryLockWrite(void)
	{ return TryLock(); }

	void UnLockWrite(void)
	{ UnLock(); }

	void UnLockRead(void)
	{ UnLock(); }
};
#endif

/*
class Condition
{
	pthread_cond_t cond;
	pthread_mutex_t mutex;
public:
	Condition()
	{ pthread_cond_init(&cond,NULL); pthread_mutex_init(&mutex,NULL); }
	~Condition()
	{ pthread_cond_destroy(&cond); pthread_mutex_destroy(&mutex);}
	void operator()(void)
	{ pthread_cond_signal(&cond); }
	void Wait(void)
	{
		pthread_mutex_lock(&mutex);
		pthread_cond_wait(&cond,&mutex);
		pthread_mutex_unlock(&mutex);
	}
};
*/

#else // if defined HAVE_PTHREAD
#ifdef HAVE_CREATETHREAD


#ifdef THREAD_ENTRYPOINT
#undef THREAD_ENTRYPOINT
#endif
#define THREAD_ENTRYPOINT	__stdcall
class Thread
{
public:
	typedef unsigned long entrypoint_return;
private:

	unsigned long thread;
	HANDLE handle;
	int *references;

	entrypoint_return  (THREAD_ENTRYPOINT *entrypoint)(void *);

	void *context;

	HDC hdc;
	HGLRC hglrc;

	static entrypoint_return THREAD_ENTRYPOINT thread_prefix(void*data)
	{
		Thread *thread=(Thread *)data;

		if(thread->hglrc)
			wglMakeCurrent(thread->hdc, thread->hglrc);

		return thread->entrypoint(thread->context);
	}

public:
	Thread(entrypoint_return  (THREAD_ENTRYPOINT *ep)(void *)=NULL,void *context=NULL):
		references(NULL),entrypoint(ep),context(context) { }
	Thread(const Thread &t)
	{
		thread=t.thread;
		handle=t.handle;
		references=t.references;
		entrypoint=t.entrypoint;
		context=t.context;
		handle=NULL;
		if(references)
			(*references)++;
	}
	const Thread &operator=(const Thread &rhs)
	{
		if(references)
		{
			(*references)--;
			if(*references==0)
				stop();
		}
		thread=rhs.thread;
		handle=rhs.handle;
		references=rhs.references;
		entrypoint=rhs.entrypoint;
		context=rhs.context;
		if(references)
			(*references)++;
		return *this;
	}

	void start(void)
	{
		references = new int;
		*references = 1;

		hglrc=wglGetCurrentContext();
		hdc=wglGetCurrentDC();

		handle=CreateThread(
			NULL,		// Security stuff
			0,	// STACK
			thread_prefix,    // thread function
			(void*)this,                       // thread argument
			0,                    // creation option
			&thread                        // thread identifier
		);
	}

	void stop(void)
	{
		delete references;
		references=NULL;

		TerminateThread(handle, FALSE);
	}

	int wait(void)
	{
		if(handle)
		{
			WaitForSingleObject(handle, INFINITE);
			CloseHandle(handle);
		}
		return 0;
	}

	static void TestStop()
	{
	}

	static void SyncStop()
	{
	}

	static void AsyncStop()
	{
	}

	~Thread()
	{
		if(references)
		{
			(*references)--;
			if(*references==0)
				stop();
		}
	}
};

class Mutex
{
	HANDLE handle;
public:

	Mutex()
	{
		handle = CreateMutex(NULL, FALSE, NULL);
	}

	~Mutex()
	{
		CloseHandle(handle);
	}

	void Lock(void)
	{
		WaitForSingleObject(handle, INFINITE);
	}

	bool TryLock(void)
	{
		return WaitForSingleObject(handle, INFINITE)==WAIT_FAILED;
	}

	void UnLock(void)
	{
		ReleaseMutex(handle);
	}
};


#endif // if defined HAVE_CREATETHREAD
#endif // if defined HAVE_PTHREAD_CREATE
#endif // if defined CALLISTO_THREADS


#if !defined(CALLISTO_THREADS)
// Dummy object used when not threading
class ReadWriteLock
{
public:

	ReadWriteLock() {}
	~ReadWriteLock() {}
	void LockRead(void) {}
	void LockWrite(void) {}
	bool TryLockRead(void) {return true;}
	bool TryLockWrite(void) {return true;}
	void UnLockRead(void) {}
	void UnLockWrite(void) {}
};

class Mutex
{
public:

	Mutex(){}
	~Mutex(){}
	void Lock(void){}
	bool TryLock(void){return true;}
	void UnLock(void){}
};

#endif

class Condition : private Mutex
{
	bool flag;
public:
	Condition()
	{ flag=false; }
	~Condition()
	{ }
	void operator()(void)
	{ flag=true; }
	void Wait(void)
	{
		Lock();
		while(!flag)Yield();
		flag=false;
		UnLock();
	}
	void WaitNext(void)
	{
		Lock();
		flag=false;
		while(!flag)Yield();
		UnLock();
	}
};

/* === E X T E R N S ======================================================= */

/* === E N D =============================================================== */

#endif
