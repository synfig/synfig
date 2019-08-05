/*! ========================================================================
** Extended Template and Library
** Clock Abstraction Implementation
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

#ifndef __ETL__CLOCK_H
#define __ETL__CLOCK_H

/* === H E A D E R S ======================================================= */

#ifndef _WIN32
#include <unistd.h>
#else
inline void sleep(int i) { Sleep(i*1000); }
#endif

/* === M A C R O S ========================================================= */

/* === T Y P E D E F S ===================================================== */

/* === C L A S S E S & S T R U C T S ======================================= */

namespace etl {

inline void yield() { sleep(0); }

/*! ========================================================================
** \class	clock_base
** \brief	clock abstraction
**
** A more detailed description needs to be written.
*/
template <class DESC>
class clock_base : public DESC
{
public:
	typedef typename DESC::value_type value_type;

private:
	typedef clock_base<DESC> _clock;
	typedef typename DESC::timestamp timestamp;

	timestamp base_time;

	using DESC::get_current_time;
	using DESC::realtime;
	using DESC::one_second;
public:

	clock_base() { reset(); }

	void reset()
	{ get_current_time(base_time); }

	value_type operator()()const
	{ return this->timestamp_to_seconds(get_current_time()-base_time); }

	value_type pop_time()
	{
		// Grab the old base time
		timestamp old_time=base_time;

		// Put the current time into base_time
		get_current_time(base_time);

		return this->timestamp_to_seconds(base_time-old_time);
	}

	static void
	sleep(const value_type &length)
	{
		if(!realtime())
			::sleep((int)(length+0.5));
		else
		{
			_clock timer;
			timer.reset();
			value_type val;
			for(val=timer();one_second()<length-val;val=timer())
				::sleep((int)((length-val)/2.0+0.4));
			while(timer()<length)
			  ;
		}


		/* This is a different waiting mechanism that uses
		** the native timestamp type of the clock rather
		** than converting it to a double (or whatever).
		** You would think that this would be at least a
		** few microseconds faster, but a few tests on my
		** PowerBook G4 have proved otherwise. Indeed I loose
		** several microseconds using this "optimized" method.
		** Bizarre.
		**	- darco (8-17-2002)
		{
			timestamp endtime=get_current_time()+seconds_to_timestamp(length);
			timestamp loopendtime=get_current_time()+seconds_to_timestamp(length-1.0);
			while(get_current_time()<loopendtime)
				::sleep((int)timestamp_to_seconds(loopendtime-get_current_time())/2.0);
			while(get_current_time()<endtime);
		}
		*/

		return;
	}
};

};

/* === E N D =============================================================== */

#endif
