/*! ========================================================================
** Extended Template and Library
** Mutex Abstraction Class Implementation
** $Id$
**
** Copyright (c) 2002 Robert B. Quattlebaum Jr.
** Copyright (c) 2008 Chris Moore
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

#ifndef __ETL__MUTEX_PTHREADS_SIMPLE_H_
#define __ETL__MUTEX_PTHREADS_SIMPLE_H_

/* === H E A D E R S ======================================================= */

#include <pthread.h>

/* === M A C R O S ========================================================= */

/* === C L A S S E S & S T R U C T S ======================================= */

namespace etl {

class mutex
{
	pthread_mutex_t	mtx;
public:
			mutex()			{ pthread_mutex_init(&mtx,NULL);	}
		   ~mutex()			{ pthread_mutex_destroy(&mtx);		}
	void	lock_mutex()	{ pthread_mutex_lock(&mtx);			}
	void	unlock_mutex()	{ pthread_mutex_unlock(&mtx);		}

	//! Exception-safe mutex lock class
	class lock
	{
		mutex *_mtx;
	public:
		 lock(mutex &x):_mtx(&x)	{ _mtx->lock_mutex();		}
		~lock()						{ _mtx->unlock_mutex();		}
	};
};

};

/* === E N D =============================================================== */

#endif
