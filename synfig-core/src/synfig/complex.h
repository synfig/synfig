/* === S Y N F I G ========================================================= */
/*!	\file complex.h
**	\brief Provides the synfig::Complex typedef
**
**	\legal
**	Copyright (c) 2002-2005 Robert B. Quattlebaum Jr., Adrian Bentley
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

#ifndef __SYNFIG_COMPLEX_H
#define __SYNFIG_COMPLEX_H

#include <complex>

#include "real.h"

/* === T Y P E D E F S ===================================================== */

namespace synfig {

/*!	\typedef Complex
**	\todo writeme
*/
typedef std::complex<Real> Complex;

}; // END of namespace synfig

/* === E N D =============================================================== */

#endif
