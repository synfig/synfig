/* === S Y N F I G ========================================================= */
/*!	\file mptr_mplayer.cpp
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

#include <synfig/synfig.h>
#include <ETL/stringf>
#include "mptr_mplayer.h"
#include <cstdio>
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

SYNFIG_IMPORTER_INIT(mplayer_mptr);
SYNFIG_IMPORTER_SET_NAME(mplayer_mptr,"mplayer");
SYNFIG_IMPORTER_SET_EXT(mplayer_mptr,"avi");
SYNFIG_IMPORTER_SET_VERSION(mplayer_mptr,"0.1");
SYNFIG_IMPORTER_SET_CVS_ID(mplayer_mptr,"$Id$");
SYNFIG_IMPORTER_SET_SUPPORTS_FILE_SYSTEM_WRAPPER(mplayer_mptr, false);

/* === M E T H O D S ======================================================= */

bool
mplayer_mptr::get_frame(synfig::Surface &surface, const synfig::RendDesc &renddesc, synfig::Time time, synfig::ProgressCallback *callback)
{

#error This code has vulnerabilities: arbitrary shell command execution and tmpfile issues

	int ret;
	ret=system(
		strprintf("/usr/local/bin/mencoder \"%s\" -ovc rawrgb -ss %f -endpos 0 -nosound -o /tmp/tmp.synfig.rgbdata | grep \"VIDEO\" > /tmp/tmp.synfig.size",
			identifier.filename.c_str(),
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
	FILE *sizefile=fopen("/tmp/tmp.synfig.size","rt");
	FILE *rgbfile=fopen("/tmp/tmp.synfig.rgbdata","rb");
	if(!rgbfile)
	{
		cerr<<"unable to open /tmp/tmp.synfig.rgbdata"<<endl;
		return false;
	}
	if(!sizefile)
	{
		cerr<<"unable to open /tmp/tmp.synfig.size"<<endl;
		return false;
	}

	int w=4,h=4,x,y;
	char bleh[500];

	fscanf(sizefile,"%499s %499s %dx%d",bleh,bleh,&w,&h);

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
