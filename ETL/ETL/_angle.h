/* ========================================================================
** Extended Template and Library
** Angle Abstraction Class Implementation
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

#ifndef __ETL__ANGLE_H
#define __ETL__ANGLE_H

/* === H E A D E R S ======================================================= */

#include <cstdio>
#include <cmath>
#include <functional>

/* === M A C R O S ========================================================= */

#ifndef PI
# define PI (3.1415926535897932384626433832795029L)
# define HALF_PI (PI/2)
#endif

#define ANGLE_EPSILON (1.0e-6)

/* === T Y P E D E F S ===================================================== */

/* === C L A S S E S & S T R U C T S ======================================= */

namespace etl {

// ========================================================================
/*!	\class	angle	_angle.h	ETL/angle
**	\brief	Abstraction of the concept of an angle
**  \see angle::deg, angle::rad, angle::rot, angle::sin, angle::cos, angle::tan, fastangle
**	\writeme
*/
class angle
{
public:
	typedef float value_type;

protected:
	typedef value_type unit;

	unit v;	//! Stored in radians; positive values indicate counter-clockwise.

public:

	/*
	** Arithmetic Operators
	*/

	const angle	&
	operator+=(const angle &rhs)
	{ v+=rhs.v; return *this; }

	const angle	&
	operator-=(const angle &rhs)
	{ v-=rhs.v; return *this; }

	const angle	&
	operator*=(const unit &rhs)
	{ v*=rhs; return *this; }

	const angle	&
	operator/=(const unit &rhs)
	{ v/=rhs; return *this; }

	//! Angle Addition Operator
	angle
	operator+(const angle &rhs)const
	{ return angle(*this)+=rhs; }

	//! Angle Subtraction Operator
	/*! \sa angle dist(const angle &) */
	angle
	operator-(const angle &rhs)const
	{ return angle(*this)-=rhs; }

	//! Angle Scalar Multiplication Operator
	/*! This operator will multiply the given
		angle by the given scalar value. */
	angle
	operator*(const unit &rhs)const
	{ return angle(*this)*=rhs; }

	angle
	operator/(const unit &rhs)const
	{ return angle(*this)/=rhs; }

	//! Angle Negation
	angle
	operator-()const
	{
		angle ret;
		ret.v=-v;
		return ret;
	}

#ifdef ETL_NOT_USED
	//! 180 degree rotation operator
	/*! Returns the angle directly opposite of
		the given angle, and will yield a result
		between 0 and 2PI */
	angle
	operator~()const
	{
		angle ret;
		ret.v = v+PI;
		return ret.mod();
	}
#endif // ETL_NOT_USED

#ifdef ETL_WRAP_ANGLES
	/*! Returns true if the shortest
		angle from the left-hand to the
		right-hand side is counter-clockwise */
	bool
	operator<(const angle &rhs)const
	{ return dist(rhs).v<(value_type)0.0; }

	/*! Returns true if the shortest
		angle from the left-hand to the
		right-hand side is clockwise */
	bool
	operator>(const angle &rhs)const
	{ return dist(rhs).v>(value_type)0.0; }

	/*! Returns true if the shortest
		angle from the left-hand to the
		right-hand side is counter-clockwise,
		or if the angles are refer to the same
		point on the unit circle. */
	bool
	operator<=(const angle &rhs)const
	{ return dist(rhs).v<=(value_type)0.0; }

	/*! Returns true if the shortest
		angle from the left-hand to the
		right-hand side is clockwise,
		or if the angles are refer to the same
		point on the unit circle. */
	bool
	operator>=(const angle &rhs)const
	{ return dist(rhs).v>=(value_type)0.0; }

	/*! Returns true if the angles
		are refer to the same point
		on the unit circle. */
	bool
	operator==(const angle &rhs)const
	{ return std::abs(dist(rhs).v)<ANGLE_EPSILON; }

	/*! Returns false if the angles
		are refer to the same point
		on the unit circle. */
	bool
	operator!=(const angle &rhs)const
	{ return std::abs(dist(rhs).v)>ANGLE_EPSILON; }
#else // ETL_WRAP_ANGLES
	/*! Returns true if the left-hand
		side is less than the
		right-hand side */
	bool
	operator<(const angle &rhs)const
	{ return v < rhs.v; }

	/*! Returns true if the left-hand
		side is greater than the
		right-hand side */
	bool
	operator>(const angle &rhs)const
	{ return v > rhs.v; }

	/*! Returns true if the left-hand
		side is less or equal to the
		right-hand side */
	bool
	operator<=(const angle &rhs)const
	{ return v <= rhs.v; }

