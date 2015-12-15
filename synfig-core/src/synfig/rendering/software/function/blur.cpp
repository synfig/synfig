/* === S Y N F I G ========================================================= */
/*!	\file synfig/rendering/software/function/blur.cpp
**	\brief Blur
**
**	$Id$
**
**	\legal
**	......... ... 2015 Ivan Mahonin
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

#ifndef WIN32
#include <unistd.h>
#include <sys/types.h>
#include <signal.h>
#endif

#include <cassert>

#include <algorithm>

#include "blur.h"

#include "fft.h"
#include <synfig/angle.h>

#endif

using namespace std;
using namespace synfig;
using namespace rendering;

/* === M A C R O S ========================================================= */

/* === G L O B A L S ======================================================= */

/* === P R O C E D U R E S ================================================= */

/* === M E T H O D S ======================================================= */

Real
software::Blur::gauss(Real x, Real radius)
{
	const Real precision = 1e-10;
	static const Real k = 1.0/sqrt(2.0*PI);
	if (fabs(radius) < precision)
		return fabs(x) < precision ? 1.0 : 0.0;
	return k*exp(-0.5*x*x/(radius*radius))/radius;
}

Real
software::Blur::get_extra_size(rendering::Blur::Type type)
{
	static const synfig::Real min_value = 1.0 / 4096.0;
	static const synfig::Real gauss_size = sqrt(-2.0*log(min_value/gauss(0.0, 1.0)));

	switch(type)
	{
		case rendering::Blur::BOX:
		case rendering::Blur::CROSS:
		case rendering::Blur::DISC:
			return 1.0;
		case rendering::Blur::FASTGAUSSIAN:
		case rendering::Blur::GAUSSIAN:
			return gauss_size;
		default:
			break;
	}
	return 0.0;
}

VectorInt
software::Blur::get_extra_size(
	rendering::Blur::Type type,
	const Vector &size )
{
	const Real precision = 1e-10;
	Vector s = size*get_extra_size(type);
	return VectorInt( (int)ceil(fabs(s[0]) + 0.5 - precision), (int)ceil(fabs(s[1]) + 0.5 - precision) );
}

void
software::Blur::fill_pattern_box(
	Complex *pattern,
	int count,
	int stride,
	Real size )
{
	const Real precision = 1e-10;
	Real s = 0.5 + fabs(size);
	Complex w = 0.5/s;
	int sizei = floor(s - precision);

	// discrete fill
	if (sizei < 1 || sizei > count/2)
	{
		/// assume that pattern already contains zeros
		//for(Complex *x = pattern, *end = x + count*stride; x < end; x += pattern)
		//	*x = 0.0;
	}
	else
	{
		for(Complex *x = pattern, *end = x + sizei*stride; x < end; x += stride)
			*x = w;
		/// assume that pattern already contains zeros
		//for(Complex *x = pattern + (sizei+1)*stride, *end = pattern + (count - sizei)*stride; x < end; x += stride)
		//	*x = 0.0;
		for(Complex *x = pattern + (count - sizei + 1)*stride, *end = pattern + count*stride; x < end; x += stride)
			*x = w;
	}

	// antialiasing
	if (sizei >= 0) {
		Complex aa = w*(1.0 - s + Real(sizei));
		pattern[sizei*stride] = aa;
		if (sizei >= 1) pattern[(count-sizei)*stride] = aa;
	}
}

void
software::Blur::fill_pattern_gauss(
	Complex *pattern,
	int count,
	int stride,
	Real size )
{
	pattern[0] = gauss(0, size);
	Real s = fabs(size);
	for(int i = count/2; i; --i)
		pattern[i*stride] = pattern[(count-i)*stride] = gauss(i, s);
}

