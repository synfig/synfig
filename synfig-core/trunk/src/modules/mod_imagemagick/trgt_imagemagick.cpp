/*! ========================================================================
** Sinfg
** ppm Target Module
** $Id: trgt_imagemagick.cpp,v 1.1.1.1 2005/01/04 01:23:11 darco Exp $
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
#include "trgt_imagemagick.h"
#include <stdio.h>
#include <algorithm>
#include <functional>
#include <ETL/clock>
#include <ETL/misc>

#endif

/* === M A C R O S ========================================================= */

using namespace sinfg;
using namespace std;
using namespace etl;

/* === G L O B A L S ======================================================= */

SINFG_TARGET_INIT(imagemagick_trgt);
SINFG_TARGET_SET_NAME(imagemagick_trgt,"imagemagick");
SINFG_TARGET_SET_EXT(imagemagick_trgt,"miff");
SINFG_TARGET_SET_VERSION(imagemagick_trgt,"0.1");
SINFG_TARGET_SET_CVS_ID(imagemagick_trgt,"$Id: trgt_imagemagick.cpp,v 1.1.1.1 2005/01/04 01:23:11 darco Exp $");

/* === M E T H O D S ======================================================= */

imagemagick_trgt::imagemagick_trgt(const char *Filename)
{
	file=NULL;
	filename=Filename;
	multi_image=false;
	buffer=NULL;
	color_buffer=0;
}

imagemagick_trgt::~imagemagick_trgt()
{
	if(file)
		pclose(file);
	file=NULL;
	delete [] buffer;
	delete [] color_buffer;
}

bool
imagemagick_trgt::set_rend_desc(RendDesc *given_desc)
{
	String	ext(find(filename.begin(),filename.end(),'.')+1,filename.end());
	if(ext=="xpm")
		pf=PF_RGB;
	else
		pf=PF_RGB|PF_A;

	desc=*given_desc;
	return true;
}

bool
imagemagick_trgt::init()
{
	imagecount=desc.get_frame_start();
	if(desc.get_frame_end()-desc.get_frame_start()>0)
		multi_image=true;

	delete [] buffer;
	buffer=new unsigned char[channels(pf)*desc.get_w()];
	delete [] color_buffer;
	color_buffer=new Color[desc.get_w()];
	return true;
}

void
imagemagick_trgt::end_frame()
{
	if(file)
	{
		fputc(0,file);
		fflush(file);
		pclose(file);
	}
	file=NULL;
}

bool
imagemagick_trgt::start_frame(sinfg::ProgressCallback *cb)
{
	string command;

	if(channels(pf)==4)
		command=strprintf("convert -depth 8 -size %dx%d rgba:-[0] -density %dx%d  \"%s\"\n",desc.get_w(),desc.get_h(),round_to_int(desc.get_x_res()/39.3700787402),round_to_int(desc.get_y_res()/39.3700787402),filename.c_str());
	else
		command=strprintf("convert -depth 8 -size %dx%d rgb:-[0] -density %dx%d \"%s\"\n",desc.get_w(),desc.get_h(),round_to_int(desc.get_x_res()/39.3700787402),round_to_int(desc.get_y_res()/39.3700787402),filename.c_str());

	file=popen(command.c_str(),"w");

	if(!file)
	{
		const char *msg=_("Unable to open pipe to imagemagick's convert utility");
		if(cb)cb->error(N_(msg));
		else sinfg::error(N_(msg));
		return false;
	}

	//etl::yield();	

	return true;
}

Color *
imagemagick_trgt::start_scanline(int scanline)
{
	return color_buffer;
}

bool
imagemagick_trgt::end_scanline(void)
{
	if(!file)
		return false;

	convert_color_format(buffer, color_buffer, desc.get_w(), pf, gamma());

	if(!fwrite(buffer,channels(pf),desc.get_w(),file))
		return false;
	
	return true;
}
