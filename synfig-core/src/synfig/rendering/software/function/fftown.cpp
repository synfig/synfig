/* === S Y N F I G ========================================================= */
/*!	\file synfig/rendering/software/function/fftown.cpp
**	\brief FFTOwn
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

#include "fftown.h"

#include <synfig/angle.h>

#endif

using namespace synfig;
using namespace rendering;

/* === M A C R O S ========================================================= */

/* === G L O B A L S ======================================================= */

/* === P R O C E D U R E S ================================================= */

/* === M E T H O D S ======================================================= */

int
software::FFTOwn::get_valid_count(int x)
{
	for(int i = 0; i < (int)sizeof(x)*8; ++i)
		if (x <= 1 << i) return 1 << i;
	assert(false);
	return 0;
}

bool
software::FFTOwn::is_valid_count(int x)
{
	for(int i = 0; i < (int)sizeof(x)*8; ++i)
		if (x == 1 << i) return true;
	return false;
}

void
software::FFTOwn::fft(Complex *x, int count, int stride, bool invert)
{
	if (count == 0 || count == 1) return;

	assert(is_valid_count(count));
	Complex *end = x + count*stride;

	// bit-reversal permutation
	for(int i=0, j=0; i < count; ++i)
	{
		assert(i < count && j < count);
		if (j > i)
			swap(x[i*stride], x[j*stride]);
		int m = count/2;
		while(m >= 1 && j >= m)
			{ j -= m; m /= 2; }
		j += m;
	}

	Real k = invert ? PI : -PI;
	for(int mmax = 1; mmax < count; mmax *= 2)
	{
		// rotation coefficients
		Real angle = k/(Real)mmax;
		Real sn = sin(0.5*angle);
		Complex wp(2.0*sn*sn, sin(angle));

		Complex w(1.0);
		int mmax_x_stride = mmax*stride;
		int mmax_x_stride2 = 2*mmax_x_stride;
		for(int m = 0; m < mmax_x_stride; m += stride)
		{
			// rotate w
			w = Complex( w.real() - w.real()*wp.real() - w.imag()*wp.imag(),
					     w.imag() + w.real()*wp.imag() - w.imag()*wp.real() );
			// process subsequences
			for(Complex *i = x+m, *j = i+mmax_x_stride; i < end; i += mmax_x_stride2, j += mmax_x_stride2)
			{
				// radix
				Complex t = *j*w;
				*j = *i - t;
				*i = *i + t;
			}
		}
	}

	// reverse order
	for(Complex *i = x, *j = end - stride; i < j; i += stride, j -= stride)
		swap(*i, *j);

	// divide by count to complete back-FFT
	if (invert)
	{
		Real k = 1.0/(Real)count;
		for(Complex *i = x; i < end; i += stride)
			*i *= k;
	}
}

void
software::FFTOwn::fft2d(Complex *x, int rows, int row_stride, int cols, int col_stride, bool invert, bool do_rows, bool do_cols)
{
	if (rows == 0 || cols == 0) return;

	assert(is_valid_count(rows) && is_valid_count(cols));

	// fft rows
	if (do_cols && cols > 1)
		for(Complex *r = x, *end = x + rows*row_stride; r < end; r += row_stride)
			fft(r, cols, col_stride, invert);
	// fft cols
	if (do_rows && rows > 1)
		for(Complex *c = x, *end = x + cols*col_stride; c < end; c += col_stride)
			fft(c, rows, row_stride, invert);
}

/* === E N T R Y P O I N T ================================================= */
