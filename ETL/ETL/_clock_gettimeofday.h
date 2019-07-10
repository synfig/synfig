/*! ========================================================================
** Extended Template and Library
** gettimeofday() Clock Description Implementation
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

#ifndef __ETL__CLOCK_GETTIMEOFDAY_H
#define __ETL__CLOCK_GETTIMEOFDAY_H

/* === H E A D E R S ======================================================= */

#include <sys/time.h>
#include <cmath>

/* === M A C R O S ========================================================= */

/* === T Y P E D E F S ===================================================== */

/* === C L A S S E S & S T R U C T S ======================================= */

namespace etl {

class clock_desc_gettimeofday
{
public:
	typedef double value_type;

	inline static bool realtime()
	{ return true; }

	inline static bool proctime()
	{ return false; }

	inline static value_type
	one_second()
	{ return 1.0f; }

	inline static value_type precision()
	{ return one_second()/(value_type)1000000.0f; }

	inline static const char *description()
	{ return "UNIX gettimeofday()"; };

protected:
	class timestamp : public timeval
	{
		timestamp(int sec, int usec)
		{ tv_sec=sec; tv_usec=usec; }

		friend class clock_desc_gettimeofday;
	public:
		timestamp() { }


		inline timestamp operator-(const timestamp &rhs)const
		{
			timestamp ret;
			ret.tv_usec=tv_usec-rhs.tv_usec;

			if(ret.tv_usec<0)
			{
				ret.tv_sec=tv_sec-rhs.tv_sec-1;
				ret.tv_usec+=1000000;
			}
			else
				ret.tv_sec=tv_sec-rhs.tv_sec;
			return ret;
		}

		inline timestamp operator+(timestamp rhs)const
		{
			rhs.tv_usec+=tv_usec;

			if(rhs.tv_usec>1000000)
			{
				rhs.tv_sec+=tv_sec+1;
				rhs.tv_usec-=1000000;
			}
			else
				rhs.tv_sec+=tv_sec;
			return rhs;
		}

		inline bool operator<(const timestamp &rhs)const
		{ return tv_sec<rhs.tv_sec || tv_usec<rhs.tv_usec; }

		inline bool operator==(const timestamp &rhs)const
		{ return tv_usec==rhs.tv_usec && tv_sec==rhs.tv_sec; }

		inline bool operator!=(const timestamp &rhs)const
		{ return tv_usec!=rhs.tv_usec || tv_sec!=rhs.tv_sec; }
	};

	static void
	get_current_time(timestamp &x)
	{ gettimeofday(&x,NULL);}

	static timestamp
	get_current_time()
	{ timestamp ret; get_current_time(ret); return ret; }

	static value_type
	timestamp_to_seconds(const timestamp &x)
	{ return (value_type)x.tv_sec + precision()*x.tv_usec; }

	static timestamp
	seconds_to_timestamp(const value_type &x)
	{ return timestamp((int)floor(x), (int)((x-floor(x))/precision()+0.5)); }
};

};

/* === E N D =============================================================== */

#endif

