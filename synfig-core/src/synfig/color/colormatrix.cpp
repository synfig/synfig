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

#include <cassert>
#include <cstring>
#include <ETL/stringf>
#include <synfig/real.h>

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
		// TODO remove inlining to see if it helps/hurts
		template<int channel, int mode_r, int mode_g, int mode_b, int mode_a, int mode_o>
		static inline ColorMatrix::value_type transform(const ColorMatrix &m, const Color &c)
		{
			ColorMatrix::value_type x(mode_o ? m[4][channel] : ColorMatrix::value_type(0.0)),
			r = 0, g = 0, b = 0, a = 0;

			if (mode_r == -1) r = -c.get_r();
			if (mode_r ==  1) r =  c.get_r();
			if (mode_r ==  2) r =  m[0][channel]*c.get_r();

			if (mode_g == -1) g = -c.get_g();
			if (mode_g ==  1) g =  c.get_g();
			if (mode_g ==  2) g =  m[1][channel]*c.get_g();

			if (mode_b == -1) b = -c.get_b();
			if (mode_b ==  1) b =  c.get_b();
			if (mode_b ==  2) b =  m[2][channel]*c.get_b();

			if (mode_a == -1) a = -c.get_a();
			if (mode_a ==  1) a =  c.get_a();
			if (mode_a ==  2) a =  m[3][channel]*c.get_a();

			return x + r + g + b + a;
		}

		template<int channel, int mode_r, int mode_g, int mode_b, int mode_a, int mode_o>
		static inline void batch_transform(const ColorMatrix &m, ColorMatrix::value_type *dest, const Color *src, const Color *src_end)
		{
			for(; src < src_end; dest += 4, ++src)
				*dest = transform<channel, mode_r, mode_g, mode_b, mode_a, mode_o>(m, *src);
		}


		// transform funcs
		template<int channel, int mode_r, int mode_g, int mode_b, int mode_a>
		static ColorMatrix::value_type transform_crgba(const ColorMatrix &m, const Color &src)
		{
			return approximate_equal_lp(m[4][channel], ColorMatrix::value_type( 0.0)) ? transform<channel, mode_r, mode_g, mode_b, mode_a,  0>(m, src)
				 : approximate_equal_lp(m[4][channel], ColorMatrix::value_type( 1.0)) ? transform<channel, mode_r, mode_g, mode_b, mode_a,  1>(m, src)
				 : approximate_equal_lp(m[4][channel], ColorMatrix::value_type(-1.0)) ? transform<channel, mode_r, mode_g, mode_b, mode_a, -1>(m, src)
																					  : transform<channel, mode_r, mode_g, mode_b, mode_a,  2>(m, src);
		}

		template<int channel, int mode_r, int mode_g, int mode_b>
		static ColorMatrix::value_type transform_crgb(const ColorMatrix &m, const Color &src)
		{
			return approximate_equal_lp(m[3][channel], ColorMatrix::value_type( 0.0)) ? transform_crgba<channel, mode_r, mode_g, mode_b,  0>(m, src)
				 : approximate_equal_lp(m[3][channel], ColorMatrix::value_type( 1.0)) ? transform_crgba<channel, mode_r, mode_g, mode_b,  1>(m, src)
				 : approximate_equal_lp(m[3][channel], ColorMatrix::value_type(-1.0)) ? transform_crgba<channel, mode_r, mode_g, mode_b, -1>(m, src)
																					  : transform_crgba<channel, mode_r, mode_g, mode_b,  2>(m, src);
		}

		template<int channel, int mode_r, int mode_g>
		static ColorMatrix::value_type transform_crg(const ColorMatrix &m, const Color &src)
		{
			return approximate_equal_lp(m[2][channel], ColorMatrix::value_type( 0.0)) ? transform_crgb<channel, mode_r, mode_g,  0>(m, src)
				 : approximate_equal_lp(m[2][channel], ColorMatrix::value_type( 1.0)) ? transform_crgb<channel, mode_r, mode_g,  1>(m, src)
				 : approximate_equal_lp(m[2][channel], ColorMatrix::value_type(-1.0)) ? transform_crgb<channel, mode_r, mode_g, -1>(m, src)
																					  : transform_crgb<channel, mode_r, mode_g,  2>(m, src);
		}

		template<int channel, int mode_r>
		static ColorMatrix::value_type transform_cr(const ColorMatrix &m, const Color &src)
		{
			return approximate_equal_lp(m[1][channel], ColorMatrix::value_type( 0.0)) ? transform_crg<channel, mode_r,  0>(m, src)
				 : approximate_equal_lp(m[1][channel], ColorMatrix::value_type( 1.0)) ? transform_crg<channel, mode_r,  1>(m, src)
				 : approximate_equal_lp(m[1][channel], ColorMatrix::value_type(-1.0)) ? transform_crg<channel, mode_r, -1>(m, src)
																					  : transform_crg<channel, mode_r,  2>(m, src);
		}

		template<int channel>
		static ColorMatrix::value_type transform_c(const ColorMatrix &m, const Color &src)
		{
			return approximate_equal_lp(m[0][channel], ColorMatrix::value_type( 0.0)) ? transform_cr<channel,  0>(m, src)
				 : approximate_equal_lp(m[0][channel], ColorMatrix::value_type( 1.0)) ? transform_cr<channel,  1>(m, src)
				 : approximate_equal_lp(m[0][channel], ColorMatrix::value_type(-1.0)) ? transform_cr<channel, -1>(m, src)
																					  : transform_cr<channel,  2>(m, src);
		}


		// batch funcs
		template<int channel, int mode_r, int mode_g, int mode_b, int mode_a>
		static void batch_crgba(const ColorMatrix &m, ColorMatrix::value_type *dest, const Color *src, const Color *src_end)
		{
			return approximate_equal_lp(m[4][channel], ColorMatrix::value_type( 0.0)) ? batch_transform<channel, mode_r, mode_g, mode_b, mode_a,  0>(m, dest, src, src_end)
				 : approximate_equal_lp(m[4][channel], ColorMatrix::value_type( 1.0)) ? batch_transform<channel, mode_r, mode_g, mode_b, mode_a,  1>(m, dest, src, src_end)
				 : approximate_equal_lp(m[4][channel], ColorMatrix::value_type(-1.0)) ? batch_transform<channel, mode_r, mode_g, mode_b, mode_a, -1>(m, dest, src, src_end)
																					  : batch_transform<channel, mode_r, mode_g, mode_b, mode_a,  2>(m, dest, src, src_end);
		}

		template<int channel, int mode_r, int mode_g, int mode_b>
		static void batch_crgb(const ColorMatrix &m, ColorMatrix::value_type *dest, const Color *src, const Color *src_end)
		{
			return approximate_equal_lp(m[3][channel], ColorMatrix::value_type( 0.0)) ? batch_crgba<channel, mode_r, mode_g, mode_b,  0>(m, dest, src, src_end)
				 : approximate_equal_lp(m[3][channel], ColorMatrix::value_type( 1.0)) ? batch_crgba<channel, mode_r, mode_g, mode_b,  1>(m, dest, src, src_end)
				 : approximate_equal_lp(m[3][channel], ColorMatrix::value_type(-1.0)) ? batch_crgba<channel, mode_r, mode_g, mode_b, -1>(m, dest, src, src_end)
																					  : batch_crgba<channel, mode_r, mode_g, mode_b,  2>(m, dest, src, src_end);
		}

		template<int channel, int mode_r, int mode_g>
		static void batch_crg(const ColorMatrix &m, ColorMatrix::value_type *dest, const Color *src, const Color *src_end)
		{
			return approximate_equal_lp(m[2][channel], ColorMatrix::value_type( 0.0)) ? batch_crgb<channel, mode_r, mode_g,  0>(m, dest, src, src_end)
				 : approximate_equal_lp(m[2][channel], ColorMatrix::value_type( 1.0)) ? batch_crgb<channel, mode_r, mode_g,  1>(m, dest, src, src_end)
				 : approximate_equal_lp(m[2][channel], ColorMatrix::value_type(-1.0)) ? batch_crgb<channel, mode_r, mode_g, -1>(m, dest, src, src_end)
																					  : batch_crgb<channel, mode_r, mode_g,  2>(m, dest, src, src_end);
		}

		template<int channel, int mode_r>
		static void batch_cr(const ColorMatrix &m, ColorMatrix::value_type *dest, const Color *src, const Color *src_end)
		{
			return approximate_equal_lp(m[1][channel], ColorMatrix::value_type( 0.0)) ? batch_crg<channel, mode_r,  0>(m, dest, src, src_end)
				 : approximate_equal_lp(m[1][channel], ColorMatrix::value_type( 1.0)) ? batch_crg<channel, mode_r,  1>(m, dest, src, src_end)
				 : approximate_equal_lp(m[1][channel], ColorMatrix::value_type(-1.0)) ? batch_crg<channel, mode_r, -1>(m, dest, src, src_end)
																					  : batch_crg<channel, mode_r,  2>(m, dest, src, src_end);
		}

		template<int channel>
		static void batch_c(const ColorMatrix &m, ColorMatrix::value_type *dest, const Color *src, const Color *src_end)
		{
			return approximate_equal_lp(m[0][channel], ColorMatrix::value_type( 0.0)) ? batch_cr<channel,  0>(m, dest, src, src_end)
				 : approximate_equal_lp(m[0][channel], ColorMatrix::value_type( 1.0)) ? batch_cr<channel,  1>(m, dest, src, src_end)
				 : approximate_equal_lp(m[0][channel], ColorMatrix::value_type(-1.0)) ? batch_cr<channel, -1>(m, dest, src, src_end)
																					  : batch_cr<channel,  2>(m, dest, src, src_end);
		}
	};
}

