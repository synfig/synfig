/*! ========================================================================
** Sinfg
** ppm Target Module
** $Id: mptr_openexr.cpp,v 1.1.1.1 2005/01/04 01:23:14 darco Exp $
**
** Copyright (c) 2002 Robert B. Quattlebaum Jr.
**
** This software and associated documentation
** are CONFIDENTIAL and PROPRIETARY property of
** the above-mentioned copyright holder.
**
** You may not copy, print, publish, or in any
** other way distribute this software without
** a prior written agreement with
** the copyright holder.
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

#include "mptr_openexr.h"
#include <sinfg/sinfg.h>
#include <ETL/stringf>
#include <cstdio>
#include <algorithm>
#include <functional>

#include <ImfArray.h>
#include <ImfRgbaFile.h>
#include <exception>

#endif

/* === M A C R O S ========================================================= */

using namespace sinfg;
using namespace std;
using namespace etl;

/* === G L O B A L S ======================================================= */

SINFG_IMPORTER_INIT(exr_mptr);
SINFG_IMPORTER_SET_NAME(exr_mptr,"exr_mptr");
SINFG_IMPORTER_SET_EXT(exr_mptr,"exr");
SINFG_IMPORTER_SET_VERSION(exr_mptr,"0.1");
SINFG_IMPORTER_SET_CVS_ID(exr_mptr,"$Id: mptr_openexr.cpp,v 1.1.1.1 2005/01/04 01:23:14 darco Exp $");

/* === M E T H O D S ======================================================= */

exr_mptr::exr_mptr(const char *file)
{
	filename=file;
}

exr_mptr::~exr_mptr()
{
}

bool
exr_mptr::get_frame(sinfg::Surface &out_surface,Time, sinfg::ProgressCallback *cb)
{
    try
    {

	Imf::RgbaInputFile in(filename.c_str());

    int w = in.dataWindow().max.x - in.dataWindow().min.x + 1;
    int h = in.dataWindow().max.y - in.dataWindow().min.y + 1;
    //int dx = in.dataWindow().min.x;
    //int dy = in.dataWindow().min.y;

#ifdef USE_HALF_TYPE
    out_surface.set_wh(w,h);
	in.setFrameBuffer (reinterpret_cast<Imf::Rgba *>(out_surface[0]), 1, w);

	in.readPixels (in.dataWindow().min.y, in.dataWindow().max.y);

#else
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
#endif
	}
	catch (const std::exception &e)
    {
		if(cb)cb->error(e.what());
		else sinfg::error(e.what());
		return false;
    }

	return true;		
}
