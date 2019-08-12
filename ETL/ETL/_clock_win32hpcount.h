/*! ========================================================================
** Extended Template and Library
** Win32 Clock Description Implementation
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

#ifndef __ETL__CLOCK_WIN32HPCOUNT_H
#define __ETL__CLOCK_WIN32HPCOUNT_H

/* === H E A D E R S ======================================================= */

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

/* === M A C R O S ========================================================= */

#if defined(__GNUG__) && defined(__int64)
#undef __int64
#define __int64 long long int
#endif

/* === T Y P E D E F S ===================================================== */

/* === C L A S S E S & S T R U C T S ======================================= */

namespace etl {

class clock_desc_win32hpcount
{
public:
	typedef double value_type;

	static bool realtime()
	{ return true; }

	static bool proctime()
	{ return false; }

	static value_type
	one_second()
	{ return 1.0f; }

	static value_type precision()
	{
		__int64 freq;
		QueryPerformanceFrequency((LARGE_INTEGER*)&freq);
		return one_second()/(value_type)freq;
	}

	static const char *description()
	{ return "Win32 QueryPerformanceCounter()"; };

protected:
	typedef __int64 timestamp;

	static void
	get_current_time(timestamp &x)
	{ QueryPerformanceCounter((LARGE_INTEGER*)&x);}

	static timestamp
	get_current_time()
	{ timestamp ret; QueryPerformanceCounter((LARGE_INTEGER*)&ret); return ret; }

	static value_type
	timestamp_to_seconds(const timestamp &x)
	{ return precision()*x; }

	static timestamp
	seconds_to_timestamp(const value_type &x)
	{ return (timestamp)(x/precision()); }
};

};

/* === E N D =============================================================== */

#endif

