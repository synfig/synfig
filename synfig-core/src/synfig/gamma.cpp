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

#include <cmath>
#include <cassert>
#include <cstring>

#include <algorithm>

#include "real.h"
#include "gamma.h"

#endif

/* === U S I N G =========================================================== */

using namespace synfig;

/* === M A C R O S ========================================================= */

/* === G L O B A L S ======================================================= */

/* === P R O C E D U R E S ================================================= */

const Gamma Gamma::no_gamma;

static void
build_table(float *table_U16_to_F32, unsigned short *table_U16_to_U16, float gamma)
{
	const float k = 1.f/65535.f;
	for(int i = 0; i < 65536; ++i) {
		float f = Gamma::calculate(k*float(i), gamma);
		table_U16_to_F32[i] = f;
		table_U16_to_U16[i] = (unsigned short)(f*65535.9f);
	}
}

/* === M E T H O D S ======================================================= */

Gamma::Gamma(float x):
	Gamma(x, x, x, x) { }

Gamma::Gamma(float r, float g, float b, float a) {
	gamma[0] = gamma[1] = gamma[2] = gamma[3] = -1.f;
	memset(table_U16_to_F32, 0, sizeof(table_U16_to_F32));
	memset(table_U16_to_U16, 0, sizeof(table_U16_to_U16));
	set_all(r, g, b, a);
}

void
Gamma::set_gamma(int channel, float x)
{
	if (gamma[channel] == x) return;

	assert(x > 0.f);
	gamma[channel] = x;
	for(int i = 0; i < 4; ++i)
		if (i != channel && gamma[i] == gamma[channel]) {
			memcpy(table_U16_to_F32[channel], table_U16_to_F32[i], sizeof(table_U16_to_F32[channel]));
			memcpy(table_U16_to_U16[channel], table_U16_to_U16[i], sizeof(table_U16_to_U16[channel]));
			return;
		}
	build_table(table_U16_to_F32[channel], table_U16_to_U16[channel], gamma[channel]);
}

