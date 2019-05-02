/* === S Y N F I G ========================================================= */
/*!	\file colormatrix.cpp
**	\brief Template File
**
**	$Id$
**
**	\legal
**  ......... ... 2016 Ivan Mahonin
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

#include <cstring>

#include <synfig/general.h>
#include "colormatrix.h"

#endif

/* === U S I N G =========================================================== */

using namespace std;
using namespace etl;
using namespace synfig;

/* === M A C R O S ========================================================= */

/* === G L O B A L S ======================================================= */

/* === M E T H O D S ======================================================= */


// Internal

namespace {
	class Internal
	{
	public:
		template<int channel, int mode_r, int mode_g, int mode_b, int mode_a, int mode_o>
		static inline ColorMatrix::value_type transform(const ColorMatrix &m, const Color &c)
		{
			ColorMatrix::value_type x(mode_o ? m[4][channel] : ColorMatrix::value_type(0.0));

			if (mode_r == -1) x -= c.get_r();
			if (mode_r ==  1) x += c.get_r();
			if (mode_r ==  2) x += m[0][channel]*c.get_r();

			if (mode_g == -1) x -= c.get_g();
			if (mode_g ==  1) x += c.get_g();
			if (mode_g ==  2) x += m[1][channel]*c.get_g();

			if (mode_b == -1) x -= c.get_b();
			if (mode_b ==  1) x += c.get_b();
			if (mode_b ==  2) x += m[2][channel]*c.get_b();

			if (mode_a == -1) x -= c.get_a();
			if (mode_a ==  1) x += c.get_a();
			if (mode_a ==  2) x += m[3][channel]*c.get_a();

			return x;
		}

		template<int channel, int mode_r, int mode_g, int mode_b, int mode_a, int mode_o>
		static inline void batch_transform(const ColorMatrix &m, ColorMatrix::value_type *dest, const Color *src, const Color *src_end)
		{
			for(; src < src_end; dest += 4, ++src)
				*dest = transform<channel, mode_r, mode_g, mode_b, mode_a, mode_o>(m, *src);
		}


		// transform func generator

		template<int channel, int mode_r, int mode_g, int mode_b, int mode_a, int mode_o>
		static ColorMatrix::transform_func_ptr get_transform_func_crgbao()
			{ return transform<channel, mode_r, mode_g, mode_b, mode_a, mode_o>; }

		template<int channel, int mode_r, int mode_g, int mode_b, int mode_a>
		static ColorMatrix::transform_func_ptr get_transform_func_crgba(const ColorMatrix &m)
		{
			return approximate_equal_lp(m[4][channel], ColorMatrix::value_type( 0.0)) ? get_transform_func_crgbao<channel, mode_r, mode_g, mode_b, mode_a,  0>()
				 : approximate_equal_lp(m[4][channel], ColorMatrix::value_type( 1.0)) ? get_transform_func_crgbao<channel, mode_r, mode_g, mode_b, mode_a,  1>()
				 : approximate_equal_lp(m[4][channel], ColorMatrix::value_type(-1.0)) ? get_transform_func_crgbao<channel, mode_r, mode_g, mode_b, mode_a, -1>()
				                                                                      : get_transform_func_crgbao<channel, mode_r, mode_g, mode_b, mode_a,  2>();
		}

		template<int channel, int mode_r, int mode_g, int mode_b>
		static ColorMatrix::transform_func_ptr get_transform_func_crgb(const ColorMatrix &m)
		{
			return approximate_equal_lp(m[3][channel], ColorMatrix::value_type( 0.0)) ? get_transform_func_crgba<channel, mode_r, mode_g, mode_b,  0>(m)
				 : approximate_equal_lp(m[3][channel], ColorMatrix::value_type( 1.0)) ? get_transform_func_crgba<channel, mode_r, mode_g, mode_b,  1>(m)
				 : approximate_equal_lp(m[3][channel], ColorMatrix::value_type(-1.0)) ? get_transform_func_crgba<channel, mode_r, mode_g, mode_b, -1>(m)
				                                                                      : get_transform_func_crgba<channel, mode_r, mode_g, mode_b,  2>(m);
		}

