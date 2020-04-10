/* === S Y N F I G ========================================================= */
/*!	\file
**	\brief CairoColor Class
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

#ifndef __SYNFIG_COLOR_CAIROCOLOR_H
#define __SYNFIG_COLOR_CAIROCOLOR_H

#include <synfig/color/common.h>
#include <synfig/color/color.h>

namespace synfig {

class CairoColorAccumulator;

/*!  \class CairoColor
** \ ARGB 32 bits Color class implementation
** \ for Cairo Image usage.
** \ Color channels are stored in a 32 bits value native endian aligened
** \ with this order A, R, G, B
** \ Operations over color channels
** \ that overflow or underflow the unsigned char value
** \ (get the value out of 0-255) aren't allowed and the
** \ value will remain at 0 or 255. Otherwise, there could
** \ be color cycling what will produce artifacts, since 
** \ values outside 0-255 aren't allowed.
** 
** \ In this class color channels aren't alpha premultiplied
** \ When used on a alpha premultiplied surface the premultiplication
** \ and demultiplication has to be explicitly done by the user before 
** \ and after being used on the Cairo Image surface.
*/
class CairoColor
{
public:
	typedef uint32_t value_type;

private:
	value_type pixel;

public:
	static const unsigned char ceil=255;	
	static const unsigned char floor=0;
	static const float range;
	static const value_type amask=0xFF<<24;
	static const value_type rmask=0xFF<<16;
	static const value_type gmask=0xFF<<8;
	static const value_type bmask=0xFF;

	// Operators
public:
	inline unsigned char ceil_clamp(int x)
	{
		if(x>ceil) return ceil;
		else return (unsigned char)(x);
	}
	inline unsigned char floor_clamp(int x)
	{
		if(x<floor) return floor;
		else return (unsigned char)(x);
	}
	inline unsigned char clamp(int x)
	{
		if(x > ceil) return ceil;
		else if (x < floor) return floor;
		else return (unsigned char)(x);
	}
	inline unsigned char clamp(float x)
	{
		return clamp((int) (x));
	}

	CairoColor&
	operator+=(const CairoColor &rhs)
	{		
		set_r(ceil_clamp((int)(get_r()) + rhs.get_r()));
		set_g(ceil_clamp((int)(get_g()) + rhs.get_g()));
		set_b(ceil_clamp((int)(get_b()) + rhs.get_b()));
		set_a(ceil_clamp((int)(get_a()) + rhs.get_a()));
		return *this;
	}

	CairoColor&
	operator-=(const CairoColor &rhs)
	{		
		set_r(floor_clamp((int)(get_r()) - rhs.get_r()));
		set_g(floor_clamp((int)(get_g()) - rhs.get_g()));
		set_b(floor_clamp((int)(get_b()) - rhs.get_b()));
		set_a(floor_clamp((int)(get_a()) - rhs.get_a()));
		return *this;
	}
	
	CairoColor &
	operator*=(const float &rhs)
	{
		set_r(clamp(get_r()*rhs));
		set_g(clamp(get_g()*rhs));
		set_b(clamp(get_b()*rhs));
		set_a(clamp(get_a()*rhs));
		return *this;
	}

	CairoColor &
	operator/=(const float &rhs)
	{
		const float temp(1.0f/rhs);
		set_r(clamp(get_r()*temp));
		set_g(clamp(get_g()*temp));
		set_b(clamp(get_b()*temp));
		set_a(clamp(get_a()*temp));
		return *this;
	}

	CairoColor
	operator+(const CairoColor &rhs)const
	{ return CairoColor(*this)+=rhs; }
	
	CairoColor
	operator-(const CairoColor &rhs)const
	{ return CairoColor(*this)-=rhs; }
	
	CairoColor
	operator*(const float &rhs)const
	{ return CairoColor(*this)*=rhs; }
	
	CairoColor
	operator/(const float &rhs)const
	{ return CairoColor(*this)/=rhs; }
	
	bool
	operator==(const CairoColor &rhs)const
	{ return get_r()==rhs.get_r()
          && get_g()==rhs.get_g()
		  && get_b()==rhs.get_b()
		  && get_a()==rhs.get_a(); }
	
