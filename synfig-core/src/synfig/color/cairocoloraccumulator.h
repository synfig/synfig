/* === S Y N F I G ========================================================= */
/*!	\file
**	\brief CairoColorAccumulator Class Implementation
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

#ifndef __SYNFIG_COLOR_CAIROCOLORACUMULATOR_H
#define __SYNFIG_COLOR_CAIROCOLORACUMULATOR_H

#include <synfig/color/common.h>

namespace synfig {

class CairoColor;

class CairoColorAccumulator
{
	friend class CairoColor;
public:
	typedef float value_type;
	
private:
	value_type a_, r_, g_, b_;
	
public:
	
	CairoColorAccumulator &
	operator+=(const CairoColorAccumulator &rhs)
	{
		r_+=rhs.r_;
		g_+=rhs.g_;
		b_+=rhs.b_;
		a_+=rhs.a_;
		return *this;
	}
	
	CairoColorAccumulator &
	operator-=(const CairoColorAccumulator &rhs)
	{
		r_-=rhs.r_;
		g_-=rhs.g_;
		b_-=rhs.b_;
		a_-=rhs.a_;
		return *this;
	}
	
	CairoColorAccumulator &
	operator*=(const float &rhs)
	{
		r_*=rhs;
		g_*=rhs;
		b_*=rhs;
		a_*=rhs;
		return *this;
	}
	
	CairoColorAccumulator &
	operator/=(const float &rhs)
	{
		const float temp(value_type(1)/rhs);
		r_*=temp;
		g_*=temp;
		b_*=temp;
		a_*=temp;
		return *this;
	}
	
	CairoColorAccumulator
	operator+(const CairoColorAccumulator &rhs)const
	{ return CairoColorAccumulator(*this)+=rhs; }
	
	CairoColorAccumulator
	operator-(const CairoColorAccumulator &rhs)const
	{ return CairoColorAccumulator(*this)-=rhs; }
	
	CairoColorAccumulator
	operator*(const float &rhs)const
	{ return CairoColorAccumulator(*this)*=rhs; }
	
	CairoColorAccumulator
	operator/(const float &rhs)const
	{ return CairoColorAccumulator(*this)/=rhs; }
	
	bool
	operator==(const CairoColorAccumulator &rhs)const
	{ return r_==rhs.r_ && g_==rhs.g_ && b_==rhs.b_ && a_==rhs.a_; }
	
	bool
	operator!=(const CairoColorAccumulator &rhs)const
	{ return r_!=rhs.r_ || g_!=rhs.g_ || b_!=rhs.b_ || a_!=rhs.a_; }
	
	CairoColorAccumulator
	operator-()const
	{ return CairoColorAccumulator(-r_,-g_,-b_,-a_); }
	
	bool is_valid()const
	{ return !std::isnan(r_) && !std::isnan(g_) && !std::isnan(b_) && !std::isnan(a_); }
	
public:
	CairoColorAccumulator(): a_(), r_(), g_(), b_() { }
	
	/*!	\param R Red
	 **	\param G Green
	 **	\param B Blue
	 **	\param A Opacity(alpha) */
	CairoColorAccumulator(const value_type& R, const value_type& G, const value_type& B, const value_type& A=1):
	a_(A),
	r_(R),
	g_(G),
	b_(B) { }
	
	//!	Converter
	CairoColorAccumulator(const CairoColor& c):
	a_(c.get_a()/CairoColor::range),
	r_(c.get_r()/CairoColor::range),
	g_(c.get_g()/CairoColor::range),
	b_(c.get_b()/CairoColor::range) { }
	
	//! Converter
	CairoColorAccumulator(int c): a_(c),r_(c), g_(c), b_(c) { }
	
	//! Returns the RED component
	const value_type& get_r()const { return r_; }
	
	//! Returns the GREEN component
	const value_type& get_g()const { return g_; }
	
	//! Returns the BLUE component
	const value_type& get_b()const { return b_; }
	
	//! Returns the amount of opacity (alpha)
	const value_type& get_a()const { return a_; }
	
	//! Synonym for get_a(). \see get_a()
	const value_type& get_alpha()const { return get_a(); }
	
	//! Sets the RED component to \a x
	CairoColorAccumulator& set_r(const value_type& x) { r_ = x; return *this; }
	
	//! Sets the GREEN component to \a x
	CairoColorAccumulator& set_g(const value_type& x) { g_ = x; return *this; }
	
	//! Sets the BLUE component to \a x
	CairoColorAccumulator& set_b(const value_type& x) { b_ = x; return *this; }
	
	//! Sets the opacity (alpha) to \a x
	CairoColorAccumulator& set_a(const value_type& x) { a_ = x; return *this; }
	
	//! Synonym for set_a(). \see set_a()
	CairoColorAccumulator& set_alpha(const value_type& x) { return set_a(x); }
};


} // synfig namespace

#endif // __SYNFIG_COLOR_CAIROCOLORACUMULATOR_H
