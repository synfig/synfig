/* === S Y N F I G ========================================================= */
/*!	\file color.cpp
**	\brief Color Class
**
**	$Id$
**
**	\legal
**	Copyright (c) 2002-2005 Robert B. Quattlebaum Jr., Adrian Bentley
**	Copyright (c) 2007, 2008 Chris Moore
**	Copyright (c) 2012-2013 Carlos López
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
#include <cstdio>
#include <sstream>
#include <iostream>
#include <iomanip>

#endif

using namespace synfig;
using namespace etl;
using namespace std;

/* === M A C R O S ========================================================= */

#define COLOR_EPSILON	(0.000001f)

/* === G L O B A L S ======================================================= */

/* === P R O C E D U R E S ================================================= */

/* === M E T H O D S ======================================================= */



ColorReal
Color::hex2real(String s)
{
	std::istringstream i(s);
	int n;
	i.fill('0');
	if (!(i >> hex >> n))
		throw String("bad conversion from hex string \"") + s + String("\"");
	return n / 255.0f;
}

unsigned char
CairoColor::hex2char(String s)
{
	ColorReal cr(Color::hex2real(s));
	return (unsigned char)(cr*255.0f);
}

const String
Color::real2hex(ColorReal c)
{
	std::ostringstream o;
	o.width(2);
	o.fill('0');
	if (c<0) c = 0;
	if (c>1) c = 1;
	o << hex << int(c*255.0f);
	return o.str();
}

const String
CairoColor::char2hex(unsigned char c)
{
	String s(Color::real2hex((ColorReal)(c/((float)ceil))));
	return s.c_str();
}

