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

static const float EncodeYUV[3][3]=
{
	{ 0.299f, 0.587f, 0.114f },
	{ -0.168736f, -0.331264f, 0.5f },
	{ 0.5f, -0.418688f, -0.081312f }
};

static const float DecodeYUV[3][3]=
{
	{ 1.0f, 0.0f, 1.402f },
	{ 1.0f, -0.344136f, -0.714136f },
	{ 1.0f, 1.772f, 0.0f }
};

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

	//! Returns color's luminance
	float get_y() const
	{
		return
			(float)get_r()*EncodeYUV[0][0]+
			(float)get_g()*EncodeYUV[0][1]+
			(float)get_b()*EncodeYUV[0][2];
	}

	//! Returns U component of chromanance
	float get_u() const
	{
		return
			(float)get_r()*EncodeYUV[1][0]+
			(float)get_g()*EncodeYUV[1][1]+
			(float)get_b()*EncodeYUV[1][2];
	}

	//! Returns V component of chromanance
	float get_v() const
	{
		return
			(float)get_r()*EncodeYUV[2][0]+
			(float)get_g()*EncodeYUV[2][1]+
			(float)get_b()*EncodeYUV[2][2];
	}

	//! Returns the color's saturation
	/*!	This is is the magnitude of the U and V components.
	**	\see set_s() */
	float get_s() const
	{
		const float u(get_u()), v(get_v());
		return sqrt(u*u+v*v);
	}

	//! Returns the hue of the chromanance
	/*!	This is the angle of the U and V components.
	**	\see set_hue() */
	Angle get_hue() const	{ return Angle::tan(get_u(),get_v()); }

	//! Synonym for get_hue(). \see get_hue()
	Angle get_uv_angle() const { return get_hue(); }

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

	static bool is_onto(BlendMethod x)
	{
		return x==BLEND_BRIGHTEN
			|| x==BLEND_DARKEN
			|| x==BLEND_ADD
			|| x==BLEND_SUBTRACT
			|| x==BLEND_MULTIPLY
			|| x==BLEND_DIVIDE
			|| x==BLEND_COLOR
			|| x==BLEND_HUE
			|| x==BLEND_SATURATION
			|| x==BLEND_LUMINANCE
			|| x==BLEND_ONTO
			|| x==BLEND_STRAIGHT_ONTO
			|| x==BLEND_SCREEN
			|| x==BLEND_OVERLAY
			|| x==BLEND_DIFFERENCE
			|| x==BLEND_HARD_LIGHT
		;
	}

	//! a blending method is considered 'straight' if transparent pixels in the upper layer can affect the result of the blend
	static bool is_straight(BlendMethod x)
	{
		return x==BLEND_STRAIGHT
			|| x==BLEND_STRAIGHT_ONTO
			|| x==BLEND_ALPHA_BRIGHTEN
		;
	}

	bool operator==(const ColorBase<T>& rhs) const
	{ return r_==rhs.r_ && g_==rhs.g_ && b_==rhs.b_ && a_==rhs.a_; }

	bool operator!=(const ColorBase<T>& rhs) const
	{ return r_!=rhs.r_ || g_!=rhs.g_ || b_!=rhs.b_ || a_!=rhs.a_; }

	virtual ~ColorBase() {}
};

} /* namespace synfig */

#endif /* __SYNFIG_COLORBASE_H */

