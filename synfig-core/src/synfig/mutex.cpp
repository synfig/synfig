/* === S Y N F I G ========================================================= */
/*!	\file mutex.cpp
**	\brief Template File
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

/* === H E A D E R S ======================================================= */

#ifdef USING_PCH
#	include "pch.h"
#else
#ifdef HAVE_CONFIG_H
#	include <config.h>
#endif

#include "mutex.h"

#ifdef HAVE_LIBPTHREAD
#define USING_PTHREADS 1
#else
#ifdef _WIN32
#define USING_WIN32_THREADS 1
#else
#error Need either libpthread of win32 threads
#endif
#endif

#ifdef USING_WIN32_THREADS
#include <windows.h>
#endif

#ifdef USING_PTHREADS
#include <pthread.h>
#endif

#endif

/* === U S I N G =========================================================== */

//using namespace std;
//using namespace etl;
using namespace synfig;

/* === M A C R O S ========================================================= */

/* === G L O B A L S ======================================================= */

/* === P R O C E D U R E S ================================================= */

/* === M E T H O D S ======================================================= */

#ifdef USING_PTHREADS

RWLock::RWLock()
{
	pthread_rwlock_t*const rwlock_ptr(new pthread_rwlock_t);

	pthread_rwlock_init(rwlock_ptr, NULL);

	blackbox=rwlock_ptr;
}

RWLock::~RWLock()
{
	pthread_rwlock_t*const rwlock_ptr(static_cast<pthread_rwlock_t*>(blackbox));

	pthread_rwlock_destroy(rwlock_ptr);

	delete rwlock_ptr;
}

void
RWLock::reader_lock()
{
	pthread_rwlock_t*const rwlock_ptr(static_cast<pthread_rwlock_t*>(blackbox));

	pthread_rwlock_rdlock(rwlock_ptr);
}

void
RWLock::reader_unlock()
{
	pthread_rwlock_t*const rwlock_ptr(static_cast<pthread_rwlock_t*>(blackbox));

	pthread_rwlock_unlock(rwlock_ptr);
}

bool
RWLock::reader_trylock()
{
	pthread_rwlock_t*const rwlock_ptr(static_cast<pthread_rwlock_t*>(blackbox));

	return !pthread_rwlock_tryrdlock(rwlock_ptr);
}

void
RWLock::writer_lock()
{
	pthread_rwlock_t*const rwlock_ptr(static_cast<pthread_rwlock_t*>(blackbox));

	pthread_rwlock_wrlock(rwlock_ptr);
}

void
RWLock::writer_unlock()
{
	pthread_rwlock_t*const rwlock_ptr(static_cast<pthread_rwlock_t*>(blackbox));

	pthread_rwlock_unlock(rwlock_ptr);
}

bool
RWLock::writer_trylock()
{
	pthread_rwlock_t*const rwlock_ptr(static_cast<pthread_rwlock_t*>(blackbox));

	return !pthread_rwlock_trywrlock(rwlock_ptr);
}

#endif

#ifdef USING_WIN32_THREADS

RWLock::RWLock()
{
}

RWLock::~RWLock()
{
}

void
RWLock::reader_lock()
{
}

void
RWLock::reader_unlock()
{
}

bool
RWLock::reader_trylock()
{
}

void
RWLock::writer_lock()
{
}

void
RWLock::writer_unlock()
{
}

bool
RWLock::writer_trylock()
{
}

#endif
