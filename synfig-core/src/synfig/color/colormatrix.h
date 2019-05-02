/* === S Y N F I G ========================================================= */
/*!	\file colormatrix.h
**	\brief Matrix definitions for 4D affine transformations of color channels
**
**	$Id$
**
**	\legal
**	......... ... 2016 Ivan Mahonin
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

#ifndef __SYNFIG_COLORMATRIX_H
#define __SYNFIG_COLORMATRIX_H

/* === H E A D E R S ======================================================= */

#include <cassert>
#include <cmath>

#include <ETL/stringf>

#include <synfig/angle.h>
#include <synfig/real.h>
#include <synfig/string.h>

#include "color.h"


/* === M A C R O S ========================================================= */

/* === T Y P E D E F S ===================================================== */

/* === C L A S S E S & S T R U C T S ======================================= */

namespace synfig {

/*!	\class ColorMatrix
**	\todo writeme (Matrix 5x5, actually used sub-rectangle 5x4)
*/
class ColorMatrix
{
public:
	typedef Color::value_type value_type;
	typedef value_type value_row[5];
	typedef value_type value_array[25];
	typedef value_row value_matrix[5];

	typedef value_type (*transform_func_ptr)(const ColorMatrix &m, const Color &src);
	typedef void (*batch_func_ptr)(const ColorMatrix &m, value_type *dest, const Color *src, const Color *src_end);

	class BatchProcessor;

	//! The matrix array
	union {
		value_matrix m;
		value_array c;
		struct
		{
			value_type m00, m01, m02, m03, m04;
			value_type m10, m11, m12, m13, m14;
			value_type m20, m21, m22, m23, m24;
			value_type m30, m31, m32, m33, m34;
			value_type m40, m41, m42, m43, m44;
		};
	};

	//!Default constructor makes an identity matrix
	ColorMatrix():
		m00(1.0), m01(0.0), m02(0.0), m03(0.0), m04(0.0),
		m10(0.0), m11(1.0), m12(0.0), m13(0.0), m14(0.0),
		m20(0.0), m21(0.0), m22(1.0), m23(0.0), m24(0.0),
		m30(0.0), m31(0.0), m32(0.0), m33(1.0), m34(0.0),
		m40(0.0), m41(0.0), m42(0.0), m43(0.0), m44(1.0) { }

	ColorMatrix(
		value_type m00, value_type m01, value_type m02, value_type m03, value_type m04,
		value_type m10, value_type m11, value_type m12, value_type m13, value_type m14,
		value_type m20, value_type m21, value_type m22, value_type m23, value_type m24,
		value_type m30, value_type m31, value_type m32, value_type m33, value_type m34,
		value_type m40, value_type m41, value_type m42, value_type m43, value_type m44
	):
		m00(m00), m01(m01), m02(m02), m03(m03), m04(m04),
		m10(m10), m11(m11), m12(m12), m13(m13), m14(m14),
		m20(m20), m21(m21), m22(m22), m23(m23), m24(m24),
		m30(m30), m31(m31), m32(m32), m33(m33), m34(m34),
		m40(m40), m41(m41), m42(m42), m43(m43), m44(m44)
	{ }

	ColorMatrix(Color axis_r, Color axis_g, Color axis_b, Color axis_a, Color offset):
		m00(axis_r.get_r()), m01(axis_r.get_g()), m02(axis_r.get_b()), m03(axis_r.get_a()), m04(0.0),
		m10(axis_g.get_r()), m11(axis_g.get_g()), m12(axis_g.get_b()), m13(axis_g.get_a()), m14(0.0),
		m20(axis_b.get_r()), m21(axis_b.get_g()), m22(axis_b.get_b()), m23(axis_b.get_a()), m24(0.0),
		m30(axis_a.get_r()), m31(axis_a.get_g()), m32(axis_a.get_b()), m33(axis_a.get_a()), m34(0.0),
		m40(offset.get_r()), m41(offset.get_g()), m42(offset.get_b()), m43(offset.get_a()), m44(1.0)
	{ }

	Color get_axis_r()const { return Color(m00, m01, m02, m03); }
	Color get_axis_g()const { return Color(m10, m11, m12, m13); }
	Color get_axis_b()const { return Color(m20, m21, m22, m23); }
	Color get_axis_a()const { return Color(m30, m31, m32, m33); }
	Color get_offset()const { return Color(m40, m41, m42, m43); }
	Color get_constant()const { return get_offset(); }

	//!set_identity member. Set an identity matrix
	ColorMatrix& set_identity()
		{ return *this = ColorMatrix(); }

	bool is_identity() const
		{ return *this == ColorMatrix(); }

	bool is_zero() const;
	bool is_zero(int channel) const;

	bool is_constant() const;
	bool is_constant(int channel) const;

