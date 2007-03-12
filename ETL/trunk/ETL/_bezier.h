/*! ========================================================================
** Extended Template Library
** Bezier Template Class Implementation
** $Id: _bezier.h,v 1.1.1.1 2005/01/04 01:31:46 darco Exp $
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

#ifndef __ETL_BEZIER_H
#define __ETL_BEZIER_H

/* === H E A D E R S ======================================================= */

#include "_curve_func.h"
#include <ETL/fixed>

/* === M A C R O S ========================================================= */

/* === T Y P E D E F S ===================================================== */

/* === C L A S S E S & S T R U C T S ======================================= */

_ETL_BEGIN_NAMESPACE

template<typename V,typename T> class bezier;

//! Cubic Bezier Curve Base Class
// This generic implementation uses the DeCasteljau algorithm.
// Works for just about anything that has an affine combination function
template <typename V,typename T=float>
class bezier_base : public std::unary_function<T,V>
{
public:
	typedef V value_type;
	typedef T time_type;

private:
	value_type a,b,c,d;
	time_type r,s;

protected:
	affine_combo<value_type,time_type> affine_func;

public:
	bezier_base():r(0.0),s(1.0) { }
	bezier_base(
		const value_type &a, const value_type &b, const value_type &c, const value_type &d,
		const time_type &r=0.0, const time_type &s=1.0):
		a(a),b(b),c(c),d(d),r(r),s(s) { sync(); }

	void sync()
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

	/*
	void evaluate(time_type t, value_type &f, value_type &df) const
	{
		t=(t-r)/(s-r);

		value_type p1 = affine_func(
							affine_func(a,b,t),
							affine_func(b,c,t)
							,t);
		value_type p2 = affine_func(
							affine_func(b,c,t),
							affine_func(c,d,t)
						,t);

		f = affine_func(p1,p2,t);
		df = (p2-p1)*3;
	}
	*/

	void set_rs(time_type new_r, time_type new_s) { r=new_r; s=new_s; }
	void set_r(time_type new_r) { r=new_r; }
	void set_s(time_type new_s) { s=new_s; }
	const time_type &get_r()const { return r; }
	const time_type &get_s()const { return s; }
	time_type get_dt()const { return s-r; }

	bool intersect_hull(const bezier_base<value_type,time_type> &x)const
	{
		return 0;
	}

	//! Bezier curve intersection function
	/*! Calculates the time of intersection
	**	for the calling curve.
	**
	**	I still have not figured out a good generic
	**	method of doing this for a bi-infinite
	**	cubic bezier curve calculated with the DeCasteljau
	**	algorithm.
	**
	**	One method, although it does not work for the
	**	entire bi-infinite curve, is to iteratively
	**	intersect the hulls. However, we would only detect
	**	intersections that occur between R and S.
	**
	**	It is entirely possible that a new construct similar
	**	to the affine combination function will be necessary
	**	for this to work properly.
	**
	**	For now, this function is BROKEN. (although it works
	**	for the floating-point specializations, using newton's method)
	*/
	time_type intersect(const bezier_base<value_type,time_type> &x, time_type near=0.0)const
	{
		return 0;
	}

	/* subdivide at some time t into 2 separate curves left and right

		b0 l1
		*		0+1 l2
		b1 		*		1+2*1+2 l3
		*		1+2		*			0+3*1+3*2+3 l4,r1
		b2 		*		1+2*2+2	r2	*
		*		2+3	r3	*
		b3 r4	*
		*

		0.1 2.3 ->	0.1 2 3 4 5.6
	*/
/*	void subdivide(bezier_base *left, bezier_base *right, const time_type &time = (time_type)0.5) const
	{
		time_type t = (time-r)/(s-r);
		bezier_base lt,rt;

		value_type temp;

		//1st stage points to keep
		lt.a = a;
		rt.d = d;

		//2nd stage calc
		lt.b = affine_func(a,b,t);
		temp = affine_func(b,c,t);
		rt.c = affine_func(c,d,t);

		//3rd stage calc
		lt.c = affine_func(lt.b,temp,t);
		rt.b = affine_func(temp,rt.c,t);

		//last stage calc
		lt.d = rt.a = affine_func(lt.c,rt.b,t);

		//set the time range for l,r (the inside values should be 1, 0 respectively)
		lt.r = r;
		rt.s = s;

		//give back the curves
		if(left) *left = lt;
		if(right) *right = rt;
	}
	*/
	value_type &
	operator[](int i)
	{ return (&a)[i]; }