// BatchProcessor

ColorMatrix::BatchProcessor::BatchProcessor(const ColorMatrix &matrix):
	matrix(matrix),
	zero_all(matrix.is_zero()),
	constant_value(matrix.get_constant()),
	constant_all(matrix.is_constant()),
	copy_all(matrix.is_copy()),
	affects_transparent(matrix.is_affects_transparent())
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
			Internal::batch_c<0>(matrix, (value_type*)dest + 0, src, src_end);
			Internal::batch_c<1>(matrix, (value_type*)dest + 1, src, src_end);
			Internal::batch_c<2>(matrix, (value_type*)dest + 2, src, src_end);
			Internal::batch_c<3>(matrix, (value_type*)dest + 3, src, src_end);
		}
	}
	else
	{
		assert(src_stride == dest_stride);
		Color c;
		for(; dest != dest_end; dest += dest_dr)
			for(Color *dest_row_end = dest + width; dest < dest_row_end; ++dest)
			{
				c.set_r(Internal::transform_c<0>(matrix, *dest));
				c.set_g(Internal::transform_c<1>(matrix, *dest));
				c.set_b(Internal::transform_c<2>(matrix, *dest));
				c.set_a(Internal::transform_c<3>(matrix, *dest));
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
	const value_type r1 = m00, g1 = m01, b1 = m02, a1 = m03, w1 = m04;
	const value_type r2 = m10, g2 = m11, b2 = m12, a2 = m13, w2 = m14;
	const value_type r3 = m20, g3 = m21, b3 = m22, a3 = m23, w3 = m24;
	const value_type r4 = m30, g4 = m31, b4 = m32, a4 = m33, w4 = m34;
	const value_type r5 = m40, g5 = m41, b5 = m42, a5 = m43, w5 = m44;

	m00 = r1*rhs.m00 + g1*rhs.m10 + b1*rhs.m20 + a1*rhs.m30 + w1*rhs.m40;
	m01 = r1*rhs.m01 + g1*rhs.m11 + b1*rhs.m21 + a1*rhs.m31 + w1*rhs.m41;
	m02 = r1*rhs.m02 + g1*rhs.m12 + b1*rhs.m22 + a1*rhs.m32 + w1*rhs.m42;
	m03 = r1*rhs.m03 + g1*rhs.m13 + b1*rhs.m23 + a1*rhs.m33 + w1*rhs.m43;
	m04 = r1*rhs.m04 + g1*rhs.m14 + b1*rhs.m24 + a1*rhs.m34 + w1*rhs.m44;

	m10 = r2*rhs.m00 + g2*rhs.m10 + b2*rhs.m20 + a2*rhs.m30 + w2*rhs.m40;
	m11 = r2*rhs.m01 + g2*rhs.m11 + b2*rhs.m21 + a2*rhs.m31 + w2*rhs.m41;
	m12 = r2*rhs.m02 + g2*rhs.m12 + b2*rhs.m22 + a2*rhs.m32 + w2*rhs.m42;
	m13 = r2*rhs.m03 + g2*rhs.m13 + b2*rhs.m23 + a2*rhs.m33 + w2*rhs.m43;
	m14 = r2*rhs.m04 + g2*rhs.m14 + b2*rhs.m24 + a2*rhs.m34 + w2*rhs.m44;

	m20 = r3*rhs.m00 + g3*rhs.m10 + b3*rhs.m20 + a3*rhs.m30 + w3*rhs.m40;
	m21 = r3*rhs.m01 + g3*rhs.m11 + b3*rhs.m21 + a3*rhs.m31 + w3*rhs.m41;
	m22 = r3*rhs.m02 + g3*rhs.m12 + b3*rhs.m22 + a3*rhs.m32 + w3*rhs.m42;
	m23 = r3*rhs.m03 + g3*rhs.m13 + b3*rhs.m23 + a3*rhs.m33 + w3*rhs.m43;
	m24 = r3*rhs.m04 + g3*rhs.m14 + b3*rhs.m24 + a3*rhs.m34 + w3*rhs.m44;

	m30 = r4*rhs.m00 + g4*rhs.m10 + b4*rhs.m20 + a4*rhs.m30 + w4*rhs.m40;
	m31 = r4*rhs.m01 + g4*rhs.m11 + b4*rhs.m21 + a4*rhs.m31 + w4*rhs.m41;
	m32 = r4*rhs.m02 + g4*rhs.m12 + b4*rhs.m22 + a4*rhs.m32 + w4*rhs.m42;
	m33 = r4*rhs.m03 + g4*rhs.m13 + b4*rhs.m23 + a4*rhs.m33 + w4*rhs.m43;
	m34 = r4*rhs.m04 + g4*rhs.m14 + b4*rhs.m24 + a4*rhs.m34 + w4*rhs.m44;

	m40 = r5*rhs.m00 + g5*rhs.m10 + b5*rhs.m20 + a5*rhs.m30 + w5*rhs.m40;
	m41 = r5*rhs.m01 + g5*rhs.m11 + b5*rhs.m21 + a5*rhs.m31 + w5*rhs.m41;
	m42 = r5*rhs.m02 + g5*rhs.m12 + b5*rhs.m22 + a5*rhs.m32 + w5*rhs.m42;
	m43 = r5*rhs.m03 + g5*rhs.m13 + b5*rhs.m23 + a5*rhs.m33 + w5*rhs.m43;
	m44 = r5*rhs.m04 + g5*rhs.m14 + b5*rhs.m24 + a5*rhs.m34 + w5*rhs.m44;

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
