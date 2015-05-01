/* === S Y N F I G ========================================================= */
/*!	\file
**	\brief Color blending function specializations for CairoColor
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

#ifndef __SYNFIG_COLOR_CAIROCOLORBLENDINGFUNCTIONS_H
#define __SYNFIG_COLOR_CAIROCOLORBLENDINGFUNCTIONS_H

#include "colorblendingfunctions.h"

namespace synfig {

typedef CairoColor (*cairoblendfunc)(CairoColor&, CairoColor&, float);

template <>
CairoColor
blendfunc_COMPOSITE(CairoColor &a, CairoColor &b, float amount)
{
	int ra, ga, ba, aa;
	int rb, gb, bb, ab;
	int rc, gc, bc;
	float ac;
	
	float faa, fab, A, AA;
	
	ra=a.get_r();
	ga=a.get_g();
	ba=a.get_b();
	aa=a.get_a();
	aa=aa*amount;
	A=aa/255.0;
	AA=1.0-A;
	
	rb=b.get_r();
	gb=b.get_g();
	bb=b.get_b();
	ab=b.get_a();

	ac=aa+ab*AA;
	if(fabsf(ac)<COLOR_EPSILON)
		return CairoColor::alpha();

	faa=aa/ac;
	fab=ab*AA/ac;

	rc=ra*faa+rb*fab;
	gc=ga*faa+gb*fab;
	bc=ba*faa+bb*fab;
	
	return CairoColor(rc, gc, bc, ac);

}

template <>
CairoColor
blendfunc_STRAIGHT(CairoColor &a, CairoColor &b, float amount)
{	
	int ra, ga, ba, aa; //src
	int rb, gb, bb, ab; //bg
	int rc, gc, bc;
	float ac; //out
	
	ra=a.get_r();
	ga=a.get_g();
	ba=a.get_b();
	aa=a.get_a();
	
	rb=b.get_r();
	gb=b.get_g();
	bb=b.get_b();
	ab=b.get_a();
	
	ac=(aa-ab)*amount+ab;
	
	// if ac!=0.0
	if(fabsf(ac)>COLOR_EPSILON)
	{
		rc= ((ra*aa-rb*ab)*amount + rb*ab)/ac;
		gc= ((ga*aa-gb*ab)*amount + gb*ab)/ac;
		bc= ((ba*aa-bb*ab)*amount + bb*ab)/ac;
		return CairoColor(rc, gc, bc, ac);
	}
	else
		return CairoColor::alpha();
}

template <>
CairoColor
blendfunc_ONTO(CairoColor &a, CairoColor &b, float amount)
{
	unsigned char alpha(b.get_a());
	return blendfunc_COMPOSITE(a,b.set_a(255),amount).set_a(alpha);
}

template <>
CairoColor
blendfunc_STRAIGHT_ONTO(CairoColor &a, CairoColor &b, float amount)
{
	a.set_a(a.get_a()*b.get_a()/255.0);
	return CairoColor::blend(a, b, amount, Color::BLEND_STRAIGHT);
}

template <>
CairoColor
blendfunc_BRIGHTEN(CairoColor &a, CairoColor &b, float amount)
{
	int ra, ga, ba, aa;
	int rb, gb, bb, ab;
	int rc, gc, bc, ac;
	
	ra=a.get_r();
	ga=a.get_g();
	ba=a.get_b();
	aa=a.get_a();
	
	rb=b.get_r();
	gb=b.get_g();
	bb=b.get_b();
	ab=b.get_a();
	
	const float alpha = aa*amount/255.0;
	const int raab(ra*alpha);
	const int gaab(ga*alpha);
	const int baab(ba*alpha);
	
	if(rb<raab)
		rc=raab;
	else
		rc=rb;
		
	if(gb<gaab)
		gc=gaab;
	else
		gc=gb;
	
	if(bb<baab)
		bc=baab;
	else
		bc=bb;

	ac=ab;
		
	return CairoColor(rc, gc, bc, ac);
}

template <>
CairoColor
blendfunc_DARKEN(CairoColor &a, CairoColor &b, float amount)
{
	int ra, ga, ba, aa;
	int rb, gb, bb, ab;
	int rc, gc, bc, ac;
	
	ra=a.get_r();
	ga=a.get_g();
	ba=a.get_b();
	aa=a.get_a();
	
	rb=b.get_r();
	gb=b.get_g();
	bb=b.get_b();
	ab=b.get_a();
	
	const float alpha=aa*amount/255.0;

	int rcompare=(ra-255)*alpha+255;
	if(rb > rcompare)
		rc=rcompare;
	else
		rc=rb;
		
	int gcompare=(ga-255)*alpha+255;
	if(gb > gcompare)
		gc=gcompare;
	else
		gc=gb;

	int bcompare=(ba-255)*alpha+255;
	if(bb > bcompare)
		bc=bcompare;
	else
		bc=bb;
	
	ac=ab;
	
	return CairoColor(rc, gc, bc, ac);
}

template <>
CairoColor
blendfunc_ADD(CairoColor &a, CairoColor &b, float amount)
{
	int ra, ga, ba, aa;
	int rb, gb, bb, ab;
	int rc, gc, bc, ac;
	
	ra=a.get_r();
	ga=a.get_g();
	ba=a.get_b();
	aa=a.get_a();
	
	rb=b.get_r();
	gb=b.get_g();
	bb=b.get_b();
	ab=b.get_a();
	
	const float aaa=aa*amount/255.0;
	
	rc=rb+ra*aaa;
	gc=gb+ga*aaa;
	bc=bb+ba*aaa;
	ac=ab;

	return CairoColor(rc, gc, bc, ac);
}

//Specialization for CairoColor
template <>
CairoColor
blendfunc_SUBTRACT(CairoColor &a, CairoColor &b, float amount)
{
	int ra, ga, ba, aa;
	int rb, gb, bb, ab;
	int rc, gc, bc, ac;
	
	ra=a.get_r();
	ga=a.get_g();
	ba=a.get_b();
	aa=a.get_a();
	
	rb=b.get_r();
	gb=b.get_g();
	bb=b.get_b();
	ab=b.get_a();
	
	const float aaa=aa*amount/255.0;
	
	rc=rb-ra*aaa;
	gc=gb-ga*aaa;
	bc=bb-ba*aaa;
	ac=ab;
	
	return CairoColor(rc, gc, bc, ac);
}

template <>
CairoColor
blendfunc_DIFFERENCE(CairoColor &a, CairoColor &b, float amount)
{
	int ra, ga, ba, aa;
	int rb, gb, bb, ab;
	int rc, gc, bc, ac;
	
	ra=a.get_r();
	ga=a.get_g();
	ba=a.get_b();
	aa=a.get_a();
	
	rb=b.get_r();
	gb=b.get_g();
	bb=b.get_b();
	ab=b.get_a();
	
	const float aaa=aa*amount/255.0;
	
	rc=std::abs(rb-ra*aaa);
	gc=std::abs(gb-ga*aaa);
	bc=std::abs(bb-ba*aaa);
	ac=ab;
	
	return CairoColor(rc, gc, bc, ac);
}

template <>
CairoColor
blendfunc_MULTIPLY(CairoColor &a,CairoColor &b, float amount)
{
	if(amount<0) a=~a, amount=-amount;
	amount*=a.get_a()/255.0;
	int ra, ga, ba;
	int rb, gb, bb;

	ra=a.get_r();
	ga=a.get_g();
	ba=a.get_b();
	
	rb=b.get_r();
	gb=b.get_g();
	bb=b.get_b();
	
	b.set_r((rb*ra*amount/255.0)+rb*(1.0-amount));
	b.set_g((gb*ga*amount/255.0)+gb*(1.0-amount));
	b.set_b((bb*ba*amount/255.0)+bb*(1.0-amount));
	return b;
}

template <>
CairoColor
blendfunc_DIVIDE(CairoColor &a, CairoColor &b, float amount)
{
	int ra, ga, ba, aa;
	int rb, gb, bb, ab;
	int rc, gc, bc, ac;

	ra=a.get_r();
	ga=a.get_g();
	ba=a.get_b();
	aa=a.get_a();
	
	rb=b.get_r();
	gb=b.get_g();
	bb=b.get_b();
	ab=b.get_a();
	
	const float alpha=amount*aa/255.0;
	const float ahpla=1.0-alpha;
	
	if(alpha<COLOR_EPSILON)
		return b;
	
	ac=ab;
	if(ra==0)
		rc=rb;
	else
		rc=rb*(alpha*255)/(ra) + ahpla*rb;
		
	if(ga==0)
		gc=gb;
	else
		gc=gb*(alpha*255)/(ga) + ahpla*gb;
		
	if(ba==0)
		bc=bb;
	else
		bc=bb*(alpha*255)/(ba) + ahpla*bb;
		
	return CairoColor(rc, gc, bc, ac);
}

template <>
CairoColor
blendfunc_COLOR(CairoColor &a, CairoColor &b, float amount)
{
	return CairoColor(Color::blend(Color(a), Color(b), amount, Color::BLEND_COLOR));
}

template <>
CairoColor
blendfunc_HUE(CairoColor &a, CairoColor &b, float amount)
{
	return CairoColor(Color::blend(Color(a), Color(b), amount, Color::BLEND_HUE));
}

template <>
CairoColor
blendfunc_SATURATION(CairoColor &a, CairoColor &b, float amount)
{
	return CairoColor(Color::blend(Color(a), Color(b), amount, Color::BLEND_SATURATION));
}

template <>
CairoColor
blendfunc_LUMINANCE(CairoColor &a, CairoColor &b, float amount)
{
	return CairoColor(Color::blend(Color(a), Color(b), amount, Color::BLEND_LUMINANCE));
}

template <>
CairoColor
blendfunc_BEHIND(CairoColor &a, CairoColor &b, float amount)
{
	a.set_a(a.get_a()*amount);
	return CairoColor::blend(b, a, 1.0, Color::BLEND_COMPOSITE);
}

template <>
CairoColor
blendfunc_ALPHA_BRIGHTEN(CairoColor &a, CairoColor &b, float amount)
{
	// \todo can this be right, multiplying amount by *b*'s alpha?
	// compare with blendfunc_BRIGHTEN where it is multiplied by *a*'s
	//if(a.get_a() < b.get_a()*amount)
	//	return a.set_a(a.get_a()*amount);
	//return b;
	unsigned char ra, ga, ba, aa;
	unsigned char ab;
	unsigned char rc, gc, bc, ac;
	
	ra=a.get_r();
	ga=a.get_g();
	ba=a.get_b();
	aa=a.get_a();
	
	ab=b.get_a();
	
	ac=aa*amount;
	if(aa < ab*amount)
	{
		float acaa=(aa*amount)/aa;
		rc=ra*acaa;
		gc=ga*acaa;
		bc=ba*acaa;
		return CairoColor(rc, gc, bc, ac);
	}
	else
		return b;	
}

template <>
CairoColor
blendfunc_ALPHA_DARKEN(CairoColor &a, CairoColor &b, float amount)
{
	unsigned char ra, ga, ba, aa;
	unsigned char ab;
	unsigned char rc, gc, bc, ac;
	
	ra=a.get_r();
	ga=a.get_g();
	ba=a.get_b();
	aa=a.get_a();
	
	ab=b.get_a();
	
	ac=aa*amount;
	if(ac > ab)
	{
		float acaa=(aa*amount)/aa;
		rc=ra*acaa;
		gc=ga*acaa;
		bc=ba*acaa;
		return CairoColor(rc, gc, bc, ac);
	}
	else
		return b;
}

template <>
CairoColor
blendfunc_SCREEN(CairoColor &a, CairoColor &b, float amount)
{
	if(amount<0) a=~a, amount=-amount;
	
	a.set_r(255-(255-a.get_r())*(1.0-b.get_r()/255.0));
	a.set_g(255-(255-a.get_g())*(1.0-b.get_g()/255.0));
	a.set_b(255-(255-a.get_b())*(1.0-b.get_b()/255.0));

	return blendfunc_ONTO(a,b,amount);
}

template <>
CairoColor
blendfunc_OVERLAY<CairoColor>(CairoColor &a,CairoColor &b,float amount)
{
	if(amount<0) a=~a, amount=-amount;
	
	int ra, ga, ba, aa, ras, gas, bas;
	int rb, gb, bb, ab;

	ra=a.get_r();
	ras=ra*ra;
	ga=a.get_g();
	gas=ga*ga;
	ba=a.get_b();
	bas=ba*ba;
	aa=a.get_a();

	rb=b.get_r();
	gb=b.get_g();
	bb=b.get_b();
	ab=b.get_a();
	
	
	int rc, gc, bc, ac;
	
	if(aa==0 || ab==0) return CairoColor();
	
	rc=(2*rb*ra+ras-2*rb*ras/255.0)/255.0;
	gc=(2*gb*ga+gas-2*gb*gas/255.0)/255.0;
	bc=(2*bb*ba+bas-2*bb*bas/255.0)/255.0;
	ac=aa;
	
	return CairoColor::blend(CairoColor(rc, gc, bc, ac), b, amount, Color::BLEND_ONTO);
}

template <>
CairoColor
blendfunc_HARD_LIGHT(CairoColor &a, CairoColor &b, float amount)
{
	if(amount<0) a=~a, amount=-amount;
	
	int ra, ga, ba, aa;
	int rb, gb, bb;
	int rc, gc, bc;
	
	ra=a.get_r();
	ga=a.get_g();
	ba=a.get_b();
	aa=a.get_a();
	
	rb=b.get_r();
	gb=b.get_g();
	bb=b.get_b();
	
	if(ra>127)	rc =255 -  (255-(ra*2-255))  *  (255-rb)/255.0;
	else		rc= rb*(ra*2)/255.0;
	if(ga>127)	gc =255 -  (255-(ga*2-255))  *  (255-gb)/255.0;
	else		gc= gb*(ga*2)/255.0;
	if(ba>127)	bc =255 -  (255-(ba*2-255))  *  (255-bb)/255.0;
	else		bc= bb*(ba*2)/255.0;

	return CairoColor::blend(CairoColor(rc, gc, bc, aa),b,amount, Color::BLEND_ONTO);
//
//	if(a.get_r()>half)	a.set_r(one-(one-(a.get_r()*2*one-one))*(one-b.get_r()));
//	else				a.set_r(b.get_r()*(a.get_r()*2*one));
//	if(a.get_g()>half)	a.set_g(one-(one-(a.get_g()*2*one-one))*(one-b.get_g()));
//	else				a.set_g(b.get_g()*(a.get_g()*2*one));
//	if(a.get_b()>half)	a.set_b(one-(one-(a.get_b()*2*one-one))*(one-b.get_b()));
//	else				a.set_b(b.get_b()*(a.get_b()*2*one));
//	
//	return blendfunc_ONTO(a,b,amount);
}

template <>
CairoColor
blendfunc_ALPHA_OVER(CairoColor &a, CairoColor &b, float amount)
{
	CairoColor rm(b);
	
	//multiply the inverse alpha channel with the one below us
	rm.set_a((255-a.get_a())*b.get_a()/255.0);
	
	return CairoColor::blend(rm,b,amount, Color::BLEND_STRAIGHT);
}


} // synfig namespace


#endif // __SYNFIG_COLOR_CAIROCOLORBLENDINGFUNCTIONS_H

