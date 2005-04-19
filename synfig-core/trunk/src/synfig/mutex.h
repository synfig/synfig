/* === S Y N F I G ========================================================= */
/*!	\file mutex.h
**	\brief Template Header
**
**	$Id: mutex.h,v 1.2 2005/01/10 07:40:26 darco Exp $
**
**	\legal
**	Copyright (c) 2002 Robert B. Quattlebaum Jr.
**
**	This software and associated documentation
**	are CONFIDENTIAL and PROPRIETARY property of
**	the above-mentioned copyright holder.
**
**	You may not copy, print, publish, or in any
**	other way distribute this software without
**	a prior written agreement with
**	the copyright holder.
**	\endlegal
*/
/* ========================================================================= */

/* === S T A R T =========================================================== */

#ifndef __SYNFIG_MUTEX_H
#define __SYNFIG_MUTEX_H

/* === H E A D E R S ======================================================= */

/* === M A C R O S ========================================================= */

/* === T Y P E D E F S ===================================================== */

/* === C L A S S E S & S T R U C T S ======================================= */

namespace synfig {

class RecMutex;

class Mutex
{
	friend class RecMutex;

protected:
	void* blackbox;
	
public:
	
	class Lock
	{
		Mutex& mutex;
	public:
		Lock(Mutex& x):mutex(x) { mutex.lock(); }
		~Lock() { mutex.unlock(); }
	};
	
	Mutex();
	~Mutex();
	
	void lock();
	void unlock();
	bool try_lock();
	bool is_locked();
	
private:
	//! Non-copyable
	Mutex(const Mutex&);
	
	//! Non-assignable
	void operator=(const Mutex&);
};

class RecMutex : public Mutex
{
public:
	RecMutex();

	void unlock_all();
};

class RWLock
{
	void* blackbox;
	
public:
	
	class ReaderLock
	{
		RWLock& rw_lock;
	public:
		ReaderLock(RWLock& x):rw_lock(x) { rw_lock.reader_lock(); }
		~ReaderLock() { rw_lock.reader_unlock(); }
	};
	class WriterLock
	{
		RWLock& rw_lock;
	public:
		WriterLock(RWLock& x):rw_lock(x) { rw_lock.writer_lock(); }
		~WriterLock() { rw_lock.writer_unlock(); }
	};
	
	RWLock();
	~RWLock();

	void reader_lock();
	void reader_unlock();
	bool reader_trylock();

	void writer_lock();
	void writer_unlock();
	bool writer_trylock();
};

}; // END of namespace synfig

/* === E N D =============================================================== */

#endif
