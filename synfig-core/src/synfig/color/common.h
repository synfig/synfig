/* === S Y N F I G ========================================================= */
/*!	\file
**	\brief Common definitions for color classes
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

/* === S T A R T =========================================================== */

#ifndef __SYNFIG_COLOR_COMMON_H
#define __SYNFIG_COLOR_COMMON_H

/* === H E A D E R S ======================================================= */

#include <cmath>
#include <stdint.h>
#include <synfig/gamma.h>
#include <synfig/string.h>
#include <synfig/angle.h>

#ifdef USE_HALF_TYPE
#include <OpenEXR/half.h>
#endif

/* === M A C R O S ========================================================= */

#define use_colorspace_gamma()	App::use_colorspace_gamma
#define colorspace_gamma()		(2.2f)
#define gamma_in(x)				((x>=0) ? pow((float)x,1.0f/colorspace_gamma()) : -pow((float)-x,1.0f/colorspace_gamma()))
#define gamma_out(x)			((x>=0) ? pow((float)x,     colorspace_gamma()) : -pow((float)-x,     colorspace_gamma()))

// For some reason isnan() isn't working on macosx any more.
// This is a quick fix.
#if defined(__APPLE__) && !defined(SYNFIG_ISNAN_FIX)
    #ifdef isnan
        #undef isnan
    #endif
    inline bool isnan(double x) { return x != x; }
    inline bool isnan(float x) { return x != x; }
    #define SYNFIG_ISNAN_FIX 1
#else
    #ifndef isnan
        #define isnan(x) (std::isnan)(x)
    #endif
#endif

namespace synfig {

#ifdef USE_HALF_TYPE
typedef half ColorReal;
#else
typedef float ColorReal;
#endif

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


} // synfig namespace 

#endif // __SYNFIG_COLOR_COMMON_H

