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


#ifdef WIN32
#include <float.h>
#ifndef isnan
extern "C" { int _isnan(double x); }
#define isnan _isnan
#endif
#endif

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

Matrix::Matrix()
{
	set_identity();
}

Matrix &
Matrix::set_identity()
{
	m00=1.0; m01=0.0; m02=0.0;
	m10=0.0; m11=1.0; m12=0.0;
	m20=0.0; m21=0.0; m22=1.0;
	return (*this);
}

Matrix &
Matrix::set_scale(const value_type &sx, const value_type &sy)
{
	m00=sx;  m01=0.0; m02=0.0;
	m10=0.0; m11=sy;  m12=0.0;
	m20=0.0; m21=0.0; m22=1.0;
	return (*this);
}

Matrix &
Matrix::set_scale(const value_type &sxy)
{
	return set_scale(sxy, sxy);
}

Matrix &
Matrix::set_scale(const Vector &s)
{
	return set_scale(s[0], s[1]);
}

Matrix &
Matrix::set_rotate(const Angle &a)
{
	value_type c(Angle::cos(a).get());
	value_type s(Angle::sin(a).get());
	m00= c;     m01=s;    m02=0.0;
	m10=-1.0*s; m11=c;    m12=0.0;
	m20=0.0;    m21=0.0;  m22=1.0;
	return (*this);
}

Matrix &
Matrix::set_translate(const Vector &t)
{
	return set_translate(t[0], t[1]);
}

Matrix &
Matrix::set_translate(value_type x, value_type y)
{
	m00=1.0; m01=0.0; m02=0.0;
	m10=0.0; m11=1.0; m12=0.0;
	m20=x  ; m21=y  ; m22=1.0;
	return (*this);
}

Vector
Matrix::get_transformed(const Vector &v)const
{
	return Vector(v[0]*m00+v[1]*m10+m20,
				  v[0]*m01+v[1]*m11+m21);
}

Matrix
Matrix::operator*=(const Matrix &rhs)
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

Matrix
Matrix::operator*=(const value_type &rhs)
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

Matrix
Matrix::operator+=(const Matrix &rhs)
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

Matrix
Matrix::operator*(const Matrix &rhs)
{
	return Matrix(*this)*=rhs;
}

Matrix
Matrix::operator*(const value_type &rhs)
{
	return Matrix(*this)*=rhs;
}

Matrix
Matrix::operator+(const Matrix &rhs)
{
	return Matrix(*this)+=rhs;
}

bool
Matrix::is_invertible()const
{
//	printf("%s:%d Matrix::is_invertible() checking %f * %f != %f*%f\nie. %f != %f\nie. %d\n",
//		   __FILE__, __LINE__, m00,m11, m01,m10, m00*m11, m01*m10, abs(m00*m11 - m01*m10) > epsilon);
	return abs(m00*m11 - m01*m10) > epsilon;
}

Matrix&
Matrix::invert()
{
	assert(is_invertible() && !m02 && !m12 && m22==1);

	value_type det(m00*m11-m01*m10);
	value_type tmp(m20);
	m20=(m10*m21-m11*m20)/det;
	m21=(m01*tmp-m00*m21)/det;
	tmp=m00;
	m00=m11/det;
	m11=tmp/det;
	m01=-m01/det;
	m10=-m10/det;
	return *this;
}

String
Matrix::get_string(int spaces, String before, String after)const
{
	return etl::strprintf("%*s [%7.2f %7.2f %7.2f] %s\n%*s [%7.2f %7.2f %7.2f]\n%*s [%7.2f %7.2f %7.2f]\n",
						  spaces, before.c_str(), m00, m01, m02, after.c_str(),
						  spaces, "",			  m10, m11, m12,
						  spaces, "",			  m20, m21, m22);
}
