/* === S Y N F I G ========================================================= */
/*!	\file
**	\brief CairoColor class implementation
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

/* === H E A D E R S ======================================================= */

#ifdef USING_PCH
#	include "pch.h"
#else
#ifdef HAVE_CONFIG_H
#	include <config.h>
#endif

#include <ETL/angle>
#include "color.h"
#include "cairocolor.h"
#include <cstdio>
#include <sstream>
#include <iostream>
#include <iomanip>
#include "cairocolorblendingfunctions.h"

#endif

namespace synfig {

const float CairoColor::range = CairoColor::ceil - CairoColor::floor;

/* === M E T H O D S ======================================================= */

unsigned char
CairoColor::hex2char(String s)
{
	ColorReal cr(Color::hex2real(s));
	return (unsigned char)(cr*255.0f);
}


const String
CairoColor::char2hex(unsigned char c)
{
	String s(Color::real2hex((ColorReal)(c/((float)ceil))));
	return s.c_str();
}

void 
CairoColor::set_hex(String& str)
{
	CairoColor ret(*this);
	Color c;
	c.set_hex(str);
	c=c.clamped();
	ret=CairoColor(c);
}


const String
CairoColor::get_string(void)const
{
	std::ostringstream o;
	o << std::fixed << std::setprecision(3) << "#" << get_hex().c_str() << " : " << std::setw(6) << get_a();
	return String(o.str().c_str());
}

CairoColor
CairoColor::blend(CairoColor a, CairoColor b, float amount, Color::BlendMethod type)
{
	// No matter what blend method is being used,
	// if the amount is equal to zero, then only B
	// will shine through
	if(fabsf(amount)<=COLOR_EPSILON)return b;

	assert(type<Color::BLEND_END);

	const static cairoblendfunc vtable[Color::BLEND_END]=
	{
        // WARNING: any change here must be coordinated with
        // other specializations of the functions.
		blendfunc_COMPOSITE<CairoColor>,	// 0
		blendfunc_STRAIGHT<CairoColor>,
		blendfunc_BRIGHTEN<CairoColor>,
		blendfunc_DARKEN<CairoColor>,
		blendfunc_ADD<CairoColor>,
		blendfunc_SUBTRACT<CairoColor>,		// 5
		blendfunc_MULTIPLY<CairoColor>,
		blendfunc_DIVIDE<CairoColor>,
		blendfunc_COLOR<CairoColor>,
		blendfunc_HUE<CairoColor>,
		blendfunc_SATURATION<CairoColor>,	// 10
		blendfunc_LUMINANCE<CairoColor>,
		blendfunc_BEHIND<CairoColor>,
		blendfunc_ONTO<CairoColor>,
		blendfunc_ALPHA_BRIGHTEN<CairoColor>,
		blendfunc_ALPHA_DARKEN<CairoColor>,	// 15
		blendfunc_SCREEN<CairoColor>,
		blendfunc_HARD_LIGHT<CairoColor>,
		blendfunc_DIFFERENCE<CairoColor>,
		blendfunc_ALPHA_OVER<CairoColor>,
		blendfunc_OVERLAY<CairoColor>,		// 20
		blendfunc_STRAIGHT_ONTO<CairoColor>,
		blendfunc_STRAIGHT_ONTO<CairoColor>, // dummy instead of new blend method
		blendfunc_STRAIGHT_ONTO<CairoColor>  // dummy instead of new blend method
	};

	return vtable[type](a,b,amount);
}

} // synfig namespace

