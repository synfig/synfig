/*! ========================================================================
** Extended Template and Library
** Proc Clock Description Implementation
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

#ifndef __ETL__CLOCK_SYSTEM_H
#define __ETL__CLOCK_SYSTEM_H

/* === H E A D E R S ======================================================= */

#ifndef _WIN32
# include <time.h>
# define __sys_clock	::clock
# define __sys_time	::time
#else
# ifdef __GNUG__
#  include <time.h>
#  define __sys_clock	::clock
#  define __sys_time	::time
# else
typedef int clock_t;
typedef int time_t;
extern clock_t _clock();
extern time_t _time(time_t *);
#  define CLOCKS_PER_SEC 1000
#  define __sys_clock	_clock
#  define __sys_time	_time
# endif
#endif

/* === M A C R O S ========================================================= */

/* === T Y P E D E F S ===================================================== */

/* === C L A S S E S & S T R U C T S ======================================= */

namespace etl {

class clock_desc_sys_clock
{
public:
	typedef float value_type;

	inline static bool realtime()
	{ return false; }

	inline static bool proctime()
	{ return true; }

	inline static value_type
	one_second()
	{ return 1.0f; }

	inline static value_type precision()
	{ return one_second()/(value_type)CLOCKS_PER_SEC; }

	inline static const char *description()
	{ return "ANSI C clock()"; };

protected:
	typedef clock_t timestamp;

	static void
	get_current_time(timestamp &time)
	{ time=__sys_clock(); }

	static timestamp
	get_current_time()
	{ return __sys_clock(); }

	static value_type
	timestamp_to_seconds(const timestamp &x)
	{ return precision()*x; }

	static timestamp
	seconds_to_timestamp(const value_type &x)
	{ return (timestamp)(x*(value_type)CLOCKS_PER_SEC+0.5); }

};

class clock_desc_sys_time
{
public:
	typedef float value_type;

	inline static bool realtime()
	{ return true; }

	inline static bool proctime()
	{ return false; }

	inline static value_type
	one_second()
	{ return 1.0f; }

	inline static value_type precision()
	{ return one_second(); }

	inline static const char *description()
	{ return "ANSI C time()"; };

protected:
	typedef time_t timestamp;

	static void
	get_current_time(timestamp &time)
	{ __sys_time(&time); }

	static timestamp
	get_current_time()
	{ return __sys_time(NULL); }

	static value_type
	timestamp_to_seconds(const timestamp &x)
	{ return (value_type)x; }

	static timestamp
	seconds_to_timestamp(const value_type &x)
	{ return (timestamp)(x+(value_type)0.5f); }
};

};

/* === E N D =============================================================== */

#endif