	const value_type &
	operator[](int i) const
	{ return (&a)[i]; }
};


#if 1
// Fast float implementation of a cubic bezier curve
template <>
class bezier_base<float,float> : public std::unary_function<float,float>
{
public:
	typedef float value_type;
	typedef float time_type;
private:
	affine_combo<value_type,time_type> affine_func;
	value_type a,b,c,d;
	time_type r,s;

	value_type _coeff[4];
	time_type drs; // reciprocal of (s-r)
public:
	bezier_base():r(0.0),s(1.0),drs(1.0) { }
	bezier_base(
		const value_type &a, const value_type &b, const value_type &c, const value_type &d,
		const time_type &r=0.0, const time_type &s=1.0):
		a(a),b(b),c(c),d(d),r(r),s(s),drs(1.0/(s-r)) { sync(); }

	void sync()
	{
//		drs=1.0/(s-r);
		_coeff[0]=                 a;
		_coeff[1]=           b*3 - a*3;
		_coeff[2]=     c*3 - b*6 + a*3;
		_coeff[3]= d - c*3 + b*3 - a;
	}

	// Cost Summary: 4 products, 3 sums, and 1 difference.
	inline value_type
	operator()(time_type t)const
	{ t-=r; t*=drs; return _coeff[0]+(_coeff[1]+(_coeff[2]+(_coeff[3])*t)*t)*t; }

	void set_rs(time_type new_r, time_type new_s) { r=new_r; s=new_s; drs=1.0/(s-r); }
	void set_r(time_type new_r) { r=new_r; drs=1.0/(s-r); }
	void set_s(time_type new_s) { s=new_s; drs=1.0/(s-r); }
	const time_type &get_r()const { return r; }
	const time_type &get_s()const { return s; }
	time_type get_dt()const { return s-r; }

	//! Bezier curve intersection function
	/*! Calculates the time of intersection
	**	for the calling curve.
	*/
	time_type intersect(const bezier_base<value_type,time_type> &x, time_type t=0.0,int i=15)const
	{
		//BROKEN - the time values of the 2 curves should be independent
		value_type system[4];
		system[0]=_coeff[0]-x._coeff[0];
		system[1]=_coeff[1]-x._coeff[1];
		system[2]=_coeff[2]-x._coeff[2];
		system[3]=_coeff[3]-x._coeff[3];

		t-=r;
		t*=drs;

		// Newton's method
		// Inner loop cost summary: 7 products, 5 sums, 1 difference
		for(;i;i--)
			t-= (system[0]+(system[1]+(system[2]+(system[3])*t)*t)*t)/
				(system[1]+(system[2]*2+(system[3]*3)*t)*t);

		t*=(s-r);
		t+=r;

		return t;
	}

	value_type &
	operator[](int i)
	{ return (&a)[i]; }

	const value_type &
	operator[](int i) const
	{ return (&a)[i]; }
};


// Fast double implementation of a cubic bezier curve
template <>
class bezier_base<double,float> : public std::unary_function<float,double>
{
public:
	typedef double value_type;
	typedef float time_type;
private:
	affine_combo<value_type,time_type> affine_func;
	value_type a,b,c,d;
	time_type r,s;

	value_type _coeff[4];
	time_type drs; // reciprocal of (s-r)
public:
	bezier_base():r(0.0),s(1.0),drs(1.0) { }
	bezier_base(
		const value_type &a, const value_type &b, const value_type &c, const value_type &d,
		const time_type &r=0.0, const time_type &s=1.0):
		a(a),b(b),c(c),d(d),r(r),s(s),drs(1.0/(s-r)) { sync(); }

	void sync()
	{
//		drs=1.0/(s-r);
		_coeff[0]=                 a;
		_coeff[1]=           b*3 - a*3;
		_coeff[2]=     c*3 - b*6 + a*3;
		_coeff[3]= d - c*3 + b*3 - a;
	}

	// 4 products, 3 sums, and 1 difference.
	inline value_type
	operator()(time_type t)const
	{ t-=r; t*=drs; return _coeff[0]+(_coeff[1]+(_coeff[2]+(_coeff[3])*t)*t)*t; }

	void set_rs(time_type new_r, time_type new_s) { r=new_r; s=new_s; drs=1.0/(s-r); }
	void set_r(time_type new_r) { r=new_r; drs=1.0/(s-r); }
	void set_s(time_type new_s) { s=new_s; drs=1.0/(s-r); }
	const time_type &get_r()const { return r; }
	const time_type &get_s()const { return s; }
	time_type get_dt()const { return s-r; }