		template<int channel, int mode_r, int mode_g>
		static ColorMatrix::transform_func_ptr get_transform_func_crg(const ColorMatrix &m)
		{
			return approximate_equal_lp(m[2][channel], ColorMatrix::value_type( 0.0)) ? get_transform_func_crgb<channel, mode_r, mode_g,  0>(m)
				 : approximate_equal_lp(m[2][channel], ColorMatrix::value_type( 1.0)) ? get_transform_func_crgb<channel, mode_r, mode_g,  1>(m)
				 : approximate_equal_lp(m[2][channel], ColorMatrix::value_type(-1.0)) ? get_transform_func_crgb<channel, mode_r, mode_g, -1>(m)
				                                                                      : get_transform_func_crgb<channel, mode_r, mode_g,  2>(m);
		}

		template<int channel, int mode_r>
		static ColorMatrix::transform_func_ptr get_transform_func_cr(const ColorMatrix &m)
		{
			return approximate_equal_lp(m[1][channel], ColorMatrix::value_type( 0.0)) ? get_transform_func_crg<channel, mode_r,  0>(m)
				 : approximate_equal_lp(m[1][channel], ColorMatrix::value_type( 1.0)) ? get_transform_func_crg<channel, mode_r,  1>(m)
				 : approximate_equal_lp(m[1][channel], ColorMatrix::value_type(-1.0)) ? get_transform_func_crg<channel, mode_r, -1>(m)
				                                                                      : get_transform_func_crg<channel, mode_r,  2>(m);
		}

		template<int channel>
		static ColorMatrix::transform_func_ptr get_transform_func_c(const ColorMatrix &m)
		{
			return approximate_equal_lp(m[0][channel], ColorMatrix::value_type( 0.0)) ? get_transform_func_cr<channel,  0>(m)
				 : approximate_equal_lp(m[0][channel], ColorMatrix::value_type( 1.0)) ? get_transform_func_cr<channel,  1>(m)
				 : approximate_equal_lp(m[0][channel], ColorMatrix::value_type(-1.0)) ? get_transform_func_cr<channel, -1>(m)
				                                                                      : get_transform_func_cr<channel,  2>(m);
		}


		// batch func generator

		template<int channel, int mode_r, int mode_g, int mode_b, int mode_a, int mode_o>
		static ColorMatrix::batch_func_ptr get_batch_func_crgbao()
			{ return batch_transform<channel, mode_r, mode_g, mode_b, mode_a, mode_o>; }

		template<int channel, int mode_r, int mode_g, int mode_b, int mode_a>
		static ColorMatrix::batch_func_ptr get_batch_func_crgba(const ColorMatrix &m)
		{
			return approximate_equal_lp(m[4][channel], ColorMatrix::value_type( 0.0)) ? get_batch_func_crgbao<channel, mode_r, mode_g, mode_b, mode_a,  0>()
				 : approximate_equal_lp(m[4][channel], ColorMatrix::value_type( 1.0)) ? get_batch_func_crgbao<channel, mode_r, mode_g, mode_b, mode_a,  1>()
				 : approximate_equal_lp(m[4][channel], ColorMatrix::value_type(-1.0)) ? get_batch_func_crgbao<channel, mode_r, mode_g, mode_b, mode_a, -1>()
				                                                                      : get_batch_func_crgbao<channel, mode_r, mode_g, mode_b, mode_a,  2>();
		}

		template<int channel, int mode_r, int mode_g, int mode_b>
		static ColorMatrix::batch_func_ptr get_batch_func_crgb(const ColorMatrix &m)
		{
			return approximate_equal_lp(m[3][channel], ColorMatrix::value_type( 0.0)) ? get_batch_func_crgba<channel, mode_r, mode_g, mode_b,  0>(m)
				 : approximate_equal_lp(m[3][channel], ColorMatrix::value_type( 1.0)) ? get_batch_func_crgba<channel, mode_r, mode_g, mode_b,  1>(m)
				 : approximate_equal_lp(m[3][channel], ColorMatrix::value_type(-1.0)) ? get_batch_func_crgba<channel, mode_r, mode_g, mode_b, -1>(m)
				                                                                      : get_batch_func_crgba<channel, mode_r, mode_g, mode_b,  2>(m);
		}

		template<int channel, int mode_r, int mode_g>
		static ColorMatrix::batch_func_ptr get_batch_func_crg(const ColorMatrix &m)
		{
			return approximate_equal_lp(m[2][channel], ColorMatrix::value_type( 0.0)) ? get_batch_func_crgb<channel, mode_r, mode_g,  0>(m)
				 : approximate_equal_lp(m[2][channel], ColorMatrix::value_type( 1.0)) ? get_batch_func_crgb<channel, mode_r, mode_g,  1>(m)
				 : approximate_equal_lp(m[2][channel], ColorMatrix::value_type(-1.0)) ? get_batch_func_crgb<channel, mode_r, mode_g, -1>(m)
				                                                                      : get_batch_func_crgb<channel, mode_r, mode_g,  2>(m);
		}

