/*! ========================================================================
** Extended Template Library
** Bezier Template Class Implementation
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
** This is an internal header file, included by other ETL headers.
** You should not attempt to use it directly.
**
** ========================================================================= */

/* === S T A R T =========================================================== */

#ifndef __ETL__BEZIER_H
#define __ETL__BEZIER_H

/* === H E A D E R S ======================================================= */

#include "_curve_func.h"
#include <cmath>				// for ldexp
// #include <ETL/fixed>			// not used

/* === M A C R O S ========================================================= */

#define MAXDEPTH 64	/*  Maximum depth for recursion */

/* take binary sign of a, either -1, or 1 if >= 0 */
#define SGN(a)		(((a)<0) ? -1 : 1)

/* find minimum of a and b */
#ifndef MIN
#define MIN(a,b)	(((a)<(b))?(a):(b))
#endif

/* find maximum of a and b */
#ifndef MAX
#define MAX(a,b)	(((a)>(b))?(a):(b))
#endif

#define	BEZIER_EPSILON	(ldexp(1.0,-MAXDEPTH-1)) /*Flatness control value */
//#define	BEZIER_EPSILON	0.00005 /*Flatness control value */
#define	DEGREE	3			/*  Cubic Bezier curve		*/
#define	W_DEGREE 5			/*  Degree of eqn to find roots of */

/* === T Y P E D E F S ===================================================== */

/* === C L A S S E S & S T R U C T S ======================================= */

namespace etl {

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
	bezier_base(): r(0.0), s(1.0) { }
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

	bool intersect_hull(const bezier_base<value_type,time_type> &/*x*/)const
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
	time_type intersect(const bezier_base<value_type,time_type> &/*x*/, time_type /*near=0.0*/)const
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
	// affine_combo<value_type,time_type> affine_func;
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
	// affine_combo<value_type,time_type> affine_func;
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

	time_type find_closest(bool fast, const value_type& x, int i=7)const
	{
	    if (!fast)
	    {
			value_type array[4] = {
				bezier<V,T>::operator[](0),
				bezier<V,T>::operator[](1),
				bezier<V,T>::operator[](2),
				bezier<V,T>::operator[](3)};
			return NearestPointOnCurve(x, array);
	    }
	    else
	    {
			time_type r(0), s(1);
			float t((r+s)*0.5); /* half way between r and s */

			for(;i;i--)
			{
				// compare 33% of the way between r and s with 67% of the way between r and s
				if(dist(this->operator()((s-r)*(1.0/3.0)+r), x) <
				   dist(this->operator()((s-r)*(2.0/3.0)+r), x))
					s=t;
				else
					r=t;
				t=((r+s)*0.5);
			}
			return t;
		}
	}

