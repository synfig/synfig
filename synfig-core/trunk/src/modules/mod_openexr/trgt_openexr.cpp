/*! ========================================================================
** Sinfg
** exr_trgt Target Module
** $Id: trgt_openexr.cpp,v 1.1.1.1 2005/01/04 01:23:14 darco Exp $
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

#define SINFG_TARGET

#ifdef USING_PCH
#	include "pch.h"
#else
#ifdef HAVE_CONFIG_H
#	include <config.h>
#endif

#include "trgt_openexr.h"
#include <sinfg/sinfg.h>
#include <ETL/stringf>
#include <cstdio>
#include <algorithm>
#include <functional>
#endif

/* === M A C R O S ========================================================= */

using namespace sinfg;
using namespace std;
using namespace etl;

/* === G L O B A L S ======================================================= */

SINFG_TARGET_INIT(exr_trgt);
SINFG_TARGET_SET_NAME(exr_trgt,"exr_trgt");
SINFG_TARGET_SET_EXT(exr_trgt,"exr");
SINFG_TARGET_SET_VERSION(exr_trgt,"1.0.4");
SINFG_TARGET_SET_CVS_ID(exr_trgt,"$Id: trgt_openexr.cpp,v 1.1.1.1 2005/01/04 01:23:14 darco Exp $");

/* === M E T H O D S ======================================================= */

bool
exr_trgt::ready()
{
	return (bool)exr_file;
}

exr_trgt::exr_trgt(const char *Filename):
	multi_image(false),
	imagecount(0),
	filename(Filename),
	exr_file(0)
{
	buffer=0;
#ifndef USE_HALF_TYPE
	buffer_color=0;
#endif
	
	// OpenEXR uses linear gamma
	gamma().set_gamma(1.0);
}

exr_trgt::~exr_trgt()
{
	if(exr_file)
		delete exr_file;

	if(buffer) delete [] buffer;
#ifndef USE_HALF_TYPE
	if(buffer_color) delete [] buffer_color;
#endif
}

bool
exr_trgt::set_rend_desc(RendDesc *given_desc)
{
	//given_desc->set_pixel_format(PixelFormat((int)PF_RAW_COLOR));
	desc=*given_desc;
	imagecount=desc.get_frame_start();
	if(desc.get_frame_end()-desc.get_frame_start()>0)
		multi_image=true;
	else
		multi_image=false;
	return true;
}

bool
exr_trgt::start_frame(sinfg::ProgressCallback *cb)
{
	int w=desc.get_w(),h=desc.get_h();
	
	String frame_name;
	
	if(exr_file)
		delete exr_file;
	if(multi_image)
	{
		String
			newfilename(filename),
			ext(find(filename.begin(),filename.end(),'.'),filename.end());
		newfilename.erase(find(newfilename.begin(),newfilename.end(),'.'),newfilename.end());
		
		newfilename+=etl::strprintf("%04d",imagecount)+ext;
		frame_name=newfilename;
		if(cb)cb->task(newfilename);
	}
	else
	{
		frame_name=filename;
		if(cb)cb->task(filename);
	}
	exr_file=new Imf::RgbaOutputFile(frame_name.c_str(),w,h,Imf::WRITE_RGBA,desc.get_pixel_aspect());
#ifndef USE_HALF_TYPE
	if(buffer_color) delete [] buffer_color;
	buffer_color=new Color[w];
#endif
	//if(buffer) delete [] buffer;
	//buffer=new Imf::Rgba[w];
	out_surface.set_wh(w,h);
	
	return true;
}

void
exr_trgt::end_frame()
{
	if(exr_file)
	{
		exr_file->setFrameBuffer(out_surface[0],1,desc.get_w());
		exr_file->writePixels(desc.get_h());

		delete exr_file;
	}
	
	exr_file=0;
	
	imagecount++;
}

Color *
exr_trgt::start_scanline(int i)
{
	scanline=i;
#ifndef USE_HALF_TYPE
	return reinterpret_cast<Color *>(buffer_color);
#else
	return reinterpret_cast<Color *>(out_surface[scanline]);
//	return reinterpret_cast<unsigned char *>(buffer);
#endif
}

bool
exr_trgt::end_scanline()
{
	if(!ready())
		return false;

#ifndef USE_HALF_TYPE
	int i;
	for(i=0;i<desc.get_w();i++)
	{
//		Imf::Rgba &rgba=buffer[i];
		Imf::Rgba &rgba=out_surface[scanline][i];
		Color &color=buffer_color[i];
		rgba.r=color.get_r();
		rgba.g=color.get_g();
		rgba.b=color.get_b();
		rgba.a=color.get_a();
	}
#endif

    //exr_file->setFrameBuffer(buffer,1,desc.get_w());
	//exr_file->writePixels(1);

	return true;
}
