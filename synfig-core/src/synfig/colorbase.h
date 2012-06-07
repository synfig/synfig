/* === S Y N F I G ========================================================= */
/*!	\file colorbase.h
**	\brief Base class for Color classes
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

#ifndef __SYNFIG_COLORBASE_H
#define __SYNFIG_COLORBASE_H

namespace synfig {

template<typename T>
class ColorBase
{
protected:
	T a_, r_, g_, b_;
public:
	// ETL/trunk/ETL/_gaussian.h does:
	//   SR1=SR2=SR3=typename T::value_type();
	// and expects that to give it initialized colors
	// Otherwise the 'gaussian' blur type is random.
	ColorBase() :a_(0), r_(0), g_(0), b_(0) { }
	ColorBase(const T &f) :a_(f),r_(f), g_(f), b_(f) { }

	/*!	\param R Red
	**	\param G Green
	**	\param B Blue
	**	\param A Opacity(alpha) */
	ColorBase(const T& R, const T& G, const T& B, const T& A):
		a_(A),
		r_(R),
		g_(G),
		b_(B) { }

	/*!	\param c Source for color components
	**	\param A Opacity(alpha) */
	ColorBase(const ColorBase<T>& c, const T& A):
		a_(A),
		r_(c.r_),
		g_(c.g_),
		b_(c.b_) { }

	//!	Copy constructor
	ColorBase(const ColorBase& c):
		a_(c.a_),
		r_(c.r_),
		g_(c.g_),
		b_(c.b_) { }

	//! Returns the RED component
	const T& get_r()const { return r_; }

	//! Returns the GREEN component
	const T& get_g()const { return g_; }

	//! Returns the BLUE component
	const T& get_b()const { return b_; }

	//! Returns the amount of opacity (alpha)
	const T& get_a()const { return a_; }

	//! Synonym for get_a(). \see get_a()
	const T& get_alpha()const { return get_a(); }

	//! Sets the RED component to \a x
	ColorBase<T>& set_r(const T& x) { r_ = x; return *this; }

	//! Sets the GREEN component to \a x
	ColorBase<T>& set_g(const T& x) { g_ = x; return *this; }

	//! Sets the BLUE component to \a x
	ColorBase<T>& set_b(const T& x) { b_ = x; return *this; }

	//! Sets the opacity (alpha) to \a x
	ColorBase<T>& set_a(const T& x) { a_ = x; return *this; }

	//! Synonym for set_a(). \see set_a()
	ColorBase<T>& set_alpha(const T& x) { return set_a(x); }

	//! \writeme
	enum BlendMethod
	{
		BLEND_COMPOSITE=0,			//!< Color A is composited onto B (Taking A's alpha into account)
		BLEND_STRAIGHT=1,			//!< Straight linear interpolation from A->B (Alpha ignored)
		BLEND_ONTO=13,				//!< Similar to BLEND_COMPOSITE, except that B's alpha is maintained
		BLEND_STRAIGHT_ONTO=21,		//!< \deprecated \writeme
		BLEND_BEHIND=12,			//!< Similar to BLEND_COMPOSITE, except that B is composited onto A.
		BLEND_SCREEN=16,			//!< \writeme
		BLEND_OVERLAY=20,			//!< \writeme
		BLEND_HARD_LIGHT=17,		//!< \writeme
		BLEND_MULTIPLY=6,			//!< Simple A*B.
		BLEND_DIVIDE=7,				//!< Simple B/A
		BLEND_ADD=4,				//!< Simple A+B.
		BLEND_SUBTRACT=5,			//!< Simple A-B.
		BLEND_DIFFERENCE=18,		//!< Simple |A-B|.
		BLEND_BRIGHTEN=2,			//!< If composite is brighter than B, use composite. B otherwise.
		BLEND_DARKEN=3,				//!< If composite is darker than B, use composite. B otherwise.
		BLEND_COLOR=8,				//!< Preserves the U and V channels of color A
		BLEND_HUE=9,				//!< Preserves the angle of the UV vector of color A
		BLEND_SATURATION=10,		//!< Preserves the magnitude of the UV Vector of color A
		BLEND_LUMINANCE=11,			//!< Preserves the Y channel of color A

		BLEND_ALPHA_BRIGHTEN=14,	//!< \deprecated If A is less opaque than B, use A
		BLEND_ALPHA_DARKEN=15,		//!< \deprecated If A is more opaque than B, use B
		BLEND_ALPHA_OVER=19,		//!< \deprecated multiply alphas and then straight blends using the amount

		BLEND_END=22,				//!< \internal
		BLEND_BY_LAYER=999			//! Used to let the layer decides what Blend Method use by
									//! default when the layer is created
	};


	virtual ColorBase<T> blend(ColorBase<T> a, ColorBase<T> b, float amount, BlendMethod type) {}

	virtual ~ColorBase() {}
};

} /* namespace synfig */

#endif /* __SYNFIG_COLORBASE_H */

