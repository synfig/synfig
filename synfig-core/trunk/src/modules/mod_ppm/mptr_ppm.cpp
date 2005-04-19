/*! ========================================================================
** Synfig
** ppm Target Module
** $Id: mptr_ppm.cpp,v 1.1.1.1 2005/01/04 01:23:14 darco Exp $
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

#include "mptr_ppm.h"
#include <synfig/importer.h>
#include <synfig/time.h>
#include <synfig/surface.h>
#include <synfig/general.h>
#include <synfig/smartfile.h>

#include <cstdio>
#include <algorithm>
#include <functional>

#endif

/* === M A C R O S ========================================================= */

using namespace synfig;
using namespace std;
using namespace etl;

/* === G L O B A L S ======================================================= */

SYNFIG_IMPORTER_INIT(ppm_mptr);
SYNFIG_IMPORTER_SET_NAME(ppm_mptr,"ppm_mptr");
SYNFIG_IMPORTER_SET_EXT(ppm_mptr,"ppm");
SYNFIG_IMPORTER_SET_VERSION(ppm_mptr,"0.1");
SYNFIG_IMPORTER_SET_CVS_ID(ppm_mptr,"$Id: mptr_ppm.cpp,v 1.1.1.1 2005/01/04 01:23:14 darco Exp $");

/* === M E T H O D S ======================================================= */

ppm_mptr::ppm_mptr(const char *file)
{
	filename=file;
}

ppm_mptr::~ppm_mptr()
{
}

bool
ppm_mptr::get_frame(synfig::Surface &surface,Time, synfig::ProgressCallback *cb)
{
	SmartFILE file(fopen(filename.c_str(),"rb"));
	if(!file)
	{
		if(cb)cb->error("pp_mptr::GetFrame(): "+strprintf(_("Unable to open %s"),filename.c_str())); 
		return false;
	}
	int w,h;
	float divisor;
	
	if(fgetc(file.get())!='P' || fgetc(file.get())!='6')
	{
		if(cb)cb->error("pp_mptr::GetFrame(): "+strprintf(_("%s was not in PPM format"),filename.c_str())); 
		return false;
	}
	
	fgetc(file.get());
	fscanf(file.get(),"%d %d\n",&w,&h);
	fscanf(file.get(),"%f",&divisor);
	fgetc(file.get());
	
	int x;
	int y;
	surface.set_wh(w,h);
	for(y=0;y<surface.get_h();y++)
		for(x=0;x<surface.get_w();x++)
		{
/*
			surface[y][x]=Color(
				(float)(unsigned char)fgetc(file)/divisor,
				(float)(unsigned char)fgetc(file)/divisor,
				(float)(unsigned char)fgetc(file)/divisor,
				1.0
			);
*/
			float r=gamma().r_U8_to_F32((unsigned char)fgetc(file.get()));
			float g=gamma().g_U8_to_F32((unsigned char)fgetc(file.get()));
			float b=gamma().b_U8_to_F32((unsigned char)fgetc(file.get()));
			surface[y][x]=Color(
				r,
				g,
				b,
				1.0
			);
		}
	return true;
}
