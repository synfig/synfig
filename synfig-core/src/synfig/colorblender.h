/* === S Y N F I G ========================================================= */
/*!	\file colorblender.h
**	\brief Generic color blending class
**
**	$Id$
**
**	\legal
**	Copyright (c) 2002-2005 Robert B. Quattlebaum Jr., Adrian Bentley
**	Copyright (c) 2007, 2008 Chris Moore
**	Copyright (c) 2012 Diego Barrios Romero
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

#ifndef __COLORBLENDER_H
#define __COLORBLENDER_H

namespace synfig {

class ColorBlender
{
private:
	ColorBlender() {}
public:

	template <class C>
	static C blendfunc_COMPOSITE(C &src,C &dest,float amount);

	template <class C>
	static C blendfunc_STRAIGHT(C &src,C &bg,float amount);

	template <class C>
	static C blendfunc_ONTO(C &a,C &b,float amount);

	template <class C>
	static C blendfunc_STRAIGHT_ONTO(C &a,C &b,float amount);

	template <class C>
	static C blendfunc_BRIGHTEN(C &a,C &b,float amount);

	template <class C>
	static C blendfunc_DARKEN(C &a,C &b,float amount);

	template <class C>
	static C blendfunc_ADD(C &a,C &b,float amount);

	template <class C>
	static C blendfunc_SUBTRACT(C &a,C &b,float amount);

	template <class C>
	static C blendfunc_DIFFERENCE(C &a,C &b,float amount);

	template <class C>
	static C blendfunc_MULTIPLY(C &a,C &b,float amount);

	template <class C>
	static C blendfunc_DIVIDE(C &a,C &b,float amount);

	template <class C>
	static C blendfunc_COLOR(C &a,C &b,float amount);

	template <class C>
	static C blendfunc_HUE(C &a,C &b,float amount);

	template <class C>
	static C blendfunc_SATURATION(C &a,C &b,float amount);

	template <class C>
	static C blendfunc_LUMINANCE(C &a,C &b,float amount);

	template <class C>
	static C blendfunc_BEHIND(C &a,C &b,float amount);

	template <class C>
	static C blendfunc_ALPHA_BRIGHTEN(C &a,C &b,float amount);

	template <class C>
	static C blendfunc_ALPHA_DARKEN(C &a,C &b,float amount);

	template <class C>
	static C blendfunc_SCREEN(C &a,C &b,float amount);

	template <class C>
	static C blendfunc_OVERLAY(C &a,C &b,float amount);

	template <class C>
	static C blendfunc_HARD_LIGHT(C &a,C &b,float amount);

	template <class C>
	static C blendfunc_ALPHA_OVER(C &a,C &b,float amount);

	template <class C>
	static C blend(C a, C b, float amount, Color::BlendMethod type);

};

#include "colorblender.cpp"

} /* namespace synfig */

#endif /* __COLORBLENDER_H */