	//! Bezier curve intersection function
	/*! Calculates the time of intersection
	**	for the calling curve.
	*/
	time_type intersect(const bezier_base<value_type,time_type> &x, time_type t=0.0,int i=15)const
	{
		//BROKEN - the time values of the 2 curves should be independent
		value_type system[4];
		system[0]=_coeff[0]-x._coeff[0];
		system[1]=_coeff[1]-x._coeff[1];
		system[2]=_coeff[2]-x._coeff[2];
		system[3]=_coeff[3]-x._coeff[3];

		t-=r;
		t*=drs;

		// Newton's method
		// Inner loop: 7 products, 5 sums, 1 difference
		for(;i;i--)
			t-= (system[0]+(system[1]+(system[2]+(system[3])*t)*t)*t)/
				(system[1]+(system[2]*2+(system[3]*3)*t)*t);

		t*=(s-r);
		t+=r;

		return t;
	}

	value_type &
	operator[](int i)
	{ return (&a)[i]; }

	const value_type &
	operator[](int i) const
	{ return (&a)[i]; }
};

//#ifdef __FIXED__

// Fast double implementation of a cubic bezier curve
/*
template <>
template <class T,unsigned int FIXED_BITS>
class bezier_base<fixed_base<T,FIXED_BITS> > : std::unary_function<fixed_base<T,FIXED_BITS>,fixed_base<T,FIXED_BITS> >
{
public:
	typedef fixed_base<T,FIXED_BITS> value_type;
	typedef fixed_base<T,FIXED_BITS> time_type;

private:
	affine_combo<value_type,time_type> affine_func;
	value_type a,b,c,d;
	time_type r,s;

	value_type _coeff[4];
	time_type drs; // reciprocal of (s-r)
public:
	bezier_base():r(0.0),s(1.0),drs(1.0) { }
	bezier_base(
		const value_type &a, const value_type &b, const value_type &c, const value_type &d,
		const time_type &r=0, const time_type &s=1):
		a(a),b(b),c(c),d(d),r(r),s(s),drs(1.0/(s-r)) { sync(); }

	void sync()
	{
		drs=time_type(1)/(s-r);
		_coeff[0]=                 a;
		_coeff[1]=           b*3 - a*3;
		_coeff[2]=     c*3 - b*6 + a*3;
		_coeff[3]= d - c*3 + b*3 - a;
	}

	// 4 products, 3 sums, and 1 difference.
	inline value_type
	operator()(time_type t)const
	{ t-=r; t*=drs; return _coeff[0]+(_coeff[1]+(_coeff[2]+(_coeff[3])*t)*t)*t; }

	void set_rs(time_type new_r, time_type new_s) { r=new_r; s=new_s; drs=time_type(1)/(s-r); }
	void set_r(time_type new_r) { r=new_r; drs=time_type(1)/(s-r); }
	void set_s(time_type new_s) { s=new_s; drs=time_type(1)/(s-r); }
	const time_type &get_r()const { return r; }
	const time_type &get_s()const { return s; }
	time_type get_dt()const { return s-r; }

	//! Bezier curve intersection function
	//! Calculates the time of intersection
	//	for the calling curve.
	//
	time_type intersect(const bezier_base<value_type,time_type> &x, time_type t=0,int i=15)const
	{
		value_type system[4];
		system[0]=_coeff[0]-x._coeff[0];
		system[1]=_coeff[1]-x._coeff[1];
		system[2]=_coeff[2]-x._coeff[2];
		system[3]=_coeff[3]-x._coeff[3];

		t-=r;
		t*=drs;

		// Newton's method
		// Inner loop: 7 products, 5 sums, 1 difference
		for(;i;i--)
			t-=(time_type) ( (system[0]+(system[1]+(system[2]+(system[3])*t)*t)*t)/
				(system[1]+(system[2]*2+(system[3]*3)*t)*t) );

		t*=(s-r);
		t+=r;

		return t;
	}

	value_type &
	operator[](int i)
	{ return (&a)[i]; }

	const value_type &
	operator[](int i) const
	{ return (&a)[i]; }
};
*/
//#endif

#endif



template <typename V, typename T>
class bezier_iterator
{
public:

