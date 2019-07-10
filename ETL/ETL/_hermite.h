/*! ========================================================================
** Extended Template Library
** Hermite Template Class Implementation
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

#ifndef __ETL__HERMITE_H
#define __ETL__HERMITE_H

/* === H E A D E R S ======================================================= */

#include "bezier"

/* === M A C R O S ========================================================= */

/* === T Y P E D E F S ===================================================== */

/* === C L A S S E S & S T R U C T S ======================================= */

namespace etl {

/*
template <typename T>
class hermite_base : std::unary_function<float,T>
{
public:
	typedef T value_type;
	typedef float time_type;
private:
	affine_combo<value_type,time_type> affine_func;
	value_type a,b,c,d;
	time_type r,s;

	value_type _coeff[3];
	time_type drs; // reciprocal of (s-r)
public:
	hermite_base():r(0.0),s(1.0) { drs=1.0/(s-r); }
	hermite_base(
		const value_type &a, const value_type &b, const value_type &c, const value_type &d,
		const time_type &r=0.0, const time_type &s=1.0):
		a(a),b(b),c(c),d(d),r(r),s(s) { sync(); }

	void sync(void)
	{
		drs=1.0/(s-r);
		_coeff[0]=           c;
		_coeff[1]=-d*1 - c*2 + b*3 - a*3;
		_coeff[2]= d*1 + c*1 - b*2 + a*2;
	}

	inline value_type
	operator()(time_type t)const
	{ t-=r; t*=drs; return a + (_coeff[0]+(_coeff[1]+(_coeff[2])*t)*t)*t; }

	void set_rs(time_type new_r, time_type new_s) { r=new_r; s=new_s; drs=1.0/(s-r); }
	void set_r(time_type new_r) { r=new_r; drs=1.0/(s-r); }
	void set_s(time_type new_s) { s=new_s; drs=1.0/(s-r); }
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


template <typename T>
class hermite : public hermite_base<T>
{
public:
	typedef T value_type;
	typedef float time_type;



public:
	hermite() { }
	hermite(const value_type &p1, const value_type &p2, const value_type &t1, const value_type &t2):
    	P1(p1),P2(p2),T1(t1),T2(t2) { sync(); }
	hermite(const value_type &p1, const value_type &p2):
    	P1(p1),P2(p2),T1(p2-p1),T2(p2-p1) { sync(); }

	value_type P1,P2,T1,T2;

	value_type &p1(void) { return P1; }
	value_type &p2(void) { return P2; }
	value_type &t1(void) { return T1; }
	value_type &t2(void) { return T2; }

	void sync(void)
	{
//		hermite_base<T>::operator[](0)=P1;
//		bezier<T>::operator[](1)=P1+T1/3;
//		bezier<T>::operator[](2)=P2-T2/3;
//		bezier<T>::operator[](3)=P2;

		hermite_base<T>::operator[](0)=P1;
		hermite_base<T>::operator[](1)=P2;
		hermite_base<T>::operator[](2)=T1;
		hermite_base<T>::operator[](3)=T2;

		hermite_base<T>::sync();
	}

};

*/

template <typename V,typename T=float>
class hermite : public bezier<V,T>
{
public:
	typedef V value_type;
	typedef T time_type;



public:
	hermite() { }
	hermite(const value_type &p1, const value_type &p2, const value_type &t1, const value_type &t2):
    	P1(p1),P2(p2),T1(t1),T2(t2) { sync(); }
	hermite(const value_type &p1, const value_type &p2):
    	P1(p1),P2(p2),T1(p2-p1),T2(p2-p1) { sync(); }

	value_type P1,P2,T1,T2;

	value_type &p1() { return P1; }
	value_type &p2() { return P2; }
	value_type &t1() { return T1; }
	value_type &t2() { return T2; }

	void sync()
	{
		bezier<V,T>::operator[](0)=P1;
		bezier<V,T>::operator[](1)=P1+T1/3;
		bezier<V,T>::operator[](2)=P2-T2/3;
		bezier<V,T>::operator[](3)=P2;

		bezier<V,T>::sync();
	}
};

};

/* === E X T E R N S ======================================================= */

/* === E N D =============================================================== */

#endif
