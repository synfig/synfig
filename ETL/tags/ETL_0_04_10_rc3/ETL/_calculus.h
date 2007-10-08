/*! ========================================================================
** Extended Template and Library
** Calculus Functional Classes Implementation
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
** ========================================================================= */

/* === S T A R T =========================================================== */

#ifndef __ETL__CALCULUS_H
#define __ETL__CALCULUS_H

/* === H E A D E R S ======================================================= */

#include <functional>

/* === M A C R O S ========================================================= */

//#ifndef _EPSILON
//#define _EPSILON		0.0000001
//#endif

#define ETL_FIXED_DERIVATIVE 1

/* === T Y P E D E F S ===================================================== */

/* === C L A S S E S & S T R U C T S ======================================= */

_ETL_BEGIN_NAMESPACE

template <typename T>
class derivative : public std::unary_function<typename T::argument_type,typename T::result_type>
{
	T func;
	typename T::argument_type epsilon;
public:
	explicit derivative(const T &x, const typename T::argument_type &epsilon=0.000001):func(x),epsilon(epsilon) { }

	typename T::result_type
	operator()(const typename T::argument_type &x)const
	{
#ifdef ETL_FIXED_DERIVATIVE
		return (func(x+epsilon)-func(x))/epsilon;
#else
		return (func(x)-func(x+epsilon))/epsilon;
#endif
	}
};

template <typename T>
class integral : public std::binary_function<typename T::argument_type,typename T::argument_type,typename T::result_type>
{
	T func;
	int samples;
public:
	explicit integral(const T &x, const int &samples=500):func(x),samples(samples) { }

	typename T::result_type
	operator()(typename T::argument_type x,typename T::argument_type y)const
	{
		typename T::result_type ret=0;
		int i=samples;
		const typename T::argument_type increment=(y-x)/i;

		for(;i;i--,x+=increment)
			ret+=(func(x)+func(x+increment))*increment/2;
		return ret;
	}
};

_ETL_END_NAMESPACE

/* === E N D =============================================================== */

#endif
