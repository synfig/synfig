/* === S Y N F I G ========================================================= */
/*!	\file color.h
**	\brief Color Class
**
**	$Id$
**
**	\legal
**	Copyright (c) 2002-2005 Robert B. Quattlebaum Jr., Adrian Bentley
**	Copyright (c) 2007, 2008 Chris Moore
**	Copyright (c) 2012-2013 Carlos López
**  Copyright (c) 2015 Diego Barrios Romero
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

#ifndef __SYNFIG_COLOR_COLOR_H
#define __SYNFIG_COLOR_COLOR_H

#include <synfig/color/common.h>

namespace synfig {

class CairoColor;

/*!	\class Color
**	\ RGBA 128 bits Color class implementation
**	Future optimizations: lookup table for sqrt()?
*/
class Color
{
public:
	typedef ColorReal value_type;

private:
	value_type r_, g_, b_, a_;

public:
	
	static const value_type ceil;
	static const value_type floor;
	
	const String get_string(void) const;

	inline Color &	operator+= (const Color &rhs);
    inline Color &	operator-= (const Color &rhs);
	inline Color &	operator*= (const float &rhs);
	inline Color &	operator/= (const float &rhs);

	inline Color operator+ (const Color &rhs) const;
	inline Color operator- (const Color &rhs) const;
	inline Color operator* (const float &rhs) const;
	inline Color operator/ (const float &rhs) const;
	inline bool operator<  (const Color &rhs)const;
	inline bool operator== (const Color &rhs) const;
	inline bool operator!= (const Color &rhs) const;
	inline Color operator- () const;
	//! Effectively 1.0-color
	inline Color operator~() const;

	inline bool is_valid() const;

	inline Color premult_alpha() const;
	inline Color demult_alpha() const;

public:
	// ETL/trunk/ETL/_gaussian.h does:
	//   SR1=SR2=SR3=typename T::value_type();
	// and expects that to give it initialized colors
	// Otherwise the 'gaussian' blur type is random.
	inline Color();
	explicit inline Color(const value_type &f);
	explicit inline Color(int f);

	/*!	\param R Red
	**	\param G Green
	**	\param B Blue
	**	\param A Opacity(alpha) */
	inline Color(const value_type& R, const value_type& G,
          const value_type& B, const value_type& A=1);

	/*!	\param c Source for color components
	**	\param A Opacity(alpha) */
	inline Color(const Color& c, const value_type& A);

	//!	Copy constructor
	inline Color(const Color& c);

	//! Convert from CairoColor to Color
	inline Color(const CairoColor& c);
	
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

	//! Converts a 2 character hex string \a s (00-ff) into a ColorReal (0.0-1.0)
	static ColorReal hex2real(String s);

	//! Converts a ColorReal \a c (0.0-1.0) into a 2 character hex string (00-ff)
	static const String real2hex(ColorReal c);

	//! Returns the color as a 6 character hex string
	inline const String get_hex()const;

	//! Sets the color's R, G, and B from a 3 or 6 character hex string
	void set_hex(String& hex);

	//! Sets the RED component to \a x
	Color& set_r(const value_type& x) { r_ = x; return *this; }

	//! Sets the GREEN component to \a x
	Color& set_g(const value_type& x) { g_ = x; return *this; }

	//! Sets the BLUE component to \a x
	Color& set_b(const value_type& x) { b_ = x; return *this; }

	//! Sets the opacity (alpha) to \a x
	Color& set_a(const value_type& x) { a_ = x; return *this; }

	//! Synonym for set_a(). \see set_a()
	Color& set_alpha(const value_type& x) { return set_a(x); }

	//! Returns color's luminance
	inline float get_y() const;

	//! Returns U component of chromanance
	inline float get_u() const;

	//! Returns V component of chromanance
	inline float get_v() const;

	//! Returns the color's saturation
	/*!	This is is the magnitude of the U and V components.
	**	\see set_s() */
	inline float get_s() const;

	//! Sets the luminance (\a y) and chromanance (\a u and \a v)
	inline Color& set_yuv(const float &y, const float &u, const float &v);

	//! Sets color luminance
	inline Color& set_y(const float &y);

	//! Set U component of chromanance
	inline Color& set_u(const float &u);

	//! Set V component of chromanance
	inline Color& set_v(const float &v);

	//! Set the U and V components of chromanance
	inline Color& set_uv(const float& u, const float& v);

	//! Sets the color's saturation
	/*!	\see get_s() */
	inline Color& set_s(const float &x);

	//! YUV Color constructor
	inline static Color YUV(const float& y, const float& u,
                     const float& v, const value_type& a=1);

	//! Returns the hue of the chromanance
	/*!	This is the angle of the U and V components.
	**	\see set_hue() */
	inline Angle get_hue() const;