	struct iterator_category {};
	typedef V value_type;
	typedef T difference_type;
	typedef V reference;

private:
	difference_type t;
	difference_type dt;
	bezier_base<V,T>	curve;

public:

/*
	reference
	operator*(void)const { return curve(t); }
	const surface_iterator&

	operator++(void)
	{ t+=dt; return &this; }

	const surface_iterator&
	operator++(int)
	{ hermite_iterator _tmp=*this; t+=dt; return _tmp; }

	const surface_iterator&
	operator--(void)
	{ t-=dt; return &this; }

	const surface_iterator&
	operator--(int)
	{ hermite_iterator _tmp=*this; t-=dt; return _tmp; }


	surface_iterator
	operator+(difference_type __n) const
	{ return surface_iterator(data+__n[0]+__n[1]*pitch,pitch); }

	surface_iterator
	operator-(difference_type __n) const
	{ return surface_iterator(data-__n[0]-__n[1]*pitch,pitch); }
*/

};

template <typename V,typename T=float>
class bezier : public bezier_base<V,T>
{
public:
	typedef V value_type;
	typedef T time_type;
	typedef float distance_type;
	typedef bezier_iterator<V,T> iterator;
	typedef bezier_iterator<V,T> const_iterator;

	distance_func<value_type> dist;

	using bezier_base<V,T>::get_r;
	using bezier_base<V,T>::get_s;
	using bezier_base<V,T>::get_dt;

public:
	bezier() { }
	bezier(const value_type &a, const value_type &b, const value_type &c, const value_type &d):
		bezier_base<V,T>(a,b,c,d) { }


	const_iterator begin()const;
	const_iterator end()const;

	time_type find_closest(const value_type& x, int i=7, time_type r=(0), time_type s=(1))const
	{
		float t((r+s)*0.5);
		for(;i;i--)
		{
			if(dist(operator()((s-r)*(1.0/3.0)+r),x) < dist(operator()((s-r)*(2.0/3.0)+r),x))
				s=t;
			else
				r=t;
			t=((r+s)*0.5);
		}
		return t;
	}


	distance_type find_distance(time_type r, time_type s, int steps=7)const
	{
		const time_type inc((s-r)/steps);
		distance_type ret(0);
		value_type last(operator()(r));

		for(r+=inc;r<s;r+=inc)
		{
			const value_type n(operator()(r));
			ret+=dist.uncook(dist(last,n));
			last=n;
		}
		ret+=dist.uncook(dist(last,operator()(r)))*(s-(r-inc))/inc;

		return ret;
	}

	distance_type length()const { return find_distance(get_r(),get_s()); }

	/* subdivide at some time t into 2 separate curves left and right

		b0 l1
		*		0+1 l2
		b1 		*		1+2*1+2 l3
		*		1+2		*			0+3*1+3*2+3 l4,r1
		b2 		*		1+2*2+2	r2	*
		*		2+3	r3	*
		b3 r4	*
		*

		0.1 2.3 ->	0.1 2 3 4 5.6
	*/
	void subdivide(bezier *left, bezier *right, const time_type &time = (time_type)0.5) const
	{
		time_type t=(t-get_r())/get_dt();
		bezier lt,rt;

		value_type temp;
		const value_type& a((*this)[0]);
		const value_type& b((*this)[1]);
		const value_type& c((*this)[2]);
		const value_type& d((*this)[3]);

		//1st stage points to keep
		lt[0] = a;
		rt[3] = d;

		//2nd stage calc
		lt[1] = affine_func(a,b,t);
		temp = affine_func(b,c,t);
		rt[2] = affine_func(c,d,t);

		//3rd stage calc
		lt[2] = affine_func(lt[1],temp,t);
		rt[1] = affine_func(temp,rt[2],t);

		//last stage calc
		lt[3] = rt[0] = affine_func(lt[2],rt[1],t);

		//set the time range for l,r (the inside values should be 1, 0 respectively)
		lt.set_r(get_r());
		rt.set_s(get_s());

		lt.sync();
		rt.sync();

		//give back the curves
		if(left) *left = lt;
		if(right) *right = rt;
	}


	void evaluate(time_type t, value_type &f, value_type &df) const
	{
		t=(t-get_r())/get_dt();

		const value_type& a((*this)[0]);
		const value_type& b((*this)[1]);
		const value_type& c((*this)[2]);
		const value_type& d((*this)[3]);

		const value_type p1 = affine_func(
							affine_func(a,b,t),
							affine_func(b,c,t)
							,t);
		const value_type p2 = affine_func(
							affine_func(b,c,t),
							affine_func(c,d,t)
						,t);

		f = affine_func(p1,p2,t);
		df = (p2-p1)*3;
	}
};

_ETL_END_NAMESPACE

/* === E X T E R N S ======================================================= */

/* === E N D =============================================================== */

#endif
