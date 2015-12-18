/* === S Y N F I G ========================================================= */
/*!	\file synfig/rendering/software/function/fftw.cpp
**	\brief FFTW
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
#include <climits>
//#include <ccomplex>

#include <vector>
#include <set>

#include <fftw3.h>

#include "fftw.h"

#endif

using namespace synfig;
using namespace rendering;

/* === M A C R O S ========================================================= */

/* === G L O B A L S ======================================================= */

/* === P R O C E D U R E S ================================================= */

/* === M E T H O D S ======================================================= */

class software::FFTW::Internal
{
public:
	static std::set<int> counts;
};

std::set<int> software::FFTW::Internal::counts;

void
software::FFTW::initialize()
{
	const int max2 = INT_MAX/2 + 1;
	const int max3 = INT_MAX/3 + 1;
	const int max5 = INT_MAX/5 + 1;
	const int max7 = INT_MAX/7 + 1;
	for(int c2 = 1; c2 < max2; c2 *= 2)
		for(int c3 = c2; c3 < max3; c3 *= 3)
			for(int c5 = c3; c5 < max5; c5 *= 5)
				for(int c7 = c5; c7 < max7; c7 *= 7)
					Internal::counts.insert(c7);
	fftw_set_timelimit(0.0);
}

void
software::FFTW::deinitialize()
{

}

int
software::FFTW::get_valid_count(int x)
{
	if (x > 0)
	{
		std::set<int>::const_iterator i = Internal::counts.upper_bound(x - 1);
		if (i != Internal::counts.end())
			return *i;
	}
	assert(false);
	return 0;
}

bool
software::FFTW::is_valid_count(int x)
{
	return Internal::counts.count(x);
}

void
software::FFTW::multiply(Complex *x, int count, int stride, Real y)
{
	for(Complex *i = x, *end = i + count*stride; i < end; i += stride)
		*i *= y;
}

void
software::FFTW::fft(Complex *x, int count, int stride, bool invert)
{
	if (count == 0 || count == 1) return;

	assert(is_valid_count(count));

	fftw_iodim iodim;
	iodim.n = count;
	iodim.is = stride;
	iodim.os = stride;

    fftw_plan plan = fftw_plan_guru_dft(
    	1, &iodim, 0, NULL,
        (fftw_complex*)x, (fftw_complex*)x,
		invert ? FFTW_BACKWARD : FFTW_FORWARD, FFTW_ESTIMATE );
	fftw_execute(plan);
	fftw_destroy_plan(plan);

	// divide by count to complete back-FFT
	if (invert)
	{
		Real k = 1.0/(Real)count;
		for(Complex *i = x, *end = i + count*stride; i < end; i += stride)
			*i *= k;
	}
}

void
software::FFTW::fft2d(Complex *x, int rows, int row_stride, int cols, int col_stride, bool invert, bool do_rows, bool do_cols)
{
	if (rows == 0 || cols == 0) return;

	assert(is_valid_count(rows) && is_valid_count(cols));

	if (!do_rows && !do_cols) return;

	if (do_rows && do_cols)
	{
		fftw_iodim iodim[2];
		iodim[0].n = cols;
		iodim[0].is = col_stride;
		iodim[0].os = col_stride;
		iodim[1].n = rows;
		iodim[1].is = row_stride;
		iodim[1].os = row_stride;

		fftw_plan plan = fftw_plan_guru_dft(
			2, iodim, 0, NULL,
			(fftw_complex*)x, (fftw_complex*)x,
			invert ? FFTW_BACKWARD : FFTW_FORWARD, FFTW_ESTIMATE );
		fftw_execute(plan);
		fftw_destroy_plan(plan);

		// divide by count to complete back-FFT
		if (invert)
		{
			Real k = 1.0/((Real)cols*(Real)rows);
			for(int i = 0; i < rows; ++i)
				for(Complex *j = x + i*row_stride, *end = j + cols*col_stride; j < end; j += col_stride)
					*j *= k;
		}
	}
	else
	{
		fftw_iodim iodim[2];
		iodim[0].n = cols;
		iodim[0].is = col_stride;
		iodim[0].os = col_stride;
		iodim[1].n = rows;
		iodim[1].is = row_stride;
		iodim[1].os = row_stride;

		fftw_plan plan = fftw_plan_guru_dft(
			1, &iodim[do_rows ? 0 : 1], 1, &iodim[do_rows ? 1 : 0],
			(fftw_complex*)x, (fftw_complex*)x,
			invert ? FFTW_BACKWARD : FFTW_FORWARD, FFTW_ESTIMATE );
		fftw_execute(plan);
		fftw_destroy_plan(plan);

		// divide by count to complete back-FFT
		if (invert)
		{
			Real k = 1.0/(do_rows ? (Real)rows : (Real)cols);
			for(int i = 0; i < rows; ++i)
				for(Complex *j = x + i*row_stride, *end = j + cols*col_stride; j < end; j += col_stride)
					*j *= k;
		}
	}
}

/* === E N T R Y P O I N T ================================================= */
