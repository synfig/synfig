/*! ========================================================================
** Sinfg
** ppm Target Module
** $Id: mptr_mplayer.cpp,v 1.1.1.1 2005/01/04 01:23:14 darco Exp $
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

#include <sinfg/sinfg.h>
#include <ETL/stringf>
#include "mptr_mplayer.h"
#include <stdio.h>
#include <iostream>
#include <algorithm>
#include <functional>
#include <ETL/stringf>
#endif

/* === M A C R O S ========================================================= */

using namespace sinfg;
using namespace std;
using namespace etl;

/* === G L O B A L S ======================================================= */

const char mplayer_mptr::Name[]="avi";
const char mplayer_mptr::Ext[]="avi";

/* === M E T H O D S ======================================================= */

Importer *
mplayer_mptr::New(const char *file)
{
	return new mplayer_mptr(file);
}

mplayer_mptr::mplayer_mptr(const char *file)
{
	filename=file;
}

mplayer_mptr::~mplayer_mptr()
{
}

bool
mplayer_mptr::GetFrame(Time time, sinfg::Surface &surface, sinfg::ProgressCallback *)
{
	int ret;
	ret=system(
		strprintf("/usr/local/bin/mencoder \"%s\" -ovc rawrgb -ss %f -endpos 0 -nosound -o /tmp/tmp.sinfg.rgbdata | grep \"VIDEO\" > /tmp/tmp.sinfg.size",
			filename.c_str(),
			time
		).c_str()
	);
	/*
	if(ret!=0)
	{
		cerr<<"mencoder execution failed."<<endl;
		return false;
	}
*/
	FILE *sizefile=fopen("/tmp/tmp.sinfg.size","rt");
	FILE *rgbfile=fopen("/tmp/tmp.sinfg.rgbdata","rb");
	if(!rgbfile)
	{
		cerr<<"unable to open /tmp/tmp.sinfg.rgbdata"<<endl;
		return false;
	}
	if(!sizefile)
	{
		cerr<<"unable to open /tmp/tmp.sinfg.size"<<endl;
		return false;
	}
	
	int w=4,h=4,x,y;
	char bleh[500];
	
	fscanf(sizefile,"%s %s %dx%d",bleh,bleh,&w,&h);
	
	cerr<<strprintf("w:%d, h:%d, time:%f",w,h,time)<<endl;
	fseek(rgbfile,2047+3*8,SEEK_CUR);
	surface.set_wh(w,h);
	for(y=0;y<h;y++)
		for(x=0;x<w;x++)
		{
			unsigned char
				b=(unsigned char)fgetc(rgbfile),
				g=(unsigned char)fgetc(rgbfile),
				r=(unsigned char)fgetc(rgbfile);
				
			surface[h-y-1][x]=Color(
				(float)r/255.0,
				(float)g/255.0,
				(float)b/255.0,
				1.0
			);
		}
	
	fclose(rgbfile);
	fclose(sizefile);
	
	return true;
}
