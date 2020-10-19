/* === S Y N F I G ========================================================= */
/*!	\file interpolation.h
**	\brief Template Header
**
**	$Id$
**
**	\legal
**	Copyright (c) 2002-2005 Robert B. Quattlebaum Jr., Adrian Bentley
**	Copyright (c) 2007, 2008 Chris Moore
**	Copyright (c) 2012, Carlos López
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

#ifndef __SYNFIG_INTERPOLATION_H
#define __SYNFIG_INTERPOLATION_H

/* === H E A D E R S ======================================================= */

/* === M A C R O S ========================================================= */

/* === T Y P E D E F S ===================================================== */

/* === C L A S S E S & S T R U C T S ======================================= */

namespace synfig {

enum Interpolation
{
	INTERPOLATION_TCB,          // 0
	INTERPOLATION_CONSTANT,     // 1
	INTERPOLATION_LINEAR,       // 2
	INTERPOLATION_HALT,         // 3
	INTERPOLATION_MANUAL,       // 4
	INTERPOLATION_UNDEFINED,    // 5
	INTERPOLATION_NIL,          // 6
	INTERPOLATION_CLAMPED       // 7
}; // END enum Interpolation

}; // END of namespace synfig

/* === E N D =============================================================== */

#endif