	bool
	operator!=(const CairoColor &rhs)const
	{ return get_r()!=rhs.get_r()
		  || get_g()!=rhs.get_g()
		  || get_b()!=rhs.get_b()
		  || get_a()!=rhs.get_a(); }

// Not suitable for CairoColor
//	operator-()const
//	{ return CairoColor(-r_,-g_,-b_,-a_); }

	CairoColor
	operator~()const
	{ return CairoColor((unsigned char)(ceil-get_r()),(unsigned char)(ceil-get_g()),(unsigned char)(ceil-get_b()),get_a()); }

	bool is_valid()const
	{ return true; }

	
	CairoColor premult_alpha() const
	{
		const float a(get_a()/range);
		return CairoColor (get_r()*a, get_g()*a, get_b()*a, get_a());
	}
	
	CairoColor demult_alpha() const
	{
		if(get_a())
		{
			const float inva = range/get_a();
			return CairoColor (get_r()*inva, get_g()*inva, get_b()*inva, get_a());
		}else return alpha();
	}

	// Constructors
public:
	CairoColor() :pixel(0x0) { }
	CairoColor(const unsigned char u): pixel((u<<24)|(u<<16)|(u<<8)|(u)) { }

	CairoColor(const unsigned char R, const unsigned char G, const unsigned char B, const unsigned char A=ceil)
		: pixel((A<<24)|(R<<16)|(G<<8)|(B)) { }
	CairoColor(const CairoColor& c, const unsigned char A)
		: pixel(c.pixel) { set_a(A); }

	// Converter constructor
	CairoColor(const Color& c)
		: pixel(0)
	{
		set_r((ceil-floor)*c.get_r()/(Color::ceil-Color::floor));
		set_g((ceil-floor)*c.get_g()/(Color::ceil-Color::floor));
		set_b((ceil-floor)*c.get_b()/(Color::ceil-Color::floor));
		set_a((ceil-floor)*c.get_a()/(Color::ceil-Color::floor));
	}
	// From CairoColorAccumulator
	friend class CairoColorAccumulator;
	inline CairoColor(const CairoColorAccumulator& c);
	CairoColor(int r, int g, int b, int a);
	
	unsigned char get_a()const { return pixel>>24; }
	unsigned char get_r()const { return pixel>>16; }
	unsigned char get_g()const { return pixel>>8; }
	unsigned char get_b()const { return pixel; }
	unsigned char get_alpha()const { return get_a(); }
	
	const String get_string(void)const;

	static const String char2hex(unsigned char c);
	static unsigned char hex2char(String s);
	
	void set_hex( String& str);
	const String get_hex()const { return String(char2hex(get_r())+char2hex(get_g())+char2hex(get_b())); }

	CairoColor& set_r(const unsigned char x) {pixel &= ~rmask; pixel |=(x<<16); return *this; }
	CairoColor& set_g(const unsigned char x) {pixel &= ~gmask; pixel |=(x<<8 ); return *this; }
	CairoColor& set_b(const unsigned char x) {pixel &= ~bmask; pixel |=(x    ); return *this; }
	CairoColor& set_a(const unsigned char x) {pixel &= ~amask; pixel |=(x<<24); return *this; }
	CairoColor& set_alpha(const unsigned char x) { return set_a(x); }
	
	float
	get_y() const
	{
		return(
		(float)get_r()*EncodeYUV[0][0]+
		(float)get_g()*EncodeYUV[0][1]+
		(float)get_b()*EncodeYUV[0][2]
		)/CairoColor::range;
	}

	float
	get_u() const
	{
		return(
		(float)get_r()*EncodeYUV[1][0]+
		(float)get_g()*EncodeYUV[1][1]+
		(float)get_b()*EncodeYUV[1][2]
		)/CairoColor::range;
	}

	float
	get_v() const
	{
		return(
		(float)get_r()*EncodeYUV[2][0]+
		(float)get_g()*EncodeYUV[2][1]+
		(float)get_b()*EncodeYUV[2][2]
		)/CairoColor::range;
	}

	float
	get_s() const
	{
		const float u(get_u()), v(get_v());
		return sqrt(u*u+v*v);
	}
	
