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

/*!	\class Matrix2
**	\todo writeme (Matrix 2x2)
*/
class Matrix2
{
public:
	typedef Real value_type;

public:
	//! The matrix array
	union {
		value_type m[2][2];
		struct
		{
			value_type m00, m01;
			value_type m10, m11;
		};
	};

	//!Default constructor makes an identity matrix
	Matrix2(): m00(1.0), m01(0.0), m10(0.0), m11(1.0) { }

	Matrix2(value_type m00, value_type m01, value_type m10, value_type m11):
		m00(m00), m01(m01), m10(m10), m11(m11) { }

	Matrix2(const Vector &axis_x, const Vector &axis_y):
		m00(axis_x[0]), m01(axis_x[1]), m10(axis_y[0]), m11(axis_y[1]) { }

	Vector get_axis_x()const { return Vector(m00, m01); }
	Vector get_axis_y()const { return Vector(m10, m11); }

	//!set_identity member. Set an identity matrix
	Matrix2 &set_identity()
		{ return *this = Matrix2(); }

	bool is_identity() const
		{ return *this == Matrix2(); }

	//!set_scale member function. Sets a scale matrix
	//! @param sx Scale by X axis
	//! @param sy Scale by Y axis
	//! @return A matrix reference filled with the sx, sy values
	Matrix2 &set_scale(const value_type &sx, const value_type &sy);

	//!set_scale member fucntion. Sets a scale matrix
	//! @param sxy Scale by X and Y axis
	//! @return A matrix reference filled with the sxy values
	Matrix2 &set_scale(const value_type &sxy)
		{ return set_scale(sxy, sxy); }

	//!set_scale member fucntion. Sets a scale matrix
	//! @param s Vector that defines the scale
	//! @return A matrix reference filled with the proper scale parameters
	Matrix2 &set_scale(const Vector &s)
		{ return set_scale(s[0], s[1]); }

	//!set_rotate member function. Sets a rotate matrix
	//! @param a Rotation angle counter clockwise
	//! @return A matrix reference filled with the proper rotation parameters
	Matrix2 &set_rotate(const Angle &a);

	void get_transformed(value_type &out_x, value_type &out_y, const value_type x, const value_type y)const;

	//!get_transformed member function.
	//! @param v 2D Vector to transform
	//! @return The Vector result
	Vector get_transformed(const Vector &v)const
		{ Vector vv; get_transformed(vv[0], vv[1], v[0], v[1]); return vv; }

	bool operator==(const Matrix2 &rhs) const;
	bool operator!=(const Matrix2 &rhs) const
		{ return !(*this == rhs); }

	//! operator*=. Multiplication and assignment of one matrix by another
	//! @param rhs the right hand side of the multiplication operation
	//! @return the modified resulting matrix
	Matrix2 operator*=(const Matrix2 &rhs);

	//! operator*=. Multiplication and assignment of one matrix by a scalar
	//! @param rhs the number to multiply by
	//! @return the modifed resulting matrix
	Matrix2 operator*=(const value_type &rhs);

	//! operator+=. Sum and assignment of two matrixes
	//! @param rhs the matrix to sum
	//! @return modified matrix with the summed matrix
	Matrix2 operator+=(const Matrix2 &rhs);

	//! operator*. Multiplication of one matrix by another
	//! @param rhs the right hand side of the multiplication operation
	//! @return the resulting matrix
	Matrix2 operator*(const Matrix2 &rhs)const
		{ return Matrix2(*this) *= rhs; }

	//! operator*. Multiplication of one matrix by a number
	//! @param rhs the number to multiply by
	//! @return the resulting matrix
	Matrix2 operator*(const value_type &rhs)const
		{ return Matrix2(*this) *= rhs; }

	//! operator+. Sum two matrixes
	//! @param rhs the matrix to sum
	//! @return the resulting matrix
	Matrix2 operator+(const Matrix2 &rhs)const
		{ return Matrix2(*this) += rhs; }

	bool is_invertible()const;

	Matrix2 &invert();

	//!Get the string of the Matrix
	//!@return String type. A string representation of the matrix
	//!components.
	String get_string(int spaces = 0, String before = String(), String after = String())const;
};


/*!	\class Matrix3
**	\todo writeme (Matrix 3x3)
*/
class Matrix3
{
public:
	typedef Real value_type;

public:
	//! The matrix array
	union {
		value_type m[3][3];
		struct
		{
			value_type m00, m01, m02;
			value_type m10, m11, m12;
			value_type m20, m21, m22;
		};
	};

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

	//!Default constructor makes an identity matrix
	Matrix3():
		m00(1.0), m01(0.0), m02(0.0),
		m10(0.0), m11(1.0), m12(0.0),
		m20(0.0), m21(0.0), m22(1.0) { }

