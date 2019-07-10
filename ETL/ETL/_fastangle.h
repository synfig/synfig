/*! ========================================================================
** Extended Template and Library
** Fast fastangle Abstraction Class Implementation
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

#ifndef __ETL__FASTANGLE_H
#define __ETL__FASTANGLE_H

/* === H E A D E R S ======================================================= */

#include <cmath>
#include <ETL/fixed>

#include "_fastangle_tables.h"

/* === M A C R O S ========================================================= */

#ifndef PI
# define PI (3.1415926535897932384626433832795029L)
#endif

#define ETL_FASTANGLE_INIT()

/* === T Y P E D E F S ===================================================== */

/* === C L A S S E S & S T R U C T S ======================================= */

namespace etl {

/*! ========================================================================
** \class	fastangle
** \brief	Optimized abstraction of the concept of an angle
**
** A more detailed description needs to be written.
*/
class fastangle
{
public:
	typedef double value_type;

protected:
	typedef fixed_base<ETL_FIXED_TYPE,ETL_FASTANGLE_LOOKUP_RES> unit;

	unit v;	//! Stored in rotations

public:

	/*
	** Arithmetic Operators
	*/

	//! fastangle Addition Operator
	fastangle
	operator+(const fastangle &rhs)const
	{
		fastangle ret;
		ret.v=v+rhs.v;
		return ret;
	}

	//! fastangle Subtraction Operator
	/*! \sa fastangle dist(const fastangle &) */
	fastangle
	operator-(const fastangle &rhs)const
	{
		fastangle ret;
		ret.v=v-rhs.v;
		return ret;
	}

	//! fastangle Scalar Multiplication Operator
	/*! This operator will multiply the given
		fastangle by the given scalar value. */
	fastangle
	operator*(const unit &rhs)const
	{
		fastangle ret;
		ret.v=v*rhs;
		return ret;
	}

	fastangle
	operator/(const unit &rhs)const
	{
		fastangle ret;
		ret.v=v/rhs;
		return ret;
	}

	const fastangle	&
	operator+=(const fastangle &rhs)
	{
		v+=rhs.v;
		return *this;
	}

	const fastangle	&
	operator-=(const fastangle &rhs)
	{
		v-=rhs.v;
		return *this;
	}

	const fastangle	&
	operator*=(const unit &rhs)
	{
		v*=rhs;
		return *this;
	}

	const fastangle	&
	operator/=(const unit &rhs)
	{
		v/=rhs;
		return *this;
	}

	//! fastangle Negation
	fastangle
	operator-()const
	{
		fastangle ret;
		ret.v=-v;
		return ret;
	}

	//! 180 degree rotation operator
	/*! Returns the fastangle directly opposite of
		the given fastangle, and will yield a result
		between 0 and 2PI */
	fastangle
	operator~()const
	{
		fastangle ret;
		ret.v=(unit)std::floor(v+0.5f);
		return ret;
	}

	/*! Returns true if the shortest
		fastangle between the left-hand and
		right-hand side is clockwise */
	bool
	operator<(const fastangle &rhs)const
	{ return v<rhs.v; }
//	{ return dist(rhs).v<(value_type)0.0; }

	/*! Returns true if the shortest
		fastangle between the left-hand and
		right-hand side is counter-clockwise */
	bool
	operator>(const fastangle &rhs)const
	{ return v>rhs.v; }
//	{ return dist(rhs).v>(value_type)0.0; }

	/*! Returns true if the shortest
		fastangle between the left-hand and
		right-hand side is clockwise,
		or if the angles are refer to the same
		point on the unit circle. */
	bool
	operator<=(const fastangle &rhs)const
	{ return v<=rhs.v; }
//	{ return dist(rhs).v<=(value_type)0.0; }

	/*! Returns true if the shortest
		fastangle between the left-hand and
		right-hand side is counter-clockwise,
		or if the angles are refer to the same
		point on the unit circle. */
	bool
	operator>=(const fastangle &rhs)const
	{ return v>=rhs.v; }
//	{ return dist(rhs).v>=(value_type)0.0; }

	/*! Returns true if the angles
		are refer to the same point
		on the unit circle. */
	bool
	operator==(const fastangle &rhs)const
	{ return v==rhs.v; }
//	{ return dist(rhs).v==(value_type)0.0; }

	/*! Returns false if the angles
		are refer to the same point
		on the unit circle. */
	bool
	operator!=(const fastangle &rhs)const
	{ return v!=rhs.v; }
//	{ return dist(rhs).v!=(value_type)0.0; }

	//! fastangle Difference Function
	/*! This function will return the
		shortest physical distance between
		two angles, from -PI/2 to PI/2
		\warning Not yet tested
		\sa fastangle operator-(const fastangle &) */
	fastangle
	dist(const fastangle &rhs)const
	{
		fastangle ret;
		ret.v=v-rhs.v;
		ret.v-=(unit)std::floor(ret.v+0.5f);
		return ret;
	}

