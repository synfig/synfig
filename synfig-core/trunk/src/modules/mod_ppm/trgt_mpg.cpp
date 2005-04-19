/*! ========================================================================
** Synfig
** bsd_mpeg1 Target Module
** $Id: trgt_mpg.cpp,v 1.1.1.1 2005/01/04 01:23:14 darco Exp $
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

#define SYNFIG_TARGET

#ifdef USING_PCH
#	include "pch.h"
#else
#ifdef HAVE_CONFIG_H
#	include <config.h>
#endif

#include <ETL/stringf>
#include "trgt_mpg.h"
#include <stdio.h>
#include <iostream>
#include <algorithm>
#include <functional>
#endif

/* === M A C R O S ========================================================= */

using namespace synfig;
using namespace std;
using namespace etl;

/* === G L O B A L S ======================================================= */

const char bsd_mpeg1::Name[]="mpeg1";
const char bsd_mpeg1::Ext[]="mpg";

#define tmp_dir		string("/tmp/")

/* === M E T H O D S ======================================================= */

Target *
bsd_mpeg1::New(const char *filename)
{
	return new bsd_mpeg1(filename);
}

bsd_mpeg1::bsd_mpeg1(const char *Filename)
{
	filename=Filename;
	passthru=ppm::New((tmp_dir+"temp.ppm").c_str());
	paramfile=NULL;
	
}

bsd_mpeg1::~bsd_mpeg1()
{
	if(paramfile)
		fclose(paramfile);
	delete passthru;
	cerr<<"Encoding "<<filename<<"with \"mpeg_encode\" utility..."<<endl;
	if(system("mpeg_encode -float-dct -realquiet /tmp/temp.param")!=0)
	{
		cerr<<"Failed to encode "<<filename<<"with \"mpeg_encode\" utility"<<endl;
		cerr<<"Are you sure it is installed?"<<endl;
	}
}

bool
bsd_mpeg1::set_rend_desc(RendDesc *given_desc)
{
	if(paramfile)
		fclose(paramfile);		
	
	
	paramfile=fopen((tmp_dir+"temp.param").c_str(),"wt");
	int bitrate=150; // kbytes per second
	int buffer_drift=50; // bitrate drift (in kbytes per second)
	
	bitrate*=8*1024;
	buffer_drift*=8*1024;
	
	fprintf(paramfile,
		"PATTERN		IBBPBBPBBPBBPBBP\n"
		"OUTPUT		%s\n"
		"BASE_FILE_FORMAT	PPM\n"
		"INPUT_CONVERT	*\n"
		"GOP_SIZE	16\n"
		"SLICES_PER_FRAME	1\n"
		"INPUT_DIR	\n"
		"PIXEL		HALF\n"
		"RANGE		10\n"
		"PSEARCH_ALG	LOGARITHMIC\n"
		"BSEARCH_ALG	CROSS2\n"
//		"IQSCALE		8\n"
//		"PQSCALE		10\n"
//		"BQSCALE		25\n"
		"IQSCALE		3\n"
		"PQSCALE		5\n"
		"BQSCALE		10\n"
		"REFERENCE_FRAME	ORIGINAL\n"
		"BIT_RATE  %d\n"
//		"BIT_RATE  1000000\n"
//		"BUFFER_SIZE 327680\n"
		"BUFFER_SIZE %d\n"
		,filename.c_str(),bitrate,buffer_drift);
	 float fps=given_desc->get_frame_rate();
	
	// Valid framerates:
	// 23.976, 24, 25, 29.97, 30, 50 ,59.94, 60
		
	if(fps <24.0)
	{
		fprintf(paramfile,"FRAME_RATE 23.976\n");
		given_desc->set_frame_rate(23.976);
	}
	if(fps>=24.0 && fps <25.0)
	{
		fprintf(paramfile,"FRAME_RATE 24\n");
		given_desc->set_frame_rate(24);
	}
	if(fps>=25.0 && fps <29.97)
	{
		fprintf(paramfile,"FRAME_RATE 25\n");
		given_desc->set_frame_rate(25);
	}
	if(fps>=29.97 && fps <30.0)
	{
		fprintf(paramfile,"FRAME_RATE 29.97\n");
		given_desc->set_frame_rate(29.97);
	}
	if(fps>=29.97 && fps <30.0)
	{
		fprintf(paramfile,"FRAME_RATE 29.97\n");
		given_desc->set_frame_rate(29.97);
	}
	if(fps>=30.0 && fps <50.0)
	{
		fprintf(paramfile,"FRAME_RATE 30\n");
		given_desc->set_frame_rate(30.0);
	}
	if(fps>=50.0 && fps <59.94)
	{
		fprintf(paramfile,"FRAME_RATE 50\n");
		given_desc->set_frame_rate(50);
	}
	if(fps>=59.94)
	{
		fprintf(paramfile,"FRAME_RATE 59.94\n");
		given_desc->set_frame_rate(59.94);
	}
	
	// Make sure that the width and height
	// are multiples of 8
	given_desc->set_w((given_desc->get_w()+4)/8*8);
	given_desc->set_h((given_desc->get_h()+4)/8*8);
	
	if(!passthru->set_rend_desc(given_desc))
		return false;
		
	desc=*given_desc;

	fprintf(paramfile,
		"INPUT\n"
		"tmp/temp*.ppm	[%04d-%04d]\n"
		"END_INPUT\n",desc.get_frame_start(),desc.get_frame_end()-1);

	fclose(paramfile);
	paramfile=NULL;
	
	return true;
}

void
bsd_mpeg1::end_frame()
{
	passthru->end_frame();
}

bool
bsd_mpeg1::start_frame(synfig::ProgressCallback *callback)
{
	return passthru->start_frame(callback);
}

unsigned char *
bsd_mpeg1::start_scanline(int scanline)
{
	return passthru->start_scanline(scanline);
}

bool
bsd_mpeg1::end_scanline(void)
{
	return passthru->end_scanline();
}
