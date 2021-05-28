/* === S Y N F I G ========================================================= */
/*!   \file synfig/color/pixelformat.h
**    \brief PixelFormat and conversions
**
**    \legal
**    Copyright (c) 2002-2005 Robert B. Quattlebaum Jr., Adrian Bentley
**    Copyright (c) 2007, 2008 Chris Moore
**    Copyright (c) 2012-2013 Carlos López
**    Copyright (c) 2015 Diego Barrios Romero
**    ......... ... 2018 Ivan Mahonin
**
**    This file is part of Synfig.
**
**    Synfig is free software: you can redistribute it and/or modify
**    it under the terms of the GNU General Public License as published by
**    the Free Software Foundation, either version 2 of the License, or
**    (at your option) any later version.
**
**    Synfig is distributed in the hope that it will be useful,
**    but WITHOUT ANY WARRANTY; without even the implied warranty of
**    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
**    GNU General Public License for more details.
**
**    You should have received a copy of the GNU General Public License
**    along with Synfig.  If not, see <https://www.gnu.org/licenses/>.
**    \endlegal
*/
/* ========================================================================= */

#ifndef __SYNFIG_COLOR_PIXELFORMAT_H
#define __SYNFIG_COLOR_PIXELFORMAT_H

#include "color.h"
#include "gamma.h"


#define FLAGS(x,y) (((x)&(y))==(y))

namespace synfig {

enum
{
/* Bit    Descriptions (ON/OFF)
** ----+-------------
** 0    Color Channels (Gray/RGB)
** 1    Alpha Channel (WITH/WITHOUT)
** 2    Endian (BGR/RGB)
** 3    Alpha Location (Start/End)
** 5    Premult Alpha
** 15   Raw Color (not conversion)
*/
    PF_RGB       = 0,
    PF_GRAY      = (1<<0), //!< If set, use one grayscale channel. If clear, use three channels for RGB
    PF_A         = (1<<1), //!< If set, include alpha channel
    PF_BGR       = (1<<2), //!< If set, reverse the order of the RGB channels
    PF_A_START   = (1<<3) | PF_A, //!< If set, alpha channel is before the color data. If clear, it is after.
    PF_A_PREMULT = (1<<6) | PF_A, //!< If set, the encoded color channels are alpha-premulted
    PF_RAW_COLOR = (1<<15)| PF_A, //!< If set, the data represents a raw Color data structure, and all other bits are ignored.
};

typedef unsigned int PixelFormat;

//! Returns the size of bytes of pixel in given PixelFormat
size_t pixel_size(PixelFormat x);

//! Converts pixels from synfig::Color to PixelFormat
//! Returns the pointer to the next dst pixel
//! dst_stride and src_stride - offset to next row in bytes (may be negative)
//! if stride is zero, then stride assumed to be equal width*sizeof(the_pixel_type)
//! src_stride must be evenly divisible by the sizeof(synfig::Color)
unsigned char*
color_to_pixelformat(
	unsigned char *dst,
	const Color *src,
	PixelFormat pf,
	const Gamma *gamma = NULL,
	int width = 1,
	int height = 1,
	int dst_stride = 0,
	int src_stride = 0 );

//! Converts pixels from PixelFormat to synfig::Color
//! Returns the pointer to the next src pixel
//! dst_stride and src_stride - offset to next row in bytes (may be negative)
//! if stride is zero, then stride assumed to be equal width*sizeof(the_pixel_type)
//! dst_stride must be evenly divisible by the sizeof(synfig::Color)
const unsigned char*
pixelformat_to_color(
	Color *dst,
	const unsigned char *src,
	PixelFormat pf,
	int width = 1,
	int height = 1,
	int dst_stride = 0,
	int src_stride = 0 );

} // synfig namespace

#endif // __SYNFIG_COLOR_PIXELFORMAT_H