	/*! Returns true if the left-hand
		side is greater than or equal
		to the right-hand side */
	bool
	operator>=(const angle &rhs)const
	{ return v >= rhs.v; }

	/*! Returns true if the angles
		are the same, or close */
	bool
	operator==(const angle &rhs)const
	{ return std::abs(v - rhs.v)<ANGLE_EPSILON; }

	/*! Returns false if the angles
		are different */
	bool
	operator!=(const angle &rhs)const
		{ return std::abs(v - rhs.v)>ANGLE_EPSILON; }
#endif // ETL_WRAP_ANGLES

	//! Absolute Angle Function
	/*! This function will return the
		absolute value of the angle. */
	angle
	abs()const
	{
		angle ret;
		ret.v=std::abs(v);
		return ret;
	}

#ifdef ETL_WRAP_ANGLES
	//! Angle Difference Function
	/*! This function will return the
		shortest physical distance between
		two angles, from -PI/2 to PI/2
		\sa angle operator-(const angle &) */
	angle
	dist(const angle &rhs)const
	{
		angle ret;
		ret.v=v-rhs.v;
		ret.v-=rot_floor(ret.v+PI);
		return ret;
	}

	//! Rotation Modulus
	/*! This function will return the
		value of the angle between 0 and 2PI */
	angle
	mod()const
	{
		angle ret(*this);
		ret.v-=rot_floor(ret.v);
		return ret;
	}
#else // ETL_WRAP_ANGLES
	//! Angle Difference Function
	/*! This function will return the
		difference between
		two angles, just like
		\sa angle operator-(const angle &) */
	angle
	dist(const angle &rhs)const
	{ return angle(*this)-=rhs; }

	//! Rotation Modulus
	/*! This function will return the
		value of the angle */
	angle
	mod()const
	{
		angle ret(*this);
		return ret;
	}
#endif // ETL_WRAP_ANGLES

	//! Zero Rotation (0 degrees)
	static angle
	zero()
	{
		angle ret;
		ret.v=0;
		return ret;
	}

	//! One Complete Rotation (360 degrees)
	static angle
	one()
	{
		angle ret;
		ret.v=PI*2;
		return ret;
	}

	//! One Half Rotation (180 degrees)
	static angle
	half()
	{
		angle ret;
		ret.v=PI;
		return ret;
	}

	bool operator!()const { return std::abs(mod().v) < ANGLE_EPSILON; }

private:

#ifdef ETL_WRAP_ANGLES
	static value_type rot_floor(value_type x)
	{ return static_cast<value_type>(std::floor(x/(PI*2))*PI*2); }
#endif // ETL_WRAP_ANGLES

public:
	/*
	** Conversion Classes
	*/

	class rad;
	class deg;
	class rot;

	/*
	** Trigonometric Classes
	*/

	class sin;
	class cos;
	class tan;

	/*
	** Friend classes
	*/

	friend class rad;
	friend class deg;
	friend class rot;
	friend class sin;
	friend class cos;
	friend class tan;

