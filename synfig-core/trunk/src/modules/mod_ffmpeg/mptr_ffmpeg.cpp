/* === S Y N F I G ========================================================= */
/*!	\file mptr_ffmpeg.cpp
**	\brief ppm Target Module
**
**	$Id$
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

#include <ETL/stringf>
#include "mptr_ffmpeg.h"
#include <stdio.h>
#include <iostream>
#include <algorithm>
#include <functional>
#include <ETL/stringf>
#endif

/* === M A C R O S ========================================================= */

using namespace synfig;
using namespace std;
using namespace etl;

/* === G L O B A L S ======================================================= */

SYNFIG_IMPORTER_INIT(ffmpeg_mptr);
SYNFIG_IMPORTER_SET_NAME(ffmpeg_mptr,"ffmpeg");
SYNFIG_IMPORTER_SET_EXT(ffmpeg_mptr,"avi");
SYNFIG_IMPORTER_SET_VERSION(ffmpeg_mptr,"0.1");
SYNFIG_IMPORTER_SET_CVS_ID(ffmpeg_mptr,"$Id$");

/* === M E T H O D S ======================================================= */

bool
ffmpeg_mptr::seek_to(int frame)
{
	if(frame<cur_frame || !file)
	{
		if(file)
		{
			pclose(file);
		}

		string command;

		command=strprintf("ffmpeg -i \"%s\" -an -f image2pipe -vcodec ppm -\n",filename.c_str());

		file=popen(command.c_str(),"r");

		if(!file)
		{
			cerr<<"Unable to open pipe to ffmpeg"<<endl;
			return false;
		}
		cur_frame=-1;
	}

	while(cur_frame<frame-1)
	{
		cerr<<"Seeking to..."<<frame<<'('<<cur_frame<<')'<<endl;
		if(!grab_frame())
			return false;
	}
	return true;
}

bool
ffmpeg_mptr::grab_frame(void)
{
	if(!file)
	{
		cerr<<"unable to open "<<filename<<endl;
		return false;
	}
	int w,h;
	float divisor;
	char cookie[2];
	cookie[0]=fgetc(file);
	cookie[1]=fgetc(file);

	if(cookie[0]!='P' || cookie[1]!='6')
	{
		cerr<<"stream not in PPM format \""<<cookie[0]<<cookie[1]<<'"'<<endl;
		return false;
	}

	fgetc(file);
	fscanf(file,"%d %d\n",&w,&h);
	fscanf(file,"%f",&divisor);
	fgetc(file);

	if(feof(file))
		return false;

	int x;
	int y;
	frame.set_wh(w,h);
	for(y=0;y<frame.get_h();y++)
		for(x=0;x<frame.get_w();x++)
		{
			if(feof(file))
				return false;
/*
			frame[y][x]=Color(
				(float)(unsigned char)fgetc(file)/divisor,
				(float)(unsigned char)fgetc(file)/divisor,
				(float)(unsigned char)fgetc(file)/divisor,
				1.0
*/
			float r=gamma().r_U8_to_F32((unsigned char)fgetc(file));
			float g=gamma().g_U8_to_F32((unsigned char)fgetc(file));
			float b=gamma().b_U8_to_F32((unsigned char)fgetc(file));
			frame[y][x]=Color(
				r,
				g,
				b,
				1.0
			);
		}
	cur_frame++;
	return true;
}

ffmpeg_mptr::ffmpeg_mptr(const char *f)
{
#ifdef HAVE_TERMIOS_H
	tcgetattr (0, &oldtty);
#endif
	filename=f;
	file=NULL;
	fps=23.98;
	cur_frame=-1;
}

ffmpeg_mptr::~ffmpeg_mptr()
{
	if(file)
		pclose(file);
#ifdef HAVE_TERMIOS_H
	tcsetattr(0,TCSANOW,&oldtty);
#endif
}

bool
ffmpeg_mptr::get_frame(synfig::Surface &surface,Time time, synfig::ProgressCallback *)
{
	int i=(int)(time*fps);
	if(i!=cur_frame)
	{
		if(!seek_to(i))
			return false;
		if(!grab_frame());
			return false;
	}

	surface=frame;
	return false;
}
