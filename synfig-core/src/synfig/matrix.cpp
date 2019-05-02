/* === S Y N F I G ========================================================= */
/*!	\file matrix.cpp
**	\brief Template File
**
**	$Id$
**
**	\legal
**  Copyright (c) 2008 Carlos López
**  Copyright (c) 2008 Chris Moore
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

/* === H E A D E R S ======================================================= */

#ifdef USING_PCH
#	include "pch.h"
#else
#ifdef HAVE_CONFIG_H
#	include <config.h>
#endif

#include "matrix.h"


#endif

/* === U S I N G =========================================================== */

using namespace std;
using namespace etl;
using namespace synfig;

/* === M A C R O S ========================================================= */

/* === G L O B A L S ======================================================= */

/* === M E T H O D S ======================================================= */

// Matrix2

Matrix2 &
Matrix2::set_scale(const value_type &sx, const value_type &sy)
{
	m00=sx;  m01=0.0;
	m10=0.0; m11=sy;
	return *this;
}

Matrix2 &
Matrix2::set_rotate(const Angle &a)
{
	value_type c(Angle::cos(a).get());
	value_type s(Angle::sin(a).get());
	m00= c; m01=s;
	m10=-s; m11=c;
	return *this;
}

void
Matrix2::get_transformed(value_type &out_x, value_type &out_y, const value_type x, const value_type y)const
	{ out_x = x*m00+y*m10; out_y = x*m01+y*m11; }

Matrix2
Matrix2::operator*(const Matrix2 &rhs) const
{
	return Matrix2( *this * rhs.row_x(),
					*this * rhs.row_y() );
}

bool
Matrix2::operator==(const Matrix2 &rhs) const
{
	return approximate_equal(m00, rhs.m00)
		&& approximate_equal(m01, rhs.m01)
		&& approximate_equal(m10, rhs.m10)
		&& approximate_equal(m11, rhs.m11);
}

Matrix2&
Matrix2::operator*=(const value_type &rhs)
{
	m00*=rhs;
	m01*=rhs;

	m10*=rhs;
	m11*=rhs;

	return *this;
}

Matrix2&
Matrix2::operator+=(const Matrix2 &rhs)
{
	m00+=rhs.m00;
	m01+=rhs.m01;

	m10+=rhs.m10;
	m11+=rhs.m11;

	return *this;
}

bool
Matrix2::is_invertible()const
	{ return approximate_not_equal(m00*m11, m01*m10); }

Matrix2&
Matrix2::invert()
{
	value_type det(m00*m11 - m01*m10);
	if (approximate_not_equal(det, 0.0))
	{
		value_type k = 1.0/det;
		std::swap(m00, m11);
		std::swap(m01, m10);
		m00 *= k; m01 *= -k;
		m11 *= k; m10 *= -k;
	}
	else
	if (m00*m00 + m01*m01 > m10*m10 + m11*m11)
	{
		m10 = m01; m01 = 0; m11 = 0;
	}
	else
	{
		m01 = m10; m00 = 0; m10 = 0;
	}
	return *this;
}

String
Matrix2::get_string(int spaces, String before, String after)const
{
	return etl::strprintf("%*s [%7.2f %7.2f] %s\n%*s [%7.2f %7.2f]\n",
						  spaces, before.c_str(), m00, m01, after.c_str(),
						  spaces, "",			  m10, m11);
}

// Matrix3

Matrix3&
Matrix3::set_scale(const value_type &sx, const value_type &sy)
{
	m00=sx;  m01=0.0; m02=0.0;
	m10=0.0; m11=sy;  m12=0.0;
	m20=0.0; m21=0.0; m22=1.0;
	return *this;
}

Matrix3&
Matrix3::set_rotate(const Angle &a)
{
	value_type c(Angle::cos(a).get());
	value_type s(Angle::sin(a).get());
	m00= c;   m01=s;   m02=0.0;
	m10=-s;   m11=c;   m12=0.0;
	m20= 0.0; m21=0.0; m22=1.0;
	return *this;
}

Matrix3&
Matrix3::set_translate(value_type x, value_type y)
{
	m00=1.0; m01=0.0; m02=0.0;
	m10=0.0; m11=1.0; m12=0.0;
	m20=x  ; m21=y  ; m22=1.0;
	return *this;
}

void
Matrix3::get_transformed(
	value_type &out_x, value_type &out_y, value_type &out_z,
	const value_type x, const value_type y, const value_type z )const
{
	out_x = x*m00 + y*m10 + z*m20;
	out_y = x*m01 + y*m11 + z*m21;
	out_z = x*m02 + y*m12 + z*m22;
}

