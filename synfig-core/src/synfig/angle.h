/* === S Y N F I G ========================================================= */
/*!	\file angle.h
**	\brief Template Header
**
**	\legal
**	Copyright (c) 2002-2005 Robert B. Quattlebaum Jr., Adrian Bentley
**	Copyright (c) 2007 Chris Moore
**
**	This file is part of Synfig.
**
**	Synfig is free software: you can redistribute it and/or modify
**	it under the terms of the GNU General Public License as published by
**	the Free Software Foundation, either version 2 of the License, or
**	(at your option) any later version.
**
**	Synfig is distributed in the hope that it will be useful,
**	but WITHOUT ANY WARRANTY; without even the implied warranty of
**	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
**	GNU General Public License for more details.
**
**	You should have received a copy of the GNU General Public License
**	along with Synfig.  If not, see <https://www.gnu.org/licenses/>.
**	\endlegal
*/
/* ========================================================================= */

/* === S T A R T =========================================================== */

#ifndef SYNFIG_ANGLE_H
#define SYNFIG_ANGLE_H

/* === H E A D E R S ======================================================= */

#include <cmath>

#include <synfig/_curve_func.h>

/* === M A C R O S ========================================================= */

#ifndef PI
# define PI (3.1415926535897932384626433832795029L)
#endif

/* === T Y P E D E F S ===================================================== */

/* === C L A S S E S & S T R U C T S ======================================= */

namespace synfig {

/**
 * Abstraction of the concept of an angle
 * @see Angle::deg, Angle::rad, Angle::rot, Angle::sin, Angle::cos, Angle::tan
 */
class Angle
{
public:
	typedef float value_type;

protected:
	typedef value_type unit;

	unit v;	//*< Stored in radians; positive values indicate counter-clockwise. */

public:

	Angle&
	operator+=(const Angle &rhs)
	{ v+=rhs.v; return *this; }

	Angle&
	operator-=(const Angle &rhs)
	{ v-=rhs.v; return *this; }

	Angle&
	operator*=(const unit &rhs)
	{ v*=rhs; return *this; }

	Angle&
	operator/=(const unit &rhs)
	{ v/=rhs; return *this; }

	Angle
	operator+(const Angle &rhs)const
	{ return Angle(*this)+=rhs; }

	/**
	 * Angle Subtraction Operator
	 * @see Angle dist(const Angle &)
	 */
	Angle
	operator-(const Angle &rhs)const
	{ return Angle(*this)-=rhs; }

	Angle
	operator*(const unit &rhs)const
	{ return Angle(*this)*=rhs; }

	Angle
	operator/(const unit &rhs)const
	{ return Angle(*this)/=rhs; }

	/** Angle Negation */
	Angle
	operator-()const
	{
		Angle ret;
		ret.v=-v;
		return ret;
	}

	bool
	operator<(const Angle &rhs)const
	{ return v < rhs.v; }

	bool
	operator>(const Angle &rhs)const
	{ return v > rhs.v; }

	bool
	operator<=(const Angle &rhs)const
	{ return v <= rhs.v; }

	bool
	operator>=(const Angle &rhs)const
	{ return v >= rhs.v; }

	bool
	operator==(const Angle &rhs)const
	{ return std::abs(v - rhs.v)<ANGLE_EPSILON; }

	bool
	operator!=(const Angle &rhs)const
		{ return std::abs(v - rhs.v)>ANGLE_EPSILON; }

	/**
	 * Absolute Angle Function.
	 * This function will return the absolute value of the angle
	 */
	Angle
	abs()const
	{
		Angle ret;
		ret.v=std::abs(v);
		return ret;
	}

	/**
	 * Angle Difference Function.
	 * This function will return the difference between
	 * two angles, just like @see Angle operator-(const Angle &)
	 *
	 * It was originally intended to compute the shortest arc angle
	 * between this angle and \c rhs in the interval [-PI/2, PI/2)
	 */
	Angle
	dist(const Angle &rhs)const
	{ return Angle(*this)-=rhs; }

	/**
	 * Rotation Modulus.
	 * This function will return the value of the angle
	 *
	 * It was originally intended to return the value of the angle
	 * in the interval [0, PI/2)
	 */
	Angle
	mod()const
	{
		Angle ret(*this);
		return ret;
	}

