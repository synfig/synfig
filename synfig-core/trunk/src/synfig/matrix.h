/* === S Y N F I G ========================================================= */
/*!	\file matrix.h
**	\brief Matrix definitions for 2D affine transformations
**
**	$Id$
**
**	\legal
**	Copyright (c) 2008 Carlos López & Chirs Moore
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

#ifndef __SYNFIG_MATRIX_H
#define __SYNFIG_MATRIX_H

/* === H E A D E R S ======================================================= */

#include "angle.h"
#include "real.h"
#include "vector.h"
#include "string.h"
#include <cassert>
#include <math.h>
#include <iostream>
#include <ETL/stringf>

/* === M A C R O S ========================================================= */


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

#define COUT_MATRIX(m)													\
	cout<<"["<<m.m00<<"]["<<m.m01<<"]["<<m.m02<<"]"<<endl;				\
	cout<<"["<<m.m10<<"]["<<m.m11<<"]["<<m.m12<<"]"<<endl;				\
	cout<<"["<<m.m20<<"]["<<m.m21<<"]["<<m.m22<<"]"<<endl

/* === T Y P E D E F S ===================================================== */

/* === C L A S S E S & S T R U C T S ======================================= */

namespace synfig {

/*!	\class Matrix
**	\todo writeme
*/
class Matrix
{
public:
	typedef Real value_type;

private:
	//! The matrix array
	value_type m00, m01, m02;
	value_type m10, m11, m12;
	value_type m20, m21, m22;
	// Index convention
	// 00 01 02
	// 10 11 12
	// 20 21 22
	// vectors are premultiplied when the matrix transformation is applied
	// we consider the vectors as [1]x[3] arrays.
	// [1]x[3] * [3]x[3] = [1]x[3]
	// vector  * matrix  = vector
	// In affine transformation matrixes the values of
	// m02=0, m12=0 and m22=1 for non projective transformations.

public:
	//!Default constructor makes an identity matrix
	Matrix()
	{
		m00=1.0; m01=0.0; m02=0.0;
		m10=0.0; m11=1.0; m12=0.0;
		m20=0.0; m21=0.0; m22=1.0;
	}

	//!set_identity member. Set an identity matrix
	Matrix &
	set_identity()
	{
		m00=1.0; m01=0.0; m02=0.0;
		m10=0.0; m11=1.0; m12=0.0;
		m20=0.0; m21=0.0; m22=1.0;
		return (*this);
	}

	//!set_scale member function. Sets a scale matrix
	//! @param sx Scale by X axis
	//! @param sy Scale by Y axis
	//! @return A matrix reference filled with the sx, sy values
	Matrix &
	set_scale(const value_type &sx, const value_type &sy)
	{
		m00=sx;  m01=0.0; m02=0.0;
		m10=0.0; m11=sy;  m12=0.0;
		m20=0.0; m21=0.0; m22=1.0;
		return (*this);
	}

	//!set_scale member fucntion. Sets a scale matrix
	//! @param sxy Scale by X and Y axis
	//! @return A matrix reference filled with the sxy values
	Matrix &
	set_scale(const value_type &sxy)
	{
		m00=sxy; m01=0.0; m02=0.0;
		m10=0.0; m11=sxy; m12=0.0;
		m20=0.0; m21=0.0; m22=1.0;
		return (*this);
	}

	//!set_rotate member function. Sets a rotate matrix
	//! @param a Rotation angle counter clockwise
	//! @return A matrix reference filled with the proper rotation parameters
	Matrix &
	set_rotate(const Angle &a)
	{
		value_type c(Angle::cos(a).get());
		value_type s(Angle::sin(a).get());
		m00= c;     m01=s;    m02=0.0;
		m10=-1.0*s; m11=c;    m12=0.0;
		m20=0.0;    m21=0.0;  m22=1.0;
		return (*this);
	}

	//!translate member function. Sets a translate matrix
	//! @param t Vector that defines the translation
	//! @return A matrix reference filled with the proper translation parameters
	Matrix &
	set_translate(const Vector &t)
	{
		return set_translate(t[0], t[1]);
	}

	//!translate member function. Sets a translate matrix
	//! @param x Scalar that defines the x component of the translation
	//! @param y Scalar that defines the y component of the translation
	//! @return A matrix reference filled with the proper translation parameters
	Matrix &
	set_translate(value_type x, value_type y)
	{
		m00=1.0; m01=0.0; m02=0.0;
		m10=0.0; m11=1.0; m12=0.0;
		m20=x  ; m21=y  ; m22=1.0;
		return (*this);
	}

	//!get_transformed member function.
	//! @param v 2D Vector to transform
	//! @return The Vector result
	Vector
	get_transformed(const Vector &v)
	{
		return Vector(v[0]*m00+v[1]*m10+m20,
					  v[0]*m01+v[1]*m11+m21);
	}

	//! operator*=. Multiplication and assignment of one matrix by another
	//! @param rhs the right hand side of the multiplication operation
	//! @return the modified resulting matrix
	Matrix
	operator*=(const Matrix &rhs)
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

	//! operator*=. Multiplication and assignment of one matrix by a scalar
	//! @param rhs the number to multiply by
	//! @return the modifed resulting matrix
	Matrix
	operator*=(const value_type &rhs)
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

	//! operator+=. Sum and assignment of two matrixes
	//! @param rhs the matrix to sum
	//! @return modified matrix with the summed matrix
	Matrix
	operator+=(const Matrix &rhs)
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

	//! operator*. Multiplication of one matrix by another
	//! @param rhs the right hand side of the multiplication operation
	//! @return the resulting matrix
	Matrix
	operator*(const Matrix &rhs)
	{
		return Matrix(*this)*=rhs;
	}

	//! operator*. Multiplication of one matrix by a number
	//! @param rhs the number to multiply by
	//! @return the resulting matrix
	Matrix
	operator*(const value_type &rhs)
	{
		return Matrix(*this)*=rhs;
	}

	//! operator+. Sum two matrixes
	//! @param rhs the matrix to sum
	//! @return the resulting matrix
	Matrix
	operator+(const Matrix &rhs)
	{
		return Matrix(*this)+=rhs;
	}

	bool
	is_invertible()const
	{
		return m00*m11 != m01*m10;
	}

	//         (m00 m01 0)       1               (     m11     )   (    -m01     )   (      0      )
	// inverse (m10 m11 0)  =  -----          x  (    -m10     )   (     m00     )   (      0      )
	//         (m20 m21 1)     m00m11-m01m10     (m10m21-m11m20)   (m01m20-m00m21)   (m00m11-m01m10)
	Matrix &
	invert()
	{
		assert(is_invertible() && !m02 && !m12 && m22==1);

		value_type det(m00*m11-m01*m10);
		value_type tmp(m20/det);
		m20=(m10*m21-m11*m20)/det;
		m21=(m01*tmp-m00*m21)/det;
		tmp=m00;
		m00=m11/det;
		m11=tmp/det;
		m01=-m01/det;
		m10=-m10/det;
		return *this;
	}

	//!Get the string of the Matrix
	//!@return String type. A string representation of the matrix
	//!components.
	String
	get_string(int spaces = 0, String before = String(), String after = String())
	{
		return etl::strprintf("%*s [%7.2f %7.2f %7.2f] %s\n%*s [%7.2f %7.2f %7.2f]\n%*s [%7.2f %7.2f %7.2f]\n",
							  spaces, before.c_str(), m00, m01, m02, after.c_str(),
							  spaces, "",			  m10, m11, m12,
							  spaces, "",			  m20, m21, m22);
	}
};

}; // END of namespace synfig

#endif
