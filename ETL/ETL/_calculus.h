/*! ========================================================================
** Extended Template and Library
** \file _calculus.h
** \brief Calculus Functional Classes Implementation
** \internal
**
** \legal
** Copyright (c) 2002 Robert B. Quattlebaum Jr.
** Copyright (c) 2008 Chris Moore
**
** This file is part of Synfig.
**
** Synfig is free software: you can redistribute it and/or modify
** it under the terms of the GNU General Public License as published by
** the Free Software Foundation, either version 2 of the License, or
** (at your option) any later version.
**
** Synfig is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
** GNU General Public License for more details.
**
** You should have received a copy of the GNU General Public License
** along with Synfig.  If not, see <https://www.gnu.org/licenses/>.
** \endlegal
**
** ========================================================================= */

/* === S T A R T =========================================================== */

#ifndef __ETL__CALCULUS_H
#define __ETL__CALCULUS_H

/* === H E A D E R S ======================================================= */

#include <functional>

#include "hermite"

/* === M A C R O S ========================================================= */

//#ifndef _EPSILON
//#define _EPSILON		0.0000001
//#endif

/* === T Y P E D E F S ===================================================== */

/* === C L A S S E S & S T R U C T S ======================================= */

namespace etl {

template <typename T>
class derivative
{
	T func;
	typename T::time_type epsilon;
public:
	explicit derivative(const T &x, const typename T::time_type& epsilon=0.000001):func(x),epsilon(epsilon) { }

	typename T::value_type
	operator()(const typename T::time_type &x)const
	{
		return (func(x+epsilon)-func(x))/epsilon;
	}
};

template <typename T>
class derivative<hermite<T> >
{
	hermite<T> func;
public:
	explicit derivative(const hermite<T> &x):func(x) { }

	typename hermite<T>::value_type
	operator()(const typename hermite<T>::time_type &x)const
	{
		T a = func[0], b = func[1], c = func[2], d = func[3];
		typename hermite<T>::time_type y(1-x);
		return ((b-a)*y*y + (c-b)*x*y*2 + (d-c)*x*x) * 3;
	}
};

template <typename T>
class integral
{
	T func;
	int samples;
public:
	explicit integral(const T &x, const int &samples=500):func(x),samples(samples) { }

	typename T::value_type
	operator()(typename T::time_type x,typename T::time_type y)const
	{
		typename T::value_type ret=0;
		int i=samples;
		const typename T::time_type increment=(y-x)/i;

		for(;i;i--,x+=increment)
			ret+=(func(x)+func(x+increment))*increment/2;
		return ret;
	}
};

};

/* === E N D =============================================================== */

#endif