	/** Zero Rotation (0 degrees) */
	static Angle
	zero()
	{
		Angle ret;
		ret.v=0;
		return ret;
	}

	/** One Complete Rotation (360 degrees) */
	static Angle
	one()
	{
		Angle ret;
		ret.v=PI*2;
		return ret;
	}

	/** One Half Rotation (180 degrees) */
	static Angle
	half()
	{
		Angle ret;
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

}; // END of class Angle

/**	
 * Angle representation in radians
 * @see Angle
*/
class Angle::rad : public Angle
{
public:
	explicit rad(const value_type &x) { v=x; }
	rad(const Angle &a):Angle(a) { }
	rad	mod()const { return Angle::mod(); }
	rad dist(const Angle &rhs)const { return Angle::dist(rhs); }
	value_type get()const { return v; }
}; // END of class Angle::radians

/**	
 * Angle representation in degrees
 * @see Angle
*/
class Angle::deg : public Angle
{
public:
	explicit deg(const value_type &x) { v=x*((PI*2)/360); }
	deg(const Angle &a):Angle(a) { }
	deg	mod()const { return Angle::mod(); }
	deg dist(const Angle &rhs)const { return Angle::dist(rhs); }
	value_type get()const { return v*360/(PI*2); }
}; // END of class Angle::degrees

/**	
 * Angle representation in rotations
 * @see Angle
*/
class Angle::rot : public Angle
{
public:
	explicit rot(const value_type &x) { v=x*(PI*2); }
	rot(const Angle &a):Angle(a) { }
	rot mod()const { return Angle::mod(); }
	rot dist(const Angle &rhs)const { return Angle::dist(rhs); }
	value_type get()const { return v/(PI*2); }
}; // END of class Angle::rotations

/**	
 * Angle representation as a sine function
 * @see Angle
*/
class Angle::sin : public Angle
{
public:
	explicit sin(const value_type &x) { v=static_cast<value_type>(std::asin(x)); }
	sin(const Angle &a):Angle(a) { }
	sin	mod()const { return Angle::mod(); }
	sin dist(const Angle &rhs)const { return Angle::dist(rhs); }
	value_type get()const { return static_cast<value_type>(std::sin(v)); }
}; // END of class Angle::sin

/**	
 * Angle representation as a cosine function
 * @see Angle
*/
class Angle::cos : public Angle
{
public:
	explicit cos(const value_type &x)	{ v=(value_type)(std::acos(x)); }
	cos(const Angle &a):Angle(a) { }
	cos	mod()const { return Angle::mod(); }
	cos dist(const Angle &rhs)const { return Angle::dist(rhs); }
	value_type get()const { return (value_type)std::cos(v); }
}; // END of class Angle::cos

/**	
 * Angle representation as a tangent function
 * @see Angle
*/
class Angle::tan : public Angle
{
public:
	explicit tan(const value_type &x)	{ v=(value_type)(std::atan(x)); }
	tan(const value_type &y,const value_type &x) { v=(value_type)(std::atan2(y,x)); }
	tan(const Angle &a):Angle(a) { }
	tan	mod()const { return Angle::mod(); }
	tan dist(const Angle &rhs)const { return Angle::dist(rhs); }
	value_type get()const { return (value_type)std::tan(v); }
}; // END of class Angle::tan

}; // END of namespace synfig

template <typename T>
struct affine_combo<synfig::Angle, T>
{
	typedef T time_type;

	synfig::Angle operator()(const synfig::Angle &a,const synfig::Angle &b,const time_type &t)const
	{
		return b.dist(a)*(float)t+a;
	}

	synfig::Angle reverse(const synfig::Angle &x, const synfig::Angle &b, const time_type &t)const
	{
		return x.dist(b*(float)t)*(float)(time_type(1)/(time_type(1)-t));
	}
};

template <>
struct distance_func<synfig::Angle>
{
	synfig::Angle operator()(const synfig::Angle &a,const synfig::Angle &b)const
	{
		synfig::Angle delta=b.dist(a);
		return delta;
	}

	synfig::Angle cook(const synfig::Angle &x)const { return x; }
	synfig::Angle uncook(const synfig::Angle &x)const { return x; }
};

/* === E N D =============================================================== */

#endif