void
software::Blur::fill_pattern_2d_disk(
	Complex *pattern,
	int rows,
	int row_stride,
	int cols,
	int col_stride,
	const Vector &size )
{
	const Real precision = 1e-10;
	Vector s(0.5+fabs(size[0]), 0.5+fabs(size[1]));

	VectorInt sizei(
		std::min(cols/2, (int)ceil(s[0] - precision)),
		std::min(rows/2, (int)ceil(s[1] - precision)) );

	Real sum = 0.0;

	Real minr_squared = (s[0]-0.5)*(s[0]-0.5) + (s[1]-0.5)*(s[1]-0.5);
	Real minr = sqrt(minr_squared);
	Real maxr_squared = (s[0]+0.5)*(s[0]+0.5) + (s[1]+0.5)*(s[1]+0.5);
	Real maxr = sqrt(maxr_squared);
	Real delta_r = maxr - minr;
	Real one_div_delta_r = delta_r > precision ? 1.0/delta_r : 0.0;

	// draw one sector
	Vector k(1.0/s[0], 1.0/s[1]);
	Vector pos(0.0, 0.0);
	Vector sumk(1.0, 1.0);
	for(int r = 0; r <= sizei[1]; ++r)
	{
		for(int c = 0; c <= sizei[0]; ++c)
		{
			Complex &x = pattern[r*row_stride + c*col_stride];
			//Real r_squared = pos.mag_squared();
			/// assume that pattern already contains zeros
			//if (r_squared <= 1.0)
			//{
				//if (r_squared <= minr_squared)
					x = 1.0;
				//else
				//	x = (maxr - sqrt(r_squared))*one_div_delta_r;
				sum += sumk[0]*sumk[1]*x.real();
			//}
		}
		sumk[0] = 1.0;
		sumk[1] = 2.0;
		pos[0] = 0.0;
		pos[1] += k[1];
	}

	// normalize sector
	if (sum > precision)
	{
		double nk = 1.0/sum;
		for(int r = 0; r <= sizei[1]; ++r)
			for(int c = 0; c <= sizei[0]; ++c)
				pattern[r*row_stride + c*col_stride].real() *= nk;
	}

	// clone sector
	for(int r = 0; r <= sizei[1]; ++r)
	{
		for(int c = 0; c <= sizei[0]; ++c)
		{
			Complex &x = pattern[r*row_stride + c*col_stride];
			if (c > 0)
				pattern[r*row_stride + (cols - c)*col_stride] = x;
			if (r > 0)
				pattern[(rows - r)*row_stride + c*col_stride] = x;
			if (r > 0 && c > 0)
				pattern[(rows - r)*row_stride + (cols - c)*col_stride] = x;
		}
	}

	/// assume that pattern already contains zeros
	/// and don't touch other cells
}

void
software::Blur::multiply(
	Complex *target,
	int count,
	int stride,
	const Complex *amplfier,
	int amplfier_stride )
{
	const Complex *y = amplfier;
	for(Complex *x = target, *end = x + count*stride; x < end; x += stride, y += amplfier_stride)
		*x *= *y;
}

void
software::Blur::multiply_2d(
	Complex *target,
	int rows,
	int row_stride,
	int cols,
	int col_stride,
	const Complex *amplfier,
	int amplfier_row_stride,
	int amplfier_col_stride )
{
	const Complex *y = amplfier;
	for(Complex *x = target, *end = x + rows*row_stride; x < end; x += row_stride, y += amplfier_row_stride)
		multiply(x, cols, col_stride, y, amplfier_col_stride);
}

void
software::Blur::add(
	Complex *target,
	int count,
	int stride,
	const Complex *amplfier,
	int amplfier_stride )
{
	const Complex *y = amplfier;
	for(Complex *x = target, *end = x + count*stride; x < end; x += stride, y += amplfier_stride)
		*x += *y;
}

void
software::Blur::add_2d(
	Complex *target,
	int rows,
	int row_stride,
	int cols,
	int col_stride,
	const Complex *amplfier,
	int amplfier_row_stride,
	int amplfier_col_stride )
{
	const Complex *y = amplfier;
	for(Complex *x = target, *end = x + rows*row_stride; x < end; x += row_stride, y += amplfier_row_stride)
		add(x, cols, col_stride, y, amplfier_col_stride);
}