	//! Synonym for get_hue(). \see get_hue()
	inline Angle get_uv_angle() const;

	//! Sets the color's hue
	/*!	\see get_hue() */
	inline Color& set_hue(const Angle& theta);

	//! Synonym for set_hue(). \see set_hue()
	inline Color& set_uv_angle(const Angle& theta);

	//! Rotates the chromanance vector by amount specified by \a theta
	inline Color& rotate_uv(const Angle& theta);

	//! Sets the luminance (\a y) and chromanance (\a s and \a theta).
	/*!	\param y Luminance
	**	\param s Saturation
	**	\param theta Hue */
	inline Color& set_yuv(const float& y, const float& s, const Angle& theta);

	//! YUV color constructor where the chroma is in the saturation/hue form.
	/*!	\param y Luminance
	**	\param s Saturation
	**	\param theta Hue
	**	\param a Opacity (alpha) */
	inline static Color YUV(const float& y,
                     const float& s,
                     const Angle& theta,
                     const value_type& a=1);


	//! Clamps a color so that its values are in range. Ignores attempting to visualize negative colors.
    Color clamped() const;

	//! Clamps a color so that its values are in range.
    Color clamped_negative() const;

	/* Preset Colors */

	//! Preset Color Constructors
	//@{
#ifdef HAS_VIMAGE
	static inline Color alpha() { return Color(0,0,0,0.0000001f); }
#else
	static inline Color alpha() { return Color(0,0,0,0); }
#endif
	static inline Color black() { return Color(0,0,0); }
	static inline Color white() { return Color(1,1,1); }
	static inline Color gray() { return Color(0.5f,0.5f,0.5f); }
	static inline Color magenta() { return Color(1,0,1); }
	static inline Color red() { return Color(1,0,0); }
	static inline Color green() { return Color(0,1,0); }
	static inline Color blue() { return Color(0,0,1); }
	static inline Color cyan() { return Color(0,1,1); }
	static inline Color yellow() { return Color(1,1,0); }
	//@}

	enum Interpolation
	{
		INTERPOLATION_NEAREST = 0,
		INTERPOLATION_LINEAR = 1,
		INTERPOLATION_COSINE = 2,
		INTERPOLATION_CUBIC = 3
	};

	enum {
		INTERPOLATION_COUNT = 4
	};

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
		BLEND_ADD_COMPOSITE=22,		//!< Simple A+B (without cropping the outer part).
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

		BLEND_END=23,				//!< \internal
		BLEND_BY_LAYER=999			//! Used to let the layer decides what Blend Method use by
									//! default when the layer is created
	};

	typedef unsigned int BlendMethodFlags;

	enum {
		BLEND_METHODS_ONTO = 0
			| (1 << BLEND_BRIGHTEN)
			| (1 << BLEND_DARKEN)
			| (1 << BLEND_MULTIPLY)
			| (1 << BLEND_DIVIDE)
			| (1 << BLEND_COLOR)
			| (1 << BLEND_HUE)
			| (1 << BLEND_SATURATION)
			| (1 << BLEND_LUMINANCE)
			| (1 << BLEND_ONTO)
			| (1 << BLEND_STRAIGHT_ONTO)
			| (1 << BLEND_SCREEN)
			| (1 << BLEND_OVERLAY)
			| (1 << BLEND_HARD_LIGHT),

		BLEND_METHODS_STRAIGHT = 0
			| (1 << BLEND_STRAIGHT)
			| (1 << BLEND_STRAIGHT_ONTO)
			| (1 << BLEND_ALPHA_BRIGHTEN),

		BLEND_METHODS_OVERWRITE_ON_ALPHA_ONE = 0
			| (1 << BLEND_COMPOSITE),

		BLEND_METHODS_ASSOCIATIVE = 0
			| (1 << BLEND_COMPOSITE)
			| (1 << BLEND_ADD_COMPOSITE)
			| (1 << BLEND_BEHIND)
			| (1 << BLEND_ALPHA_DARKEN),

		BLEND_METHODS_COMMUTATIVE = 0
			| (1 << BLEND_ADD_COMPOSITE),

		BLEND_METHODS_ALL = (1 << BLEND_END) - 1
	};

	/* Other */
	static Color blend(Color a, Color b,float amount,BlendMethod type=BLEND_COMPOSITE);

	static bool is_onto(BlendMethod x)
		{ return BLEND_METHODS_ONTO & (1 << x); }

	//! a blending method is considered 'straight' if transparent pixels in the upper layer can affect the result of the blend
	static bool is_straight(BlendMethod x)
		{ return BLEND_METHODS_STRAIGHT & (1 << x); }
}; // END of class Color

} // synfig namespace

#include "color.hpp"

#endif // __SYNFIG_COLOR_COLOR_H

