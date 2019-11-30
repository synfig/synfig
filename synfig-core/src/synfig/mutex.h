/* === S Y N F I G ========================================================= */
/*!	\file mutex.h
**	\brief Template Header
**
**	$Id$
**
**	\legal
**	Copyright (c) 2002-2005 Robert B. Quattlebaum Jr., Adrian Bentley
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

#ifndef __SYNFIG_MUTEX_H
#define __SYNFIG_MUTEX_H

/* === H E A D E R S ======================================================= */

/* === M A C R O S ========================================================= */

/* === T Y P E D E F S ===================================================== */

/* === C L A S S E S & S T R U C T S ======================================= */

namespace synfig {

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
