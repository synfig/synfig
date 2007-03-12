/* === S Y N F I G ========================================================= */
/*!	\file trgt_yuv.cpp
**	\brief Template File
**
**	$Id: trgt_yuv.cpp,v 1.1.1.1 2005/01/04 01:23:14 darco Exp $
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

#define SYNFIG_TARGET

#ifdef USING_PCH
#	include "pch.h"
#else
#ifdef HAVE_CONFIG_H
#	include <config.h>
#endif

#include "trgt_yuv.h"
#include <ETL/stringf>
#include <cstdio>
#include <algorithm>
#include <functional>
#endif

using namespace synfig;
using namespace std;
using namespace etl;

/* === M A C R O S ========================================================= */

#define Y_FLOOR	(16)
#define Y_CEIL	(235)
#define Y_RANGE (Y_CEIL-Y_FLOOR)

#define UV_FLOOR	(16)
#define UV_CEIL		(240)
#define UV_RANGE (UV_CEIL-UV_FLOOR)

/* === G L O B A L S ======================================================= */

SYNFIG_TARGET_INIT(yuv);
SYNFIG_TARGET_SET_NAME(yuv,"yuv420p");
SYNFIG_TARGET_SET_EXT(yuv,"yuv");
SYNFIG_TARGET_SET_VERSION(yuv,"0.1");
SYNFIG_TARGET_SET_CVS_ID(yuv,"$Id: trgt_yuv.cpp,v 1.1.1.1 2005/01/04 01:23:14 darco Exp $");

/* === M E T H O D S ======================================================= */

yuv::yuv(const char *FILENAME):
	filename(FILENAME),
	file( (filename=="-")?stdout:fopen(filename.c_str(),"wb") ),
	dithering(true)
{
	// YUV420P doesn't have an alpha channel
	set_remove_alpha();
}

yuv::~yuv()
{
}

bool
yuv::set_rend_desc(RendDesc *given_desc)
{
	given_desc->clear_flags();

	// Make sure our width is divisible by two
	given_desc->set_w(given_desc->get_w()*2/2);
	given_desc->set_h(given_desc->get_h()*2/2);

	desc=*given_desc;

	// Set up our surface
	surface.set_wh(desc.get_w(),desc.get_h());

	return true;
}

bool
yuv::start_frame(synfig::ProgressCallback *callback)
{
	return static_cast<bool>(file);
}

Color *
yuv::start_scanline(int x)
{
	return surface[x];
}

bool
yuv::end_scanline()
{
	return static_cast<bool>(file);
}

void
yuv::end_frame()
{
	const int w=desc.get_w(),h=desc.get_h();
	int x,y;

	assert(file);

	// Output Y' channel, adjusting
	// the gamma as we go
	for(y=0;y<h;y++)
		for(x=0;x<w;x++)
		{
			Color& c(surface[y][x]);
			c=c.clamped();
			c.set_r(gamma().r_F32_to_F32(c.get_r()));
			c.set_g(gamma().g_F32_to_F32(c.get_g()));
			c.set_b(gamma().b_F32_to_F32(c.get_b()));
			float f(c.get_y());
			int i(max(min(round_to_int(c.get_y()*Y_RANGE),Y_RANGE),0)+Y_FLOOR);

			if(dithering)
			{
				const float er(f-((float)i-Y_FLOOR)/Y_RANGE);
				const Color error(er,er,er);

				if(surface.get_h()>y+1)
				{
					surface[y+1][x-1]+=error * ((float)3/(float)16);
					surface[y+1][x]+=error * ((float)5/(float)16);
					if(surface.get_w()>x+1)
						surface[y+1][x+1]+=error * ((float)1/(float)16);
				}
				if(surface.get_w()>x+1)
					surface[y][x+1]+=error * ((float)7/(float)16);
			}

			fputc(i,file.get());
		}


	// Create new super-sampled surface
	Surface sm_surface(w/2,h/2);
	for(y=0;y<h;y+=2)
		for(x=0;x<w;x+=2)
		{
			Color c(Color::alpha());
			c+=surface[y][x];
			c+=surface[y+1][x];
			c+=surface[y][x+1];
			c+=surface[y+1][x+1];
			c/=4;
			sm_surface[y/2][x/2]=c;
		}

	// Output U channel
	for(y=0;y<sm_surface.get_h();y++)
		for(x=0;x<sm_surface.get_w();x++)
		{
			const Color& c(sm_surface[y][x]);
			const float f(c.get_u());
			const int i(max(min(round_to_int((f+0.5f)*UV_RANGE),UV_RANGE),0)+UV_FLOOR);

			if(dithering)
			{
				const float er(f-((((float)i-UV_FLOOR)/UV_RANGE)-0.5f));
				const Color error(Color::YUV(0,er,0));

				if(sm_surface.get_h()>y+1)
				{
					sm_surface[y+1][x-1]+=error * ((float)3/(float)16);
					sm_surface[y+1][x]+=error * ((float)5/(float)16);
					if(sm_surface.get_w()>x+1)
						sm_surface[y+1][x+1]+=error * ((float)1/(float)16);
				}
				if(sm_surface.get_w()>x+1)
					sm_surface[y][x+1]+=error * ((float)7/(float)16);
			}
			fputc(i,file.get());
		}

	// Output V channel
	for(y=0;y<sm_surface.get_h();y++)
		for(x=0;x<sm_surface.get_w();x++)
		{
			const Color& c(sm_surface[y][x]);
			const float f(c.get_v());
			const int i(max(min(round_to_int((f+0.5f)*UV_RANGE),UV_RANGE),0)+UV_FLOOR);

			if(dithering)
			{
				const float er(f-((((float)i-UV_FLOOR)/UV_RANGE)-0.5f));
				const Color error(Color::YUV(0,0,er));

				if(sm_surface.get_h()>y+1)
				{
					sm_surface[y+1][x-1]+=error * ((float)3/(float)16);
					sm_surface[y+1][x]+=error * ((float)5/(float)16);
					if(sm_surface.get_w()>x+1)
						sm_surface[y+1][x+1]+=error * ((float)1/(float)16);
				}
				if(sm_surface.get_w()>x+1)
					sm_surface[y][x+1]+=error * ((float)7/(float)16);
			}
			fputc(i,file.get());
		}

	// Flush out the frame
	fflush(file.get());
}