void
software::Blur::blur_fft(
	synfig::Surface &dest,
	const RectInt &dest_rect,
	const synfig::Surface &src,
	const VectorInt src_offset,
	rendering::Blur::Type type,
	const Vector &size,
	bool blend,
	Color::BlendMethod blend_method,
	Color::value_type amount )
{
	const Real precision = 1e-10;

	if (blend && fabs(amount) < precision)
		return;

	if ( !dest.is_valid()
	  || !dest_rect.valid()
	  || !src.is_valid() ) return;

	Vector s(fabs(size[0]), fabs(size[1]));

	// rects
	VectorInt offset = dest_rect.get_min() - src_offset;

	VectorInt extra_size = get_extra_size(type, size);
	RectInt dr = dest_rect;
	etl::set_intersect(dr, dr, RectInt(0, 0, dest.get_w(), dest.get_h()));
	if (!dr.valid()) return;

	RectInt sr = dr - offset;
	sr.minx -= extra_size[0];
	sr.miny -= extra_size[1];
	sr.maxx += extra_size[0];
	sr.maxy += extra_size[1];
	if (!sr.valid()) return;
	etl::set_intersect(sr, sr, RectInt(0, 0, src.get_w(), src.get_h()));
	if (!sr.valid()) return;
	etl::set_intersect(dr, dr, sr + offset);
	if (!dr.valid()) return;
	offset -= sr.get_min();

	VectorInt sr_size = sr.get_size();
	VectorInt dr_size = dr.get_size();

	// init
	const int chanels = 4;
	int rows = FFT::get_power_of_two(sr_size[1]);
	int cols = FFT::get_power_of_two(sr_size[0]);
	int col_stride = chanels;
	int row_stride = cols*col_stride;
	vector<Complex> surface;
	vector<Complex> full_pattern;
	vector<Complex> row_pattern;
	vector<Complex> col_pattern;
	bool full = false;
	bool cross = false;

	// convert surface to complex
	surface.resize(rows*row_stride);
	for(int r = 0; r < sr_size[1]; ++r)
	{
		Complex *cmpl = &surface[r*row_stride];
		for(const Color *color = &src[r+sr.miny][sr.minx], *end = color + sr_size[0]; color < end; ++color, cmpl += col_stride)
		{
			Color::value_type a = color->get_a();
			cmpl[0].real() = color->get_r()*a;
			cmpl[1].real() = color->get_g()*a;
			cmpl[2].real() = color->get_b()*a;
			cmpl[3].real() = a;
		}
	}

	// create patterns
	switch(type)
	{
	case rendering::Blur::BOX:
		row_pattern.resize(cols);
		fill_pattern_box(&row_pattern.front(), cols, 1, s[0]);
		col_pattern.resize(rows);
		fill_pattern_box(&col_pattern.front(), rows, 1, s[1]);
		break;
	case rendering::Blur::CROSS:
		row_pattern.resize(cols);
		fill_pattern_box(&row_pattern.front(), cols, 1, s[0]);
		//fill_pattern_gauss(&row_pattern.front(), cols, 1, s[0]);
		col_pattern.resize(rows);
		fill_pattern_box(&col_pattern.front(), rows, 1, s[1]);
		//fill_pattern_gauss(&col_pattern.front(), rows, 1, s[1]);
		cross = true;
		break;
	case rendering::Blur::DISC:
		full_pattern.resize(rows*cols);
		fill_pattern_2d_disk(&full_pattern.front(), rows, cols, cols, 1, s);
		full = true;
		break;
	case rendering::Blur::GAUSSIAN:
	case rendering::Blur::FASTGAUSSIAN:
		row_pattern.resize(cols);
		fill_pattern_gauss(&row_pattern.front(), cols, 1, s[0]);
		col_pattern.resize(rows);
		fill_pattern_gauss(&col_pattern.front(), rows, 1, s[1]);
		break;
	default:
		assert(false);
		break;
	}

	// process

	if (full)
	{
		FFT::fft2d(&full_pattern.front(), rows, cols, cols, 1, false);
		for(int channel = 0; channel < chanels; ++channel)
		{
			FFT::fft2d(&surface.front() + channel, rows, row_stride, cols, col_stride, false);
			multiply_2d(&surface.front() + channel, rows, row_stride, cols, col_stride, &full_pattern.front(), cols, 1);
			FFT::fft2d(&surface.front() + channel, rows, row_stride, cols, col_stride, true);
		}
	}
	else
	{
		vector<Complex> surface_copy;
		vector<Complex> *surface_rows = &surface;
		vector<Complex> *surface_cols = &surface;

		if (cross)
		{
			for(Complex *x = &surface.front() + 3, *end = x + rows*row_stride; x < end; x += col_stride)
				x->real() *= 0.5;
			surface_copy = surface;
			surface_cols = &surface_copy;
		}

		FFT::fft(&row_pattern.front(), cols, 1, false);
		FFT::fft(&col_pattern.front(), rows, 1, false);
		for(int channel = 0; channel < chanels; ++channel)
		{
			for(int r = 0; r < rows; ++r)
			{
				FFT::fft(&surface_rows->front() + r*row_stride + channel, cols, col_stride, false);
				multiply(&surface_rows->front() + r*row_stride + channel, cols, col_stride, &row_pattern.front(), 1);
				FFT::fft(&surface_rows->front() + r*row_stride + channel, cols, col_stride, true);
			}
			for(int c = 0; c < cols; ++c)
			{
				FFT::fft(&surface_cols->front() + c*col_stride + channel, rows, row_stride, false);
				multiply(&surface_cols->front() + c*col_stride + channel, rows, row_stride, &col_pattern.front(), 1);
				FFT::fft(&surface_cols->front() + c*col_stride + channel, rows, row_stride, true);
			}
		}

		if (cross)
			add(&surface.front(), rows*row_stride, 1, &surface_copy.front(), 1);
	}

	// convert surface from complex to color
	if (blend)
	{
		Surface::alpha_pen apen(dest.get_pen(0, 0));
		apen.set_blend_method(blend_method);
		for(int r = 0; r < dr_size[1]; ++r)
		{
			apen.move_to(dr.minx, r + dr.miny);
			for( Complex *cmpl = &surface[(r+offset[1])*row_stride + offset[0]*col_stride], *end = cmpl + dr_size[0]*col_stride;
				 cmpl < end;
				 apen.inc_x(), cmpl += col_stride )
			{
				Real a = abs(cmpl[3]);
				Real one_div_a = fabs(a) < precision ? 0.0 : 1.0/a;
				apen.put_value(
					Color( abs(cmpl[0])*one_div_a, abs(cmpl[1])*one_div_a, abs(cmpl[2])*one_div_a, a ),
					amount );
				apen.inc_x();
			}
		}
	}
	else
	{
		for(int r = 0; r < dr_size[1]; ++r)
		{
			Complex *cmpl = &surface[(r+offset[1])*row_stride + offset[0]*col_stride];
			for(Color *color = &dest[r + dr.miny][dr.minx], *end = color + dr_size[0]; color < end; ++color, cmpl += col_stride)
			{
				Real a = abs(cmpl[3]);
				Real one_div_a = fabs(a) < precision ? 0.0 : 1.0/a;
				color->set_r( abs(cmpl[0])*one_div_a );
				color->set_g( abs(cmpl[1])*one_div_a );
				color->set_b( abs(cmpl[2])*one_div_a );
				color->set_a( a );
			}
		}
	}
}

void
software::Blur::blur(
	synfig::Surface &dest,
	const RectInt &dest_rect,
	const synfig::Surface &src,
	const VectorInt src_offset,
	rendering::Blur::Type type,
	const Vector &size,
	bool blend,
	Color::BlendMethod blend_method,
	Color::value_type amount )
{
	// TODO: special functions for small sizes and for fast-gaussian blur
	blur_fft(dest, dest_rect, src, src_offset, type, size, blend, blend_method, amount);
}

/* === E N T R Y P O I N T ================================================= */