		template<int channel, int mode_r>
		static ColorMatrix::batch_func_ptr get_batch_func_cr(const ColorMatrix &m)
		{
			return approximate_equal_lp(m[1][channel], ColorMatrix::value_type( 0.0)) ? get_batch_func_crg<channel, mode_r,  0>(m)
				 : approximate_equal_lp(m[1][channel], ColorMatrix::value_type( 1.0)) ? get_batch_func_crg<channel, mode_r,  1>(m)
				 : approximate_equal_lp(m[1][channel], ColorMatrix::value_type(-1.0)) ? get_batch_func_crg<channel, mode_r, -1>(m)
				                                                                      : get_batch_func_crg<channel, mode_r,  2>(m);
		}

		template<int channel>
		static ColorMatrix::batch_func_ptr get_batch_func_c(const ColorMatrix &m)
		{
			return approximate_equal_lp(m[0][channel], ColorMatrix::value_type( 0.0)) ? get_batch_func_cr<channel,  0>(m)
				 : approximate_equal_lp(m[0][channel], ColorMatrix::value_type( 1.0)) ? get_batch_func_cr<channel,  1>(m)
				 : approximate_equal_lp(m[0][channel], ColorMatrix::value_type(-1.0)) ? get_batch_func_cr<channel, -1>(m)
				                                                                      : get_batch_func_cr<channel,  2>(m);
		}
	};
}

// BatchProcessor

ColorMatrix::BatchProcessor::BatchProcessor(const ColorMatrix &matrix):
	matrix(matrix),
	zero_all(matrix.is_zero()),
	zero_r(matrix.is_zero(0)),
	zero_g(matrix.is_zero(1)),
	zero_b(matrix.is_zero(2)),
	zero_a(matrix.is_zero(3)),
	constant_value(matrix.get_constant()),
	constant_all(matrix.is_constant()),
	constant_r(matrix.is_constant(0)),
	constant_g(matrix.is_constant(1)),
	constant_b(matrix.is_constant(2)),
	constant_a(matrix.is_constant(3)),
	copy_all(matrix.is_copy()),
	copy_r(matrix.is_copy(0)),
	copy_g(matrix.is_copy(1)),
	copy_b(matrix.is_copy(2)),
	copy_a(matrix.is_copy(3)),
	affects_transparent(matrix.is_affects_transparent()),
	transform_func_r(Internal::get_transform_func_c<0>(matrix)),
	transform_func_g(Internal::get_transform_func_c<1>(matrix)),
	transform_func_b(Internal::get_transform_func_c<2>(matrix)),
	transform_func_a(Internal::get_transform_func_c<3>(matrix)),
	batch_func_r(Internal::get_batch_func_c<0>(matrix)),
	batch_func_g(Internal::get_batch_func_c<1>(matrix)),
	batch_func_b(Internal::get_batch_func_c<2>(matrix)),
	batch_func_a(Internal::get_batch_func_c<3>(matrix))
{ }


