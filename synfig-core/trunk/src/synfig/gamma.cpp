/* === S Y N F I G ========================================================= */
/*!	\file gamma.cpp
**	\brief Template File
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

/* === H E A D E R S ======================================================= */

#ifdef USING_PCH
#	include "pch.h"
#else
#ifdef HAVE_CONFIG_H
#	include <config.h>
#endif

#include "gamma.h"
#include <cmath>
#include <algorithm>
#endif

/* === U S I N G =========================================================== */

using namespace std;
//using namespace etl;
using namespace synfig;

/* === M A C R O S ========================================================= */

/* === G L O B A L S ======================================================= */

/* === P R O C E D U R E S ================================================= */

/* === M E T H O D S ======================================================= */

void
Gamma::set_gamma(float x)
{
	gamma_r=gamma_g=gamma_b=x;
	int i;
	red_blue_level=1.0f;
	for(i=0;i<65536;i++)
	{
		float f(float(i)/65536.0f);
		f=pow(f,gamma_r);
		table_r_U16_to_U8[i]=table_g_U16_to_U8[i]=table_b_U16_to_U8[i]=(unsigned char)(f*(255.0f-(black_level*255))+0.5f + black_level*255.0f);
	}

	for(i=0;i<256;i++)
		table_r_U8_to_F32[i]=table_g_U8_to_F32[i]=table_b_U8_to_F32[i]=pow((float(i)/255.0f)*(1.0f-black_level)+black_level,gamma_r);
}


void
Gamma::refresh_gamma_r()
{
	int i;
//	const float scalar(min(red_blue_level,1.0f));
	const float scalar(1.0f);
	for(i=0;i<65536;i++)
	{
		float f(float(i)/65536.0f);
		f=pow(f,gamma_r)*scalar;
		table_r_U16_to_U8[i]=(unsigned char)(f*(255.0f-(black_level*255))+0.5f + black_level*255.0f);
	}

	for(i=0;i<256;i++)
		table_r_U8_to_F32[i]=pow((float(i)/255.0f)*(1.0f-black_level)+black_level,gamma_r)*scalar;
}

void
Gamma::refresh_gamma_g()
{
	int i;
//	const float scalar(sqrt(min(red_blue_level,2.0f-red_blue_level)));
	const float scalar(1.0f);
	for(i=0;i<65536;i++)
	{
		float f(float(i)/65536.0f);
		f=pow(f,gamma_g)*scalar;
		table_g_U16_to_U8[i]=(unsigned char)(f*(255.0f-(black_level*255))+0.5f + black_level*255.0f);
	}
	for(i=0;i<256;i++)
		table_g_U8_to_F32[i]=pow((float(i)/255.0f)*(1.0f-black_level)+black_level,gamma_g)*scalar;
}

void
Gamma::refresh_gamma_b()
{
	int i;
//	const float scalar(min(2.0f-red_blue_level,1.0f));
	const float scalar(1.0f);
	for(i=0;i<65536;i++)
	{
		float f(float(i)/65536.0f);
		f=pow(f,gamma_b)*scalar;
		table_b_U16_to_U8[i]=(unsigned char)(f*(255.0f-(black_level*255))+0.5f + black_level*255.0f);
	}
	for(i=0;i<256;i++)
		table_b_U8_to_F32[i]=pow((float(i)/255.0f)*(1.0f-black_level)+black_level,gamma_b)*scalar;
}

void
Gamma::set_gamma_r(float x)
{
	// If the gamma hasn't changed, then don't recompute the tables
	if(x==gamma_r) return;

	gamma_r=x;
	refresh_gamma_r();
}

void
Gamma::set_gamma_g(float x)
{
	// If the gamma hasn't changed, then don't recompute the tables
	if(x==gamma_g) return;

	gamma_g=x;
	refresh_gamma_g();
}

void
Gamma::set_gamma_b(float x)
{
	// If the gamma hasn't changed, then don't recompute the tables
	if(x==gamma_b) return;

	gamma_b=x;
	refresh_gamma_b();
}

void
Gamma::set_black_level(float x)
{
	// If the black_level hasn't changed, then don't recompute the tables
	if(x==black_level) return;

	black_level=x;

	// Rebuild tables
	refresh_gamma_r();
	refresh_gamma_g();
	refresh_gamma_b();
}

void
Gamma::set_red_blue_level(float x)
{
	// If the black_level hasn't changed, then don't recompute the tables
	if(x==red_blue_level) return;

	red_blue_level=x;

	// Rebuild tables
	refresh_gamma_r();
	refresh_gamma_g();
	refresh_gamma_b();
}

void
Gamma::set_all(float r, float g, float b, float black, float red_blue)
{
	// If nothing has changed, then don't recompute the tables
	if(gamma_r==r && gamma_g==g && gamma_b==b && black_level==black && red_blue_level==red_blue)
		return;

	gamma_r=r;
	gamma_g=g;
	gamma_b=b;
	black_level=black;
	red_blue_level=red_blue;

	// Rebuild tables
	refresh_gamma_r();
	refresh_gamma_g();
	refresh_gamma_b();
}