bool
Matrix3::operator==(const Matrix3 &rhs) const
{
	return approximate_equal(m00, rhs.m00)
		&& approximate_equal(m01, rhs.m01)
		&& approximate_equal(m02, rhs.m02)
		&& approximate_equal(m10, rhs.m10)
		&& approximate_equal(m11, rhs.m11)
		&& approximate_equal(m12, rhs.m12)
		&& approximate_equal(m20, rhs.m20)
		&& approximate_equal(m21, rhs.m21)
		&& approximate_equal(m22, rhs.m22);
}

Matrix3
Matrix3::operator*(const Matrix3 &rhs) const
{
	return Matrix3( *this * rhs.row_x(),
			        *this * rhs.row_y(),
					*this * rhs.row_z() );
}

Matrix3&
Matrix3::operator*=(const value_type &rhs)
{
	m00*=rhs;
	m01*=rhs;
	m02*=rhs;

	m10*=rhs;
	m11*=rhs;
	m12*=rhs;

	m20*=rhs;
	m21*=rhs;
	m22*=rhs;

	return *this;
}

Matrix3&
Matrix3::operator+=(const Matrix3 &rhs)
{
	m00+=rhs.m00;
	m01+=rhs.m01;
	m02+=rhs.m02;

	m10+=rhs.m10;
	m11+=rhs.m11;
	m12+=rhs.m12;

	m20+=rhs.m20;
	m21+=rhs.m21;
	m22+=rhs.m22;

	return *this;
}

Matrix3::value_type
Matrix3::det()const
{
	return m00*(m11*m22 - m12*m21)
		 - m01*(m10*m22 - m12*m20)
		 + m02*(m10*m21 - m11*m20);
}


bool
Matrix3::is_invertible()const
	{ return approximate_not_equal(det(), 0.0); }

Matrix3
Matrix3::get_inverted()const
{
	value_type d = det();
	if (approximate_equal(d, value_type(0))) {
		// result of transformation is not 3d
		// all points always transforms into one plane (2d), or line (1d), or dot (0d)

		// we cannot do the real back transform, but we will try
		// to make valid matrix for X-axis only or for Y-axis only

		// try to make back transform for X-axis
		if ( approximate_equal(row_x()[2], value_type(0))
		  && row_y().is_equal_to(Vector3::zero())
		  && approximate_equal(row_z()[2], value_type(1)) )
		{
			d = axis_x().mag_squared();
			if (d > real_precision<value_type>()) {
				d = 1.0/d;
				return Matrix3(
				    d*m00,                 0.0, 0.0,
				    d*m01,                 0.0, 0.0,
				   -d*(m20*m00 + m21*m01), 0.0, 1.0 );
			}
		}

		// try to make back transform for Y-axis
		if ( row_x().is_equal_to(Vector3::zero())
		  && approximate_equal(row_y()[2], value_type(0))
		  && approximate_equal(row_z()[2], value_type(1)) )
		{
			d = axis_y().mag_squared();
			if (d > real_precision<value_type>()) {
				d = 1.0/d;
				return Matrix3(
				    0.0,  d*m10,                 0.0,
					0.0,  d*m11,                 0.0,
					0.0, -d*(m20*m10 + m21*m11), 1.0 );
			}
		}

		// give up
		return Matrix3( 0.0, 0.0, 0.0,
				        0.0, 0.0, 0.0,
						0.0, 0.0, 0.0 );
	}

	// proper inversion
	value_type p = 1.0/d;
	value_type m = -p;
	return Matrix3(
		p*(m11*m22 - m12*m21), // row0
		m*(m01*m22 - m02*m21),
		p*(m01*m12 - m02*m11),
		m*(m10*m22 - m12*m20), // row1
		p*(m00*m22 - m02*m20),
		m*(m00*m12 - m02*m10),
		p*(m10*m21 - m11*m20), // row2
		m*(m00*m21 - m01*m20),
		p*(m00*m11 - m01*m10) );
}

String
Matrix3::get_string(int spaces, String before, String after)const
{
	return etl::strprintf("%*s [%7.2f %7.2f %7.2f] %s\n%*s [%7.2f %7.2f %7.2f]\n%*s [%7.2f %7.2f %7.2f]\n",
						  spaces, before.c_str(), m00, m01, m02, after.c_str(),
						  spaces, "",			  m10, m11, m12,
						  spaces, "",			  m20, m21, m22);
}
