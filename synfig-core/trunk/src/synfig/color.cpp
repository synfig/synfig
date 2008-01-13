/* === S Y N F I G ========================================================= */
/*!	\file color.cpp
**	\brief Color Class
**
**	$Id$
**
**	\legal
**	Copyright (c) 2002-2005 Robert B. Quattlebaum Jr., Adrian Bentley
**	Copyright (c) 2007 Chris Moore
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

const String
Color::get_string(void)const
{
	std::ostringstream o;
	o << std::fixed << std::setprecision(3) << "#" << get_hex() << " : " << std::setw(6) << a_;
	return String(o.str().c_str());
}

#if 0
Color&
Color::rotate_uv(const Angle& theta)const
{
/*/
	Color ret(*this);
	ret.set_hue(ret.get_hue()+theta);
	return ret;
/*/
	const float
		a(angle::sin(theta).get()),
		b(angle::cos(theta).get());
	const float
		u(get_u()),
		v(get_v());

	return set_uv(b*u-a*v,a*u+b*v);
	//return YUV(get_y(),b*u-a*v,a*u+b*v,get_a());
//*/
}
#endif

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

typedef Color (*blendfunc)(Color &,Color &,float);

static Color
blendfunc_COMPOSITE(Color &src,Color &dest,float amount)
{
	//c_dest'=c_src+(1.0-a_src)*c_dest
	//a_dest'=a_src+(1.0-a_src)*a_dest

	float a_src=src.get_a()*amount;
	float a_dest=dest.get_a();

	// if a_arc==0.0
	//if(fabsf(a_src)<COLOR_EPSILON) return dest;

	// Scale the source and destination by their alpha values
	src*=a_src;
	dest*=a_dest;

	dest=src + dest*(1.0f-a_src);

	a_dest=a_src + a_dest*(1.0f-a_src);

	// if a_dest!=0.0
	if(fabsf(a_dest)>COLOR_EPSILON)
	{
		dest/=a_dest;
		dest.set_a(a_dest);
	}
	else
	{
		dest=Color::alpha();
	}
	assert(dest.is_valid());
	return dest;
}

static Color
blendfunc_STRAIGHT(Color &src,Color &bg,float amount)
{
	//a_out'=(a_src-a_bg)*amount+a_bg
	//c_out'=(((c_src*a_src)-(c_bg*a_bg))*amount+(c_bg*a_bg))/a_out'

	// ie: if(amount==1.0)
	//if(fabsf(amount-1.0f)<COLOR_EPSILON)return src;

	Color out;

	float a_out((src.get_a()-bg.get_a())*amount+bg.get_a());

	// if a_out!=0.0
	if(fabsf(a_out)>COLOR_EPSILON)
//	if(a_out>COLOR_EPSILON || a_out<-COLOR_EPSILON)
	{
		out=((src*src.get_a()-bg*bg.get_a())*amount+bg*bg.get_a())/a_out;
		out.set_a(a_out);
	}
	else
		out=Color::alpha();

	assert(out.is_valid());
	return out;
}

static Color
blendfunc_ONTO(Color &a,Color &b,float amount)
{
	float alpha(b.get_a());
	return blendfunc_COMPOSITE(a,b.set_a(1.0f),amount).set_a(alpha);
}

static Color
blendfunc_STRAIGHT_ONTO(Color &a,Color &b,float amount)
{
	a.set_a(a.get_a()*b.get_a());
	return blendfunc_STRAIGHT(a,b,amount);
}

static Color
blendfunc_BRIGHTEN(Color &a,Color &b,float amount)
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

static Color
blendfunc_DARKEN(Color &a,Color &b,float amount)
{
	const float alpha(a.get_a()*amount);

	if(b.get_r()>(a.get_r()-1.0f)*alpha+1.0f)
		b.set_r((a.get_r()-1.0f)*alpha+1.0f);

	if(b.get_g()>(a.get_g()-1.0f)*alpha+1.0f)
		b.set_g((a.get_g()-1.0f)*alpha+1.0f);

	if(b.get_b()>(a.get_b()-1.0f)*alpha+1.0f)
		b.set_b((a.get_b()-1.0f)*alpha+1.0f);


	return b;
}

