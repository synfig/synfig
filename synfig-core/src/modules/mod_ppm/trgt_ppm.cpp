/* === S Y N F I G ========================================================= */
/*!	\file trgt_ppm.cpp
**	\brief ppm Target Module
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
** ========================================================================= */

/* === H E A D E R S ======================================================= */

#ifdef USING_PCH
#	include "pch.h"
#else
#ifdef HAVE_CONFIG_H
#	include <config.h>
#endif

#include "trgt_ppm.h"

#include <ETL/stringf>

#include <synfig/general.h>
#include <synfig/localization.h>

#endif

/* === M A C R O S ========================================================= */

using namespace synfig;

/* === G L O B A L S ======================================================= */

SYNFIG_TARGET_INIT(ppm);
SYNFIG_TARGET_SET_NAME(ppm,"ppm");
SYNFIG_TARGET_SET_EXT(ppm,"ppm");
SYNFIG_TARGET_SET_VERSION(ppm,"0.1");

/* === M E T H O D S ======================================================= */

ppm::ppm(const synfig::filesystem::Path& Filename, const synfig::TargetParam& params):
	imagecount(),
	multi_image(false),
	file(),
	filename(Filename),
	sequence_separator(params.sequence_separator)
{
	set_alpha_mode(TARGET_ALPHA_MODE_FILL);
}

ppm::~ppm()
{
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

	if (filename.u8string() == "-") {
		if (callback)
			callback->task(strprintf("(stdout) %d", imagecount));
		file = SmartFILE(stdout);
	} else {
		synfig::filesystem::Path newfilename(filename);
		if (multi_image) {
			newfilename.add_suffix(sequence_separator + strprintf("%04d", imagecount));
		}
		file = SmartFILE(newfilename, "wb");
		if (callback)
			callback->task(newfilename.u8string());
	}

	if (!file) {
		if (callback)
			callback->error(_("Unable to open file"));
		else
			synfig::error(_("Unable to open file"));
		return false;
	}

	fprintf(file.get(), "P6\n");
	fprintf(file.get(), "%d %d\n", w, h);
	fprintf(file.get(), "%d\n", 255);

	buffer.resize(3*w);
	color_buffer.resize(desc.get_w());

	return true;
}

Color *
ppm::start_scanline(int /*scanline*/)
{
	return color_buffer.empty() ? nullptr : color_buffer.data();
}

bool
ppm::end_scanline()
{
	if(!file)
		return false;

	color_to_pixelformat(buffer.data(), color_buffer.data(), PF_RGB, 0, desc.get_w());

	if (!fwrite(buffer.data(), 1, desc.get_w()*3, file.get()))
		return false;

	return true;
}
