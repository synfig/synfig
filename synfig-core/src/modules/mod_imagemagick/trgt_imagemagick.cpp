/* === S Y N F I G ========================================================= */
/*!	\file trgt_imagemagick.cpp
**	\brief ImageMagick Target Module
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

#include <synfig/localization.h>
#include <synfig/general.h>
#include <synfig/misc.h>

#include "trgt_imagemagick.h"

#include <ETL/stringf>

#endif

/* === M A C R O S ========================================================= */

using namespace synfig;
using namespace etl;

/* === G L O B A L S ======================================================= */

SYNFIG_TARGET_INIT(imagemagick_trgt);
SYNFIG_TARGET_SET_NAME(imagemagick_trgt,"imagemagick");
SYNFIG_TARGET_SET_EXT(imagemagick_trgt,"miff");
SYNFIG_TARGET_SET_VERSION(imagemagick_trgt,"0.1");

/* === M E T H O D S ======================================================= */

imagemagick_trgt::imagemagick_trgt(const char *Filename,  const synfig::TargetParam &params):
	imagecount(),
	multi_image(false),
	pipe(nullptr),
	filename(Filename),
	buffer(nullptr),
	color_buffer(nullptr),
	pf(),
	sequence_separator(params.sequence_separator)
{ }

imagemagick_trgt::~imagemagick_trgt()
{
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
imagemagick_trgt::init(synfig::ProgressCallback * /* cb */)
{
	imagecount=desc.get_frame_start();
	if(desc.get_frame_end()-desc.get_frame_start()>0)
		multi_image=true;

	delete [] buffer;
	buffer=new unsigned char[pixel_size(pf)*desc.get_w()];
	delete [] color_buffer;
	color_buffer=new Color[desc.get_w()];
	return true;
}

void
imagemagick_trgt::end_frame()
{
	if (pipe) {
		pipe->close();
		pipe = nullptr;
	}
	imagecount++;
}

bool
imagemagick_trgt::start_frame(synfig::ProgressCallback *cb)
{
	const char *msg=_("Unable to open pipe to imagemagick's convert utility");

	std::string newfilename;

	if (multi_image)
		newfilename = (filename_sans_extension(filename) +
					   sequence_separator +
					   strprintf("%04d",imagecount) +
					   filename_extension(filename));
	else
		newfilename = filename;

	OS::RunArgs args;
	args.push_back({"-depth", "8"});
	args.push_back({"-size", strprintf("%dx%d", desc.get_w(), desc.get_h())});
	args.push_back(pixel_size(pf) == 4 ? "rgba:-[0]" : "rgb:-[0]");
	args.push_back({"-density", strprintf("%dx%d", round_to_int(desc.get_x_res()/39.3700787402), round_to_int(desc.get_y_res()/39.3700787402))});
	args.push_back(filesystem::Path(newfilename));

	pipe = OS::run_async("convert", args, OS::RUN_MODE_WRITE);

	if (!pipe) {
		if(cb)cb->error(N_(msg));
		else synfig::error(N_(msg));
		return false;
	}

	return true;
}

Color*
imagemagick_trgt::start_scanline(int /*scanline*/)
{
	return color_buffer;
}

bool
imagemagick_trgt::end_scanline(void)
{
	if (!pipe)
		return false;

	color_to_pixelformat(buffer, color_buffer, pf, 0, desc.get_w());

	if (!pipe->write(buffer, pixel_size(pf), desc.get_w()))
		return false;

	return true;
}
