/* === S Y N F I G ========================================================= */
/*!	\file synfig/rendering/software/function/fft.h
**	\brief FFT Header
**
**	$Id$
**
**	\legal
**	......... ... 2015 Ivan Mahonin
**
**	This file is part of Synfig.
**
**	Synfig is free software: you can redistribute it and/or modify
**	it under the terms of the GNU General Public License as published by
**	the Free Software Foundation, either version 2 of the License, or
**	(at your option) any later version.
**
**	Synfig is distributed in the hope that it will be useful,
**	but WITHOUT ANY WARRANTY; without even the implied warranty of
**	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
**	GNU General Public License for more details.
**
**	You should have received a copy of the GNU General Public License
**	along with Synfig.  If not, see <https://www.gnu.org/licenses/>.
**	\endlegal
*/
/* ========================================================================= */

/* === S T A R T =========================================================== */

#ifndef __SYNFIG_RENDERING_SOFTWARE_FFT_H
#define __SYNFIG_RENDERING_SOFTWARE_FFT_H

/* === H E A D E R S ======================================================= */

#include "array.h"
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
private:
	class Internal;

public:
	static int get_valid_count(int x);
	static bool is_valid_count(int x);

	static void fft(const Array<Complex, 1> &x, bool invert);
	static void fft2d(const Array<Complex, 2> &x, bool invert, bool do_rows = true, bool do_cols = true);

	static void initialize();
	static void deinitialize();
};

} /* end namespace software */
} /* end namespace rendering */
} /* end namespace synfig */

/* -- E N D ----------------------------------------------------------------- */

#endif
