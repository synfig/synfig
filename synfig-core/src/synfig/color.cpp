/* === S Y N F I G ========================================================= */
/*!	\file color.cpp
**	\brief Color Class
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

/* === H E A D E R S ======================================================= */

#ifdef USING_PCH
#	include "pch.h"
#else
#ifdef HAVE_CONFIG_H
#	include <config.h>
#endif

#include <ETL/angle>
#include "color.h"
#include "colorblender.h"
#include <cstdio>
#include <sstream>
#include <iostream>
#include <iomanip>

#endif

using namespace synfig;
using namespace etl;
using namespace std;



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
	o << std::fixed << std::setprecision(3) << "#" << get_hex() << " : " << std::setw(6) << a_;
	return String(o.str().c_str());
}

const String
CairoColor::get_string(void)const
{
	std::ostringstream o;
	o << std::fixed << std::setprecision(3) << "#" << get_hex() << " : " << std::setw(6) << a_;
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

Color::Color(const CairoColor& c): ColorBase<value_type>()
	{
		set_r((ceil-floor)*c.get_r()/(CairoColor::ceil-CairoColor::floor));
		set_g((ceil-floor)*c.get_g()/(CairoColor::ceil-CairoColor::floor));
		set_b((ceil-floor)*c.get_b()/(CairoColor::ceil-CairoColor::floor));
		set_a((ceil-floor)*c.get_a()/(CairoColor::ceil-CairoColor::floor));
	}


Color
Color::blend(Color a, Color b, float amount, Color::BlendMethod type)
{
	return ColorBlender<Color>::blend(a, b, amount, type);
}

CairoColor
CairoColor::blend(CairoColor a, CairoColor b, float amount, Color::BlendMethod type)
{
	return ColorBlender<CairoColor>::blend(a, b, amount, type);
}
