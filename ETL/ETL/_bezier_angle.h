/*! ========================================================================
** Extended Template Library
** Bezier Template Class Implementation (Angle Specialization)
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

#ifndef __ETL__BEZIER_ANGLE_H
#define __ETL__BEZIER_ANGLE_H

/* === H E A D E R S ======================================================= */

#include "_curve_func.h"
#include "angle"

/* === M A C R O S ========================================================= */

/* === T Y P E D E F S ===================================================== */

/* === C L A S S E S & S T R U C T S ======================================= */

namespace etl {

/*
template <>
class bezier_base<angle> : std::unary_function<float,angle>
{
public:
	typedef angle value_type;
	typedef float time_type;
private:
	affine_combo<value_type,time_type> affine_func;
	value_type a,b,c,d;
	time_type r,s;

public:
	bezier_base():r(0.0),s(1.0) { }
	bezier_base(
		const value_type &a, const value_type &b, const value_type &c, const value_type &d,
		const time_type &r=0.0, const time_type &s=1.0):
		a(a),b(b),c(c),d(d),r(r),s(s) { sync(); }

	void sync(void)
	{
	}

	value_type
	operator()(time_type t)const
	{
		t=(t-r)/(s-r);
		return
		affine_func(
			affine_func(
				affine_func(a,b,t),
				affine_func(b,c,t)
			,t),
			affine_func(
				affine_func(b,c,t),
				affine_func(c,d,t)
			,t)
		,t);
	}

	void set_rs(time_type new_r, time_type new_s) { r=new_r; s=new_s; }
	void set_r(time_type new_r) { r=new_r; }
	void set_s(time_type new_s) { s=new_s; }
	const time_type &get_r(void)const { return r; }
	const time_type &get_s(void)const { return s; }
	time_type get_dt(void)const { return s-r; }

	value_type &
	operator[](int i)
	{ return (&a)[i]; }

	const value_type &
	operator[](int i) const
	{ return (&a)[i]; }
};
*/

};

/* === E X T E R N S ======================================================= */

/* === E N D =============================================================== */

#endif