void
ColorMatrix::BatchProcessor::process(Color *dest, int dest_stride, const Color *src, int src_stride, int width, int height) const
{
	if (width <= 0 || height <= 0) return;
	int dest_dr = dest_stride - width;
	int src_dr = src_stride - width;
	Color *dest_end = dest + dest_stride*height;
	#ifndef NDEBUG
	const Color *src_end = src + src_stride*height;
	#endif

	if (zero_all)
	{
		if (dest_dr)
			for(; dest != dest_end; dest += dest_stride)
				memset(dest, 0, sizeof(*dest)*width);
		else
			memset(dest, 0, sizeof(*dest)*width*height);
	}
	else
	if (copy_all)
	{
		if (dest == src)
		{
			assert(src_stride == dest_stride);
		}
		else
		{
			assert(src_end <= dest || dest_end <= src);
			if (dest_dr || src_dr)
				for(; dest != dest_end; dest += dest_stride, src += src_stride)
					memcpy(dest, src, sizeof(*dest)*width);
			else
				memcpy(dest, src, sizeof(*dest)*width*height);
		}
	}
	else
	if (constant_all)
	{
		if (dest_dr)
			for(; dest != dest_end; dest += dest_dr)
				for(Color *dest_row_end = dest + width; dest < dest_row_end; ++dest)
					*dest = constant_value;
		else
			for(; dest != dest_end; ++dest)
				*dest = constant_value;
	}
	else
	if (dest != src)
	{
		assert(src_end <= dest || dest_end <= src);
		for(; dest != dest_end; dest += dest_stride, src += src_stride)
		{
			const Color *src_end = src + width;
			batch_func_r(matrix, (value_type*)dest + 0, src, src_end);
			batch_func_g(matrix, (value_type*)dest + 1, src, src_end);
			batch_func_b(matrix, (value_type*)dest + 2, src, src_end);
			batch_func_a(matrix, (value_type*)dest + 3, src, src_end);
		}
	}
	else
	{
		assert(src_stride == dest_stride);
		Color c;
		for(; dest != dest_end; dest += dest_dr)
			for(Color *dest_row_end = dest + width; dest < dest_row_end; ++dest)
			{
				c.set_r(transform_func_r(matrix, *dest));
				c.set_g(transform_func_g(matrix, *dest));
				c.set_b(transform_func_b(matrix, *dest));
				c.set_a(transform_func_a(matrix, *dest));
				*dest = c;
			}
	}
}


// ColorMatrix

bool
ColorMatrix::is_constant(int channel) const
{
	return approximate_equal_lp(m[0][channel], value_type(0.0))
		&& approximate_equal_lp(m[1][channel], value_type(0.0))
		&& approximate_equal_lp(m[2][channel], value_type(0.0))
		&& approximate_equal_lp(m[3][channel], value_type(0.0));
}

bool
ColorMatrix::is_constant() const
{
	return is_constant(0)
		&& is_constant(1)
		&& is_constant(2)
		&& is_constant(3);
}

bool
ColorMatrix::is_zero(int channel) const
{
	return is_constant(channel)
	    && approximate_equal_lp(m[4][channel], value_type(0.0));
}

bool
ColorMatrix::is_zero() const
{
	return is_zero(0)
		&& is_zero(1)
		&& is_zero(2)
		&& is_zero(3);
}

bool
ColorMatrix::is_copy(int channel) const
{
	return approximate_equal_lp(m[0][channel], value_type(channel == 0 ? 1.0 : 0.0))
		&& approximate_equal_lp(m[1][channel], value_type(channel == 1 ? 1.0 : 0.0))
		&& approximate_equal_lp(m[2][channel], value_type(channel == 2 ? 1.0 : 0.0))
		&& approximate_equal_lp(m[3][channel], value_type(channel == 3 ? 1.0 : 0.0))
		&& approximate_equal_lp(m[4][channel], value_type(channel == 4 ? 1.0 : 0.0));
}

bool
ColorMatrix::is_copy() const
{
	return is_copy(0)
		&& is_copy(1)
		&& is_copy(2)
		&& is_copy(3);
}

bool ColorMatrix::is_affects_transparent() const
{
	return approximate_equal_lp(m03, value_type(0.0))
		|| approximate_equal_lp(m13, value_type(0.0))
		|| approximate_equal_lp(m23, value_type(0.0))
		|| approximate_equal_lp(m43, value_type(0.0));
}

ColorMatrix&
ColorMatrix::set_scale(value_type r, value_type g, value_type b, value_type a)
{
	m00=r;   m01=0.0; m02=0.0; m03=0.0; m04=0.0;
	m10=0.0; m11=g;   m12=0.0; m13=0.0; m14=0.0;
	m20=0.0; m21=0.0; m22=b;   m23=0.0; m24=0.0;
	m30=0.0; m31=0.0; m32=0.0; m33=a;   m34=0.0;
	m40=0.0; m41=0.0; m42=0.0; m43=0.0; m44=1.0;
	return *this;
}

ColorMatrix&
ColorMatrix::set_translate(value_type r, value_type g, value_type b, value_type a)
{
	m00=1.0; m01=0.0; m02=0.0; m03=0.0; m04=0.0;
	m10=0.0; m11=1.0; m12=0.0; m13=0.0; m14=0.0;
	m20=0.0; m21=0.0; m22=1.0; m23=0.0; m24=0.0;
	m30=0.0; m31=0.0; m32=0.0; m33=1.0; m34=0.0;
	m40=r;   m41=g;   m42=b;   m43=a;   m44=1.0;
	return *this;
}

