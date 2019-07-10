/*! ========================================================================
** Extended Template and Library
** NULL Mutex Abstraction Class Implementation
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

#ifndef __ETL__MUTEX_NULL_H_
#define __ETL__MUTEX_NULL_H_

/* === H E A D E R S ======================================================= */
/* === M A C R O S ========================================================= */

/* === C L A S S E S & S T R U C T S ======================================= */

namespace etl {

class mutex_null
{
public:

	mutex_null(){}
	~mutex_null(){}
	//! Exception-safe mutex lock class
/*
	class lock
	{
		mutex *_mtx;
	public:
		lock(mutex &x):_mtx(&x) { _mtx->lock_mutex(); }
		~lock() { _mtx->unlock_mutex(); }
		mutex &get() { return *_mtx; }
	};
*/
	class lock
	{
	public:
		lock(mutex_null &/*x*/) { }
	};

	void lock_mutex(){}
	bool try_lock_mutex(){return true;}
	void unlock_mutex(){}
};

};

/* === E X T E R N S ======================================================= */

/* === E N D =============================================================== */

#endif
