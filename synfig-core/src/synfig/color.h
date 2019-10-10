/* === S Y N F I G ========================================================= */
/*!	\file color.h
**	\brief Color Classes
**
**	$Id$
**
**	\legal
**	Copyright (c) 2002-2005 Robert B. Quattlebaum Jr., Adrian Bentley
**	Copyright (c) 2007, 2008 Chris Moore
**	Copyright (c) 2012-2013 Carlos López
**	Copyright (c) 2015 Diego Barrios Romero
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

#ifndef __SYNFIG_COLOR_H
#define __SYNFIG_COLOR_H

/* === H E A D E R S ======================================================= */

#include <synfig/color/common.h>
#include <synfig/color/color.h>
#include <synfig/color/cairocolor.h>
#include <synfig/color/cairocoloraccumulator.h>
#include <synfig/color/gamma.h>

namespace synfig {

inline Color::Color(const CairoColor& c)
{
	float div=1.0/((float)(CairoColor::ceil-CairoColor::floor));
	set_r((ceil-floor)*c.get_r()*div);
	set_g((ceil-floor)*c.get_g()*div);
	set_b((ceil-floor)*c.get_b()*div);
	set_a((ceil-floor)*c.get_a()*div);
}


inline CairoColor::CairoColor(const CairoColorAccumulator& c)
{
    set_a(CairoColor::clamp(c.a_*CairoColor::range));
    set_r(CairoColor::clamp(c.r_*CairoColor::range));
    set_g(CairoColor::clamp(c.g_*CairoColor::range));
    set_b(CairoColor::clamp(c.b_*CairoColor::range));
}

}

namespace synfig {
typedef Color ColorAccumulator;
}

#include <synfig/color/pixelformat.h>

#endif // __SYNFIG_COLOR_H

