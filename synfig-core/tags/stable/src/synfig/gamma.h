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
	float gamma_r;
	float gamma_g;
	float gamma_b;
	float black_level;
	float red_blue_level;

	unsigned char table_r_U16_to_U8[65536];
	unsigned char table_g_U16_to_U8[65536];
	unsigned char table_b_U16_to_U8[65536];

	float table_r_U8_to_F32[256];
	float table_g_U8_to_F32[256];
	float table_b_U8_to_F32[256];

public:
	Gamma(float x=1):black_level(0) { set_gamma(x); }

	void set_gamma(float x);
	void set_gamma_r(float x);
	void set_gamma_g(float x);
	void set_gamma_b(float x);
	void set_black_level(float x);

	void set_red_blue_level(float x);
	void set_all(float r, float g, float b, float black, float red_blue=1.0f);

	float get_gamma()const { return (gamma_r+gamma_g+gamma_b)*0.33333333; }
	float get_gamma_r()const { return gamma_r; }
	float get_gamma_g()const { return gamma_g; }
	float get_gamma_b()const { return gamma_b; }
	float get_black_level()const { return black_level; }
	float get_red_blue_level()const { return red_blue_level; }

	void refresh_gamma_r();
	void refresh_gamma_g();
	void refresh_gamma_b();

	const unsigned char &r_U16_to_U8(int i)const { return table_r_U16_to_U8[i]; }
	const unsigned char &g_U16_to_U8(int i)const { return table_g_U16_to_U8[i]; }
	const unsigned char &b_U16_to_U8(int i)const { return table_b_U16_to_U8[i]; }

	const unsigned char &r_F32_to_U8(float x)const { return table_r_U16_to_U8[(int)(x*65535.0f)]; }
	const unsigned char &g_F32_to_U8(float x)const { return table_g_U16_to_U8[(int)(x*65535.0f)]; }
	const unsigned char &b_F32_to_U8(float x)const { return table_b_U16_to_U8[(int)(x*65535.0f)]; }

	unsigned short r_F32_to_U16(float x)const { return (unsigned short)table_r_U16_to_U8[(int)(x*65535.0f)]<<8; }
	unsigned short g_F32_to_U16(float x)const { return (unsigned short)table_g_U16_to_U8[(int)(x*65535.0f)]<<8; }
	unsigned short b_F32_to_U16(float x)const { return (unsigned short)table_b_U16_to_U8[(int)(x*65535.0f)]<<8; }

	const float& r_U8_to_F32(int i)const { return table_r_U8_to_F32[i]; }
	const float& g_U8_to_F32(int i)const { return table_g_U8_to_F32[i]; }
	const float& b_U8_to_F32(int i)const { return table_b_U8_to_F32[i]; }

	float r_F32_to_F32(float x)const { return static_cast<float>(pow(x,gamma_r)*(1.0f-black_level)+black_level); }
	float g_F32_to_F32(float x)const { return static_cast<float>(pow(x,gamma_g)*(1.0f-black_level)+black_level); }
	float b_F32_to_F32(float x)const { return static_cast<float>(pow(x,gamma_b)*(1.0f-black_level)+black_level); }
}; // END of class Gamma

}; // END of namespace synfig

/* === E N D =============================================================== */

#endif
