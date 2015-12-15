/* === S Y N F I G ========================================================= */
/*!	\file synfig/rendering/software/function/fft.h
**	\brief FFT Header
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

/* === S T A R T =========================================================== */

#ifndef __SYNFIG_RENDERING_SOFTWARE_FFT_H
#define __SYNFIG_RENDERING_SOFTWARE_FFT_H

/* === H E A D E R S ======================================================= */

#include <synfig/complex.h>

/* === M A C R O S ========================================================= */

/* === T Y P E D E F S ===================================================== */

/* === C L A S S E S & S T R U C T S ======================================= */

namespace synfig
{
namespace rendering
{
namespace software
{

class FFT
{
public:
	static int get_power_of_two(int x);
	static bool is_power_of_two(int x);

	//! Fast Furier Transform
	//! @count should be power of 2
	//! @stride - distance (in items) between two sequential items
	static void fft(Complex *x, int count, int stride, bool invert);

	//! Fast Furier Transform
	//! @count should be power of 2
	static void fft(Complex *x, int count, bool invert)
		{ fft(x, count, 1, invert); }

	//! Fast Furier Transform 2d
	//! @rows and @cols should be power of 2
	//! @col_stride - distance (in items) between two sequential items
	//! @row_stride - distance (in items) between two sequential rows of items
	static void fft2d(Complex *x, int rows, int row_stride, int cols, int col_stride, bool invert);

	//! Fast Furier Transform 2d
	//! @rows and @cols should be power of 2
	static void fft2d(Complex *x, int rows, int cols, bool invert)
		{ fft2d(x, rows, cols, cols, 1, invert); }
};

} /* end namespace software */
} /* end namespace rendering */
} /* end namespace synfig */

/* -- E N D ----------------------------------------------------------------- */

#endif
