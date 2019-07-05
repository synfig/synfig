/* === S Y N F I G ========================================================= */
/*!	\file
**	\brief Color blending function implementation
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

#ifndef __SYNFIG_COLOR_COLORBLENDINGFUNCTIONS_H
#define __SYNFIG_COLOR_COLORBLENDINGFUNCTIONS_H

#define COLOR_EPSILON	(0.000001f)

#include <synfig/color.h>

namespace synfig {

typedef Color (*blendfunc)(Color &,Color &,float);

template <class C>
C blendfunc_COMPOSITE(C &src,C &dest,float amount)
{
	//c_dest'=c_src+(1.0-a_src)*c_dest
	//a_dest'=a_src+(1.0-a_src)*a_dest

	float a_src=src.get_a()*amount;
	float a_dest=dest.get_a();
	const float one(C::ceil); 

	// if a_arc==0.0
	//if(fabsf(a_src)<COLOR_EPSILON) return dest;

	// Scale the source and destination by their alpha values
	src*=a_src;
	dest*=a_dest;

	dest=src + dest*(one-a_src);

	a_dest=a_src + a_dest*(one-a_src);

	// if a_dest!=0.0
	if(fabsf(a_dest)>COLOR_EPSILON)
	{
		dest/=a_dest;
		dest.set_a(a_dest);
	}
	else
	{
		dest=C::alpha();
	}
	assert(dest.is_valid());
	return dest;
}

template <class C>
C blendfunc_STRAIGHT(C &src,C &bg,float amount)
{
	//a_out'=(a_src-a_bg)*amount+a_bg
	//c_out'=(((c_src*a_src)-(c_bg*a_bg))*amount+(c_bg*a_bg))/a_out'

	// ie: if(amount==1.0)
	//if(fabsf(amount-1.0f)<COLOR_EPSILON)return src;

	C out;

	float a_out((src.get_a()-bg.get_a())*amount+bg.get_a());

	// if a_out!=0.0
	if(fabsf(a_out)>COLOR_EPSILON)
//	if(a_out>COLOR_EPSILON || a_out<-COLOR_EPSILON)
	{
		out=((src*src.get_a()-bg*bg.get_a())*amount+bg*bg.get_a())/a_out;
		out.set_a(a_out);
	}
	else
		out=C::alpha();

	assert(out.is_valid());
	return out;
}

template <class C>
C blendfunc_ONTO(C &a,C &b,float amount)
{
	float alpha(b.get_a());
	const float one(C::ceil);
	return blendfunc_COMPOSITE(a,b.set_a(one),amount).set_a(alpha);
}

template <class C>
C blendfunc_STRAIGHT_ONTO(C &a,C &b,float amount)
{
	a.set_a(a.get_a()*b.get_a());
	return blendfunc_STRAIGHT(a,b,amount);
}

template <class C>
C blendfunc_BRIGHTEN(C &a,C &b,float amount)
{
	const float alpha(a.get_a()*amount);

	if(b.get_r()<a.get_r()*alpha)
		b.set_r(a.get_r()*alpha);

	if(b.get_g()<a.get_g()*alpha)
		b.set_g(a.get_g()*alpha);

	if(b.get_b()<a.get_b()*alpha)
		b.set_b(a.get_b()*alpha);

	return b;
}

template <class C>
C blendfunc_DARKEN(C &a,C &b,float amount)
{
	const float alpha(a.get_a()*amount);
	const float one(C::ceil);
	
	if(b.get_r()>(a.get_r()-one)*alpha+one)
		b.set_r((a.get_r()-one)*alpha+one);

	if(b.get_g()>(a.get_g()-one)*alpha+one)
		b.set_g((a.get_g()-one)*alpha+one);

	if(b.get_b()>(a.get_b()-one)*alpha+one)
		b.set_b((a.get_b()-one)*alpha+one);


	return b;
}

template <class C>
C blendfunc_ADD(C &a,C &b,float amount)
{
	// Color b = background color
	// Color a = color to blend on b

	float ba = b.get_a(); // Alpha from color b
	float aa = a.get_a() * amount; // Alpha from color a (multiplied with the current amount)

	// Calc alpha of the result color
	float alpha_result = ba; // Alpha value is the background pixel alpha value (this crops the outer part of the foreground object)

	// Calc the resulting rgb colors and set alpha
	b.set_r(b.get_r() * ba + a.get_r() * aa);
	b.set_g(b.get_g() * ba + a.get_g() * aa);
	b.set_b(b.get_b() * ba + a.get_b() * aa);
	b.set_a(alpha_result);

	return b;
}

template <class C>
C blendfunc_ADD_COMPOSITE(C &a,C &b,float amount)
{
	float ba(b.get_a());
	float aa(a.get_a()*amount);
	const float alpha(std::max(0.f, std::min(1.f, ba + aa)));
	const float k = fabs(alpha) > 1e-8 ? 1.0/alpha : 0.0;
	aa *= k; ba *= k;

	b.set_r(b.get_r()*ba+a.get_r()*aa);
	b.set_g(b.get_g()*ba+a.get_g()*aa);
	b.set_b(b.get_b()*ba+a.get_b()*aa);
	b.set_a(alpha);

	return b;
}

template <class C>
C blendfunc_SUBTRACT(C &a,C &b,float amount)
{
	// Color b = background color
	// Color a = color to blend on b

	float ba = b.get_a(); // Alpha from color b
	float aa = a.get_a() * amount; // Alpha from color a (multiplied with the current amount)

	// Calc alpha of the result color
	float alpha_result = ba; // Alpha value is the background pixel alpha value (this crops the outer part of the foreground object)

	// Calc the resulting rgb colors and set alpha
	b.set_r(b.get_r() * ba - a.get_r() * aa);
	b.set_g(b.get_g() * ba - a.get_g() * aa);
	b.set_b(b.get_b() * ba - a.get_b() * aa);
	b.set_a(alpha_result);

	return b;
}

template <class C>
C blendfunc_DIFFERENCE(C &a,C &b,float amount)
{
	// Color b = background color
	// Color a = color to blend on b

	float ba = b.get_a(); // Alpha from color b
	float aa = a.get_a() * amount; // Alpha from color a (multiplied with the current amount)

	// Calc alpha of the result color
	float alpha_result = ba; // Alpha value is the background pixel alpha value (this crops the outer part of the foreground object)

	// Calc the resulting rgb colors and set alpha
	b.set_r(std::abs(b.get_r() * ba - a.get_r() * aa));
	b.set_g(std::abs(b.get_g() * ba - a.get_g() * aa));
	b.set_b(std::abs(b.get_b() * ba - a.get_b() * aa));
	b.set_a(alpha_result);

	return b;
}

template <class C>
C blendfunc_MULTIPLY(C &a,C &b,float amount)
{
	if(amount<0) a=~a, amount=-amount;

	amount*=a.get_a();
	b.set_r(((b.get_r()*a.get_r())-b.get_r())*(amount)+b.get_r());
	b.set_g(((b.get_g()*a.get_g())-b.get_g())*(amount)+b.get_g());
	b.set_b(((b.get_b()*a.get_b())-b.get_b())*(amount)+b.get_b());
	return b;
}

template <class C>
C blendfunc_DIVIDE(C &a,C &b,float amount)
{
	amount*=a.get_a();

	// We add COLOR_EPSILON in order to avoid a divide-by-zero condition.
	// This causes DIVIDE to bias toward positive values, but the effect is
	// really negligible. There is a reason why we use COLOR_EPSILON--we
	// want the change to be imperceptible.

	b.set_r(((b.get_r()/(a.get_r()+COLOR_EPSILON))-b.get_r())*(amount)+b.get_r());
	b.set_g(((b.get_g()/(a.get_g()+COLOR_EPSILON))-b.get_g())*(amount)+b.get_g());
	b.set_b(((b.get_b()/(a.get_b()+COLOR_EPSILON))-b.get_b())*(amount)+b.get_b());

	return b;
}

template <class C>
C blendfunc_COLOR(C &a,C &b,float amount)
{
	C temp(b);
	temp.set_uv(a.get_u(),a.get_v());
	return (temp-b)*amount*a.get_a()+b;
}

template <class C>
C blendfunc_HUE(C &a,C &b,float amount)
{
	C temp(b);
	temp.set_hue(a.get_hue());
	return (temp-b)*amount*a.get_a()+b;
}

template <class C>
C blendfunc_SATURATION(C &a,C &b,float amount)
{
	C temp(b);
	temp.set_s(a.get_s());
	return (temp-b)*amount*a.get_a()+b;
}

template <class C>
C blendfunc_LUMINANCE(C &a,C &b,float amount)
{
	C temp(b);
	temp.set_y(a.get_y());
	return (temp-b)*amount*a.get_a()+b;
}

template <class C>
C blendfunc_BEHIND(C &a,C &b,float amount)
{
	if(a.get_a()==0)
		a.set_a(COLOR_EPSILON*amount);		//!< \todo this is a hack
	else
		a.set_a(a.get_a()*amount);
	return blendfunc_COMPOSITE(b,a,1.0);
}

template <class C>
C blendfunc_ALPHA_BRIGHTEN(C &a,C &b,float amount)
{
	// \todo can this be right, multiplying amount by *b*'s alpha?
	// compare with blendfunc_BRIGHTEN where it is multiplied by *a*'s
	if(a.get_a() < b.get_a()*amount)
		return a.set_a(a.get_a()*amount);
	return b;
}

template <class C>
C blendfunc_ALPHA_DARKEN(C &a,C &b,float amount)
{
	if(a.get_a()*amount > b.get_a())
		return a.set_a(a.get_a()*amount);
	return b;
}

template <class C>
C blendfunc_SCREEN(C &a,C &b,float amount)
{
	const float one(C::ceil);
	if(amount<0) a=~a, amount=-amount;

	a.set_r(one-(one-a.get_r())*(one-b.get_r()));
	a.set_g(one-(one-a.get_g())*(one-b.get_g()));
	a.set_b(one-(one-a.get_b())*(one-b.get_b()));

	return blendfunc_ONTO(a,b,amount);
}

template <class C>
C blendfunc_OVERLAY(C &a,C &b,float amount)
{
	const float one(C::ceil);
	if(amount<0) a=~a, amount=-amount;

	C rm;
	rm.set_r(b.get_r()*a.get_r());
	rm.set_g(b.get_g()*a.get_g());
	rm.set_b(b.get_b()*a.get_b());

	C rs;
	rs.set_r(one-(one-a.get_r())*(one-b.get_r()));
	rs.set_g(one-(one-a.get_g())*(one-b.get_g()));
	rs.set_b(one-(one-a.get_b())*(one-b.get_b()));

	C& ret(a);

	ret.set_r(a.get_r()*rs.get_r() + (one-a.get_r())*rm.get_r());
	ret.set_g(a.get_g()*rs.get_g() + (one-a.get_g())*rm.get_g());
	ret.set_b(a.get_b()*rs.get_b() + (one-a.get_b())*rm.get_b());

	return blendfunc_ONTO(ret,b,amount);
}


template <class C>
C blendfunc_HARD_LIGHT(C &a,C &b,float amount)
{
	const float one(C::ceil);
	const float half((one-C::floor)/2);
	if(amount<0) a=~a, amount=-amount;

	if(a.get_r()>half)	a.set_r(one-(one-(a.get_r()*2*one-one))*(one-b.get_r()));
	else				a.set_r(b.get_r()*(a.get_r()*2*one));
	if(a.get_g()>half)	a.set_g(one-(one-(a.get_g()*2*one-one))*(one-b.get_g()));
	else				a.set_g(b.get_g()*(a.get_g()*2*one));
	if(a.get_b()>half)	a.set_b(one-(one-(a.get_b()*2*one-one))*(one-b.get_b()));
	else				a.set_b(b.get_b()*(a.get_b()*2*one));

	return blendfunc_ONTO(a,b,amount);
}

template <class C>
C blendfunc_ALPHA_OVER(C &a,C &b,float amount)
{
	const float one(C::ceil);
	C rm(b);

	//multiply the inverse alpha channel with the one below us
	rm.set_a((one-a.get_a())*b.get_a());

	return blendfunc_STRAIGHT(rm,b,amount);
}

} // synfig namespace


#endif // __SYNFIG_COLOR_COLORBLENDINGFUNCTIONS_H

