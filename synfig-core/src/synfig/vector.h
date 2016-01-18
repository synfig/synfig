/* === S Y N F I G ========================================================= */
/*!	\file vector.h
**	\brief Various discreet type definitions
**
**	$Id$
**
**	\legal
**	Copyright (c) 2002-2005 Robert B. Quattlebaum Jr., Adrian Bentley
**	Copyright (c) 2007 Chris Moore
**
**	This package is free software; you can redistribute it and/or
**	modify it under the terms of the GNU General Public License as
**	published by the Free Software Foundation; either version 2 of
**	the License, or (at your option) any later version.
**
**	This package is distributed in the hope that it will be useful,
**	but WITHOUT ANY WARRANTY; without even the implied warranty of
**	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
**	General Public License for more details.
**	\endlegal
*/
/* ========================================================================= */

/* === S T A R T =========================================================== */

#ifndef __SYNFIG_VECTOR_H
#define __SYNFIG_VECTOR_H

/* === H E A D E R S ======================================================= */

#include "angle.h"
#include "real.h"
#include <cmath>

/* === M A C R O S ========================================================= */

// For some reason isnan() isn't working on macosx any more.
// This is a quick fix.
#if defined(__APPLE__) && !defined(SYNFIG_ISNAN_FIX)
#ifdef isnan
#undef isnan
#endif
inline bool isnan(double x) { return x != x; }
inline bool isnan(float x) { return x != x; }
#define SYNFIG_ISNAN_FIX 1
#ifdef isinf
#undef isinf
#endif
inline bool isinf(double x) { return !isnan(x) && isnan(x - x); }
inline bool isinf(float x) { return !isnan(x) && isnan(x - x); }
#define SYNFIG_ISINF_FIX 1
#else
#ifndef isnan
#define isnan(x) (std::isnan)(x)
#endif
#ifndef isinf
#define isinf(x) (std::isinf)(x)
#endif
#endif


/* === T Y P E D E F S ===================================================== */

/* === C L A S S E S & S T R U C T S ======================================= */

namespace synfig {

/*!	\class VectorInt
**	\todo writeme
*/
class VectorInt
{
public:
	typedef int value_type;

protected:
	value_type _x, _y;

public:
	VectorInt(): _x(0), _y(0) { };
	VectorInt(const value_type &x, const value_type &y):_x(x),_y(y) { };

	static VectorInt zero() { return VectorInt(0, 0); }

	value_type &
	operator[](const int& i)
		{ return i ? _y: _x; }

	const value_type &
	operator[] (const int& i) const
		{ return i ? _y: _x; }

	const VectorInt &
	operator+=(const VectorInt &rhs)
	{
		_x += rhs._x;
		_y += rhs._y;
		return *this;
	}

	const VectorInt &
	operator-=(const VectorInt &rhs)
	{
		_x -= rhs._x;
		_y -= rhs._y;
		return *this;
	}

	const VectorInt &
	operator*=(const value_type &rhs)
	{
		_x *= rhs;
		_y *= rhs;
		return *this;
	}

	const VectorInt &
	operator/=(const value_type &rhs)
	{
		_x /= rhs;
		_y /= rhs;
		return *this;
	}

	VectorInt
	operator+(const VectorInt &rhs)const
		{ return VectorInt(*this) += rhs; }

	VectorInt
	operator-(const VectorInt &rhs)const
		{ return VectorInt(*this) -= rhs; }

	VectorInt
	operator*(const value_type &rhs)const
		{ return VectorInt(*this) *= rhs; }

	VectorInt
	operator/(const value_type &rhs)const
		{ return VectorInt(*this) /= rhs; }

	VectorInt
	operator-()const
		{ return VectorInt(-_x,-_y); }

	value_type
	operator*(const VectorInt &rhs)const
		{ return _x*rhs._x+_y*rhs._y; }

	bool
	operator==(const VectorInt &rhs)const
		{ return _x==rhs._x && _y==rhs._y; }

	bool
	operator!=(const VectorInt &rhs)const
		{ return _y!=rhs._y || _x!=rhs._x; }

	//! Returns the squared magnitude of the vector
	value_type mag_squared()const
		{ return _x*_x+_y*_y; }

	//! Returns the magnitude of the vector
	Real mag()const
		{ return sqrt(mag_squared()); }

	//! Returns the reciprocal of the magnitude of the vector
	Real inv_mag()const
		{ return 1.0/sqrt(mag_squared()); }

	//! Returns a perpendicular version of the vector
	VectorInt perp()const
		{ return VectorInt(_y,-_x); }

	Angle angle()const
		{ return Angle::rad(atan2(_y, _x)); }

	VectorInt multiply_coords(const VectorInt &rhs) const
		{ return VectorInt(_x*rhs._x, _y*rhs._y); }
	VectorInt divide_coords(const VectorInt &rhs) const
		{ return VectorInt(_x/rhs._x, _y/rhs._y); }

};

/*!	\typedef PointInt
**	\todo writeme
*/
typedef VectorInt PointInt;


/*!	\class Vector
**	\todo writeme
*/
class Vector
{
public:
	typedef Real value_type;

private:
	value_type _x, _y;

public:
	Vector(): _x(0.0), _y(0.0) { };
	Vector(const value_type &x, const value_type &y):_x(x),_y(y) { };
	Vector(const value_type &radius, const Angle &angle):
	_x(radius*Angle::cos(angle).get()),
	_y(radius*Angle::sin(angle).get())
	{ };

	bool is_valid()const { return !(isnan(_x) || isnan(_y)); }
	bool is_nan_or_inf()const { return isnan(_x) || isnan(_y) || isinf(_x) || isinf(_y); }

	value_type &
	operator[](const int& i)
	{ return i?_y:_x; }

