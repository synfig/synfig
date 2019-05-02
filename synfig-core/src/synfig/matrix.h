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
#include <cmath>
#include <iostream>
#include <ETL/stringf>

/* === M A C R O S ========================================================= */

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

	Matrix2(const Vector &row_x, const Vector &row_y):
		m00(row_x[0]), m01(row_x[1]), m10(row_y[0]), m11(row_y[1]) { }

	const Vector& row(int index) const { return *(const Vector*)(m[index]); }
	const Vector& row_x() const { return row(0); }
	const Vector& row_y() const { return row(1); }
	const Vector& axis(int index) const { return row(index); }
	const Vector& axis_x() const { return row_x(); }
	const Vector& axis_y() const { return row_y(); }

	Vector& row(int index) { return *(Vector*)(m[index]); }
	Vector& row_x() { return row(0); }
	Vector& row_y() { return row(1); }
	Vector& axis(int index) { return row(index); }
	Vector& axis_x() { return row_x(); }
	Vector& axis_y() { return row_y(); }

	//!set_identity member. Set an identity matrix
	Matrix2& set_identity()
		{ return *this = Matrix2(); }

	bool is_identity() const
		{ return *this == Matrix2(); }

	//!set_scale member function. Sets a scale matrix
	//! @param sx Scale by X axis
	//! @param sy Scale by Y axis
	//! @return A matrix reference filled with the sx, sy values
	Matrix2& set_scale(const value_type &sx, const value_type &sy);

	//!set_scale member function. Sets a scale matrix
	//! @param sxy Scale by X and Y axis
	//! @return A matrix reference filled with the sxy values
	Matrix2& set_scale(const value_type &sxy)
		{ return set_scale(sxy, sxy); }

	//!set_scale member function. Sets a scale matrix
	//! @param s Vector that defines the scale
	//! @return A matrix reference filled with the proper scale parameters
	Matrix2& set_scale(const Vector &s)
		{ return set_scale(s[0], s[1]); }

	//!set_rotate member function. Sets a rotate matrix
	//! @param a Rotation angle counter clockwise
	//! @return A matrix reference filled with the proper rotation parameters
	Matrix2& set_rotate(const Angle &a);

	void get_transformed(value_type &out_x, value_type &out_y, const value_type x, const value_type y)const;

	//!get_transformed member function.
	//! @param v 2D Vector to transform
	//! @return The Vector result
	Vector get_transformed(const Vector &v)const
		{ Vector vv; get_transformed(vv[0], vv[1], v[0], v[1]); return vv; }

	bool operator==(const Matrix2 &rhs) const;
	bool operator!=(const Matrix2 &rhs) const
		{ return !(*this == rhs); }

	Vector operator*(const Vector &v)
		{ return get_transformed(v); }

	//! operator*=. Multiplication and assignment of one matrix by another
	//! @param rhs the right hand side of the multiplication operation
	//! @return the modified resulting matrix
	Matrix2& operator*=(const Matrix2 &rhs)
		{ return *this = *this * rhs; }

	//! operator*=. Multiplication and assignment of one matrix by a scalar
	//! @param rhs the number to multiply by
	//! @return the modified resulting matrix
	Matrix2& operator*=(const value_type &rhs);

	//! operator+=. Sum and assignment of two matrixes
	//! @param rhs the matrix to sum
	//! @return modified matrix with the summed matrix
	Matrix2& operator+=(const Matrix2 &rhs);

	//! operator*. Multiplication of one matrix by another
	//! @param rhs the right hand side of the multiplication operation
	//! @return the resulting matrix
	Matrix2 operator*(const Matrix2 &rhs)const;

	Vector operator*(const Vector &v)const
		{ return get_transformed(v); }

	//! operator*. Multiplication of one matrix by a number
	//! @param rhs the number to multiply by
	//! @return the resulting matrix
	Matrix2 operator*(const value_type &rhs)const;

	//! operator+. Sum two matrixes
	//! @param rhs the matrix to sum
	//! @return the resulting matrix
	Matrix2 operator+(const Matrix2 &rhs)const
		{ return Matrix2(*this) += rhs; }

	bool is_invertible()const;

	Matrix2& invert();

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

	Matrix3(
		const Vector &axis_x,
		const Vector &axis_y,
		const Vector &offset
	):
		m00(axis_x[0]), m01(axis_x[1]), m02(0.0),
		m10(axis_y[0]), m11(axis_y[1]), m12(0.0),
		m20(offset[0]), m21(offset[1]), m22(1.0)
	{ }

	Matrix3(
		const Vector3 &row_x,
		const Vector3 &row_y,
		const Vector3 &row_z
	):
		m00(row_x[0]), m01(row_x[1]), m02(row_x[2]),
		m10(row_y[0]), m11(row_y[1]), m12(row_y[2]),
		m20(row_z[0]), m21(row_z[1]), m22(row_z[2])
	{ }

	const Vector3& row(int index) const { return *(const Vector3*)(m[index]); }
	const Vector3& row_x() const { return row(0); }
	const Vector3& row_y() const { return row(1); }
	const Vector3& row_z() const { return row(2); }
	const Vector& axis(int index) const { return *(const Vector*)(m[index]); }
	const Vector& axis_x() const { return axis(0); }
	const Vector& axis_y() const { return axis(1); }
	const Vector& offset() const { return axis(2); }

	Vector3& row(int index) { return *(Vector3*)(m[index]); }
	Vector3& row_x() { return row(0); }
	Vector3& row_y() { return row(1); }
	Vector3& row_z() { return row(2); }
	Vector& axis(int index) { return *(Vector*)(m[index]); }
	Vector& axis_x() { return axis(0); }
	Vector& axis_y() { return axis(1); }
	Vector& offset() { return axis(2); }

	//!set_identity member. Set an identity matrix
	Matrix3& set_identity()
		{ return *this = Matrix3(); }

	bool is_identity() const
		{ return *this == Matrix3(); }

	//!set_scale member function. Sets a scale matrix
	//! @param sx Scale by X axis
	//! @param sy Scale by Y axis
	//! @return A matrix reference filled with the sx, sy values
	Matrix3& set_scale(const value_type &sx, const value_type &sy);

	//!set_scale member function. Sets a scale matrix
	//! @param sxy Scale by X and Y axis
	//! @return A matrix reference filled with the sxy values
	Matrix3& set_scale(const value_type &sxy)
		{ return set_scale(sxy, sxy); }

	//!set_scale member function. Sets a scale matrix
	//! @param s Vector that defines the scale
	//! @return A matrix reference filled with the proper scale parameters
	Matrix3& set_scale(const Vector &s)
		{ return set_scale(s[0], s[1]); }

	//!set_rotate member function. Sets a rotate matrix
	//! @param a Rotation angle counter clockwise
	//! @return A matrix reference filled with the proper rotation parameters
	Matrix3& set_rotate(const Angle &a);

	//!translate member function. Sets a translate matrix
	//! @param t Vector that defines the translation
	//! @return A matrix reference filled with the proper translation parameters
	Matrix3& set_translate(const Vector &t)
		{ return set_translate(t[0], t[1]); }

	//!translate member function. Sets a translate matrix
	//! @param x Scalar that defines the x component of the translation
	//! @param y Scalar that defines the y component of the translation
	//! @return A matrix reference filled with the proper translation parameters
	Matrix3& set_translate(value_type x, value_type y);

	void get_transformed(
		value_type &out_x, value_type &out_y, value_type &out_z,
		const value_type x, const value_type y, const value_type z )const;

	void get_transformed(value_type &out_x, value_type &out_y, const value_type x, const value_type y, bool translate = true)const
		{ value_type z; get_transformed(out_x, out_y, z, x, y, translate ? 1.0 : 0.0); }

	Vector3 get_transformed(const Vector3 &v)const
		{ Vector3 vv; get_transformed(vv[0], vv[1], vv[2], v[0], v[1], v[2]); return vv; }

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
	Matrix3& operator*=(const Matrix3 &rhs)
		{ return *this = *this * rhs; }

	//! operator*=. Multiplication and assignment of one matrix by a scalar
	//! @param rhs the number to multiply by
	//! @return the modified resulting matrix
	Matrix3& operator*=(const value_type &rhs);

	//! operator+=. Sum and assignment of two matrixes
	//! @param rhs the matrix to sum
	//! @return modified matrix with the summed matrix
	Matrix3& operator+=(const Matrix3 &rhs);

	//! operator*. Multiplication of one matrix by another
	//! @param rhs the right hand side of the multiplication operation
	//! @return the resulting matrix
	Matrix3 operator*(const Matrix3 &rhs)const;

	Vector3 operator*(const Vector3 &v)const
		{ return get_transformed(v); }

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

	value_type det() const;

	bool is_invertible()const;

	Matrix3 get_inverted() const;

	Matrix3& invert()
		{ return *this = get_inverted(); }

	//!Get the string of the Matrix
	//!@return String type. A string representation of the matrix
	//!components.
	String get_string(int spaces = 0, String before = String(), String after = String())const;
};

typedef Matrix3 Matrix;

}; // END of namespace synfig

#endif
