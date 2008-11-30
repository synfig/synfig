/* === S Y N F I G ========================================================= */
/*!	\file matrix.h
**	\brief Matrix definitions for 2D affine transformations
**
**	$Id$
**
**	\legal
**	Copyright (c) 2002-2005 Robert B. Quattlebaum Jr., Adrian Bentley
**	Copyright (c) 2008 Carlos López
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
#include <math.h>

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
	//!Deafult constructor makes a identity matrix
	Matrix() {
		m00=1.0; m01=0.0; m02=0.0;
		m10=0.0; m11=1.0; m12=0.0;
		m20=0.0; m21=0.0; m22=1.0;
		}

	//!Constructor from Angle create a rotate matrix
	Matrix(const Angle &a){
		(*this).set_rotate(a);
		}
	//!set_identity member. Set an identity matrix
	Matrix &
	set_identity(){
		m00=1.0; m01=0.0; m02=0.0;
		m10=0.0; m11=1.0; m12=0.0;
		m20=0.0; m21=0.0; m22=1.0;
		 return (*this);
		 }
	//!set_scale member fucntion. Sets a scale matrix
	//! @param sx Scale by X axis
	//! @param sy Scale by Y axis
	//! @return A matrix reference filled with the sx, sy values
	Matrix &
	set_scale(const value_type &sx, const value_type &sy){
		m00=sx;  m01=0.0; m02=0.0;
		m10=0.0; m11=sy;  m12=0.0;
		m20=0.0; m21=0.0; m22=1.0;
		 return (*this);
		}
	//!set_scale member fucntion. Sets a scale matrix
	//! @param sxy Scale by X and Y axis
	//! @return A matrix reference filled with the sxy values
	Matrix &
	set_scale(const value_type &sxy){
		m00=sxy; m01=0.0; m02=0.0;
		m10=0.0; m11=sxy; m12=0.0;
		m20=0.0; m21=0.0; m22=1.0;
		 return (*this);
		}
	//!set_rotate member function. Sets a rotate matrix
	//! @param a Rotation angle counterclock wise
	//! @return A matrix reference filled with the proper rotation parameters
	Matrix &
	set_rotate(const Angle &a){
		value_type c(Angle::cos(a).get());
		value_type s(Angle::sin(a).get());
		m00= c;     m01=s;    m02=0.0;
		m10=-1.0*s; m11=c;    m12=0.0;
		m20=0.0;    m21=0.0;  m22=1.0;
		 return (*this);
		}
	//!traslate member function. Sets a translate matrix
	//! @param t Vector that defines the translation
	//! @return A matrix reference filled with the proper translation parameters
	Matrix &
	set_translate(const Vector &t){
		m00=1.0;  m01=0.0;  m02=0.0;
		m10=0.0;  m11=1.0;  m12=0.0;
		m20=t[0]; m21=t[1]; m22=1.0;
		 return (*this);
		}

	//!get_transformed member function.
	//! @param v 2D Vector to transform
	//! @return The Vector result
	Vector
	get_transformed(const Vector &v){
		 return Vector(v[0]*m00+v[1]*m10+m20,v[0]*m01+v[1]*m11+m21);
		 }

	//! operator *. Multiplication of one matrix by other
	//! @param rhs the right hand side of the multiplication operation
	//! @return the resulting multiplication matrix
	Matrix
	operator *(const Matrix &rhs){
		Matrix ret;
		ret.m00=m00*rhs.m00 + m01*rhs.m10 + m02*rhs.m20;
		ret.m01=m00*rhs.m01 + m01*rhs.m11 + m02*rhs.m21;
		ret.m02=m00*rhs.m02 + m01*rhs.m12 + m02*rhs.m22;

		ret.m10=m10*rhs.m00 + m11*rhs.m10 + m12*rhs.m20;
		ret.m11=m10*rhs.m01 + m11*rhs.m11 + m12*rhs.m21;
		ret.m12=m10*rhs.m02 + m11*rhs.m12 + m12*rhs.m22;

		ret.m20=m20*rhs.m00 + m21*rhs.m10 + m22*rhs.m20;
		ret.m21=m20*rhs.m01 + m21*rhs.m11 + m22*rhs.m21;
		ret.m22=m20*rhs.m02 + m21*rhs.m12 + m22*rhs.m22;
		return ret;
		}

	//! operator *=. Multiplication and assign of one matrix by a number
	//! @param rhs the number to multiply by
	//! @return the modifed resulting multiplicated by number matrix
	Matrix
	operator *=(const value_type &rhs){
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

	//! operator +=. Sum and assign of two matrixes
	//! @param rhs the matrix to sum
	//! @return modified matrix with the summed matrix
	Matrix
	operator +=(const Matrix &rhs){
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

	//! operator *. Multiplication of one matrix by a number
	//! @param rhs the number to multiply by
	//! @return the resulting multiplicated by number matrix
	Matrix
	operator *(const value_type &rhs){
	return Matrix(*this)*=rhs;
		}

	//! operator +=. Sum and assign of two matrixes
	//! @param rhs the matrix to sum
	//! @return modified matrix with the summed matrix
	Matrix
	operator +(const Matrix &rhs){
	return Matrix(*this)+=rhs;
		}

};

}; // END of namespace synfig

#endif
