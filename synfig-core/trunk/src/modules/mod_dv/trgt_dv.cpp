/*! ========================================================================
** Sinfg
** ppm Target Module
** $Id: trgt_dv.cpp,v 1.1.1.1 2005/01/04 01:23:10 darco Exp $
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

#include <ETL/stringf>
#include "trgt_dv.h"
#include <stdio.h>
#include <algorithm>
#include <functional>
#include <ETL/clock>

#endif

/* === M A C R O S ========================================================= */

using namespace sinfg;
using namespace std;
using namespace etl;

/* === G L O B A L S ======================================================= */

SINFG_TARGET_INIT(dv_trgt);
SINFG_TARGET_SET_NAME(dv_trgt,"dv");
SINFG_TARGET_SET_EXT(dv_trgt,"dv");
SINFG_TARGET_SET_VERSION(dv_trgt,"0.1");
SINFG_TARGET_SET_CVS_ID(dv_trgt,"$Id: trgt_dv.cpp,v 1.1.1.1 2005/01/04 01:23:10 darco Exp $");

/* === M E T H O D S ======================================================= */


dv_trgt::dv_trgt(const char *Filename)
{
	file=NULL;
	filename=Filename;
	buffer=NULL;
	wide_aspect=false;
	color_buffer=0;
		set_remove_alpha();

}

dv_trgt::~dv_trgt()
{
	if(file)
		pclose(file);
	file=NULL;
	delete [] buffer;
	delete [] color_buffer;
}

bool
dv_trgt::set_rend_desc(RendDesc *given_desc)
{	
	// Set the aspect ratio
	if(wide_aspect)
	{
		// 16:9 Aspect
		given_desc->set_wh(160,90);
		
		// Widescreen should be progressive scan
		given_desc->set_interlaced(false);
	}
	else
	{
		// 4:3 Aspect
		given_desc->set_wh(400,300);
		
		// We should be interlaced
		given_desc->set_interlaced(true);
	}
	
	// but the pixel res should be 720x480
	given_desc->clear_flags(),given_desc->set_wh(720,480);
		
	// NTSC Frame rate is 29.97
	given_desc->set_frame_rate(29.97);
	
	// The pipe to encodedv is PPM, which needs RGB data
	//given_desc->set_pixel_format(PF_RGB);
	
	// Set the description
	desc=*given_desc;
	
	return true;
}

bool
dv_trgt::init()
{
	imagecount=desc.get_frame_start();
	
	string command;
	
	if(wide_aspect)
		command=strprintf("encodedv -w 1 - > \"%s\"\n",filename.c_str());
	else
		command=strprintf("encodedv - > \"%s\"\n",filename.c_str());
	
	// Open the pipe to encodedv
	file=popen(command.c_str(),"w");
	
	if(!file)
	{
		sinfg::error(_("Unable to open pipe to encodedv"));
		return false;
	}

	// Sleep for a moment to let the pipe catch up
	etl::clock().sleep(0.25f);	

	return true;
}

void
dv_trgt::end_frame()
{
	fprintf(file, " ");
	fflush(file);
	imagecount++;
}

bool
dv_trgt::start_frame(sinfg::ProgressCallback *callback)
{
	int w=desc.get_w(),h=desc.get_h();
		
	if(!file)
		return false;
	
	fprintf(file, "P6\n");
	fprintf(file, "%d %d\n", w, h);
	fprintf(file, "%d\n", 255);	
	
	delete [] buffer;
	buffer=new unsigned char[3*w];

	delete [] color_buffer;
	color_buffer=new Color[w];
	
	return true;
}

Color *
dv_trgt::start_scanline(int scanline)
{
	return color_buffer;
}

bool
dv_trgt::end_scanline()
{
	if(!file)
		return false;

	convert_color_format(buffer, color_buffer, desc.get_w(), PF_RGB, gamma());
			
	if(!fwrite(buffer,1,desc.get_w()*3,file))
		return false;
	
	return true;
}
