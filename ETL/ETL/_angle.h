/* ========================================================================
** Extended Template and Library
** \file _angle.h
** \brief Angle Abstraction Class Implementation
** \internal
**
** \legal
** Copyright (c) 2002 Robert B. Quattlebaum Jr.
** Copyright (c) 2007 Chris Moore
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

#ifndef __ETL__ANGLE_H
#define __ETL__ANGLE_H

/* === H E A D E R S ======================================================= */

#include <cmath>

/* === M A C R O S ========================================================= */

#ifndef PI
# define PI (3.1415926535897932384626433832795029L)
#endif

/* === T Y P E D E F S ===================================================== */

/* === C L A S S E S & S T R U C T S ======================================= */

namespace etl {

/**
 * Abstraction of the concept of an angle
 * @see angle::deg, angle::rad, angle::rot, angle::sin, angle::cos, angle::tan
 */
class angle
{
public:
	typedef float value_type;

protected:
	typedef value_type unit;

	unit v;	//*< Stored in radians; positive values indicate counter-clockwise. */

public:

	angle&
	operator+=(const angle &rhs)
	{ v+=rhs.v; return *this; }

	angle&
	operator-=(const angle &rhs)
	{ v-=rhs.v; return *this; }

	angle&
	operator*=(const unit &rhs)
	{ v*=rhs; return *this; }

	angle&
	operator/=(const unit &rhs)
	{ v/=rhs; return *this; }

	angle
	operator+(const angle &rhs)const
	{ return angle(*this)+=rhs; }

	/**
	 * Angle Subtraction Operator
	 * @see angle dist(const angle &)
	 */
	angle
	operator-(const angle &rhs)const
	{ return angle(*this)-=rhs; }

	angle
	operator*(const unit &rhs)const
	{ return angle(*this)*=rhs; }

	angle
	operator/(const unit &rhs)const
	{ return angle(*this)/=rhs; }

	/** Angle Negation */
	angle
	operator-()const
	{
		angle ret;
		ret.v=-v;
		return ret;
	}

	bool
	operator<(const angle &rhs)const
	{ return v < rhs.v; }

	bool
	operator>(const angle &rhs)const
	{ return v > rhs.v; }

	bool
	operator<=(const angle &rhs)const
	{ return v <= rhs.v; }

	bool
	operator>=(const angle &rhs)const
	{ return v >= rhs.v; }

	bool
	operator==(const angle &rhs)const
	{ return std::abs(v - rhs.v)<ANGLE_EPSILON; }

	bool
	operator!=(const angle &rhs)const
		{ return std::abs(v - rhs.v)>ANGLE_EPSILON; }

	/**
	 * Absolute Angle Function.
	 * This function will return the absolute value of the angle
	 */
	angle
	abs()const
	{
		angle ret;
		ret.v=std::abs(v);
		return ret;
	}

	/**
	 * Angle Difference Function.
	 * This function will return the difference between
	 * two angles, just like @see angle operator-(const angle &)
	 *
	 * It was originally intended to compute the shortest arc angle
	 * between this angle and \c rhs in the interval [-PI/2, PI/2)
	 */
	angle
	dist(const angle &rhs)const
	{ return angle(*this)-=rhs; }

	/**
	 * Rotation Modulus.
	 * This function will return the value of the angle
	 *
	 * It was originally intended to return the value of the angle
	 * in the interval [0, PI/2)
	 */
	angle
	mod()const
	{
		angle ret(*this);
		return ret;
	}

	/** Zero Rotation (0 degrees) */
	static angle
	zero()
	{
		angle ret;
		ret.v=0;
		return ret;
	}

	/** One Complete Rotation (360 degrees) */
	static angle
	one()
	{
		angle ret;
		ret.v=PI*2;
		return ret;
	}

	/** One Half Rotation (180 degrees) */
	static angle
	half()
	{
		angle ret;
		ret.v=PI;
		return ret;
	}

	bool operator!()const { return std::abs(mod().v) < ANGLE_EPSILON; }

private:

	static constexpr value_type ANGLE_EPSILON = 1.0e-6;

public:
	// Conversion Classes

	class rad;
	class deg;
	class rot;

	// Trigonometric Classes

	class sin;
	class cos;
	class tan;

	// Friend classes

	friend class rad;
	friend class deg;
	friend class rot;
	friend class sin;
	friend class cos;
	friend class tan;

}; // END of class angle

/**	
 * Angle representation in radians
 * @see angle
*/
class angle::rad : public angle
{
public:
	explicit rad(const value_type &x) { v=x; }
	rad(const angle &a):angle(a) { }
	rad	mod()const { return angle::mod(); }
	rad dist(const angle &rhs)const { return angle::dist(rhs); }
	value_type get()const { return v; }
}; // END of class angle::radians

/**	
 * Angle representation in degrees
 * @see angle
*/
class angle::deg : public angle
{
public:
	explicit deg(const value_type &x) { v=x*((PI*2)/360); }
	deg(const angle &a):angle(a) { }
	deg	mod()const { return angle::mod(); }
	deg dist(const angle &rhs)const { return angle::dist(rhs); }
	value_type get()const { return v*360/(PI*2); }
}; // END of class angle::degrees

/**	
 * Angle representation in rotations
 * @see angle
*/
class angle::rot : public angle
{
public:
	explicit rot(const value_type &x) { v=x*(PI*2); }
	rot(const angle &a):angle(a) { }
	rot mod()const { return angle::mod(); }
	rot dist(const angle &rhs)const { return angle::dist(rhs); }
	value_type get()const { return v/(PI*2); }
}; // END of class angle::rotations

/**	
 * Angle representation as a sine function
 * @see angle
*/
class angle::sin : public angle
{
public:
	explicit sin(const value_type &x) { v=static_cast<value_type>(std::asin(x)); }
	sin(const angle &a):angle(a) { }
	sin	mod()const { return angle::mod(); }
	sin dist(const angle &rhs)const { return angle::dist(rhs); }
	value_type get()const { return static_cast<value_type>(std::sin(v)); }
}; // END of class angle::sin

/**	
 * Angle representation as a cosine function
 * @see angle
*/
class angle::cos : public angle
{
public:
	explicit cos(const value_type &x)	{ v=(value_type)(std::acos(x)); }
	cos(const angle &a):angle(a) { }
	cos	mod()const { return angle::mod(); }
	cos dist(const angle &rhs)const { return angle::dist(rhs); }
	value_type get()const { return (value_type)std::cos(v); }
}; // END of class angle::cos

/**	
 * Angle representation as a tangent function
 * @see angle
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
}; // END of class angle::tan

};

template <typename T>
struct affine_combo<etl::angle, T>
{
	typedef T time_type;

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
struct distance_func<etl::angle>
{
	etl::angle operator()(const etl::angle &a,const etl::angle &b)const
	{
		etl::angle delta=b.dist(a);
		return delta;
	}

	etl::angle cook(const etl::angle &x)const { return x; }
	etl::angle uncook(const etl::angle &x)const { return x; }
};

/* === E N D =============================================================== */

#endif
