/* === S Y N F I G ========================================================= */
/*!	\file synfig/rendering/software/function/fft.cpp
**	\brief FFT
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

#include <cassert>
#include <climits>
//#include <ccomplex>

#include <mutex>

#include <vector>
#include <set>

#include <fftw3.h>

#include "fft.h"

#endif

using namespace synfig;
using namespace rendering;

/* === M A C R O S ========================================================= */

/* === G L O B A L S ======================================================= */

/* === P R O C E D U R E S ================================================= */

/* === M E T H O D S ======================================================= */

class software::FFT::Internal
{
public:
	static std::set<int> counts;
	static std::mutex mutex;
};

std::set<int> software::FFT::Internal::counts;
std::mutex software::FFT::Internal::mutex;

void
software::FFT::initialize()
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
software::FFT::deinitialize()
{
	Internal::counts.clear();
}

int
software::FFT::get_valid_count(int x)
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
software::FFT::is_valid_count(int x)
{
	return Internal::counts.count(x);
}

void
software::FFT::fft(const Array<Complex, 1> &x, bool invert)
{
	if (x.count == 0 || x.count == 1) return;

	assert(is_valid_count(x.count));

	fftw_iodim iodim;
	iodim.n  = x.count;
	iodim.is = x.stride;
	iodim.os = x.stride;

	{
		std::lock_guard<std::mutex> lock(Internal::mutex);
		fftw_plan plan = fftw_plan_guru_dft(
			1, &iodim, 0, NULL,
			(fftw_complex*)x.pointer, (fftw_complex*)x.pointer,
			invert ? FFTW_BACKWARD : FFTW_FORWARD, FFTW_ESTIMATE );
		fftw_execute(plan);
		fftw_destroy_plan(plan);
	}

	// divide by count to complete back-FFT
	if (invert)
		x.process< std::multiplies<Complex> >( Complex(1.0/(Real)x.count) );
}

void
software::FFT::fft2d(const Array<Complex, 2> &x, bool invert, bool do_rows, bool do_cols)
{
	if (x.count == 0 || x.sub().count == 0) return;
	if ( (!do_cols || x.count == 1)
	  && (!do_rows || x.sub().count == 1) )
		return;

	assert(is_valid_count(x.count) && is_valid_count(x.sub().count));

	if (!do_rows && !do_cols) return;

	fftw_iodim iodim[2];
	iodim[0].n  = x.sub().count;
	iodim[0].is = x.sub().stride;
	iodim[0].os = x.sub().stride;
	iodim[1].n  = x.count;
	iodim[1].is = x.stride;
	iodim[1].os = x.stride;

	{
		std::lock_guard<std::mutex> lock(Internal::mutex);

		fftw_plan plan;
		if (do_rows && do_cols)
		{
			plan = fftw_plan_guru_dft(
				2, iodim, 0, NULL,
				(fftw_complex*)x.pointer, (fftw_complex*)x.pointer,
				invert ? FFTW_BACKWARD : FFTW_FORWARD, FFTW_ESTIMATE );
		}
		else
		{
			plan = fftw_plan_guru_dft(
				1, &iodim[do_rows ? 0 : 1], 1, &iodim[do_rows ? 1 : 0],
				(fftw_complex*)x.pointer, (fftw_complex*)x.pointer,
				invert ? FFTW_BACKWARD : FFTW_FORWARD, FFTW_ESTIMATE );
		}
		fftw_execute(plan);
		fftw_destroy_plan(plan);
	}

	// divide by count to complete back-FFT
	if (invert)
	{
		int count = (do_cols ? x.count : 1)
			      * (do_rows ? x.sub().count : 1);
		x.process< std::multiplies<Complex> >( Complex(1.0/(Real)count) );
	}
}

/* === E N T R Y P O I N T ================================================= */