	//! Rotation Modulus
	/*! This function will return the
		value of the fastangle between 0 and 2PI */
	fastangle
	mod()const
	{
		fastangle ret(*this);
		ret.v-=(unit)std::floor(ret.v);
		return ret;
	}

	static fastangle
	zero()
	{
		fastangle ret;
		ret.v=0;
		return ret;
	}

	bool operator!()const { return v==unit(0); }

	/*
	** Conversion Classes
	*/

	class radians;
	class degrees;
	class rotations;

	/*
	** Trigonometric Classes
	*/

	class sin;
	class cos;
	class tan;

	/*
	** Friend classes
	*/

	friend class radians;
	friend class degrees;
	friend class rotations;
	friend class sin;
	friend class cos;
	friend class tan;

	/*
	** Bleh...
	*/

	typedef radians		rad;
	typedef degrees		deg;
	typedef rotations	rot;

}; // END of class fastangle

/*! ========================================================================
** \class	fastangle::radians
** \brief	fastangle representation in radians
**
** A more detailed description needs to be written.
*/
class fastangle::radians : public fastangle
{
public:
	radians(const value_type &x) { v=x/((value_type)PI*2.0f); }
	radians(const fastangle &a):fastangle(a) { }
	radians	mod()const { return fastangle::mod(); }
	radians dist(const fastangle &rhs)const { return fastangle::dist(rhs); }
	operator value_type()const { return get(); }
	value_type get()const { return (value_type)v*(value_type)PI*2.0f; }
}; // END of class fastangle::radians

/*! ========================================================================
** \class	fastangle::degrees
** \brief	fastangle representation in degrees
**
** A more detailed description needs to be written.
*/
class fastangle::degrees : public fastangle
{
public:
	degrees(const value_type &x) { v=x/360; }
	degrees(const fastangle &a):fastangle(a) { }
	degrees	mod()const { return fastangle::mod(); }
	degrees dist(const fastangle &rhs)const { return fastangle::dist(rhs); }
	operator value_type()const { return get(); }
	value_type get()const { return v*360/*(value_type)(v-::floor(v))*360*/; }
}; // END of class fastangle::degrees

/*! ========================================================================
** \class	fastangle::rotations
** \brief	fastangle representation in rotations
**
** A more detailed description needs to be written.
*/
class fastangle::rotations : public fastangle
{
public:
	rotations(const value_type &x) { v=x; }
	rotations(const fastangle &a):fastangle(a) { }
	rotations mod()const { return fastangle::mod(); }
	rotations dist(const fastangle &rhs)const { return fastangle::dist(rhs); }
	operator value_type()const { return get(); }
	value_type get()const { return v; }
}; // END of class fastangle::rotations

/*! ========================================================================
** \class	fastangle::sin
** \brief	fastangle representation as a sine function
**
** A more detailed description needs to be written.
*/
class fastangle::sin : public fastangle
{
public:
	sin(const value_type &x)	{ v.data()=_fastangle_asin_table[(int)((x+1)*(value_type)(1<<(ETL_FASTANGLE_LOOKUP_RES-1)))]; }
	sin(const fastangle &a):fastangle(a) { }
	sin	mod()const { return fastangle::mod(); }
	sin dist(const fastangle &rhs)const { return fastangle::dist(rhs); }
	operator value_type()const { return get(); }
	value_type get()const { return (value_type)_fastangle_sin_table[v.data()&( (1<<ETL_FASTANGLE_LOOKUP_RES)-1)]; }
}; // END of class fastangle::sin

/*! ========================================================================
** \class	fastangle::cos
** \brief	fastangle representation as a cosine function
**
** A more detailed description needs to be written.
*/
class fastangle::cos : public fastangle
{
public:
	cos(const value_type &x)	{ v.data()=(1<<(ETL_FASTANGLE_LOOKUP_RES-2))-_fastangle_asin_table[(int)((x+1)*(value_type)(1<<(ETL_FASTANGLE_LOOKUP_RES-1)))]; }
	cos(const fastangle &a):fastangle(a) { }
	cos	mod()const { return fastangle::mod(); }
	cos dist(const fastangle &rhs)const { return fastangle::dist(rhs); }
	operator value_type()const { return get(); }
	value_type get()const { return (value_type)_fastangle_sin_table[(v.data()+(1<<(ETL_FASTANGLE_LOOKUP_RES-2)))&( (1<<ETL_FASTANGLE_LOOKUP_RES)-1)]; }
}; // END of class fastangle::cos

/*! ========================================================================
** \class	fastangle::tan
** \brief	fastangle representation as a tangent function
**
** A more detailed description needs to be written.
*/
class fastangle::tan : public fastangle
{
public:
	tan(const value_type &x)
	{
		if(x>1)
			v.data()=(1<<(ETL_FASTANGLE_LOOKUP_RES-2))-_fastangle_atan_table[(int)(((1.0/x)+1)*(value_type)((1<<(ETL_FASTANGLE_LOOKUP_RES-1))-1))];
		else if(x<-1)
			v.data()=-(1<<(ETL_FASTANGLE_LOOKUP_RES-1)) + (1<<(ETL_FASTANGLE_LOOKUP_RES-2)) - _fastangle_atan_table[(int)(((1.0/x)+1)*(value_type)((1<<(ETL_FASTANGLE_LOOKUP_RES-1))-1))];
		else
			v.data()=_fastangle_atan_table[(int)((x+1)*(value_type)((1<<(ETL_FASTANGLE_LOOKUP_RES-1))-1))];
	}