void
Color::set_hex(String& str)
{
	value_type r, g, b;
	String hex;

	// use just the hex characters
	for (String::const_iterator iter = str.begin(); iter != str.end(); iter++)
		if (isxdigit(*iter))
			hex.push_back(*iter);

	try
	{
		if (hex.size() == 1)
		{
			r = hex2real(hex.substr(0,1)+hex.substr(0,1));
			r_ = g_ = b_ = r;
		}
		else if (hex.size() == 3)
		{
			r = hex2real(hex.substr(0,1)+hex.substr(0,1));
			g = hex2real(hex.substr(1,1)+hex.substr(1,1));
			b = hex2real(hex.substr(2,1)+hex.substr(2,1));
			r_ = r; g_ = g; b_ = b;
		}
		else if (hex.size() == 6)
		{
			r = hex2real(hex.substr(0,2));
			g = hex2real(hex.substr(2,2));
			b = hex2real(hex.substr(4,2));
			r_ = r; g_ = g; b_ = b;
		}
	}
	catch (string s)
	{
		printf("caught <%s>\n", s.c_str());
		return;
	}
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
Color::get_string(void)const
{
	std::ostringstream o;
	o << std::fixed << std::setprecision(3) << "#" << get_hex().c_str() << " : " << std::setw(6) << a_;
	return String(o.str().c_str());
}

const String
CairoColor::get_string(void)const
{
	std::ostringstream o;
	o << std::fixed << std::setprecision(3) << "#" << get_hex().c_str() << " : " << std::setw(6) << get_a();
	return String(o.str().c_str());
}

Color
Color::clamped_negative()const
{
	Color ret=*this;

	if(ret.a_==0)
		return alpha();

	if(ret.a_<0)
		ret=-ret;

	if(ret.r_<0)
	{
		ret.g_-=ret.r_;
		ret.b_-=ret.r_;
		ret.r_=0.0f;
	}
	if(ret.g_<0)
	{
		ret.r_-=ret.g_;
		ret.b_-=ret.g_;
		ret.g_=0.0f;
	}
	if(ret.b_<0)
	{
		ret.r_-=ret.b_;
		ret.g_-=ret.b_;
		ret.b_=0.0f;
	}

	if(ret.r_>1) ret.r_=1;
	if(ret.g_>1) ret.g_=1;
	if(ret.b_>1) ret.b_=1;
	if(ret.a_>1) ret.a_=1;

	if(isnan(ret.get_r())) ret.r_=0.5;
	if(isnan(ret.get_g())) ret.g_=0.5;
	if(isnan(ret.get_b())) ret.b_=0.5;
	if(isnan(ret.get_a())) ret.a_=1;

/*
	if(ret.r_>1) { ret.g_/=ret.r_; ret.b_/=ret.r_; ret.r_=1; }
	if(ret.g_>1) { ret.r_/=ret.g_; ret.b_/=ret.g_; ret.g_=1; }
	if(ret.b_>1) { ret.g_/=ret.b_; ret.r_/=ret.b_; ret.b_=1; }
	if(ret.a_>1) ret.a_=1;
*/

	return ret;
}

Color
Color::clamped()const
{
	Color ret(*this);
	if(ret.get_r()<0)
		ret.set_r(0);
	if(ret.get_g()<0)
		ret.set_g(0);
	if(ret.get_b()<0)
		ret.set_b(0);
	if(ret.get_a()<0)
		ret.set_a(0);

	if(ret.r_>1) ret.r_=1;
	if(ret.g_>1) ret.g_=1;
	if(ret.b_>1) ret.b_=1;
	if(ret.a_>1) ret.a_=1;

	if(isnan(ret.get_r())) ret.r_=0.5;
	if(isnan(ret.get_g())) ret.g_=0.5;
	if(isnan(ret.get_b())) ret.b_=0.5;
	if(isnan(ret.get_a())) ret.a_=1;

	return(ret);
}

Color::Color(const CairoColor& c)
	{
		float div=1.0/((float)(CairoColor::ceil-CairoColor::floor));
		set_r((ceil-floor)*c.get_r()*div);
		set_g((ceil-floor)*c.get_g()*div);
		set_b((ceil-floor)*c.get_b()*div);
		set_a((ceil-floor)*c.get_a()*div);
	}


typedef Color (*blendfunc)(Color &,Color &,float);
typedef CairoColor (*cairoblendfunc)(CairoColor&, CairoColor&, float);

template <class C>
static C
blendfunc_COMPOSITE(C &src,C &dest,float amount)
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

//Specialization for CairoColor
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


template <class C>
static C
blendfunc_STRAIGHT(C &src,C &bg,float amount)
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

//Specialization for CairoColor
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


template <class C>
static C
blendfunc_ONTO(C &a,C &b,float amount)
{
	float alpha(b.get_a());
	const float one(C::ceil);
	return blendfunc_COMPOSITE(a,b.set_a(one),amount).set_a(alpha);
}

template <>
CairoColor
blendfunc_ONTO(CairoColor &a, CairoColor &b, float amount)
{
	unsigned char alpha(b.get_a());
	return blendfunc_COMPOSITE(a,b.set_a(255),amount).set_a(alpha);
}


template <class C>
static C
blendfunc_STRAIGHT_ONTO(C &a,C &b,float amount)
{
	a.set_a(a.get_a()*b.get_a());
	return blendfunc_STRAIGHT(a,b,amount);
}

template <>
CairoColor
blendfunc_STRAIGHT_ONTO(CairoColor &a, CairoColor &b, float amount)
{
	a.set_a(a.get_a()*b.get_a()/255.0);
	return CairoColor::blend(a, b, amount, Color::BLEND_STRAIGHT);
}

template <class C>
static C
blendfunc_BRIGHTEN(C &a,C &b,float amount)
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

//Specialization for CairoColor
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

template <class C>
static C
blendfunc_DARKEN(C &a,C &b,float amount)
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

//Specialization for CairoColor
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

template <class C>
static C
blendfunc_ADD(C &a,C &b,float amount)
{
	const float alpha(a.get_a()*amount);

	b.set_r(b.get_r()+a.get_r()*alpha);
	b.set_g(b.get_g()+a.get_g()*alpha);
	b.set_b(b.get_b()+a.get_b()*alpha);

	return b;
}

//Specialization for CairoColor
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

template <class C>
static C
blendfunc_SUBTRACT(C &a,C &b,float amount)
{
	const float alpha(a.get_a()*amount);

	b.set_r(b.get_r()-a.get_r()*alpha);
	b.set_g(b.get_g()-a.get_g()*alpha);
	b.set_b(b.get_b()-a.get_b()*alpha);

	return b;
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

template <class C>
static C
blendfunc_DIFFERENCE(C &a,C &b,float amount)
{
	const float alpha(a.get_a()*amount);

	b.set_r(abs(b.get_r()-a.get_r()*alpha));
	b.set_g(abs(b.get_g()-a.get_g()*alpha));
	b.set_b(abs(b.get_b()-a.get_b()*alpha));

	return b;
}

//Specialization for CairoColor
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
	
	rc=abs(rb-ra*aaa);
	gc=abs(gb-ga*aaa);
	bc=abs(bb-ba*aaa);
	ac=ab;
	
	return CairoColor(rc, gc, bc, ac);
}

template <class C>
static C
blendfunc_MULTIPLY(C &a,C &b,float amount)
{
	if(amount<0) a=~a, amount=-amount;

	amount*=a.get_a();
	b.set_r(((b.get_r()*a.get_r())-b.get_r())*(amount)+b.get_r());
	b.set_g(((b.get_g()*a.get_g())-b.get_g())*(amount)+b.get_g());
	b.set_b(((b.get_b()*a.get_b())-b.get_b())*(amount)+b.get_b());
	return b;
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

template <class C>
static C
blendfunc_DIVIDE(C &a,C &b,float amount)
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

// Specialization for CairoColor
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

template <class C>
static C
blendfunc_COLOR(C &a,C &b,float amount)
{
	C temp(b);
	temp.set_uv(a.get_u(),a.get_v());
	return (temp-b)*amount*a.get_a()+b;
}


template <>
CairoColor
blendfunc_COLOR(CairoColor &a, CairoColor &b, float amount)
{
	return CairoColor(Color::blend(Color(a), Color(b), amount, Color::BLEND_COLOR));
}


template <class C>
static C
blendfunc_HUE(C &a,C &b,float amount)
{
	C temp(b);
	temp.set_hue(a.get_hue());
	return (temp-b)*amount*a.get_a()+b;
}

template <>
CairoColor
blendfunc_HUE(CairoColor &a, CairoColor &b, float amount)
{
	return CairoColor(Color::blend(Color(a), Color(b), amount, Color::BLEND_HUE));
}

template <class C>
static C
blendfunc_SATURATION(C &a,C &b,float amount)
{
	C temp(b);
	temp.set_s(a.get_s());
	return (temp-b)*amount*a.get_a()+b;
}

template <>
CairoColor
blendfunc_SATURATION(CairoColor &a, CairoColor &b, float amount)
{
	return CairoColor(Color::blend(Color(a), Color(b), amount, Color::BLEND_SATURATION));
}

template <class C>
static C
blendfunc_LUMINANCE(C &a,C &b,float amount)
{
	C temp(b);
	temp.set_y(a.get_y());
	return (temp-b)*amount*a.get_a()+b;
}

template <>
CairoColor
blendfunc_LUMINANCE(CairoColor &a, CairoColor &b, float amount)
{
	return CairoColor(Color::blend(Color(a), Color(b), amount, Color::BLEND_LUMINANCE));
}

template <class C>
static C
blendfunc_BEHIND(C &a,C &b,float amount)
{
	if(a.get_a()==0)
		a.set_a(COLOR_EPSILON*amount);		//!< \todo this is a hack
	else
		a.set_a(a.get_a()*amount);
	return blendfunc_COMPOSITE(b,a,1.0);
}

template <>
CairoColor
blendfunc_BEHIND(CairoColor &a, CairoColor &b, float amount)
{
	a.set_a(a.get_a()*amount);
	return CairoColor::blend(b, a, 1.0, Color::BLEND_COMPOSITE);
}


template <class C>
static C
blendfunc_ALPHA_BRIGHTEN(C &a,C &b,float amount)
{
	// \todo can this be right, multiplying amount by *b*'s alpha?
	// compare with blendfunc_BRIGHTEN where it is multiplied by *a*'s
	if(a.get_a() < b.get_a()*amount)
		return a.set_a(a.get_a()*amount);
	return b;
}

//Specialization for CairoColor
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

template <class C>
static C
blendfunc_ALPHA_DARKEN(C &a,C &b,float amount)
{
	if(a.get_a()*amount > b.get_a())
		return a.set_a(a.get_a()*amount);
	return b;
}

//Specialization for CairoColor
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


template <class C>
static C
blendfunc_SCREEN(C &a,C &b,float amount)
{
	const float one(C::ceil);
	if(amount<0) a=~a, amount=-amount;

	a.set_r(one-(one-a.get_r())*(one-b.get_r()));
	a.set_g(one-(one-a.get_g())*(one-b.get_g()));
	a.set_b(one-(one-a.get_b())*(one-b.get_b()));

	return blendfunc_ONTO(a,b,amount);
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


template <class C>
static C
blendfunc_OVERLAY(C &a,C &b,float amount)
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

//Specialization for CairoColors
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



template <class C>
static C
blendfunc_HARD_LIGHT(C &a,C &b,float amount)
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

template <class C>
static C
blendfunc_ALPHA_OVER(C &a,C &b,float amount)
{
	const float one(C::ceil);
	C rm(b);

	//multiply the inverse alpha channel with the one below us
	rm.set_a((one-a.get_a())*b.get_a());

	return blendfunc_STRAIGHT(rm,b,amount);
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


Color
Color::blend(Color a, Color b,float amount, Color::BlendMethod type)
{
	// No matter what blend method is being used,
	// if the amount is equal to zero, then only B
	// will shine through
	if(fabsf(amount)<=COLOR_EPSILON)return b;

	assert(type<BLEND_END);

	const static blendfunc vtable[BLEND_END]=
	{
		blendfunc_COMPOSITE<Color>,	// 0
		blendfunc_STRAIGHT<Color>,
		blendfunc_BRIGHTEN<Color>,
		blendfunc_DARKEN<Color>,
		blendfunc_ADD<Color>,
		blendfunc_SUBTRACT<Color>,		// 5
		blendfunc_MULTIPLY<Color>,
		blendfunc_DIVIDE<Color>,
		blendfunc_COLOR<Color>,
		blendfunc_HUE<Color>,
		blendfunc_SATURATION<Color>,	// 10
		blendfunc_LUMINANCE<Color>,
		blendfunc_BEHIND<Color>,
		blendfunc_ONTO<Color>,
		blendfunc_ALPHA_BRIGHTEN<Color>,
		blendfunc_ALPHA_DARKEN<Color>,	// 15
		blendfunc_SCREEN<Color>,
		blendfunc_HARD_LIGHT<Color>,
		blendfunc_DIFFERENCE<Color>,
		blendfunc_ALPHA_OVER<Color>,
		blendfunc_OVERLAY<Color>,		// 20
		blendfunc_STRAIGHT_ONTO<Color>,
	};

	return vtable[type](a,b,amount);
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
	};

	return vtable[type](a,b,amount);
}
