/* === S Y N F I G ========================================================= */
/*!	\file
**	\brief Color class function implementation
**
**	$Id$
**
**	\legal
**	Copyright (c) 2002-2005 Robert B. Quattlebaum Jr., Adrian Bentley
**	Copyright (c) 2007, 2008 Chris Moore
**	Copyright (c) 2012-2013 Carlos López
**	Copyright (c) 2015 Diego Barrios Romero
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

#ifndef __SYNFIG_COLOR_COLOR_HPP
#define __SYNFIG_COLOR_COLOR_HPP


#include <synfig/string.h>
#include <synfig/angle.h>


#ifdef USING_PCH
#	include "pch.h"
#else
#ifdef HAVE_CONFIG_H
#	include <config.h>
#endif

#include "color.h"
#endif

namespace synfig {

Color&	Color::operator+=(const Color &rhs)
{
	r_+=rhs.r_;
	g_+=rhs.g_;
	b_+=rhs.b_;
	a_+=rhs.a_;
	return *this;
}

Color& Color::operator-=(const Color &rhs)
{
	r_-=rhs.r_;
	g_-=rhs.g_;
	b_-=rhs.b_;
	a_-=rhs.a_;
	return *this;
}

Color& Color::operator*=(const float &rhs)
{
	r_*=rhs;
	g_*=rhs;
	b_*=rhs;
	a_*=rhs;
	return *this;
}

Color& Color::operator/=(const float &rhs)
{
	const float temp(value_type(1)/rhs);
	r_*=temp;
	g_*=temp;
	b_*=temp;
	a_*=temp;
	return *this;
}

Color Color::operator+(const Color &rhs) const
{
    return Color(*this)+=rhs;
}

Color Color::operator-(const Color &rhs) const
{ return Color(*this)-=rhs; }

Color Color::operator*(const float &rhs)const
{ return Color(*this)*=rhs; }

Color Color::operator/(const float &rhs)const
{ return Color(*this)/=rhs; }

bool Color::operator<(const Color &rhs)const
{
	return r_<rhs.r_ ? true  : rhs.r_<r_ ? false
		 : g_<rhs.g_ ? true  : rhs.g_<g_ ? false
		 : b_<rhs.b_ ? true  : rhs.b_<b_ ? false
		 : a_<rhs.a_;
}

bool Color::operator==(const Color &rhs)const
{ return r_==rhs.r_ && g_==rhs.g_ && b_==rhs.b_ && a_==rhs.a_; }

bool Color::operator!=(const Color &rhs)const
{ return r_!=rhs.r_ || g_!=rhs.g_ || b_!=rhs.b_ || a_!=rhs.a_; }

Color Color::operator-()const
{ return Color(-r_,-g_,-b_,-a_); }

//! Effectively 1.0-color
Color Color::operator~()const
{ return Color(1.0f-r_,1.0f-g_,1.0f-b_,a_); }

bool Color::is_valid()const
{ return !std::isnan(r_) && !std::isnan(g_) && !std::isnan(b_) && !std::isnan(a_); }

Color Color::premult_alpha() const
{
	return Color (r_*a_, g_*a_, b_*a_, a_);
}

Color Color::demult_alpha() const
{
	if(a_)
	{
		const value_type inva = 1/a_;
		return Color (r_*inva, g_*inva, b_*inva, a_);
	}else return alpha();
}

Color::Color() :r_(0), g_(0), b_(0),a_(0) { }
Color::Color(const value_type &f) :r_(f), g_(f), b_(f),a_(f) { }
Color::Color(int f) :r_(f), g_(f), b_(f),a_(f) { }

Color::Color(const value_type& R,
             const value_type& G,
             const value_type& B,
             const value_type& A):
	r_(R),
	g_(G),
	b_(B),
	a_(A) { }

Color::Color(const Color& c, const value_type& A):
	r_(c.r_),
	g_(c.g_),
	b_(c.b_),
	a_(A) { }

const String Color::get_hex()const
{
    return String(real2hex(r_) + real2hex(g_) + real2hex(b_));
}


//! Returns color's luminance
float Color::get_y() const
{
	return
		(float)get_r()*EncodeYUV[0][0]+
		(float)get_g()*EncodeYUV[0][1]+
		(float)get_b()*EncodeYUV[0][2];
}


//! Returns U component of chromanance
float Color::get_u() const
{
	return
		(float)get_r()*EncodeYUV[1][0]+
		(float)get_g()*EncodeYUV[1][1]+
		(float)get_b()*EncodeYUV[1][2];
}


	//! Returns V component of chromanance
float Color::get_v() const
{
	return
		(float)get_r()*EncodeYUV[2][0]+
		(float)get_g()*EncodeYUV[2][1]+
		(float)get_b()*EncodeYUV[2][2];
}

//! Returns the color's saturation
/*!	This is is the magnitude of the U and V components.
**	\see set_s() */
float Color::get_s() const
{
	const float u(get_u()), v(get_v());
	return sqrt(u*u+v*v);
}

