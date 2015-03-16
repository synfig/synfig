/* === S Y N F I G ========================================================= */
/*!    \file
**    \brief PixelFormat and conversions
**
**    $Id$
**
**    \legal
**    Copyright (c) 2002-2005 Robert B. Quattlebaum Jr., Adrian Bentley
**    Copyright (c) 2007, 2008 Chris Moore
**    Copyright (c) 2012-2013 Carlos López
**    Copyright (c) 2015 Diego Barrios Romero
**
**    This package is free software; you can redistribute it and/or
**    modify it under the terms of the GNU General Public License as
**    published by the Free Software Foundation; either version 2 of
**    the License, or (at your option) any later version.
**
**    This package is distributed in the hope that it will be useful,
**    but WITHOUT ANY WARRANTY; without even the implied warranty of
**    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
**    General Public License for more details.
**    \endlegal
*/
/* ========================================================================= */

#ifndef __SYNFIG_COLOR_PIXELFORMAT_H
#define __SYNFIG_COLOR_PIXELFORMAT_H

#include <synfig/color/color.h>

namespace synfig {


enum PixelFormat
{
/* Bit    Descriptions (ON/OFF)
** ----+-------------
** 0    Color Channels (Gray/RGB)
** 1    Alpha Channel (WITH/WITHOUT)
** 2    ZDepth    (WITH/WITHOUT)
** 3    Endian (BGR/RGB)
** 4    Alpha Location (Start/End)
** 5    ZDepth Location (Start/End)
** 6    Alpha/ZDepth Arrangement (ZA,AZ)
** 7    Alpha Range (Inverted,Normal)
** 8    Z Range (Inverted,Normal)
*/
    PF_RGB       = 0,
    PF_GRAY      = (1<<0), //!< If set, use one grayscale channel. If clear, use three channels for RGB
    PF_A         = (1<<1), //!< If set, include alpha channel
    PF_Z         = (1<<2), //!< If set, include ZDepth channel
    PF_BGR       = (1<<3), //!< If set, reverse the order of the RGB channels
    PF_A_START   = (1<<4), //!< If set, alpha channel is before the color data. If clear, it is after.
    PF_Z_START   = (1<<5), //!< If set, ZDepth channel is before the color data. If clear, it is after.
    PF_ZA        = (1<<6), //!< If set, the ZDepth channel will be in front of the alpha channel. If clear, they are reversed.
    PF_A_INV     = (1<<7), //!< If set, the alpha channel is stored as 1.0-a
    PF_Z_INV     = (1<<8), //!< If set, the ZDepth channel is stored as 1.0-z
    PF_RAW_COLOR = (1<<9)+(1<<1) //!< If set, the data represents a raw Color data structure, and all other bits are ignored.
};

inline PixelFormat operator|(PixelFormat lhs, PixelFormat rhs)
    { return static_cast<PixelFormat>((int)lhs|(int)rhs); }

inline PixelFormat operator&(PixelFormat lhs, PixelFormat rhs)
    { return static_cast<PixelFormat>((int)lhs&(int)rhs); }
#define FLAGS(x,y)        (((x)&(y))==(y))

//! Returns the number of channels that the given PixelFormat calls for
inline int channels(const PixelFormat x)
{
    int chan = 0;
    if(FLAGS(x, PF_GRAY))
    {
        ++chan;
    }
    else
    {
        chan += 3;
    }

    if(FLAGS(x, PF_A))
    {
        ++chan;
    }
    if(FLAGS(x, PF_Z))
    {
        ++chan;
    }
    if(FLAGS(x, PF_RAW_COLOR))
    {
        chan = sizeof(Color);
    }

    return chan;
}

inline unsigned char * Color2PixelFormat(const Color &color, const PixelFormat &pf,
                                         unsigned char *out, const Gamma &gamma)
{
    if(FLAGS(pf, PF_RAW_COLOR))
    {
        Color *outcol = reinterpret_cast<Color *>(out);
        *outcol = color;
        out += sizeof(color);
        return out;
    }

    int alpha = 0;
    if (FLAGS(pf, PF_A_INV))
    {
        alpha = (-(float)color.get_a()+1) * 255;
    }
    else
    {
        alpha = (float)color.get_a() * 255;
    }

    if(alpha < 0)
    {
        alpha=0;
    }
    if(alpha > 255)
    {
        alpha=255;
    }

    if(FLAGS(pf, PF_ZA|PF_A_START|PF_Z_START))
    {
        if(FLAGS(pf, PF_Z_START))
        {
            out++;
        }
        if(FLAGS(pf, PF_A_START))
        {
            *out++ = static_cast<unsigned char>(alpha);
        }
    }
    else
    {
        if(FLAGS(pf, PF_A_START))
        {
            *out++ = static_cast<unsigned char>(alpha);
        }
        if(FLAGS(pf, PF_Z_START))
        {
            out++;
        }
    }

    if(FLAGS(pf,PF_GRAY))
    {
        *out++ = static_cast<unsigned char>(gamma.g_F32_to_U8(color.get_y()));
    }
    else
    {
        if(FLAGS(pf,PF_BGR))
        {
            *out++ = static_cast<unsigned char>(gamma.r_F32_to_U8(color.get_b()));
            *out++ = static_cast<unsigned char>(gamma.g_F32_to_U8(color.get_g()));
            *out++ = static_cast<unsigned char>(gamma.b_F32_to_U8(color.get_r()));
        }
        else
        {
            *out++ = static_cast<unsigned char>(gamma.r_F32_to_U8(color.get_r()));
            *out++ = static_cast<unsigned char>(gamma.g_F32_to_U8(color.get_g()));
            *out++ = static_cast<unsigned char>(gamma.b_F32_to_U8(color.get_b()));
        }
    }

    if(FLAGS(pf, PF_ZA))
    {
        if(!FLAGS(pf, PF_Z_START) && FLAGS(pf, PF_Z))
        {
            out++;
        }
        if(!FLAGS(pf, PF_A_START) && FLAGS(pf, PF_A))
        {
            *out++ = static_cast<unsigned char>(alpha);
        }
    }
    else
    {
        if(!FLAGS(pf, PF_Z_START) && FLAGS(pf, PF_Z))
        {
            out++;
        }
        if(!FLAGS(pf, PF_A_START) && FLAGS(pf, PF_A))
        {
            *out++ = static_cast<unsigned char>(alpha);
        }
    }

    return out;
}

inline void convert_color_format(unsigned char *dest, const Color *src,
                                 int w, PixelFormat pf,const Gamma &gamma)
{
    assert(w >= 0);
    while(w--)
    {
        dest = Color2PixelFormat((*(src++)).clamped(),
                                 pf, dest, gamma);
    }
}

inline const unsigned char * PixelFormat2Color(Color &color,
                                               const PixelFormat &pf,
                                               const unsigned char *out)
{
    if(FLAGS(pf, PF_ZA|PF_A_START|PF_Z_START))
    {
        if(FLAGS(pf, PF_Z_START))
        {
            out++;
        }
        if(FLAGS(pf,PF_A_START))
        {
            color.set_a((float)*out++ / 255);
        }
    }
    else
    {
        if(FLAGS(pf, PF_A_START))
        {
            color.set_a((float)*out++ / 255);
        }
        if(FLAGS(pf, PF_Z_START))
        {
            out++;
        }
    }

    if(FLAGS(pf, PF_GRAY))
    {
        color.set_yuv((float)*out++ / 255, 0, 0);
    }
    else
    {
        if(FLAGS(pf, PF_BGR))
        {
            color.set_b((float)*out++ / 255);
            color.set_g((float)*out++ / 255);
            color.set_r((float)*out++ / 255);
        }
        else
        {
            color.set_r((float)*out++ / 255);
            color.set_g((float)*out++ / 255);
            color.set_b((float)*out++ / 255);
        }
    }

    if(FLAGS(pf, PF_ZA))
    {
        if(!FLAGS(pf, PF_Z_START) && FLAGS(pf, PF_Z))
        {
            out++;
        }
        if(!FLAGS(pf, PF_A_START) && FLAGS(pf, PF_A))
        {
            color.set_a((float)*out++ / 255);
        }
    }
    else
    {
        if(!FLAGS(pf, PF_A_START) && FLAGS(pf, PF_A))
        {
            color.set_a((float)*out++ / 255);
        }
        if(!FLAGS(pf, PF_Z_START) && FLAGS(pf, PF_Z))
        {
            out++;
        }
    }

    return out;
}

} // synfig namespace

#endif // __SYNFIG_COLOR_PIXELFORMAT_H