	CairoColor&
	set_yuv(const float &y, const float &u, const float &v)
	{
		Color c(*this);
		c.set_r(y*DecodeYUV[0][0]+u*DecodeYUV[0][1]+v*DecodeYUV[0][2]);
		c.set_g(y*DecodeYUV[1][0]+u*DecodeYUV[1][1]+v*DecodeYUV[1][2]);
		c.set_b(y*DecodeYUV[2][0]+u*DecodeYUV[2][1]+v*DecodeYUV[2][2]);
		(*this)=CairoColor(c);
		return *this;
	}
	
	CairoColor& set_y(const float &y) { return set_yuv(y,get_u(),get_v()); }
	
	CairoColor& set_u(const float &u) { return set_yuv(get_y(),u,get_v()); }
	
	CairoColor& set_v(const float &v) { return set_yuv(get_y(),get_u(),v); }
	
	CairoColor& set_uv(const float& u, const float& v) { return set_yuv(get_y(),u,v); }
	
	CairoColor&	set_s(const float &x)
	{
		float u(get_u()), v(get_v());
		const float s(sqrt(u*u+v*v));
		if(s)
		{
			u=(u/s)*x;
			v=(v/s)*x;
			return set_uv(u,v);
		}
		return *this;
	}

	static CairoColor YUV(const float& y, const float& u, const float& v, const unsigned char a=ceil)
	{ return CairoColor().set_yuv(y,u,v).set_a(a); }
	
	Angle get_hue() const	{ return Angle::tan(get_u(),get_v()); }
	
	Angle get_uv_angle() const { return get_hue(); }
	
	CairoColor& set_hue(const Angle& theta)
	{
		const float s(get_s());
		const float
		u(s*(float)Angle::sin(theta).get()),
		v(s*(float)Angle::cos(theta).get());
		return set_uv(u,v);
	}
	
	CairoColor& set_uv_angle(const Angle& theta) { return set_hue(theta); }
	
	CairoColor& rotate_uv(const Angle& theta)
	{
		const float	a(Angle::sin(theta).get()),	b(Angle::cos(theta).get());
		const float	u(get_u()),	v(get_v());
		return set_uv(b*u-a*v,a*u+b*v);
	}

	CairoColor& set_yuv(const float& y, const float& s, const Angle& theta)
	{
		return
		set_yuv(
				y,
				s*(float)Angle::sin(theta).get(),
				s*(float)Angle::cos(theta).get()
				);
	}
	
	static CairoColor YUV(const float& y, const float& s, const Angle& theta, const unsigned char a=ceil)
	{ return CairoColor().set_yuv(y,s,theta).set_a(a); }

	static inline CairoColor alpha() { return CairoColor(floor,floor,floor,floor); }
	static inline CairoColor black() { return CairoColor(floor,floor,floor); }
	static inline CairoColor white() { return CairoColor(ceil,ceil,ceil); }
	static inline CairoColor gray() { return CairoColor(ceil/2,ceil/2,ceil/2); }
	static inline CairoColor magenta() { return CairoColor(ceil,floor,ceil); }
	static inline CairoColor red() { return CairoColor(ceil,floor, floor); }
	static inline CairoColor green() { return CairoColor(floor, ceil,floor); }
	static inline CairoColor blue() { return CairoColor(floor,floor,ceil); }
	static inline CairoColor cyan() { return CairoColor(floor,ceil,ceil); }
	static inline CairoColor yellow() { return CairoColor(ceil,ceil,floor); }

	// Use Color::BlenMethods for the enum value
	static CairoColor blend(CairoColor a, CairoColor b, float amount, Color::BlendMethod type=Color::BLEND_COMPOSITE);

	static bool is_onto(Color::BlendMethod x)
	{
		return Color::is_onto(x);
	}
	
	static bool is_straight(Color::BlendMethod x)
	{
		return Color::is_straight(x);
	}
	
}; // End of CairoColor class


inline CairoColor::CairoColor(int r, int g, int b, int a)
{
	set_r(CairoColor::clamp(r));
	set_g(CairoColor::clamp(g));
	set_b(CairoColor::clamp(b));
	set_a(CairoColor::clamp(a));
}



} // synfig namespace

#endif // __SYNFIG_COLOR_CAIROCOLOR_H

