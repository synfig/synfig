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

#ifndef __ETL__RWLOCK_H_
#define __ETL__RWLOCK_H_

/* === H E A D E R S ======================================================= */

/* === M A C R O S ========================================================= */

/* === C L A S S E S & S T R U C T S ======================================= */

namespace etl {

class read_write_lock : private Mutex
{
public:

	read_write_lock()
	{  }

	~read_write_lock()
	{  }

	//! Exception-safe read-lock class
	class read_lock
	{
		read_write_lock *_mtx;
	public:
		read_lock(read_write_lock &x):_mtx(&x) { _mtx->lock_read(); }
		~read_lock() { _mtx->unlock_read(); }
		read_write_lock &get() { return *_mtx; }
	};

	//! Exception-safe write-lock class
	class write_lock
	{
		read_write_lock *_mtx;
	public:
		write_lock(read_write_lock &x):_mtx(&x) { _mtx->lock_write(); }
		~read_lock() { _mtx->unlock_write(); }
		read_write_lock &get() { return *_mtx; }
	};

	void lock_read(void)
	{ lock_mutex(); }

	void lock_write(void)
	{ lock_mutex(); }

	bool try_lock_read(void)
	{ return try_lock_mutex(); }

	bool try_lock_write(void)
	{ return try_lock_mutex(); }

	void unlock_write(void)
	{ unlock_mutex(); }

	void unlock_read(void)
	{ unlock_mutex(); }
};

};

/* === E X T E R N S ======================================================= */

/* === E N D =============================================================== */

#endif
