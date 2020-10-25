/* === S Y N F I G ========================================================= */
/*!	\file quick_rng.h
**	\brief Template Header
**
**	\legal
**	Copyright (c) 2002-2005 Robert B. Quattlebaum Jr., Adrian Bentley
**	Copyright (c) 2007 Chris Moore
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

#ifndef __SYNFIG_QUICK_RNG_H
#define __SYNFIG_QUICK_RNG_H

/* === H E A D E R S ======================================================= */

#include <stdint.h>

/* === M A C R O S ========================================================= */

/* === T Y P E D E F S ===================================================== */

/* === C L A S S E S & S T R U C T S ======================================= */

// A fast 32-bit linear congruential random number generator
class quick_rng
{
	uint32_t next;
public:
	quick_rng(uint32_t seed=0):next(seed) { }

	void set_seed(uint32_t x)
	{
		next=x;
	}

	uint32_t i32()
	{
		static const uint32_t a(1664525);
		static const uint32_t c(1013904223);

		return next=next*a+c;
	}

	uint32_t i16()
	{
		return i32()>>16;
	}

	float f()
	{
		static const float m(int(65535));

		return float(i16())/m;
	}

	uint32_t operator()(const uint32_t& m)
	{
		if(m==65536)
			return i16();
		else
		if(m<=65536)
			return i16()%m;
		else
			return i32()%m;
	}
};

/* === E N D =============================================================== */

#endif