	/*
	** Deprecated
	*/

#ifndef ETL_NO_DEPRECATED
	typedef rad		radians;
	typedef deg		degrees;
	typedef rot		rotations;
#endif
}; // END of class angle

// ========================================================================
/*!	\class	angle::rad	_angle.h	ETL/angle
**	\brief	Angle representation in radians
**	\see angle
**	\writeme
*/
class angle::rad : public angle
{
public:
	explicit rad(const value_type &x) { v=x; }
	rad(const angle &a):angle(a) { }
	rad	mod()const { return angle::mod(); }
	rad dist(const angle &rhs)const { return angle::dist(rhs); }
	value_type get()const { return v; }
#ifndef ETL_NO_DEPRECATED
	// operator value_type()const ETL_DEPRECATED_FUNCTION;
#endif
}; // END of class angle::radians
// inline angle::rad::operator angle::value_type()const { return get(); }

// ========================================================================
/*!	\class	angle::deg	_angle.h	ETL/angle
**	\brief	Angle representation in degrees
**	\see angle
**	\writeme
*/
class angle::deg : public angle
{
public:
	explicit deg(const value_type &x) { v=x*((PI*2)/360); }
	deg(const angle &a):angle(a) { }
	deg	mod()const { return angle::mod(); }
	deg dist(const angle &rhs)const { return angle::dist(rhs); }
	value_type get()const { return v*360/(PI*2); }
#ifndef ETL_NO_DEPRECATED
	// operator value_type()const ETL_DEPRECATED_FUNCTION;
#endif
}; // END of class angle::degrees
// inline angle::deg::operator angle::value_type()const { return get(); }

// ========================================================================
/*!	\class	angle::rot	_angle.h	ETL/angle
**	\brief	Angle representation in rotations
**	\see angle
**	\writeme
*/
class angle::rot : public angle
{
public:
	explicit rot(const value_type &x) { v=x*(PI*2); }
	rot(const angle &a):angle(a) { }
	rot mod()const { return angle::mod(); }
	rot dist(const angle &rhs)const { return angle::dist(rhs); }
	value_type get()const { return v/(PI*2); }
#ifndef ETL_NO_DEPRECATED
	// operator value_type()const ETL_DEPRECATED_FUNCTION;
#endif
}; // END of class angle::rotations
// inline angle::rot::operator angle::value_type()const { return get(); }

// ========================================================================
/*!	\class	angle::sin	_angle.h	ETL/angle
**	\brief	Angle representation as a sine function
**	\see angle
**	\writeme
*/
class angle::sin : public angle
{
public:
	explicit sin(const value_type &x) { v=static_cast<value_type>(std::asin(x)); }
	sin(const angle &a):angle(a) { }
	sin	mod()const { return angle::mod(); }
	sin dist(const angle &rhs)const { return angle::dist(rhs); }
	value_type get()const { return static_cast<value_type>(std::sin(v)); }
#ifndef ETL_NO_DEPRECATED
	// operator value_type()const ETL_DEPRECATED_FUNCTION;
#endif
}; // END of class angle::sin
// inline angle::sin::operator angle::value_type()const { return get(); }

// ========================================================================
/*!	\class	angle::cos	_angle.h	ETL/angle
**	\brief	Angle representation as a cosine function
**	\see angle
**	\writeme
*/
class angle::cos : public angle
{
public:
	explicit cos(const value_type &x)	{ v=(value_type)(std::acos(x)); }
	cos(const angle &a):angle(a) { }
	cos	mod()const { return angle::mod(); }
	cos dist(const angle &rhs)const { return angle::dist(rhs); }
	value_type get()const { return (value_type)std::cos(v); }
#ifndef ETL_NO_DEPRECATED
	// operator value_type()const ETL_DEPRECATED_FUNCTION;
#endif
}; // END of class angle::cos
// inline angle::cos::operator angle::value_type()const { return get(); }

// ========================================================================
/*!	\class	angle::tan	_angle.h	ETL/angle
**	\brief	Angle representation as a tangent function
**	\see angle
**	\writeme
*/
class angle::tan : public angle
{
public:
	explicit tan(const value_type &x)	{ v=(value_type)(std::atan(x)); }
	tan(const value_type &y,const value_type &x) { v=(value_type)(std::atan2(y,x)); }
	tan(const angle &a):angle(a) { }
	tan	mod()const { return angle::mod(); }
	tan dist(const angle &rhs)const { return angle::dist(rhs); }
	value_type get()const { return (value_type)std::tan(v); }
#ifndef ETL_NO_DEPRECATED
	// operator value_type()const ETL_DEPRECATED_FUNCTION;
#endif
}; // END of class angle::tan
// inline angle::tan::operator angle::value_type()const { return get(); }

};

//#include <iostream>

template <typename T>
struct affine_combo<etl::angle, T>
{
	typedef T time_type;

	//affine_combo() { std::cerr<<"affine_combo<etl::angle,float>: I was created!"<<std::endl; }
	//~affine_combo() { std::cerr<<"affine_combo<etl::angle,float>: I was DELETED!"<<std::endl; }

	etl::angle operator()(const etl::angle &a,const etl::angle &b,const time_type &t)const
	{
		return b.dist(a)*(float)t+a;
	}

	etl::angle reverse(const etl::angle &x, const etl::angle &b, const time_type &t)const
	{
		return x.dist(b*(float)t)*(float)(time_type(1)/(time_type(1)-t));
	}
};

template <>
struct distance_func<etl::angle> : public std::binary_function<etl::angle, etl::angle, etl::angle>
{
	etl::angle operator()(const etl::angle &a,const etl::angle &b)const
	{
		etl::angle delta=b.dist(a);
		//if(delta<etl::angle::zero())
		//	return delta+etl::angle::one();
		return delta;
	}

	etl::angle cook(const etl::angle &x)const { return x; }
	etl::angle uncook(const etl::angle &x)const { return x; }
};

/* === E N D =============================================================== */

#endif