	Matrix3(
		value_type m00, value_type m01, value_type m02,
		value_type m10, value_type m11, value_type m12,
		value_type m20, value_type m21, value_type m22
	):
		m00(m00), m01(m01), m02(m02),
		m10(m10), m11(m11), m12(m12),
		m20(m20), m21(m21), m22(m22)
	{ }

	Matrix3(Vector axis_x, Vector axis_y, Vector offset):
		m00(axis_x[0]), m01(axis_x[1]), m02(0.0),
		m10(axis_y[0]), m11(axis_y[1]), m12(0.0),
		m20(offset[0]), m21(offset[1]), m22(1.0)
	{ }

	Vector get_axis_x()const { return Vector(m00, m01); }
	Vector get_axis_y()const { return Vector(m10, m11); }
	Vector get_offset()const { return Vector(m20, m21); }

	//!set_identity member. Set an identity matrix
	Matrix3 &set_identity()
		{ return *this = Matrix3(); }

	bool is_identity() const
		{ return *this == Matrix3(); }

	//!set_scale member function. Sets a scale matrix
	//! @param sx Scale by X axis
	//! @param sy Scale by Y axis
	//! @return A matrix reference filled with the sx, sy values
	Matrix3 &set_scale(const value_type &sx, const value_type &sy);

	//!set_scale member fucntion. Sets a scale matrix
	//! @param sxy Scale by X and Y axis
	//! @return A matrix reference filled with the sxy values
	Matrix3 &set_scale(const value_type &sxy)
		{ return set_scale(sxy, sxy); }

	//!set_scale member fucntion. Sets a scale matrix
	//! @param s Vector that defines the scale
	//! @return A matrix reference filled with the proper scale parameters
	Matrix3 &set_scale(const Vector &s)
		{ return set_scale(s[0], s[1]); }

	//!set_rotate member function. Sets a rotate matrix
	//! @param a Rotation angle counter clockwise
	//! @return A matrix reference filled with the proper rotation parameters
	Matrix3 &set_rotate(const Angle &a);

	//!translate member function. Sets a translate matrix
	//! @param t Vector that defines the translation
	//! @return A matrix reference filled with the proper translation parameters
	Matrix3 &set_translate(const Vector &t)
		{ return set_translate(t[0], t[1]); }

	//!translate member function. Sets a translate matrix
	//! @param x Scalar that defines the x component of the translation
	//! @param y Scalar that defines the y component of the translation
	//! @return A matrix reference filled with the proper translation parameters
	Matrix3 &set_translate(value_type x, value_type y);

	void get_transformed(value_type &out_x, value_type &out_y, const value_type x, const value_type y, bool translate = true)const;

	//!get_transformed member function.
	//! @param v 2D Vector to transform
	//! @return The Vector result
	Vector get_transformed(const Vector &v, bool translate = true)const
		{ Vector vv; get_transformed(vv[0], vv[1], v[0], v[1], translate); return vv; }

	bool operator==(const Matrix3 &rhs) const;
	bool operator!=(const Matrix3 &rhs) const
		{ return !(*this == rhs); }

	//! operator*=. Multiplication and assignment of one matrix by another
	//! @param rhs the right hand side of the multiplication operation
	//! @return the modified resulting matrix
	Matrix3 operator*=(const Matrix3 &rhs);

	//! operator*=. Multiplication and assignment of one matrix by a scalar
	//! @param rhs the number to multiply by
	//! @return the modifed resulting matrix
	Matrix3 operator*=(const value_type &rhs);

	//! operator+=. Sum and assignment of two matrixes
	//! @param rhs the matrix to sum
	//! @return modified matrix with the summed matrix
	Matrix3 operator+=(const Matrix3 &rhs);

	//! operator*. Multiplication of one matrix by another
	//! @param rhs the right hand side of the multiplication operation
	//! @return the resulting matrix
	Matrix3 operator*(const Matrix3 &rhs)const
		{ return Matrix3(*this) *= rhs; }

	//! operator*. Multiplication of one matrix by a number
	//! @param rhs the number to multiply by
	//! @return the resulting matrix
	Matrix3 operator*(const value_type &rhs)const
		{ return Matrix3(*this) *= rhs; }

	//! operator+. Sum two matrixes
	//! @param rhs the matrix to sum
	//! @return the resulting matrix
	Matrix3 operator+(const Matrix3 &rhs)const
		{ return Matrix3(*this) += rhs; }

	bool is_invertible()const;

	//         (m00 m01 0)       1               (     m11     )   (    -m01     )   (      0      )
	// inverse (m10 m11 0)  =  -----          x  (    -m10     )   (     m00     )   (      0      )
	//         (m20 m21 1)     m00m11-m01m10     (m10m21-m11m20)   (m01m20-m00m21)   (m00m11-m01m10)
	Matrix3 &invert();

	//!Get the string of the Matrix
	//!@return String type. A string representation of the matrix
	//!components.
	String get_string(int spaces = 0, String before = String(), String after = String())const;
};

typedef Matrix3 Matrix;

}; // END of namespace synfig

#endif