//! Sets the luminance (\a y) and chromanance (\a u and \a v)
Color& Color::set_yuv(const float &y, const float &u, const float &v)
{
	set_r(y*DecodeYUV[0][0]+u*DecodeYUV[0][1]+v*DecodeYUV[0][2]);
	set_g(y*DecodeYUV[1][0]+u*DecodeYUV[1][1]+v*DecodeYUV[1][2]);
	set_b(y*DecodeYUV[2][0]+u*DecodeYUV[2][1]+v*DecodeYUV[2][2]);
	return *this;
}

//! Sets color luminance
Color& Color::set_y(const float &y) { return set_yuv(y,get_u(),get_v()); }

//! Set U component of chromanance
Color& Color::set_u(const float &u) { return set_yuv(get_y(),u,get_v()); }

//! Set V component of chromanance
Color& Color::set_v(const float &v) { return set_yuv(get_y(),get_u(),v); }

//! Set the U and V components of chromanance
Color& Color::set_uv(const float& u, const float& v) { return set_yuv(get_y(),u,v); }

//! Sets the color's saturation
/*!	\see get_s() */
Color& Color::set_s(const float &x)
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

//! YUV Color constructor
Color Color::YUV(const float& y, const float& u, const float& v, const value_type& a)
	{ return Color().set_yuv(y,u,v).set_a(a); }

//! Returns the hue of the chromanance
/*!	This is the angle of the U and V components.
**	\see set_hue() */
Angle Color::get_hue() const
	{ return Angle::tan(get_u(),get_v()); }

//! Synonym for get_hue(). \see get_hue()
Angle Color::get_uv_angle() const { return get_hue(); }

//! Sets the color's hue
/*!	\see get_hue() */
Color& Color::set_hue(const Angle& theta)
{
	const float s(get_s());
	const float
		u(s*(float)Angle::sin(theta).get()),
		v(s*(float)Angle::cos(theta).get());
	return set_uv(u,v);
}

//! Synonym for set_hue(). \see set_hue()
Color& Color::set_uv_angle(const Angle& theta) { return set_hue(theta); }

//! Rotates the chromanance vector by amount specified by \a theta
Color& Color::rotate_uv(const Angle& theta)
{
	const float	a(Angle::sin(theta).get()),	b(Angle::cos(theta).get());
	const float	u(get_u()),	v(get_v());

	return set_uv(b*u-a*v,a*u+b*v);
}

Color& Color::set_yuv(const float& y, const float& s, const Angle& theta)
{
	return
		set_yuv(
			y,
			s*(float)Angle::sin(theta).get(),
			s*(float)Angle::cos(theta).get()
		);
}

Color Color::YUV(const float& y, const float& s, const Angle& theta, const value_type& a)
	{ return Color().set_yuv(y,s,theta).set_a(a); }


} // synfig namespace

#endif // __SYNFIG_COLOR_COLOR_HPP

