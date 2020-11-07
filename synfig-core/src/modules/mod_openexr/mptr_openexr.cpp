/* === S Y N F I G ========================================================= */
/*!	\file mptr_openexr.cpp
**	\brief OpenEXR Importer Module
**
**	\legal
**	Copyright (c) 2002-2005 Robert B. Quattlebaum Jr., Adrian Bentley
**	Copyright (c) 2007 Chris Moore
**
**	This file is part of Synfig.
**
**	Synfig is free software: you can redistribute it and/or modify
**	it under the terms of the GNU General Public License as published by
**	the Free Software Foundation, either version 2 of the License, or
**	(at your option) any later version.
**
**	Synfig is distributed in the hope that it will be useful,
**	but WITHOUT ANY WARRANTY; without even the implied warranty of
**	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
**	GNU General Public License for more details.
**
**	You should have received a copy of the GNU General Public License
**	along with Synfig.  If not, see <https://www.gnu.org/licenses/>.
**	\endlegal
**
** ========================================================================= */

/* === H E A D E R S ======================================================= */

#ifdef USING_PCH
#	include "pch.h"
#else
#ifdef HAVE_CONFIG_H
#	include <config.h>
#endif

#include "mptr_openexr.h"
#include <synfig/general.h>
#include <synfig/surface.h>

#include <ImfRgbaFile.h>
#include <exception>

#endif

/* === M A C R O S ========================================================= */

using namespace synfig;
using namespace etl;

/* === G L O B A L S ======================================================= */

SYNFIG_IMPORTER_INIT(exr_mptr);
SYNFIG_IMPORTER_SET_NAME(exr_mptr,"openexr");
SYNFIG_IMPORTER_SET_EXT(exr_mptr,"exr");
SYNFIG_IMPORTER_SET_VERSION(exr_mptr,"0.1");
SYNFIG_IMPORTER_SET_SUPPORTS_FILE_SYSTEM_WRAPPER(exr_mptr, false);

/* === M E T H O D S ======================================================= */

bool
exr_mptr::get_frame(synfig::Surface &out_surface, const synfig::RendDesc &/*renddesc*/, Time, synfig::ProgressCallback *cb)
{
    try
    {

	Imf::RgbaInputFile in(identifier.filename.c_str());

    int w = in.dataWindow().max.x - in.dataWindow().min.x + 1;
    int h = in.dataWindow().max.y - in.dataWindow().min.y + 1;
    //int dx = in.dataWindow().min.x;
    //int dy = in.dataWindow().min.y;

	etl::surface<Imf::Rgba> in_surface;
	in_surface.set_wh(w,h);
	in.setFrameBuffer (reinterpret_cast<Imf::Rgba *>(in_surface[0]), 1, w);

	in.readPixels (in.dataWindow().min.y, in.dataWindow().max.y);

	int x;
	int y;
	out_surface.set_wh(w,h);
	for(y=0;y<out_surface.get_h();y++)
		for(x=0;x<out_surface.get_w();x++)
		{
			Color &color(out_surface[y][x]);
			Imf::Rgba &rgba(in_surface[y][x]);
			color.set_r(rgba.r);
			color.set_g(rgba.g);
			color.set_b(rgba.b);
			color.set_a(rgba.a);
		}
	}
	catch (const std::exception& e)
    {
		if(cb)cb->error(e.what());
		else synfig::error(e.what());
		return false;
    }

	return true;
}