	tan(const value_type &y,const value_type &x)
	{
		if(x>=0 && y>=0) // First quadrant
		{
			if(y>x)
				v.data()=(1<<(ETL_FASTANGLE_LOOKUP_RES-2))-_fastangle_atan_table[(int)(((x/y)+1)*(value_type)((1<<(ETL_FASTANGLE_LOOKUP_RES-1))-1))];
			else
				v.data()=_fastangle_atan_table[(int)(((y/x)+1)*(value_type)((1<<(ETL_FASTANGLE_LOOKUP_RES-1))-1))];
		}
		else if(x>=0 && y<0) // Fourth quadrant
		{
			if(-y>x)
				v.data()=-(1<<(ETL_FASTANGLE_LOOKUP_RES-1)) + (1<<(ETL_FASTANGLE_LOOKUP_RES-2))-_fastangle_atan_table[(int)(((x/y)+1.0)*(value_type)((1<<(ETL_FASTANGLE_LOOKUP_RES-1))-1))];
			else
				v.data()=_fastangle_atan_table[(int)(((y/x)+1.0)*(value_type)((1<<(ETL_FASTANGLE_LOOKUP_RES-1))-1))];
		}
		else if(x<0 && y>=0) // Second quadrant
		{
			if(y>-x)
				v.data()=(1<<(ETL_FASTANGLE_LOOKUP_RES-2))-_fastangle_atan_table[(int)(((x/y)+1.0)*(value_type)((1<<(ETL_FASTANGLE_LOOKUP_RES-1))-1))];
			else
				v.data()=_fastangle_atan_table[(int)(((y/x)+1.0)*(value_type)((1<<(ETL_FASTANGLE_LOOKUP_RES-1))-1))]+(1<<(ETL_FASTANGLE_LOOKUP_RES-1));
		}
		else if(x<0 && y<0) // Third Quadrant
		{
			if(-y>-x)
				v.data()=(1<<(ETL_FASTANGLE_LOOKUP_RES-2))-_fastangle_atan_table[(int)(((x/y)+1.0)*(value_type)((1<<(ETL_FASTANGLE_LOOKUP_RES-1))-1))] - (1<<(ETL_FASTANGLE_LOOKUP_RES-1));
			else
				v.data()=_fastangle_atan_table[(int)(((y/x)+1.0)*(value_type)((1<<(ETL_FASTANGLE_LOOKUP_RES-1))-1))]-(1<<(ETL_FASTANGLE_LOOKUP_RES-1));
		}
		else v.data()=0;
	}
	tan(const fastangle &a):fastangle(a) { }
	tan	mod()const { return fastangle::mod(); }
	tan dist(const fastangle &rhs)const { return fastangle::dist(rhs); }
	operator value_type()const { return get(); }
	value_type get()const { return (value_type)_fastangle_tan_table[v.data()&( (1<<ETL_FASTANGLE_LOOKUP_RES)-1)]; }
}; // END of class fastangle::tan

};

template <>
struct affine_combo<etl::fastangle,float>
{
	etl::fastangle operator()(const etl::fastangle &a,const etl::fastangle &b,const float &t)const
	{
		return b.dist(a)*t+a;
	}

	etl::fastangle reverse(const etl::fastangle &x, const etl::fastangle &b, const float &t)const
	{
		return x.dist(b*t)*((float)1/((float)1-t));
	}
};

template <>
struct distance_func<etl::fastangle> : public std::binary_function<etl::fastangle, etl::fastangle, etl::fastangle>
{
	etl::fastangle operator()(const etl::fastangle &a,const etl::fastangle &b)const
	{
		etl::fastangle delta=b.dist(a);
		if(delta<etl::fastangle::zero())
			return -delta;
		return delta;
	}

	etl::fastangle cook(const etl::fastangle &x) { return x; }
	etl::fastangle uncook(const etl::fastangle &x) { return x; }
};

/* === E N D =============================================================== */

#endif