	const value_type &
	operator[](const int& i) const
	{ return i?_y:_x; }

	const Vector &
	operator+=(const Vector &rhs)
	{
		_x+=rhs._x;
		_y+=rhs._y;
		return *this;
	}

	const Vector &
	operator-=(const Vector &rhs)
	{
		_x-=rhs._x;
		_y-=rhs._y;
		return *this;
	}

	const Vector &
	operator*=(const value_type &rhs)
	{
		_x*=rhs;
		_y*=rhs;
		return *this;
	}

	const Vector &
	operator/=(const value_type &rhs)
	{
		value_type tmp=1.0/rhs;
		_x*=tmp;
		_y*=tmp;
		return *this;
	}

	Vector
	operator+(const Vector &rhs)const
		{ return Vector(*this)+=rhs; }

	Vector
	operator-(const Vector &rhs)const
		{ return Vector(*this)-=rhs; }

	Vector
	operator*(const value_type &rhs)const
		{ return Vector(*this)*=rhs; }

	Vector
	operator/(const value_type &rhs)const
		{ return Vector(*this)/=rhs; }

	Vector
	operator-()const
		{ return Vector(-_x,-_y); }

	value_type
	operator*(const Vector &rhs)const
		{ return _x*rhs._x+_y*rhs._y; }

	bool
	operator==(const Vector &rhs)const
		{ return _x==rhs._x && _y==rhs._y; }

	bool
	operator!=(const Vector &rhs)const
		{ return _y!=rhs._y || _x!=rhs._x; }

	//! Returns the squared magnitude of the vector
	value_type mag_squared()const
		{ return _x*_x+_y*_y; }

	//! Returns the magnitude of the vector
	value_type mag()const
		{ return sqrt(mag_squared()); }

	//! Returns the reciprocal of the magnitude of the vector
	value_type inv_mag()const
		{ return 1.0/sqrt(mag_squared()); }

	//! Returns a normalized version of the vector
	Vector norm()const
		{ return (*this)*inv_mag(); }

	//! Returns a perpendicular version of the vector
	Vector perp()const
		{ return Vector(_y,-_x); }

	Angle angle()const
		{ return Angle::rad(atan2(_y, _x)); }

	bool is_equal_to(const Vector& rhs)const
	{
		static const value_type epsilon(0.0000000000001);
//		return (_x>rhs._x)?_x-rhs._x<=epsilon:rhs._x-_x<=epsilon && (_y>rhs._y)?_y-rhs._y<=epsilon:rhs._y-_y<=epsilon;
		return (*this-rhs).mag_squared()<=epsilon;
	}

	static Vector zero() { return Vector(0,0); }

	Vector multiply_coords(const Vector &rhs) const
		{ return Vector(_x*rhs._x, _y*rhs._y); }
	Vector divide_coords(const Vector &rhs) const
		{ return Vector(_x/rhs._x, _y/rhs._y); }
	Vector one_divide_coords() const
		{ return Vector(1.0/_x, 1.0/_y); }
	Vector rotate(const Angle &rhs) const
	{
		value_type s = Angle::sin(rhs).get();
		value_type c = Angle::cos(rhs).get();
		return Vector(c*_x - s*_y, s*_x + c*_y);
	}
};

/*!	\typedef Point
**	\todo writeme
*/
typedef Vector Point;



}; // END of namespace synfig

namespace std {

inline synfig::Vector::value_type
abs(const synfig::Vector &rhs)
	{ return rhs.mag(); }

}; // END of namespace std

#include <ETL/bezier>

_ETL_BEGIN_NAMESPACE

template <>
class bezier_base<synfig::Vector,float> : public std::unary_function<float,synfig::Vector>
{
public:
	typedef synfig::Vector value_type;
	typedef float time_type;
private:

	bezier_base<synfig::Vector::value_type,time_type> bezier_x,bezier_y;

	value_type a,b,c,d;

protected:
	affine_combo<value_type,time_type> affine_func;

public:
	bezier_base() { }
	bezier_base(
		const value_type &a, const value_type &b, const value_type &c, const value_type &d,
		const time_type &r=0.0, const time_type &s=1.0):
		a(a),b(b),c(c),d(d) { set_rs(r,s); sync(); }

	void sync()
	{
		bezier_x[0]=a[0],bezier_y[0]=a[1];
		bezier_x[1]=b[0],bezier_y[1]=b[1];
		bezier_x[2]=c[0],bezier_y[2]=c[1];
		bezier_x[3]=d[0],bezier_y[3]=d[1];
		bezier_x.sync();
		bezier_y.sync();
	}

	value_type
	operator()(time_type t)const
	{
		return synfig::Vector(bezier_x(t),bezier_y(t));
	}

	void evaluate(time_type t, value_type &f, value_type &df) const
	{
		t=(t-get_r())/get_dt();

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

	void set_rs(time_type new_r, time_type new_s) { bezier_x.set_rs(new_r,new_s); bezier_y.set_rs(new_r,new_s); }
	void set_r(time_type new_r) { bezier_x.set_r(new_r); bezier_y.set_r(new_r); }
	void set_s(time_type new_s) { bezier_x.set_s(new_s); bezier_y.set_s(new_s); }
	const time_type &get_r()const { return bezier_x.get_r(); }
	const time_type &get_s()const { return bezier_x.get_s(); }
	time_type get_dt()const { return bezier_x.get_dt(); }

	value_type &
	operator[](int i)
	{ return (&a)[i]; }

	const value_type &
	operator[](int i) const
	{ return (&a)[i]; }

	//! Bezier curve intersection function
	time_type intersect(const bezier_base<value_type,time_type> &/*x*/, time_type /*near*/=0.0)const
	{
		return 0;
	}
};

_ETL_END_NAMESPACE


/* === E N D =============================================================== */

#endif
