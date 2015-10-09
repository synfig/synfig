/* === S Y N F I G ========================================================= */
/*!	\file mptr_ppm.cpp
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

#include "mptr_ppm.h"
#include <synfig/importer.h>
#include <synfig/time.h>
#include <synfig/surface.h>
#include <synfig/general.h>
#include <synfig/localization.h>
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
SYNFIG_IMPORTER_SET_NAME(ppm_mptr,"ppm");
SYNFIG_IMPORTER_SET_EXT(ppm_mptr,"ppm");
SYNFIG_IMPORTER_SET_VERSION(ppm_mptr,"0.1");
SYNFIG_IMPORTER_SET_CVS_ID(ppm_mptr,"$Id$");
SYNFIG_IMPORTER_SET_SUPPORTS_FILE_SYSTEM_WRAPPER(ppm_mptr, false);

/* === M E T H O D S ======================================================= */

bool
ppm_mptr::get_frame(synfig::Surface &surface, const synfig::RendDesc &/*renddesc*/, Time, synfig::ProgressCallback *cb)
{
	SmartFILE file(fopen(identifier.filename.c_str(),"rb"));
	if(!file)
	{
		if(cb)cb->error("pp_mptr::GetFrame(): "+strprintf(_("Unable to open %s"),identifier.filename.c_str()));
		return false;
	}
	int w,h;
	float divisor;

	if(fgetc(file.get())!='P' || fgetc(file.get())!='6')
	{
		if(cb)cb->error("pp_mptr::GetFrame(): "+strprintf(_("%s was not in PPM format"),identifier.filename.c_str()));
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
