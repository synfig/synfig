/*! ========================================================================
** Sinfg
** ppm Target Module
** $Id: mptr_imagemagick.cpp,v 1.1.1.1 2005/01/04 01:23:11 darco Exp $
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

#include <ETL/stringf>
#include "mptr_imagemagick.h"
#include <stdio.h>
#include <algorithm>
#include <functional>
#include <ETL/stringf>
#include <sinfg/general.h>

#endif

/* === M A C R O S ========================================================= */

using namespace sinfg;
using namespace std;
using namespace etl;

/* === G L O B A L S ======================================================= */

SINFG_IMPORTER_INIT(imagemagick_mptr);
SINFG_IMPORTER_SET_NAME(imagemagick_mptr,"imagemagick");
SINFG_IMPORTER_SET_EXT(imagemagick_mptr,"miff");
SINFG_IMPORTER_SET_VERSION(imagemagick_mptr,"0.1");
SINFG_IMPORTER_SET_CVS_ID(imagemagick_mptr,"$Id: mptr_imagemagick.cpp,v 1.1.1.1 2005/01/04 01:23:11 darco Exp $");

/* === M E T H O D S ======================================================= */


imagemagick_mptr::imagemagick_mptr(const char *f)
{

	filename=f;
	file=NULL;
}

imagemagick_mptr::~imagemagick_mptr()
{
	if(file)
		pclose(file);
}

bool
imagemagick_mptr::get_frame(sinfg::Surface &surface,Time time, sinfg::ProgressCallback *cb)
{
//#define HAS_LIBPNG 1

#if 1
	if(file)
		pclose(file);

	string command;

	if(filename.empty())
	{
		if(cb)cb->error(_("No file to load"));
		else sinfg::error(_("No file to load"));
		return false;
	}
	string temp_file="/tmp/deleteme.png";
	
	if(filename.find("psd")!=String::npos)
		command=strprintf("convert \"%s\" -flatten \"png32:%s\"\n",filename.c_str(),temp_file.c_str());
	else
		command=strprintf("convert \"%s\" \"png32:%s\"\n",filename.c_str(),temp_file.c_str());
	
	sinfg::info("command=%s",command.c_str());
	
	if(system(command.c_str())!=0)
		return false;

	Importer::Handle importer(Importer::open(temp_file));
	
	DEBUGPOINT();

	if(!importer)
	{
		if(cb)cb->error(_("Unable to open ")+temp_file);
		else sinfg::error(_("Unable to open ")+temp_file);
		return false;
	}
	
	DEBUGPOINT();

	if(!importer->get_frame(surface,0,cb))
	{
		if(cb)cb->error(_("Unable to get frame from ")+temp_file);
		else sinfg::error(_("Unable to get frame from ")+temp_file);
		return false;
	}
	
	if(!surface)
	{
		if(cb)cb->error(_("Bad surface from ")+temp_file);
		else sinfg::error(_("Bad surface from ")+temp_file);
		return false;		
	}

	if(1)
	{
		// remove odd premultiplication
		for(int i=0;i<surface.get_w()*surface.get_h();i++)
		{
			Color c(surface[0][i]);

			if(c.get_a())
			{
				surface[0][i].set_r(c.get_r()/c.get_a()/c.get_a());
				surface[0][i].set_g(c.get_g()/c.get_a()/c.get_a());
				surface[0][i].set_b(c.get_b()/c.get_a()/c.get_a());
			}
			else
			{
				surface[0][i].set_r(0);
				surface[0][i].set_g(0);
				surface[0][i].set_b(0);
			}
			surface[0][i].set_a(c.get_a());
		}
	}

	Surface bleh(surface);
	surface=bleh;
	

	//remove(temp_file.c_str());
	DEBUGPOINT();
	return true;
	
#else
	if(file)
		pclose(file);

	string command;

	if(filename.empty())
	{
		if(cb)cb->error(_("No file to load"));
		else sinfg::error(_("No file to load"));
		return false;
	}

	command=strprintf("convert \"%s\" -flatten ppm:-\n",filename.c_str());

	file=popen(command.c_str(),"r");

	if(!file)
	{
		if(cb)cb->error(_("Unable to open pipe to imagemagick"));
		else sinfg::error(_("Unable to open pipe to imagemagick"));
		return false;
	}
	int w,h;
	float divisor;
	char cookie[2];

	while((cookie[0]=fgetc(file))!='P' && !feof(file));

	if(feof(file))
	{
		if(cb)cb->error(_("Reached end of stream without finding PPM header"));
		else sinfg::error(_("Reached end of stream without finding PPM header"));
		return false;
	}

	cookie[1]=fgetc(file);

	if(cookie[0]!='P' || cookie[1]!='6')
	{
		if(cb)cb->error(string(_("stream not in PPM format"))+" \""+cookie[0]+cookie[1]+'"');
		else sinfg::error(string(_("stream not in PPM format"))+" \""+cookie[0]+cookie[1]+'"');
		return false;
	}

	fgetc(file);
	fscanf(file,"%d %d\n",&w,&h);
	fscanf(file,"%f",&divisor);
	fgetc(file);

	if(feof(file))
	{
		if(cb)cb->error(_("Premature end of file (after header)"));
		else sinfg::error(_("Premature end of file (after header)"));
		return false;
	}

	int x;
	int y;
	frame.set_wh(w,h);
	for(y=0;y<frame.get_h();y++)
		for(x=0;x<frame.get_w();x++)
		{
			if(feof(file))
			{
				if(cb)cb->error(_("Premature end of file"));
				else sinfg::error(_("Premature end of file"));
				return false;
			}
			float b=gamma().r_U8_to_F32((unsigned char)fgetc(file));
			float g=gamma().g_U8_to_F32((unsigned char)fgetc(file));
			float r=gamma().b_U8_to_F32((unsigned char)fgetc(file));
/*
			float b=(float)(unsigned char)fgetc(file)/divisor;
			float g=(float)(unsigned char)fgetc(file)/divisor;
			float r=(float)(unsigned char)fgetc(file)/divisor;
*/
			frame[y][x]=Color(
				b,
				g,
				r,
				1.0
			);
		}

	surface=frame;

	return true;
#endif
		
	
}