static Color
blendfunc_ADD(Color &a,Color &b,float amount)
{
	const float alpha(a.get_a()*amount);

	b.set_r(b.get_r()+a.get_r()*alpha);
	b.set_g(b.get_g()+a.get_g()*alpha);
	b.set_b(b.get_b()+a.get_b()*alpha);

	return b;
}

static Color
blendfunc_SUBTRACT(Color &a,Color &b,float amount)
{
	const float alpha(a.get_a()*amount);

	b.set_r(b.get_r()-a.get_r()*alpha);
	b.set_g(b.get_g()-a.get_g()*alpha);
	b.set_b(b.get_b()-a.get_b()*alpha);

	return b;
}

static Color
blendfunc_DIFFERENCE(Color &a,Color &b,float amount)
{
	const float alpha(a.get_a()*amount);

	b.set_r(abs(b.get_r()-a.get_r()*alpha));
	b.set_g(abs(b.get_g()-a.get_g()*alpha));
	b.set_b(abs(b.get_b()-a.get_b()*alpha));

	return b;
}

static Color
blendfunc_MULTIPLY(Color &a,Color &b,float amount)
{
	if(amount<0) a=~a, amount=-amount;

	amount*=a.get_a();
	b.set_r(((b.get_r()*a.get_r())-b.get_r())*(amount)+b.get_r());
	b.set_g(((b.get_g()*a.get_g())-b.get_g())*(amount)+b.get_g());
	b.set_b(((b.get_b()*a.get_b())-b.get_b())*(amount)+b.get_b());
	return b;
}

static Color
blendfunc_DIVIDE(Color &a,Color &b,float amount)
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

static Color
blendfunc_COLOR(Color &a,Color &b,float amount)
{
	Color temp(b);
	temp.set_uv(a.get_u(),a.get_v());
	return (temp-b)*amount*a.get_a()+b;
}

static Color
blendfunc_HUE(Color &a,Color &b,float amount)
{
	Color temp(b);
	temp.set_hue(a.get_hue());
	return (temp-b)*amount*a.get_a()+b;
}

static Color
blendfunc_SATURATION(Color &a,Color &b,float amount)
{
	Color temp(b);
	temp.set_s(a.get_s());
	return (temp-b)*amount*a.get_a()+b;
}

static Color
blendfunc_LUMINANCE(Color &a,Color &b,float amount)
{
	Color temp(b);
	temp.set_y(a.get_y());
	return (temp-b)*amount*a.get_a()+b;
}

static Color
blendfunc_BEHIND(Color &a,Color &b,float amount)
{
	if(a.get_a()==0)a.set_a(COLOR_EPSILON);		//!< \todo this is a hack
	a.set_a(a.get_a()*amount);
	return blendfunc_COMPOSITE(b,a,1.0);
}

static Color
blendfunc_ALPHA_BRIGHTEN(Color &a,Color &b,float amount)
{
	if(a.get_a()<b.get_a()*amount)
		return a.set_a(a.get_a()*amount);
	return b;
}

static Color
blendfunc_ALPHA_DARKEN(Color &a,Color &b,float amount)
{
	if(a.get_a()*amount>b.get_a())
		return a.set_a(a.get_a()*amount);
	return b;
}

static Color
blendfunc_SCREEN(Color &a,Color &b,float amount)
{
	if(amount<0) a=~a, amount=-amount;

	a.set_r(1.0-(1.0f-a.get_r())*(1.0f-b.get_r()));
	a.set_g(1.0-(1.0f-a.get_g())*(1.0f-b.get_g()));
	a.set_b(1.0-(1.0f-a.get_b())*(1.0f-b.get_b()));

	return blendfunc_ONTO(a,b,amount);
}

