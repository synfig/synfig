/* === S Y N F I G ========================================================= */
/*!	\file trgt_ppm.cpp
**	\brief ppm Target Module
**
**	$Id$
**
**	\legal
**	Copyright (c) 2002-2005 Robert B. Quattlebaum Jr., Adrian Bentley
**	Copyright (c) 2007 Chris Moore
**
**	This file is part of Synfig.
**
**	Synfig is free software: you can redistribute it and/or modify
**	it under the terms of the GNU General Public License as published by
**	the Free Software Foundation, either version 2 of the License, or
**	(at your option) any later version.
**
**	Synfig is distributed in the hope that it will be useful,
**	but WITHOUT ANY WARRANTY; without even the implied warranty of
**	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
**	GNU General Public License for more details.
**
**	You should have received a copy of the GNU General Public License
**	along with Synfig.  If not, see <https://www.gnu.org/licenses/>.
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

#include <glib/gstdio.h>
#include "trgt_ppm.h"
#endif

/* === M A C R O S ========================================================= */

using namespace synfig;
using namespace etl;

/* === G L O B A L S ======================================================= */

SYNFIG_TARGET_INIT(ppm);
SYNFIG_TARGET_SET_NAME(ppm,"ppm");
SYNFIG_TARGET_SET_EXT(ppm,"ppm");
SYNFIG_TARGET_SET_VERSION(ppm,"0.1");

/* === M E T H O D S ======================================================= */

ppm::ppm(const char *Filename, const synfig::TargetParam &params):
	imagecount(),
	multi_image(false),
	file(),
	filename(Filename),
	color_buffer(NULL),
	buffer(NULL),
	sequence_separator(params.sequence_separator)
{
	set_alpha_mode(TARGET_ALPHA_MODE_FILL);
}

ppm::~ppm()
{
	delete [] buffer;
	delete [] color_buffer;
}

bool
ppm::set_rend_desc(RendDesc *given_desc)
{
	//given_desc->set_pixel_format(PF_RGB);
	desc=*given_desc;
	imagecount=desc.get_frame_start();
	if(desc.get_frame_end()-desc.get_frame_start()>0)
		multi_image=true;
	else
		multi_image=false;
	return true;
}

void
ppm::end_frame()
{
	imagecount++;
}

bool
ppm::start_frame(synfig::ProgressCallback *callback)
{
	int w=desc.get_w(),h=desc.get_h();

	if(filename=="-")
	{
		if(callback)callback->task(strprintf("(stdout) %d",imagecount).c_str());
		file=SmartFILE(stdout);
	}
	else if(multi_image)
	{
		String newfilename(filename_sans_extension(filename) +
						   sequence_separator +
						   etl::strprintf("%04d",imagecount) +
						   filename_extension(filename));
		file=SmartFILE(g_fopen(newfilename.c_str(),POPEN_BINARY_WRITE_TYPE));
		if(callback)callback->task(newfilename);
	}
	else
	{
		file=SmartFILE(g_fopen(filename.c_str(),POPEN_BINARY_WRITE_TYPE));
		if(callback)callback->task(filename);
	}

	if(!file)
		return false;

	fprintf(file.get(), "P6\n");
	fprintf(file.get(), "%d %d\n", w, h);
	fprintf(file.get(), "%d\n", 255);

	delete [] buffer;
	buffer=new unsigned char[3*w];

	delete [] color_buffer;
	color_buffer=new Color[desc.get_w()];

	return true;
}

Color *
ppm::start_scanline(int /*scanline*/)
{
	return color_buffer;
}

bool
ppm::end_scanline()
{
	if(!file)
		return false;

	color_to_pixelformat(buffer, color_buffer, PF_RGB, 0, desc.get_w());

	if(!fwrite(buffer,1,desc.get_w()*3,file.get()))
		return false;

	return true;
}
