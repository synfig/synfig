/* === S Y N F I G ========================================================= */
/*!	\file mptr_cairo_png.cpp
**	\brief png Cairo Import Module
**
**	$Id$
**
**	\legal
**	Copyright (c) 2002-2005 Robert B. Quattlebaum Jr., Adrian Bentley
**	Copyright (c) 2012-2013 Carlos López
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
**
** === N O T E S ===========================================================
**
** ========================================================================= */

/* === H E A D E R S ======================================================= */

#ifdef USING_PCH
#	include "pch.h"
#else
#ifdef HAVE_CONFIG_H
#	include <config.h>
#endif

#include "mptr_cairo_png.h"
#include <synfig/cairoimporter.h>
#include <synfig/time.h>
#include <synfig/general.h>
#include <synfig/localization.h>


#include <cstdio>
#include <algorithm>
#include <functional>
#endif

/* === M A C R O S ========================================================= */

using namespace synfig;
using namespace std;
using namespace etl;

/* === G L O B A L S ======================================================= */

SYNFIG_CAIROIMPORTER_INIT(cairo_png_mptr);
SYNFIG_CAIROIMPORTER_SET_NAME(cairo_png_mptr,"cairo_png");
SYNFIG_CAIROIMPORTER_SET_EXT(cairo_png_mptr,"png");
SYNFIG_CAIROIMPORTER_SET_VERSION(cairo_png_mptr,"0.1");
SYNFIG_CAIROIMPORTER_SET_CVS_ID(cairo_png_mptr,"$Id$");
SYNFIG_CAIROIMPORTER_SET_SUPPORTS_FILE_SYSTEM_WRAPPER(cairo_png_mptr, true);

/* === M E T H O D S ======================================================= */

cairo_status_t
cairo_png_mptr::read_callback(void *closure, unsigned char *data, unsigned int length)
{
	unsigned int s = closure == NULL ? 0
				 : ((FileSystem::ReadStream*)closure)->read_block(data, length);
	if (s < length) {
		memset(data + s, 0, length - s);
		return CAIRO_STATUS_READ_ERROR;
	}
	return CAIRO_STATUS_SUCCESS;
}

cairo_png_mptr::cairo_png_mptr(const synfig::FileSystem::Identifier &identifier):
	CairoImporter(identifier)
{
	FileSystem::ReadStream::Handle stream = identifier.get_read_stream();
	csurface_=cairo_image_surface_create_from_png_stream(read_callback, stream.get());
	stream.reset();
	if(cairo_surface_status(csurface_))
	{
		throw strprintf("Unable to physically open %s",identifier.filename.c_str());
		cairo_surface_destroy(csurface_);
		csurface_=NULL;
		return;
	}
	CairoSurface cairo_s;
	cairo_s.set_cairo_surface(csurface_);
	if(!cairo_s.map_cairo_image())
	   return;
	int w=cairo_s.get_w();
	int h=cairo_s.get_h();
	for(int y=0; y<h; y++)
		for(int x=0; x<w; x++)
		{
			CairoColor c=cairo_s[y][x];
			if (c.get_alpha()) {
				float a=c.get_alpha();
				c.set_a( gamma().a_U8_to_U8(c.get_alpha()) );
				float aa=c.get_alpha();
				unsigned char r=(unsigned char)(aa*gamma().r_F32_to_F32(c.get_r()/a));
				unsigned char g=(unsigned char)(aa*gamma().g_F32_to_F32(c.get_g()/a));
				unsigned char b=(unsigned char)(aa*gamma().b_F32_to_F32(c.get_b()/a));
				c.set_r(r);
				c.set_g(g);
				c.set_b(b);
			}
			cairo_s[y][x]=c;
		}
	cairo_s.unmap_cairo_image();
}

cairo_png_mptr::~cairo_png_mptr()
{
	if(csurface_ && !cairo_surface_status(csurface_))
		cairo_surface_destroy(csurface_);
}

bool
cairo_png_mptr::get_frame(cairo_surface_t *&csurface, const synfig::RendDesc &/*renddesc*/, Time, synfig::ProgressCallback */*cb*/)
{
	if(csurface_ && !cairo_surface_status(csurface_))
	{
		csurface=cairo_surface_reference(csurface_);
		return true;
	}
	else
		return false;
}

bool
cairo_png_mptr::get_frame(cairo_surface_t *&csurface, const synfig::RendDesc &/*renddesc*/, Time,
					bool &/*trimmed*/, unsigned int &/*width*/, unsigned int &/*height*/, unsigned int &/*top*/, unsigned int &/*left*/, synfig::ProgressCallback */*cb*/)
{
	if(csurface_ && !cairo_surface_status(csurface_))
	{
		csurface=cairo_surface_reference(csurface_);
		return true;
	}
	else
		return false;
}