ColorMatrix&
ColorMatrix::set_encode_yuv()
{
	m00 = 0.299; m01 = -0.168736; m02 =  0.5;      m03=0.0; m04=0.0;
	m10 = 0.587; m11 = -0.331264; m12 = -0.418688; m13=0.0; m14=0.0;
	m20 = 0.114; m21 =  0.5;      m22 = -0.081312; m23=0.0; m24=0.0;
	m30 = 0.0;   m31 =  0.0;      m32 =  0.0;      m33=1.0; m34=0.0;
	m40 = 0.0;   m41 =  0.0;      m42 =  0.0;      m43=0.0; m44=1.0;
	return *this;
}

ColorMatrix&
ColorMatrix::set_decode_yuv()
{
	m00 = 1.0;   m01 =  1.0;      m02 = 1.0;   m03=0.0; m04=0.0;
	m10 = 0.0;   m11 = -0.344136; m12 = 1.772; m13=0.0; m14=0.0;
	m20 = 1.402; m21 = -0.714136; m22 = 0.0;   m23=0.0; m24=0.0;
	m30 = 0.0;   m31 =  0.0;      m32 = 0.0;   m33=1.0; m34=0.0;
	m40 = 0.0;   m41 =  0.0;      m42 = 0.0;   m43=0.0; m44=1.0;
	return *this;
}

ColorMatrix&
ColorMatrix::set_rotate_uv(const Angle &a)
{
	value_type c(Angle::cos(a).get());
	value_type s(Angle::sin(a).get());
	m00 = 1.0; m01 =  0.0; m02 = 0.0; m03=0.0; m04=0.0;
	m10 = 0.0; m11 =  c;   m12 = s;   m13=0.0; m14=0.0;
	m20 = 0.0; m21 = -s;   m22 = c;   m23=0.0; m24=0.0;
	m30 = 0.0; m31 =  0.0; m32 = 0.0; m33=1.0; m34=0.0;
	m40 = 0.0; m41 =  0.0; m42 = 0.0; m43=0.0; m44=1.0;
	return *this;
}

ColorMatrix&
ColorMatrix::set_constant(const Color &c)
{
	m00 = 0.0;       m01 = 0.0;       m02 = 0.0;       m03 = 0.0;       m04 = 0.0;
	m10 = 0.0;       m11 = 0.0;       m12 = 0.0;       m13 = 0.0;       m14 = 0.0;
	m20 = 0.0;       m21 = 0.0;       m22 = 0.0;       m23 = 0.0;       m24 = 0.0;
	m30 = 0.0;       m31 = 0.0;       m32 = 0.0;       m33 = 0.0;       m34 = 0.0;
	m40 = c.get_r(); m41 = c.get_g(); m42 = c.get_b(); m43 = c.get_a(); m44 = 1.0;
	return *this;
}

ColorMatrix&
ColorMatrix::set_replace_color(const Color &c)
{
	m00 = 0.0;       m01 = 0.0;       m02 = 0.0;       m03 = 0.0; m04 = 0.0;
	m10 = 0.0;       m11 = 0.0;       m12 = 0.0;       m13 = 0.0; m14 = 0.0;
	m20 = 0.0;       m21 = 0.0;       m22 = 0.0;       m23 = 0.0; m24 = 0.0;
	m30 = 0.0;       m31 = 0.0;       m32 = 0.0;       m33 = 1.0; m34 = 0.0;
	m40 = c.get_r(); m41 = c.get_g(); m42 = c.get_b(); m43 = 0.0; m44 = 1.0;
	return *this;
}

ColorMatrix&
ColorMatrix::set_replace_alpha(value_type x)
{
	m00 = 1.0; m01 = 0.0; m02 = 0.0; m03 = 0.0; m04 = 0.0;
	m10 = 0.0; m11 = 1.0; m12 = 0.0; m13 = 0.0; m14 = 0.0;
	m20 = 0.0; m21 = 0.0; m22 = 1.0; m23 = 0.0; m24 = 0.0;
	m30 = 0.0; m31 = 0.0; m32 = 0.0; m33 = 0.0; m34 = 0.0;
	m40 = 0.0; m41 = 0.0; m42 = 0.0; m43 = x;   m44 = 1.0;
	return *this;
}

