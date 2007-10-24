/* === S Y N F I G ========================================================= */
/*!	\file trgt_imagemagick.cpp
**	\brief ppm Target Module
**
**	\legal
** $Id$
**
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
**
** === N O T E S ===========================================================
**
** ========================================================================= */

/* === H E A D E R S ======================================================= */

#define SYNFIG_TARGET

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

using namespace synfig;
using namespace std;
using namespace etl;

/* === G L O B A L S ======================================================= */

SYNFIG_TARGET_INIT(imagemagick_trgt);
SYNFIG_TARGET_SET_NAME(imagemagick_trgt,"imagemagick");
SYNFIG_TARGET_SET_EXT(imagemagick_trgt,"miff");
SYNFIG_TARGET_SET_VERSION(imagemagick_trgt,"0.1");
SYNFIG_TARGET_SET_CVS_ID(imagemagick_trgt,"$Id$");

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
	if(filename_extension(filename) == ".xpm")
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
imagemagick_trgt::start_frame(synfig::ProgressCallback *cb)
{
	string command;

	if(channels(pf)==4)
		command=strprintf("convert -depth 8 -size %dx%d rgba:-[0] -density %dx%d  \"%s\"\n",desc.get_w(),desc.get_h(),round_to_int(desc.get_x_res()/39.3700787402),round_to_int(desc.get_y_res()/39.3700787402),filename.c_str());
	else
		command=strprintf("convert -depth 8 -size %dx%d rgb:-[0] -density %dx%d \"%s\"\n",desc.get_w(),desc.get_h(),round_to_int(desc.get_x_res()/39.3700787402),round_to_int(desc.get_y_res()/39.3700787402),filename.c_str());

	file=popen(command.c_str(),POPEN_BINARY_WRITE_TYPE);

	if(!file)
	{
		const char *msg=_("Unable to open pipe to imagemagick's convert utility");
		if(cb)cb->error(N_(msg));
		else synfig::error(N_(msg));
		return false;
	}

	//etl::yield();

	return true;
}

Color *
imagemagick_trgt::start_scanline(int /*scanline*/)
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
