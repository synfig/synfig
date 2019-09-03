/* === S Y N F I G ========================================================= */
/*!	\file real.h
**	\brief Provides the synfig::Real typedef
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

#ifndef __SYNFIG_REAL_H
#define __SYNFIG_REAL_H

#include <cmath>

#include <functional>
#include <limits>

/* === T Y P E D E F S ===================================================== */

namespace synfig {


/*!	\typedef Real
**	\todo writeme
*/
typedef double Real;

template<typename T>
inline T real_nan()
	//{ return T(0.0)/T(0.0); }
	{return std::numeric_limits<T>::quiet_NaN();}

template<typename T>
inline T real_low_precision()
	{ return T(1e-6); }
template<typename T>
inline T real_precision()
	{ return T(1e-8); }
template<typename T>
inline T real_high_precision()
	{ return T(1e-10); }


// comparison should be symmetric
// approximate_equal(a, b) absolutely identical to approximate_equal(b, a)
// this code should be equal to code of functions less and less_or_equal
template<typename T>
inline bool approximate_equal_custom(const T &a, const T &b, const T &precision)
	{ return a < b ? b - a < precision : a - b < precision; }
template<typename T>
inline bool approximate_not_equal_custom(const T &a, const T &b, const T &precision)
	{ return !approximate_equal_custom(a, b, precision); }
template<typename T>
inline bool approximate_zero_custom(const T &a, const T &precision)
	{ return approximate_equal_custom(a, T(), precision); }
template<typename T>
inline bool approximate_not_zero_custom(const T &a, const T &precision)
	{ return !approximate_zero_custom(a, precision); }
template<typename T>
inline bool approximate_less_custom(const T &a, const T &b, const T &precision)
	{ return a < b && b - a >= precision; }
template<typename T>
inline bool approximate_greater_custom(const T &a, const T &b, const T &precision)
	{ return approximate_less_custom(b, a, precision); }
template<typename T>
inline bool approximate_less_or_equal_custom(const T &a, const T &b, const T &precision)
	{ return a < b || a - b < precision; }
template<typename T>
inline bool approximate_greater_or_equal_custom(const T &a, const T &b, const T &precision)
	{ return approximate_less_or_equal_custom(b, a, precision); }
template<typename T>
inline T approximate_floor_custom(const T &a, const T &precision)
	{ return std::floor(a + precision); }
template<typename T>
inline T approximate_ceil_custom(const T &a, const T &precision)
	{ return std::ceil(a - precision); }


template<typename T>
inline bool approximate_equal(const T &a, const T &b)
	{ return approximate_equal_custom(a, b, real_precision<T>()); }
template<typename T>
inline bool approximate_not_equal(const T &a, const T &b)
	{ return approximate_not_equal_custom(a, b, real_precision<T>()); }
template<typename T>
inline bool approximate_zero(const T &a)
	{ return approximate_zero_custom(a, real_precision<T>()); }
template<typename T>
inline bool approximate_not_zero(const T &a)
	{ return approximate_not_zero_custom(a, real_precision<T>()); }
template<typename T>
inline bool approximate_less(const T &a, const T &b)
	{ return approximate_less_custom(a, b, real_precision<T>()); }
template<typename T>
inline bool approximate_greater(const T &a, const T &b)
	{ return approximate_greater_custom(a, b, real_precision<T>()); }
template<typename T>
inline bool approximate_less_or_equal(const T &a, const T &b)
	{ return approximate_less_or_equal_custom(a, b, real_precision<T>()); }
template<typename T>
inline bool approximate_greater_or_equal(const T &a, const T &b)
	{ return approximate_greater_or_equal_custom(a, b, real_precision<T>()); }
template<typename T>
inline T approximate_floor(const T &a)
	{ return approximate_floor_custom(a, real_precision<T>()); }
template<typename T>
inline T approximate_ceil(const T &a)
	{ return approximate_ceil_custom(a, real_precision<T>()); }


template<typename T>
inline bool approximate_equal_lp(const T &a, const T &b)
	{ return approximate_equal_custom(a, b, real_low_precision<T>()); }
template<typename T>
inline bool approximate_not_equal_lp(const T &a, const T &b)
	{ return approximate_not_equal_custom(a, b, real_low_precision<T>()); }
template<typename T>
inline bool approximate_zero_lp(const T &a)
	{ return approximate_zero_custom(a, real_low_precision<T>()); }
template<typename T>
inline bool approximate_not_zero_lp(const T &a)
	{ return approximate_not_zero_custom(a, real_low_precision<T>()); }
template<typename T>
inline bool approximate_less_lp(const T &a, const T &b)
	{ return approximate_less_custom(a, b, real_low_precision<T>()); }
template<typename T>
inline bool approximate_greater_lp(const T &a, const T &b)
	{ return approximate_greater_custom(a, b, real_low_precision<T>()); }
template<typename T>
inline bool approximate_less_or_equal_lp(const T &a, const T &b)
	{ return approximate_less_or_equal_custom(a, b, real_low_precision<T>()); }
template<typename T>
inline bool approximate_greater_or_equal_lp(const T &a, const T &b)
	{ return approximate_greater_or_equal_custom(a, b, real_low_precision<T>()); }
template<typename T>
inline T approximate_floor_lp(const T &a)
	{ return approximate_floor_custom(a, real_low_precision<T>()); }
template<typename T>
inline T approximate_ceil_lp(const T &a)
	{ return approximate_ceil_custom(a, real_low_precision<T>()); }


template<typename T>
inline bool approximate_equal_hp(const T &a, const T &b)
	{ return approximate_equal_custom(a, b, real_high_precision<T>()); }
template<typename T>
inline bool approximate_not_equal_hp(const T &a, const T &b)
	{ return approximate_not_equal_custom(a, b, real_high_precision<T>()); }
template<typename T>
inline bool approximate_zero_hp(const T &a)
	{ return approximate_zero_custom(a, real_high_precision<T>()); }
template<typename T>
inline bool approximate_not_zero_hp(const T &a)
	{ return approximate_not_zero_custom(a, real_high_precision<T>()); }
template<typename T>
inline bool approximate_less_hp(const T &a, const T &b)
	{ return approximate_less_custom(a, b, real_high_precision<T>()); }
template<typename T>
inline bool approximate_greater_hp(const T &a, const T &b)
	{ return approximate_greater_custom(a, b, real_high_precision<T>()); }
template<typename T>
inline bool approximate_less_or_equal_hp(const T &a, const T &b)
	{ return approximate_less_or_equal_custom(a, b, real_high_precision<T>()); }
template<typename T>
inline bool approximate_greater_or_equal_hp(const T &a, const T &b)
	{ return approximate_greater_or_equal_custom(a, b, real_high_precision<T>()); }
template<typename T>
inline T approximate_floor_hp(const T &a)
	{ return approximate_floor_custom(a, real_high_precision<T>()); }
template<typename T>
inline T approximate_ceil_hp(const T &a)
	{ return approximate_ceil_custom(a, real_high_precision<T>()); }

template<typename T, bool func(const T&, const T&)>
struct RealFunctionWrapper : public std::binary_function<T,T,bool>
	{ bool operator() (const T &a, const T &b) const { return func(a, b); } };


template<typename T>
inline T clamp(const T &a, const T &min, const T &max)
	{ return a > min ? (a < max ? a : max) : min; }

}; // END of namespace synfig

/* === E N D =============================================================== */

#endif