ColorMatrix&
ColorMatrix::set_brightness(value_type x)
	{ return set_translate(x, x, x); }

ColorMatrix&
ColorMatrix::set_contrast(value_type x)
{
	set_translate(-0.5, -0.5, -0.5);
	*this *= ColorMatrix().set_scale(x, x, x);
	*this *= ColorMatrix().set_translate(0.5, 0.5, 0.5);
	return *this;
}

ColorMatrix&
ColorMatrix::set_exposure(value_type x)
	{ return set_scale_rgb(exp(x)); }

ColorMatrix&
ColorMatrix::set_hue_saturation(const Angle &hue, value_type saturation)
{
	set_encode_yuv();
	*this *= ColorMatrix().set_rotate_uv(hue);
	*this *= ColorMatrix().set_scale(1.0, saturation, saturation);
	*this *= ColorMatrix().set_decode_yuv();
	return *this;
}

ColorMatrix&
ColorMatrix::set_invert_color()
{
	m00 = -1.0; m01 =  0.0; m02 =  0.0; m03 = 0.0; m04 = 0.0;
	m10 =  0.0; m11 = -1.0; m12 =  0.0; m13 = 0.0; m14 = 0.0;
	m20 =  0.0; m21 =  0.0; m22 = -1.0; m23 = 0.0; m24 = 0.0;
	m30 =  0.0; m31 =  0.0; m32 =  0.0; m33 = 1.0; m34 = 0.0;
	m40 =  1.0; m41 =  1.0; m42 =  1.0; m43 = 0.0; m44 = 1.0;
	return *this;
}

ColorMatrix&
ColorMatrix::set_invert_alpha()
{
	m00 = 1.0; m01 = 0.0; m02 = 0.0; m03 =  0.0; m04 = 0.0;
	m10 = 0.0; m11 = 1.0; m12 = 0.0; m13 =  0.0; m14 = 0.0;
	m20 = 0.0; m21 = 0.0; m22 = 1.0; m23 =  0.0; m24 = 0.0;
	m30 = 0.0; m31 = 0.0; m32 = 0.0; m33 = -1.0; m34 = 0.0;
	m40 = 0.0; m41 = 0.0; m42 = 0.0; m43 =  1.0; m44 = 1.0;
	return *this;
}

Color
ColorMatrix::get_transformed(Color color) const
{
	Color out;
	out.set_r( color.get_r()*m00 + color.get_g()*m10  + color.get_b()*m20 + color.get_a()*m30 + m40 );
	out.set_g( color.get_r()*m01 + color.get_g()*m11  + color.get_b()*m21 + color.get_a()*m31 + m41 );
	out.set_b( color.get_r()*m02 + color.get_g()*m12  + color.get_b()*m22 + color.get_a()*m32 + m42 );
	out.set_a( color.get_r()*m03 + color.get_g()*m13  + color.get_b()*m23 + color.get_a()*m33 + m43 );
	return out;
}

bool
ColorMatrix::operator==(const ColorMatrix &rhs) const
{
	for(int i = 0; i < 25; ++i)
		if (!approximate_equal_lp(c[i], rhs.c[i]))
			return false;
	return true;
}

