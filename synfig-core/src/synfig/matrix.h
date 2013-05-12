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
	Matrix();

	//!set_identity member. Set an identity matrix
	Matrix &set_identity();

	//!set_scale member function. Sets a scale matrix
	//! @param sx Scale by X axis
	//! @param sy Scale by Y axis
	//! @return A matrix reference filled with the sx, sy values
	Matrix &set_scale(const value_type &sx, const value_type &sy);

	//!set_scale member fucntion. Sets a scale matrix
	//! @param sxy Scale by X and Y axis
	//! @return A matrix reference filled with the sxy values
	Matrix &set_scale(const value_type &sxy);

	//!set_scale member fucntion. Sets a scale matrix
	//! @param s Vector that defines the scale
	//! @return A matrix reference filled with the proper scale parameters
	Matrix &set_scale(const Vector &s);

	//!set_rotate member function. Sets a rotate matrix
	//! @param a Rotation angle counter clockwise
	//! @return A matrix reference filled with the proper rotation parameters
	Matrix &set_rotate(const Angle &a);

	//!translate member function. Sets a translate matrix
	//! @param t Vector that defines the translation
	//! @return A matrix reference filled with the proper translation parameters
	Matrix &set_translate(const Vector &t);

	//!translate member function. Sets a translate matrix
	//! @param x Scalar that defines the x component of the translation
	//! @param y Scalar that defines the y component of the translation
	//! @return A matrix reference filled with the proper translation parameters
	Matrix &set_translate(value_type x, value_type y);

	//!get_transformed member function.
	//! @param v 2D Vector to transform
	//! @return The Vector result
	Vector get_transformed(const Vector &v)const;

	//! operator*=. Multiplication and assignment of one matrix by another
	//! @param rhs the right hand side of the multiplication operation
	//! @return the modified resulting matrix
	Matrix operator*=(const Matrix &rhs);

	//! operator*=. Multiplication and assignment of one matrix by a scalar
	//! @param rhs the number to multiply by
	//! @return the modifed resulting matrix
	Matrix operator*=(const value_type &rhs);

	//! operator+=. Sum and assignment of two matrixes
	//! @param rhs the matrix to sum
	//! @return modified matrix with the summed matrix
	Matrix operator+=(const Matrix &rhs);

	//! operator*. Multiplication of one matrix by another
	//! @param rhs the right hand side of the multiplication operation
	//! @return the resulting matrix
	Matrix operator*(const Matrix &rhs);

	//! operator*. Multiplication of one matrix by a number
	//! @param rhs the number to multiply by
	//! @return the resulting matrix
	Matrix operator*(const value_type &rhs);

	//! operator+. Sum two matrixes
	//! @param rhs the matrix to sum
	//! @return the resulting matrix
	Matrix
	operator+(const Matrix &rhs);

	bool is_invertible()const;

	//         (m00 m01 0)       1               (     m11     )   (    -m01     )   (      0      )
	// inverse (m10 m11 0)  =  -----          x  (    -m10     )   (     m00     )   (      0      )
	//         (m20 m21 1)     m00m11-m01m10     (m10m21-m11m20)   (m01m20-m00m21)   (m00m11-m01m10)
	Matrix &invert();

	//!Get the string of the Matrix
	//!@return String type. A string representation of the matrix
	//!components.
	String get_string(int spaces = 0, String before = String(), String after = String())const;
};

}; // END of namespace synfig

#endif