	bool is_copy() const;
	bool is_copy(int channel) const;

	bool is_transparent() const
		{ return is_zero(3); }
	bool is_affects_transparent() const;

	ColorMatrix& set_scale(value_type r, value_type g, value_type b, value_type a = value_type(1.0));
	ColorMatrix& set_scale_rgba(value_type rgba)
		{ return set_scale(rgba, rgba, rgba, rgba); }
	ColorMatrix& set_scale_rgb(value_type rgb)
		{ return set_scale(rgb, rgb, rgb); }
	ColorMatrix& set_scale(const Color &c)
		{ return set_scale(c.get_r(), c.get_g(), c.get_b(), c.get_a()); }

	ColorMatrix& set_translate(value_type r, value_type g, value_type b, value_type a = value_type(0.0));
	ColorMatrix& set_translate(const Color &c)
		{ return set_translate(c.get_r(), c.get_g(), c.get_b(), c.get_a()); }

	ColorMatrix& set_encode_yuv();
	ColorMatrix& set_decode_yuv();
	ColorMatrix& set_rotate_uv(const Angle &a);

	ColorMatrix& set_constant(const Color &c);
	ColorMatrix& set_replace_color(const Color &c);
	ColorMatrix& set_replace_alpha(value_type x);
	ColorMatrix& set_brightness(value_type x);
	ColorMatrix& set_contrast(value_type x);
	ColorMatrix& set_exposure(value_type x);
	ColorMatrix& set_hue_saturation(const Angle &hue, value_type saturation);
	ColorMatrix& set_hue(const Angle &x)
		{ return set_hue_saturation(x, 1.0); }
	ColorMatrix& set_saturation(value_type x)
		{ return set_hue_saturation(Angle::deg(0.0), x); }
	ColorMatrix& set_invert_color();
	ColorMatrix& set_invert_alpha();

	Color get_transformed(Color color) const;

	value_row& operator[] (int r) { return m[r]; }
	const value_row& operator[] (int r) const { return m[r]; }

	bool operator==(const ColorMatrix &rhs) const;
	bool operator!=(const ColorMatrix &rhs) const
		{ return !(*this == rhs); }

	//! operator*=. Multiplication and assignment of one matrix by another
	//! @param rhs the right hand side of the multiplication operation
	//! @return the modified resulting matrix
	ColorMatrix operator*=(const ColorMatrix &rhs);

	//! operator*=. Multiplication and assignment of one matrix by a scalar
	//! @param rhs the number to multiply by
	//! @return the modified resulting matrix
	ColorMatrix operator*=(const value_type &rhs);

	//! operator*. Multiplication of one matrix by another
	//! @param rhs the right hand side of the multiplication operation
	//! @return the resulting matrix
	ColorMatrix operator*(const ColorMatrix &rhs)const
		{ return ColorMatrix(*this) *= rhs; }

	//! operator*. Multiplication of one matrix by a number
	//! @param rhs the number to multiply by
	//! @return the resulting matrix
	ColorMatrix operator*(const value_type &rhs)const
		{ return ColorMatrix(*this) *= rhs; }

	//!Get the string of the ColorMatrix
	//!@return String type. A string representation of the matrix
	//!components.
	String get_string(int spaces = 0, String before = String(), String after = String())const;
};


class ColorMatrix::BatchProcessor
{
private:
	ColorMatrix matrix;

	bool zero_all;
	union
	{
		bool zero[4];
		struct { bool zero_r, zero_g, zero_b, zero_a; };
	};

	Color constant_value;
	bool constant_all;
	union
	{
		bool constant[4];
		struct { bool constant_r, constant_g, constant_b, constant_a; };
	};

	bool copy_all;
	union
	{
		bool copy[4];
		struct { bool copy_r, copy_g, copy_b, copy_a; };
	};

	bool affects_transparent;

	union
	{
		transform_func_ptr transform_funcs[4];
		struct { transform_func_ptr transform_func_r, transform_func_g, transform_func_b, transform_func_a; };
	};

	union
	{
		batch_func_ptr batch_funcs[4];
		struct { batch_func_ptr batch_func_r, batch_func_g, batch_func_b, batch_func_a; };
	};

public:
	BatchProcessor(const ColorMatrix &matrix = ColorMatrix());

	bool is_zero() const { return zero_all; }
	bool is_constant() const { return constant_all; }
	bool is_copy() const { return copy_all; }
	bool is_affects_transparent() const { return affects_transparent; }

	const Color& get_constant_value() const { return constant_value; }

	const ColorMatrix& get_matrix() const { return matrix; }
	void process(Color *dest, int dest_stride, const Color *src, int src_stride, int width, int height) const;
};



}; // END of namespace synfig

#endif
