/*! ========================================================================
** Extended Template and Library
** Misc
** $Id$
**
** Copyright (c) 2002 Robert B. Quattlebaum Jr.
** Copyright (c) 2007 Chris Moore
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
** ========================================================================= */

/* === S T A R T =========================================================== */

#ifndef __ETL__MISC_H_
#define __ETL__MISC_H_

/* === H E A D E R S ======================================================= */
#include <cmath>

/* === M A C R O S ========================================================= */

/* === T Y P E D E F S ===================================================== */

/* === C L A S S E S & S T R U C T S ======================================= */

namespace etl {

template<typename I, typename T> inline I
binary_find(I begin, I end, const T& value)
{
#if 1
	I iter(begin+(end-begin)/2);

	while(end-begin>1 && !(*iter==value))
	{
		((*iter<value)?begin:end) = iter;

		iter = begin+(end-begin)/2;
	}
	return iter;
#else
	size_t len_(end-begin);
	size_t half_(len_/2);

	I iter(begin);
	iter+=half_;

	while(len_>1 && !(*iter==value))
	{
		((*iter<value)?begin:end) = iter;

		len_=half_;
		half_/=2;

		iter=begin;
		iter+=half_;
	}
	return iter;
#endif
}

inline int round_to_int(const float x) {
	/*!	\todo Isn't there some x86 FPU instruction for quickly
	**	converting a float to a rounded integer? It's worth
	**	looking into at some point... */
	// return static_cast<int>(x+0.5f);			// <-- (a) fast, but rounds -1.333 to 0!
	// return static_cast<int>(rintf(x));		// <-- (b) slow, but correct
    if (x>=0) return static_cast<int>(x + 0.5);	// <-- slower than (a), but correct, and faster than (b)
    else      return static_cast<int>(x - 0.5);
}
inline int round_to_int(const double x) {
	// return static_cast<int>(x+0.5);
	// return static_cast<int>(rint(x));
	if (x>=0) return static_cast<int>(x + 0.5);
    else      return static_cast<int>(x - 0.5);
}

inline int ceil_to_int(const float x) { return static_cast<int>(ceil(x)); }
inline int ceil_to_int(const double x) { return static_cast<int>(ceil(x)); }

inline int floor_to_int(const float x) { return static_cast<int>(floor(x)); }
inline int floor_to_int(const double x) { return static_cast<int>(floor(x)); }

};

/* === E X T E R N S ======================================================= */

/* === E N D =============================================================== */

#endif
