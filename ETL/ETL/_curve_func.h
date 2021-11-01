/*! ========================================================================
** Extended Template and Library
** \file _curve_func.h
** \brief Utility Curve Template Class Implementations
** \internal
**
** \legal
** Copyright (c) 2002 Robert B. Quattlebaum Jr.
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
** \note
** This is an internal header file, included by other ETL headers.
** You should not attempt to use it directly.
**
** ========================================================================= */

/* === S T A R T =========================================================== */

#ifndef __ETL__CURVE_FUNC_H
#define __ETL__CURVE_FUNC_H

/* === H E A D E R S ======================================================= */

#include <functional>
#include <cmath> // sqrt

/* -- C L A S S E S --------------------------------------------------------- */

template <class T, class K=float>
struct affine_combo
{
	// from (a) to (x) : x = a(1-t) + b(t)
	T operator()(const T &a,const T &b,const K &t)const
	{
		return T( (b-a)*t+a );
	}

	// from (x) to (a) : a = (x-b(t)) / (1-t)
	T reverse(const T &x, const T &b, const K &t)const
	{
		return T( (x-t*b)*(static_cast<K>(1)/(static_cast<K>(1)-t)) );
	}
};

template <class T, class K=float>
struct distance_func : public std::binary_function<T, T, K>
{
	K operator()(const T &a,const T &b)const
	{
		T delta=b-a;
		return static_cast<K>(delta*delta);
	}

	K cook(const K &x)const { return x*x; }
	K uncook(const K &x)const { return sqrt(x); }

};

/* -- E N D ----------------------------------------------------------------- */

#endif