ColorMatrix
ColorMatrix::operator*=(const ColorMatrix &rhs)
{
	value_type r, g, b, a, w;

	r = m00; g = m01; b = m02; a = m03; w = m04;
	m00 = r*rhs.m00 + g*rhs.m10 + b*rhs.m20 + a*rhs.m30 + w*rhs.m40;
	m01 = r*rhs.m01 + g*rhs.m11 + b*rhs.m21 + a*rhs.m31 + w*rhs.m41;
	m02 = r*rhs.m02 + g*rhs.m12 + b*rhs.m22 + a*rhs.m32 + w*rhs.m42;
	m03 = r*rhs.m03 + g*rhs.m13 + b*rhs.m23 + a*rhs.m33 + w*rhs.m43;
	m04 = r*rhs.m04 + g*rhs.m14 + b*rhs.m24 + a*rhs.m34 + w*rhs.m44;

	r = m10; g = m11; b = m12; a = m13; w = m14;
	m10 = r*rhs.m00 + g*rhs.m10 + b*rhs.m20 + a*rhs.m30 + w*rhs.m40;
	m11 = r*rhs.m01 + g*rhs.m11 + b*rhs.m21 + a*rhs.m31 + w*rhs.m41;
	m12 = r*rhs.m02 + g*rhs.m12 + b*rhs.m22 + a*rhs.m32 + w*rhs.m42;
	m13 = r*rhs.m03 + g*rhs.m13 + b*rhs.m23 + a*rhs.m33 + w*rhs.m43;
	m14 = r*rhs.m04 + g*rhs.m14 + b*rhs.m24 + a*rhs.m34 + w*rhs.m44;

	r = m20; g = m21; b = m22; a = m23; w = m24;
	m20 = r*rhs.m00 + g*rhs.m10 + b*rhs.m20 + a*rhs.m30 + w*rhs.m40;
	m21 = r*rhs.m01 + g*rhs.m11 + b*rhs.m21 + a*rhs.m31 + w*rhs.m41;
	m22 = r*rhs.m02 + g*rhs.m12 + b*rhs.m22 + a*rhs.m32 + w*rhs.m42;
	m23 = r*rhs.m03 + g*rhs.m13 + b*rhs.m23 + a*rhs.m33 + w*rhs.m43;
	m24 = r*rhs.m04 + g*rhs.m14 + b*rhs.m24 + a*rhs.m34 + w*rhs.m44;

	r = m30; g = m31; b = m32; a = m33; w = m34;
	m30 = r*rhs.m00 + g*rhs.m10 + b*rhs.m20 + a*rhs.m30 + w*rhs.m40;
	m31 = r*rhs.m01 + g*rhs.m11 + b*rhs.m21 + a*rhs.m31 + w*rhs.m41;
	m32 = r*rhs.m02 + g*rhs.m12 + b*rhs.m22 + a*rhs.m32 + w*rhs.m42;
	m33 = r*rhs.m03 + g*rhs.m13 + b*rhs.m23 + a*rhs.m33 + w*rhs.m43;
	m34 = r*rhs.m04 + g*rhs.m14 + b*rhs.m24 + a*rhs.m34 + w*rhs.m44;

	r = m40; g = m41; b = m42; a = m43; w = m44;
	m40 = r*rhs.m00 + g*rhs.m10 + b*rhs.m20 + a*rhs.m30 + w*rhs.m40;
	m41 = r*rhs.m01 + g*rhs.m11 + b*rhs.m21 + a*rhs.m31 + w*rhs.m41;
	m42 = r*rhs.m02 + g*rhs.m12 + b*rhs.m22 + a*rhs.m32 + w*rhs.m42;
	m43 = r*rhs.m03 + g*rhs.m13 + b*rhs.m23 + a*rhs.m33 + w*rhs.m43;
	m44 = r*rhs.m04 + g*rhs.m14 + b*rhs.m24 + a*rhs.m34 + w*rhs.m44;

	return *this;
}

ColorMatrix
ColorMatrix::operator*=(const value_type &rhs)
{
	m00 *= rhs; m01 *= rhs; m02 *= rhs; m03 *= rhs; m04 *= rhs;
	m10 *= rhs; m11 *= rhs; m12 *= rhs; m13 *= rhs; m14 *= rhs;
	m20 *= rhs; m21 *= rhs; m22 *= rhs; m23 *= rhs; m24 *= rhs;
	m30 *= rhs; m31 *= rhs; m32 *= rhs; m33 *= rhs; m34 *= rhs;
	m40 *= rhs; m41 *= rhs; m42 *= rhs; m43 *= rhs; m44 *= rhs;
	return *this;
}

String
ColorMatrix::get_string(int spaces, String before, String after)const
{
	return etl::strprintf(
		"%*s [%7.2f %7.2f %7.2f %7.2f %7.2f] %s\n"
		"%*s [%7.2f %7.2f %7.2f %7.2f %7.2f] %s\n"
		"%*s [%7.2f %7.2f %7.2f %7.2f %7.2f] %s\n"
		"%*s [%7.2f %7.2f %7.2f %7.2f %7.2f] %s\n"
		"%*s [%7.2f %7.2f %7.2f %7.2f %7.2f] %s\n",
		spaces, before.c_str(), m00, m01, m02, m03, m04, after.c_str(),
		spaces, before.c_str(), m10, m11, m12, m13, m14, after.c_str(),
		spaces, before.c_str(), m20, m21, m22, m23, m24, after.c_str(),
		spaces, before.c_str(), m30, m31, m32, m33, m34, after.c_str(),
		spaces, before.c_str(), m40, m41, m42, m43, m44, after.c_str() );
}