	distance_type find_distance(time_type r, time_type s, int steps=7)const
	{
		const time_type inc((s-r)/steps);
		if (!inc) return 0;
		distance_type ret(0);
		value_type last(this->operator()(r));

		for(r+=inc;r<s;r+=inc)
		{
			const value_type n(this->operator()(r));
			ret+=dist.uncook(dist(last,n));
			last=n;
		}
		ret+=dist.uncook(dist(last,this->operator()(r)))*(s-(r-inc))/inc;

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
		time_type t=(time-get_r())/get_dt();
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
		lt[1] = this->affine_func(a,b,t);
		temp = this->affine_func(b,c,t);
		rt[2] = this->affine_func(c,d,t);

		//3rd stage calc
		lt[2] = this->affine_func(lt[1],temp,t);
		rt[1] = this->affine_func(temp,rt[2],t);

		//last stage calc
		lt[3] = rt[0] = this->affine_func(lt[2],rt[1],t);

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

private:
	/*
	 *  Bezier :
	 *	Evaluate a Bezier curve at a particular parameter value
	 *      Fill in control points for resulting sub-curves if "Left" and
	 *	"Right" are non-null.
	 *
	 *    int 			degree;		Degree of bezier curve
	 *    value_type 	*VT;		Control pts
	 *    time_type 	t;			Parameter value
	 *    value_type 	*Left;		RETURN left half ctl pts
	 *    value_type 	*Right;		RETURN right half ctl pts
	 */
	static value_type Bezier(value_type *VT, int degree, time_type t, value_type *Left, value_type *Right)
	{
		int 		i, j;		/* Index variables	*/
		value_type 	Vtemp[W_DEGREE+1][W_DEGREE+1];

		/* Copy control points	*/
		for (j = 0; j <= degree; j++)
			Vtemp[0][j] = VT[j];

		/* Triangle computation	*/
		for (i = 1; i <= degree; i++)
			for (j =0 ; j <= degree - i; j++)
			{
				Vtemp[i][j][0] = (1.0 - t) * Vtemp[i-1][j][0] + t * Vtemp[i-1][j+1][0];
				Vtemp[i][j][1] = (1.0 - t) * Vtemp[i-1][j][1] + t * Vtemp[i-1][j+1][1];
			}

		if (Left != NULL)
			for (j = 0; j <= degree; j++)
				Left[j]  = Vtemp[j][0];

		if (Right != NULL)
			for (j = 0; j <= degree; j++)
				Right[j] = Vtemp[degree-j][j];

		return (Vtemp[degree][0]);
	}

	/*
	 * CrossingCount :
	 *	Count the number of times a Bezier control polygon
	 *	crosses the 0-axis. This number is >= the number of roots.
	 *
	 *    value_type	*VT;			Control pts of Bezier curve
	 */
	static int CrossingCount(value_type *VT)
	{
		int 	i;
		int 	n_crossings = 0;	/*  Number of zero-crossings	*/
		int		sign, old_sign;		/*  Sign of coefficients		*/

		sign = old_sign = SGN(VT[0][1]);
		for (i = 1; i <= W_DEGREE; i++)
		{
			sign = SGN(VT[i][1]);
			if (sign != old_sign) n_crossings++;
			old_sign = sign;
		}

		return n_crossings;
	}

	/*
	 *  ControlPolygonFlatEnough :
	 *	Check if the control polygon of a Bezier curve is flat enough
	 *	for recursive subdivision to bottom out.
	 *
	 *    value_type	*VT;		Control points
	 */
	static int ControlPolygonFlatEnough(value_type *VT)
	{
		int 			i;					/* Index variable					*/
		distance_type 	distance[W_DEGREE] = {};	/* Distances from pts to line		*/
		distance_type 	max_distance_above;	/* maximum of these					*/
		distance_type 	max_distance_below;
		time_type 		intercept_1, intercept_2, left_intercept, right_intercept;
		distance_type 	a, b, c;			/* Coefficients of implicit			*/
		/* eqn for line from VT[0]-VT[deg]			*/
		/* Find the  perpendicular distance			*/
		/* from each interior control point to 		*/
		/* line connecting VT[0] and VT[W_DEGREE]	*/
		{
			distance_type	abSquared;

			/* Derive the implicit equation for line connecting first *
			 *  and last control points */
			a = VT[0][1] - VT[W_DEGREE][1];
			b = VT[W_DEGREE][0] - VT[0][0];
			c = VT[0][0] * VT[W_DEGREE][1] - VT[W_DEGREE][0] * VT[0][1];

			abSquared = (a * a) + (b * b);

			for (i = 1; i < W_DEGREE; i++)
			{
				/* Compute distance from each of the points to that line	*/
				distance[i] = a * VT[i][0] + b * VT[i][1] + c;
				if (distance[i] > 0.0) distance[i] =  (distance[i] * distance[i]) / abSquared;
				if (distance[i] < 0.0) distance[i] = -(distance[i] * distance[i]) / abSquared;
			}
		}

		/* Find the largest distance */
		max_distance_above = max_distance_below = 0.0;

		for (i = 1; i < W_DEGREE; i++)
		{
			if (distance[i] < 0.0) max_distance_below = MIN(max_distance_below, distance[i]);
			if (distance[i] > 0.0) max_distance_above = MAX(max_distance_above, distance[i]);
		}

		/* Implicit equation for "above" line */
		intercept_1 = -(c + max_distance_above)/a;

		/*  Implicit equation for "below" line */
		intercept_2 = -(c + max_distance_below)/a;

		/* Compute intercepts of bounding box	*/
		left_intercept = MIN(intercept_1, intercept_2);
		right_intercept = MAX(intercept_1, intercept_2);

		return 0.5 * (right_intercept-left_intercept) < BEZIER_EPSILON ? 1 : 0;
	}

	/*
	 *  ComputeXIntercept :
	 *	Compute intersection of chord from first control point to last
	 *  with 0-axis.
	 *
	 *    value_type 	*VT;			Control points
	 */
	static time_type ComputeXIntercept(value_type *VT)
	{
		distance_type YNM = VT[W_DEGREE][1] - VT[0][1];
		return (YNM*VT[0][0] - (VT[W_DEGREE][0] - VT[0][0])*VT[0][1]) / YNM;
	}

	/*
	 *  FindRoots :
	 *	Given a 5th-degree equation in Bernstein-Bezier form, find
	 *	all of the roots in the interval [0, 1].  Return the number
	 *	of roots found.
	 *
	 *    value_type	*w;				The control points
	 *    time_type 	*t;				RETURN candidate t-values
	 *    int 			depth;			The depth of the recursion
	 */
	static int FindRoots(value_type *w, time_type *t, int depth)
	{
		int 		i;
		value_type 	Left[W_DEGREE+1];	/* New left and right 	*/
		value_type	Right[W_DEGREE+1];	/* control polygons		*/
		int 		left_count;			/* Solution count from	*/
		int			right_count;		/* children				*/
		time_type 	left_t[W_DEGREE+1];	/* Solutions from kids	*/
		time_type	right_t[W_DEGREE+1];

		switch (CrossingCount(w))
		{
			case 0 :
			{	/* No solutions here	*/
				return 0;
			}
			case 1 :
			{	/* Unique solution	*/
				/* Stop recursion when the tree is deep enough		*/
				/* if deep enough, return 1 solution at midpoint 	*/
				if (depth >= MAXDEPTH)
				{
					t[0] = (w[0][0] + w[W_DEGREE][0]) / 2.0;
					return 1;
				}
				if (ControlPolygonFlatEnough(w))
				{
					t[0] = ComputeXIntercept(w);
					return 1;
				}
				break;
			}
		}

		/* Otherwise, solve recursively after	*/
		/* subdividing control polygon			*/
		Bezier(w, W_DEGREE, 0.5, Left, Right);
		left_count  = FindRoots(Left,  left_t,  depth+1);
		right_count = FindRoots(Right, right_t, depth+1);

		/* Gather solutions together	*/
		for (i = 0; i < left_count;  i++) t[i] = left_t[i];
		for (i = 0; i < right_count; i++) t[i+left_count] = right_t[i];

		/* Send back total number of solutions	*/
		return (left_count+right_count);
	}

	/*
	 *  ConvertToBezierForm :
	 *		Given a point and a Bezier curve, generate a 5th-degree
	 *		Bezier-format equation whose solution finds the point on the
	 *      curve nearest the user-defined point.
	 *
	 *    value_type& 	P;				The point to find t for
	 *    value_type 	*VT;			The control points
	 */
	static void ConvertToBezierForm(const value_type& P, value_type *VT, value_type w[W_DEGREE+1])
	{
		int 	i, j, k, m, n, ub, lb;
		int 	row, column;				/* Table indices				*/
		value_type 	c[DEGREE+1];			/* VT(i)'s - P					*/
		value_type 	d[DEGREE];				/* VT(i+1) - VT(i)				*/
		distance_type 	cdTable[3][4];		/* Dot product of c, d			*/
		static distance_type z[3][4] = {	/* Precomputed "z" for cubics	*/
			{1.0, 0.6, 0.3, 0.1},
			{0.4, 0.6, 0.6, 0.4},
			{0.1, 0.3, 0.6, 1.0}};

		/* Determine the c's -- these are vectors created by subtracting */
		/* point P from each of the control points						 */
		for (i = 0; i <= DEGREE; i++)
			c[i] = VT[i] - P;

		/* Determine the d's -- these are vectors created by subtracting */
		/* each control point from the next								 */
		for (i = 0; i <= DEGREE - 1; i++)
			d[i] = (VT[i+1] - VT[i]) * 3.0;

		/* Create the c,d table -- this is a table of dot products of the */
		/* c's and d's													  */
		for (row = 0; row <= DEGREE - 1; row++)
			for (column = 0; column <= DEGREE; column++)
				cdTable[row][column] = d[row] * c[column];

		/* Now, apply the z's to the dot products, on the skew diagonal */
		/* Also, set up the x-values, making these "points"				*/
		for (i = 0; i <= W_DEGREE; i++)
		{
			w[i][0] = (distance_type)(i) / W_DEGREE;
			w[i][1] = 0.0;
		}

		n = DEGREE;
		m = DEGREE-1;
		for (k = 0; k <= n + m; k++)
		{
			lb = MAX(0, k - m);
			ub = MIN(k, n);
			for (i = lb; i <= ub; i++)
			{
				j = k - i;
				w[k][1] += cdTable[j][i] * z[j][i];
				//w[i+j][1] += cdTable[j][i] * z[j][i];
			}
		}
	}

	/*
	 *  NearestPointOnCurve :
	 *  	Compute the parameter value of the point on a Bezier
	 *		curve segment closest to some arbitrary, user-input point.
	 *		Return the point on the curve at that parameter value.
	 *
	 *    value_type& 	P;			The user-supplied point
	 *    value_type 	*VT;		Control points of cubic Bezier
	 */
	static time_type NearestPointOnCurve(const value_type& P, value_type VT[4])
	{
		value_type 	w[W_DEGREE+1];			/* Ctl pts of 5th-degree curve  */
		time_type 	t_candidate[W_DEGREE];	/* Possible roots				 */
		int 		n_solutions;			/* Number of roots found		 */
		time_type	t;						/* Parameter value of closest pt */

		/*  Convert problem to 5th-degree Bezier form */
		ConvertToBezierForm(P, VT, w);

		/* Find all possible roots of 5th-degree equation */
		n_solutions = FindRoots(w, t_candidate, 0);

		/* Compare distances of P to all candidates, and to t=0, and t=1 */
		{
			distance_type 	dist, new_dist;
			value_type 		p, v;
			int				i;

			/* Check distance to beginning of curve, where t = 0	*/
			dist = (P - VT[0]).mag_squared();
			t = 0.0;

			/* Find distances for candidate points	*/
			for (i = 0; i < n_solutions; i++)
			{
				p = Bezier(VT, DEGREE, t_candidate[i], (value_type *)NULL, (value_type *)NULL);
				new_dist = (P - p).mag_squared();
				if (new_dist < dist)
				{
					dist = new_dist;
					t = t_candidate[i];
				}
			}

			/* Finally, look at distance to end point, where t = 1.0 */
			new_dist = (P - VT[DEGREE]).mag_squared();
			if (new_dist < dist)
			{
				dist = new_dist;
				t = 1.0;
			}
		}

		/*  Return the point on the curve at parameter value t */
		return t;
	}
};

};

/* === E X T E R N S ======================================================= */

/* === E N D =============================================================== */

#endif