static Color
blendfunc_OVERLAY(Color &a,Color &b,float amount)
{
	if(amount<0) a=~a, amount=-amount;

	Color rm;
	rm.set_r(b.get_r()*a.get_r());
	rm.set_g(b.get_g()*a.get_g());
	rm.set_b(b.get_b()*a.get_b());

	Color rs;
	rs.set_r(1.0-(1.0f-a.get_r())*(1.0f-b.get_r()));
	rs.set_g(1.0-(1.0f-a.get_g())*(1.0f-b.get_g()));
	rs.set_b(1.0-(1.0f-a.get_b())*(1.0f-b.get_b()));

	Color& ret(a);

	ret.set_r(a.get_r()*rs.get_r() + (1.0-a.get_r())*rm.get_r());
	ret.set_g(a.get_g()*rs.get_g() + (1.0-a.get_g())*rm.get_g());
	ret.set_b(a.get_b()*rs.get_b() + (1.0-a.get_b())*rm.get_b());

	return blendfunc_ONTO(ret,b,amount);
}

static Color
blendfunc_HARD_LIGHT(Color &a,Color &b,float amount)
{
	if(amount<0) a=~a, amount=-amount;

	if(a.get_r()>0.5f)	a.set_r(1.0-(1.0f-(a.get_r()*2.0f-1.0f))*(1.0f-b.get_r()));
	else				a.set_r(b.get_r()*(a.get_r()*2.0f));
	if(a.get_g()>0.5f)	a.set_g(1.0-(1.0f-(a.get_g()*2.0f-1.0f))*(1.0f-b.get_g()));
	else				a.set_g(b.get_g()*(a.get_g()*2.0f));
	if(a.get_b()>0.5f)	a.set_b(1.0-(1.0f-(a.get_b()*2.0f-1.0f))*(1.0f-b.get_b()));
	else				a.set_b(b.get_b()*(a.get_b()*2.0f));

	return blendfunc_ONTO(a,b,amount);
}

static Color
blendfunc_ALPHA_OVER(Color &a,Color &b,float amount)
{
	Color rm(b);

	//multiply the inverse alpha channel with the one below us
	rm.set_a((1-a.get_a())*b.get_a());

	return blendfunc_STRAIGHT(rm,b,amount);
}


Color
Color::blend(Color a, Color b,float amount, Color::BlendMethod type)
{
#if 0
	if(isnan(a.get_r()) || isnan(a.get_g()) || isnan(a.get_b()))
	{
#ifdef _DEBUG
		a=magenta().set_a(a.get_a());
#else
		a=black().set_a(a.get_a());
#endif
	}

	if(isnan(b.get_r()) || isnan(b.get_g()) || isnan(b.get_b()))
	{
#ifdef _DEBUG
		b=magenta().set_a(b.get_a());
#else
		b=black().set_a(b.get_a());
#endif
	}
#endif

/*
	if(!a.is_valid()&&b.is_valid())
		return b;

	if(a.is_valid()&&!b.is_valid())
		return a;

	if(!a.is_valid()||!b.is_valid())
	{
#ifdef _DEBUG
		return magenta();
#else
		return black();
#endif
	}
*/

	// No matter what blend method is being used,
	// if the amount is equal to zero, then only B
	// will shine through
	if(fabsf(amount)<=COLOR_EPSILON)return b;

	assert(type<BLEND_END);

	const static blendfunc vtable[BLEND_END]=
	{
		blendfunc_COMPOSITE,
		blendfunc_STRAIGHT,
		blendfunc_BRIGHTEN,
		blendfunc_DARKEN,
		blendfunc_ADD,
		blendfunc_SUBTRACT,
		blendfunc_MULTIPLY,
		blendfunc_DIVIDE,
		blendfunc_COLOR,
		blendfunc_HUE,
		blendfunc_SATURATION,
		blendfunc_LUMINANCE,
		blendfunc_BEHIND,
		blendfunc_ONTO,
		blendfunc_ALPHA_BRIGHTEN,
		blendfunc_ALPHA_DARKEN,
		blendfunc_SCREEN,
		blendfunc_HARD_LIGHT,
		blendfunc_DIFFERENCE,
		blendfunc_ALPHA_OVER,
		blendfunc_OVERLAY,
		blendfunc_STRAIGHT_ONTO,
	};

	return vtable[type](a,b,amount);
}
