/* === S Y N F I G ========================================================= */
/*!	\file gamma.h
**	\brief Template Header
**
**	$Id$
**
**	\legal
**	Copyright (c) 2002-2005 Robert B. Quattlebaum Jr., Adrian Bentley
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

/* === S T A R T =========================================================== */

#ifndef __SYNFIG_GAMMA_H
#define __SYNFIG_GAMMA_H

/* === H E A D E R S ======================================================= */

#include <cmath>

/* === M A C R O S ========================================================= */

/* === T Y P E D E F S ===================================================== */

/* === C L A S S E S & S T R U C T S ======================================= */

namespace synfig {

/*!	\class Gamma
**	\brief This class performs color correction on Color classes.
**	\stub
*/
class Gamma
{
private:
	float gamma[4];
	float table_U16_to_F32[4][65536];
	unsigned short table_U16_to_U16[4][65536];

public:
	static const Gamma no_gamma;

	static float calculate(float f, float gamma) { return powf(f, gamma); }

	explicit Gamma(float x = 1.f);
	Gamma(float r, float g, float b, float a);

	void set_gamma(int channel, float x);
	inline void set_gamma_r(float x) { set_gamma(0, x); }
	inline void set_gamma_g(float x) { set_gamma(1, x); }
	inline void set_gamma_b(float x) { set_gamma(2, x); }
	inline void set_gamma_a(float x) { set_gamma(3, x); }
	inline void set_gamma(float x)
		{ set_all(x, x, x, x); }
	inline void set_all(float r, float g, float b, float a)
		{ set_gamma_r(r); set_gamma_g(g); set_gamma_b(b); set_gamma_a(a); }

	inline float get_gamma(int channel) const
		{ return gamma[channel]; }
	inline float get_gamma_r() const { return get_gamma(0); }
	inline float get_gamma_g() const { return get_gamma(1); }
	inline float get_gamma_b() const { return get_gamma(2); }
	inline float get_gamma_a() const { return get_gamma(3); }
	inline float get_gamma() const
		{ return (get_gamma_r() + get_gamma_g() + get_gamma_r() + get_gamma_a())*0.25f; }


	// conversions from U16

	inline float U16_to_F32(int channel, unsigned short i) const { return table_U16_to_F32[channel][i]; }
	inline float r_U16_to_F32(unsigned short i) const { return U16_to_F32(0, i); }
	inline float g_U16_to_F32(unsigned short i) const { return U16_to_F32(1, i); }
	inline float b_U16_to_F32(unsigned short i) const { return U16_to_F32(2, i); }
	inline float a_U16_to_F32(unsigned short i) const { return U16_to_F32(3, i); }

	inline unsigned short U16_to_U16(int channel, unsigned short i) const { return table_U16_to_U16[channel][i]; }
	inline unsigned short r_U16_to_U16(unsigned short i) const { return U16_to_U16(0, i); }
	inline unsigned short g_U16_to_U16(unsigned short i) const { return U16_to_U16(1, i); }
	inline unsigned short b_U16_to_U16(unsigned short i) const { return U16_to_U16(2, i); }
	inline unsigned short a_U16_to_U16(unsigned short i) const { return U16_to_U16(3, i); }

	inline unsigned char U16_to_U8(int channel, unsigned short i) const { return (unsigned char)(U16_to_U16(channel, i) >> 8); }
	inline unsigned char r_U16_to_U8(unsigned short i) const { return U16_to_U8(0, i); }
	inline unsigned char g_U16_to_U8(unsigned short i) const { return U16_to_U8(1, i); }
	inline unsigned char b_U16_to_U8(unsigned short i) const { return U16_to_U8(2, i); }
	inline unsigned char a_U16_to_U8(unsigned short i) const { return U16_to_U8(3, i); }


	// conversions from U8

	inline float U8_to_F32(int channel, unsigned char i) const { return U16_to_F32(channel, ((unsigned short)i) << 8); }
	inline float r_U8_to_F32(unsigned char i) const { return U8_to_F32(0, i); }
	inline float g_U8_to_F32(unsigned char i) const { return U8_to_F32(1, i); }
	inline float b_U8_to_F32(unsigned char i) const { return U8_to_F32(2, i); }
	inline float a_U8_to_F32(unsigned char i) const { return U8_to_F32(3, i); }

	inline unsigned short U8_to_U16(int channel, unsigned char i) const { return U16_to_U16(channel, ((unsigned short)i) << 8); }
	inline unsigned short r_U8_to_U16(unsigned char i) const { return U8_to_U16(0, i); }
	inline unsigned short g_U8_to_U16(unsigned char i) const { return U8_to_U16(1, i); }
	inline unsigned short b_U8_to_U16(unsigned char i) const { return U8_to_U16(2, i); }
	inline unsigned short a_U8_to_U16(unsigned char i) const { return U8_to_U16(3, i); }

	inline unsigned char U8_to_U8(int channel, unsigned char i) const { return U16_to_U8(channel, ((unsigned short)i) << 8); }
	inline unsigned char r_U8_to_U8(unsigned short i) const { return U8_to_U8(0, i); }
	inline unsigned char g_U8_to_U8(unsigned short i) const { return U8_to_U8(1, i); }
	inline unsigned char b_U8_to_U8(unsigned short i) const { return U8_to_U8(2, i); }
	inline unsigned char a_U8_to_U8(unsigned short i) const { return U8_to_U8(3, i); }


	// conversions from F32

	inline float F32_to_F32(int channel, float f) const { return calculate(f, get_gamma(channel)); }
	inline float r_F32_to_F32(float f) const { return F32_to_F32(0, f); }
	inline float g_F32_to_F32(float f) const { return F32_to_F32(1, f); }
	inline float b_F32_to_F32(float f) const { return F32_to_F32(2, f); }
	inline float a_F32_to_F32(float f) const { return F32_to_F32(3, f); }

	inline unsigned short F32_to_U16(int channel, float f) const { return U16_to_U16(channel, (unsigned short)(f*65535.9f)); }
	inline unsigned short r_F32_to_U16(float f) const { return F32_to_U16(0, f); }
	inline unsigned short g_F32_to_U16(float f) const { return F32_to_U16(1, f); }
	inline unsigned short b_F32_to_U16(float f) const { return F32_to_U16(2, f); }
	inline unsigned short a_F32_to_U16(float f) const { return F32_to_U16(3, f); }

	inline unsigned char F32_to_U8(int channel, float f) const { return U16_to_U8(channel, (unsigned short)(f*65535.9f)); }
	inline unsigned char r_F32_to_U8(float f) const { return F32_to_U8(0, f); }
	inline unsigned char g_F32_to_U8(float f) const { return F32_to_U8(1, f); }
	inline unsigned char b_F32_to_U8(float f) const { return F32_to_U8(2, f); }
	inline unsigned char a_F32_to_U8(float f) const { return F32_to_U8(3, f); }
}; // END of class Gamma

}; // END of namespace synfig

/* === E N D =============================================================== */

#endif
