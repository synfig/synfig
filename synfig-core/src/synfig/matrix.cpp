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

// For some reason isnan() isn't working on macosx any more.
// This is a quick fix.
#if defined(__APPLE__) && !defined(SYNFIG_ISNAN_FIX)
#ifdef isnan
#undef isnan
#endif
inline bool isnan(double x) { return x != x; }
inline bool isnan(float x) { return x != x; }
#define SYNFIG_ISNAN_FIX 1
#endif


#endif

/* === U S I N G =========================================================== */

using namespace std;
using namespace etl;
using namespace synfig;

/* === M A C R O S ========================================================= */

#define epsilon 1e-6

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
	m00= c;     m01=s;
	m10=-1.0*s; m11=c;
	return *this;
}

void
Matrix2::get_transformed(value_type &out_x, value_type &out_y, const value_type x, const value_type y)const
	{ out_x = x*m00+y*m10; out_y = x*m01+y*m11; }

Matrix2
Matrix2::operator*=(const Matrix2 &rhs)
{
	value_type x, y;

	x = m00;    y = m01;
	m00=x*rhs.m00 + y*rhs.m10;
	m01=x*rhs.m01 + y*rhs.m11;

	x = m10;    y = m11;;
	m10=x*rhs.m00 + y*rhs.m10;
	m11=x*rhs.m01 + y*rhs.m11;

	return *this;
}

bool
Matrix2::operator==(const Matrix2 &rhs) const
{
	return fabs(m00 - rhs.m00) < epsilon && fabs(m01 - rhs.m01) < epsilon
		&& fabs(m10 - rhs.m10) < epsilon && fabs(m11 - rhs.m11) < epsilon;
}

Matrix2
Matrix2::operator*=(const value_type &rhs)
{
	m00*=rhs;
	m01*=rhs;

	m10*=rhs;
	m11*=rhs;

	return *this;
}

Matrix2
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
	{ return abs(m00*m11 - m01*m10) > epsilon; }

Matrix2&
Matrix2::invert()
{
	value_type det(m00*m11 - m01*m10);
	if (fabs(det) > epsilon)
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

Matrix3 &
Matrix3::set_scale(const value_type &sx, const value_type &sy)
{
	m00=sx;  m01=0.0; m02=0.0;
	m10=0.0; m11=sy;  m12=0.0;
	m20=0.0; m21=0.0; m22=1.0;
	return *this;
}

Matrix3 &
Matrix3::set_rotate(const Angle &a)
{
	value_type c(Angle::cos(a).get());
	value_type s(Angle::sin(a).get());
	m00= c;     m01=s;    m02=0.0;
	m10=-1.0*s; m11=c;    m12=0.0;
	m20=0.0;    m21=0.0;  m22=1.0;
	return *this;
}

Matrix3 &
Matrix3::set_translate(value_type x, value_type y)
{
	m00=1.0; m01=0.0; m02=0.0;
	m10=0.0; m11=1.0; m12=0.0;
	m20=x  ; m21=y  ; m22=1.0;
	return *this;
}

void
Matrix3::get_transformed(value_type &out_x, value_type &out_y, const value_type x, const value_type y, bool translate)const
{
	if (translate)
	{
		out_x = x*m00+y*m10+m20;
		out_y = x*m01+y*m11+m21;
	}
	else
	{
		out_x = x*m00+y*m10;
		out_y = x*m01+y*m11;
	}
}

bool
Matrix3::operator==(const Matrix3 &rhs) const
{
	return fabs(m00 - rhs.m00) < epsilon && fabs(m01 - rhs.m01) < epsilon && fabs(m02 - rhs.m02) < epsilon
		&& fabs(m10 - rhs.m10) < epsilon && fabs(m11 - rhs.m11) < epsilon && fabs(m12 - rhs.m12) < epsilon
		&& fabs(m20 - rhs.m20) < epsilon && fabs(m21 - rhs.m21) < epsilon && fabs(m22 - rhs.m22) < epsilon;
}

Matrix3
Matrix3::operator*=(const Matrix3 &rhs)
{
	value_type x, y, z;

	x = m00;    y = m01;    z = m02;
	m00=x*rhs.m00 + y*rhs.m10 + z*rhs.m20;
	m01=x*rhs.m01 + y*rhs.m11 + z*rhs.m21;
	m02=x*rhs.m02 + y*rhs.m12 + z*rhs.m22;

	x = m10;    y = m11;    z = m12;
	m10=x*rhs.m00 + y*rhs.m10 + z*rhs.m20;
	m11=x*rhs.m01 + y*rhs.m11 + z*rhs.m21;
	m12=x*rhs.m02 + y*rhs.m12 + z*rhs.m22;

	x = m20;    y = m21;    z = m22;
	m20=x*rhs.m00 + y*rhs.m10 + z*rhs.m20;
	m21=x*rhs.m01 + y*rhs.m11 + z*rhs.m21;
	m22=x*rhs.m02 + y*rhs.m12 + z*rhs.m22;

	return *this;
}

Matrix3
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

Matrix3
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

bool
Matrix3::is_invertible()const
	{ return abs(m00*m11 - m01*m10) > epsilon; }

Matrix3&
Matrix3::invert()
{
	if (is_invertible())
	{
		value_type det(m00*m11-m01*m10);
		value_type tmp(m20);
		m20=(m10*m21-m11*m20)/det;
		m21=(m01*tmp-m00*m21)/det;
		tmp=m00;
		m00=m11/det;
		m11=tmp/det;
		m01=-m01/det;
		m10=-m10/det;
	}
	else
	if (m00*m00+m01*m01 > m10*m10+m11*m11)
	{
		m10=m01; m20=-m20*m00-m21*m01;
		m01=0; m11=0; m21=0;
	}
	else
	{
		m01=m10; m21=-m20*m10-m21*m11;
		m00=0; m10=0; m20=0;
	}
	return *this;
}

String
Matrix3::get_string(int spaces, String before, String after)const
{
	return etl::strprintf("%*s [%7.2f %7.2f %7.2f] %s\n%*s [%7.2f %7.2f %7.2f]\n%*s [%7.2f %7.2f %7.2f]\n",
						  spaces, before.c_str(), m00, m01, m02, after.c_str(),
						  spaces, "",			  m10, m11, m12,
						  spaces, "",			  m20, m21, m22);
}
